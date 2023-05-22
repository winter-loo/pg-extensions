#include "postgres.h"
#include "fmgr.h"
#include "catalog/pg_type.h"
#include "executor/spi.h"
#include "utils/builtins.h"

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(count_returned_rows);

Datum count_returned_rows(PG_FUNCTION_ARGS)
{
    char *command;
    int cnt;
    int ret;
    int proc;

    command = text_to_cstring(PG_GETARG_TEXT_P(0));
    cnt = PG_GETARG_INT32(1);

    SPI_connect();

    ret = SPI_exec(command, cnt);
    proc = SPI_processed;

    ereport(INFO, (errmsg("SPI_processed: %d", proc)));

    if (ret > 0 && SPI_tuptable != NULL)
    {
        TupleDesc tupdesc = SPI_tuptable->tupdesc;
        SPITupleTable *tuptable = SPI_tuptable;
        char buf[8192];
        int i, j;

        for (j = 0; j < proc; j++)
        {
            HeapTuple tuple = tuptable->vals[j];
            for (i = 1, buf[0] = 0; i <= tupdesc->natts; i++)
                snprintf(buf + strlen(buf),
                         sizeof(buf) - strlen(buf),
                         " %s(%s::%s)%s",
                         SPI_fname(tupdesc, i),
                         SPI_getvalue(tuple, tupdesc, i),
                         SPI_gettype(tupdesc, i),
                         (i == tupdesc->natts) ? " " : " |");
            ereport(INFO, (errmsg("ROW: %s", buf)));
        }
    }

    SPI_finish();
    pfree(command);

    PG_RETURN_INT32(proc);
}
