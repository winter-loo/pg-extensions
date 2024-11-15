#include "postgres.h"
#include "fmgr.h"
#include "access/parallel.h"
#include "access/xact.h"
#include "executor/spi.h"
#include "utils/builtins.h"
#include "utils/memutils.h"
#include "port.h"
#include "miscadmin.h"

PG_MODULE_MAGIC;

void execute_sql_string(dsm_segment *seg, shm_toc *toc);

#if 0
void _PG_init(void) {
  ereport(INFO, (errmsg("...loading extension parallel...")));
}
#endif

#define PARALLEL_TASKS_KEY_SHARED 1
#define PARALLEL_DEBUG_KEY_SHARED 2

/*
 * SharedContext Memory Layout:

+------------------------+ <-- Start of SharedContext
|     pg_atomic_uint64   |
|         count          |  8 bytes
+------------------------+
|        uint32          |
|        ntasks          |  4 bytes
+------------------------+
|    pg_atomic_uint32    |
|      task_index        |  4 bytes
+------------------------+ <-- offsetof(SharedContext, task_list)
|      task 0            |
|     sql length         |
+------------------------+
|      task 0 + 1        |
|     sql length         |  4 bytes
+------------------------+
|   task 0 + 1 + 2       |
|     sql length         |  4 bytes
+------------------------+
|    (Remaining Tasks)   |
|     sql length         |  4 bytes
+------------------------+
| "INSERT INTO table_1..."
| (String for task 0)    |
+------------------------+
| "INSERT INTO table_2..."
| (String for task 1)    |
+------------------------+
| "INSERT INTO table_3..."
| (String for task 2)    |
+------------------------+
|         ...            |
| (Remaining SQL strings)|
+------------------------+

Notes:
- All memory is contiguous in the DSM segment
- Pointers in task_list point to strings stored right after the array
- Total size = 16 bytes (header) 
              + (4 bytes × ntasks) for length array
              + sum(strlen(each SQL) + 1) for strings
 * 
 */
typedef struct {
  pg_atomic_uint64 count;
  uint32 ntasks;
  pg_atomic_uint32 task_index;
  char task_list[FLEXIBLE_ARRAY_MEMBER];
} SharedContext;

#define TASK_LIST_HEADER_OFFSET           offsetof(SharedContext, task_list)
#define TASK_SQL_USED_LENGTH_SLOT(m, i)   (uint32 *)((char *)m + (TASK_LIST_HEADER_OFFSET + ((i) * sizeof(uint32))))
#define TASK_SQL_USED_LENGTH(m, i)        *TASK_SQL_USED_LENGTH_SLOT(m, i)
#define TASK_SQL_SET_USED_LENGTH(m, i, n) (*TASK_SQL_USED_LENGTH_SLOT(m, i) = n)
#define TASK_SQL_STRING_DATA(m)           ((char *)m + TASK_LIST_HEADER_OFFSET + (m->ntasks) * sizeof(uint32))
#define TASK_SQL_STRING(m, i)             (char *)(TASK_SQL_STRING_DATA(m) + (i > 0 ? TASK_SQL_USED_LENGTH(m, i-1) : 0))
#define SharedContexUsedSize(m)           (TASK_SQL_STRING_DATA(m) - (char *)m + TASK_SQL_USED_LENGTH(m, m->ntasks - 1))

typedef struct {
  uint32 ntasks;
  bool wait[FLEXIBLE_ARRAY_MEMBER];
} DebuggerAttachSingal;

/*
 * Implement a parallel insert into multi-sharded tables.
 *
 * # How to Use
 *
 * ```sql
 * select parallel('t', $$
 *  1,2
 *  3,4
 *  5,6
 *  7,8
 * $$, 4);
 * ```
 *
 * # how it works
 *
 * ## Prerequisites
 * ```sql
 * create table t_1(a int, b int);
 * create table t_2(a int, b int);
 * create table t_3(a int, b int);
 * create table t_4(a int, b int);
 * ```
 *
 * ## breaking parts
 *
 * ```sql
 * insert into t_1 values (1, 2);
 * insert into t_2 values (1, 2);
 * insert into t_3 values (1, 2);
 * insert into t_4 values (1, 2);
 * ```
 *
 * It uses four parallel processes to execute the four insert into statements.
 */
PG_FUNCTION_INFO_V1(parallel);
Datum parallel(PG_FUNCTION_ARGS)
{
  ParallelContext *pcxt;
  int nworkers = 0;
  SharedContext* shared;
  char* saveptr;
  char* line;
  int ntasks = 0;
  Size shared_size;
  char* csv_copy;
  uint64 count;
  StringInfoData* query_string_list;
  uint32 query_string_total_size = 0;

  char* table_prefix = text_to_cstring(PG_GETARG_TEXT_P(0));
  char* csv_data = text_to_cstring(PG_GETARG_TEXT_P(1));
  nworkers = PG_GETARG_INT32(2);

  /* Count number of lines in csv_data */
  csv_copy = pstrdup(csv_data);
  for (line = strtok_r(csv_copy, "\n", &saveptr); 
       line != NULL;
       line = strtok_r(NULL, "\n", &saveptr)) {
    ntasks++;
  }
  pfree(csv_copy);

  query_string_list = (StringInfoData *) malloc(ntasks * sizeof(void *));

  /* Split csv_data and construct SQL statements */
  {
    int i = 0;

    for (line = strtok_r(csv_data, "\n", &saveptr);
         line != NULL;
         line = strtok_r(NULL, "\n", &saveptr)) {
      /* Trim whitespace */
      while (*line && isspace(*line)) line++;
      if (*line == '\0') continue;  /* Skip empty lines */
      
      /* Construct INSERT statement */
      initStringInfo(&query_string_list[i]);
      appendStringInfo(&query_string_list[i], "INSERT INTO %s_%d VALUES (%s)", 
                      table_prefix, (i % nworkers) + 1, line);
      elog(INFO, "task sql: %s, len: %d", query_string_list[i].data, query_string_list[i].len);
      query_string_total_size += query_string_list[i].len + 1;
      i++;
    }
  }

  /* Calculate total shared memory needed */
  /* we do not the zero-index slot in task_list */
  shared_size = TASK_LIST_HEADER_OFFSET + ntasks * sizeof(uint32) + query_string_total_size;
  elog(INFO, "shared memory in bytes will be used: %ld", shared_size);

  EnterParallelMode();    /* prohibit unsafe state changes */

  pcxt = CreateParallelContext("parallel", "execute_sql_string", nworkers);

  shm_toc_estimate_chunk(&pcxt->estimator, shared_size);
  shm_toc_estimate_keys(&pcxt->estimator, 1);

  shm_toc_estimate_chunk(&pcxt->estimator, offsetof(DebuggerAttachSingal, wait) + ntasks * sizeof(bool));
  shm_toc_estimate_keys(&pcxt->estimator, 1);

  InitializeParallelDSM(pcxt);  /* create DSM and copy state to it */

  shared = (SharedContext *)shm_toc_allocate(pcxt->toc, shared_size);
  pg_atomic_init_u64(&shared->count, 0);
  shared->ntasks = ntasks;
  pg_atomic_init_u32(&shared->task_index, 0);

  {
    int i = 0;
    int tot_len = 0;
    char* pstr;
    for (; i < ntasks; i++)
    {
      tot_len += query_string_list[i].len + 1;
      TASK_SQL_SET_USED_LENGTH(shared, i, tot_len);
      pstr = TASK_SQL_STRING(shared, i);
      strcpy(pstr, query_string_list[i].data);
    }
    elog(INFO, "actually used bytes: %ld", SharedContexUsedSize(shared));
    Assert(SharedContexUsedSize(shared) == shared_size);
  }

  {
    int i = 0;
    for(; i < ntasks; i++)
    {
      elog(INFO, "task sql in shared memory: %s", TASK_SQL_STRING(shared, i));
    }
  }

  shm_toc_insert(pcxt->toc, PARALLEL_TASKS_KEY_SHARED, shared);

  {
    DebuggerAttachSingal* signals;
    int i;

    int shared_size2 = offsetof(DebuggerAttachSingal, wait) + sizeof(bool) * ntasks;
    signals = (DebuggerAttachSingal *)shm_toc_allocate(pcxt->toc, shared_size2);
    signals->ntasks = ntasks;

    i = 0;
    for (; i < ntasks; i++)
    {
      signals->wait[i] = 0; /* DEBUG POINT */
    }

    shm_toc_insert(pcxt->toc, PARALLEL_DEBUG_KEY_SHARED, signals);
  }

  LaunchParallelWorkers(pcxt);

  WaitForParallelWorkersToFinish(pcxt);

  count = pg_atomic_read_u64(&shared->count);

  DestroyParallelContext(pcxt);
  ExitParallelMode();

  PG_RETURN_UINT64(count);
}

void execute_sql_string(dsm_segment *seg, shm_toc *toc)
{
    int ret;
    uint32 task_index;
    char* query;

    SharedContext* shared;
    DebuggerAttachSingal* signal;

    shared = (SharedContext *) shm_toc_lookup(toc, PARALLEL_TASKS_KEY_SHARED, false);
    signal = (DebuggerAttachSingal *) shm_toc_lookup(toc, PARALLEL_DEBUG_KEY_SHARED, false);

     /* Atomically fetch and increment the task index */
    task_index = pg_atomic_fetch_add_u32(&shared->task_index, 1);

    Assert(task_index < shared->ntasks);

    while (signal->wait[task_index])
    {
      /* must have or else you could not interrupt the execution when press `Ctrl-C` */
			CHECK_FOR_INTERRUPTS();
      pg_usleep(20 * 1000);
    }

    query = TASK_SQL_STRING(shared, task_index);
    elog(INFO, "worker query: %s", query);

    SPI_connect();

    ret = SPI_exec(query, 0);
    pg_atomic_fetch_add_u64(&shared->count, SPI_processed);

    if (ret != SPI_OK_INSERT)
    {
      elog(ERROR, "cound not execute INSERT via SPI");
    }

    SPI_finish();
}
