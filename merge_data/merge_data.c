#include "postgres.h"
#include "access/heapam.h"
#include "access/relscan.h"
#include "access/table.h"
#include "fmgr.h"
#include "miscadmin.h"
#include "nodes/makefuncs.h"
#include "utils/array.h"
#include "utils/builtins.h"
#include "utils/lsyscache.h"
#include "utils/rel.h"
#include "utils/relcache.h"
#include "utils/snapmgr.h"

PG_MODULE_MAGIC;

/*
 * Extracted from tablecmds.c
 *
 * `table_openrv` and `table_close` is needed to be invoked before and after
 * calling `copy_data` function.
 */
static void copy_data(Relation srcrel, Relation dstrel)
{
    Snapshot snapshot;
    TableScanDesc scan;
    TupleTableSlot *srcslot, *dstslot;
    BulkInsertState bistate;
    int ti_options = 0;
    CommandId mycid = GetCurrentCommandId(true);
    TupleConversionMap* tuple_map;

    bistate = GetBulkInsertState();
    dstslot = MakeSingleTupleTableSlot(RelationGetDescr(dstrel), table_slot_callbacks(dstrel));

    ExecStoreAllNullTuple(dstslot);

    srcslot = MakeSingleTupleTableSlot(RelationGetDescr(srcrel), table_slot_callbacks(srcrel));

    /* need this to check tuple attributes(i.e. table columns) compatibility
     * 
     * ```sql
     * create table a(a int, c date);
     * create table b(b int, c date);
     * create table c(c date);
     *
     * select merge_data('{"a", "b"}', 'c');
     * ```
     *
     * Only the attribute `c` of `a` and the attribute `c` of `b` are copied to `c`
     *
     * If any arribute of `c` not in `a` or `b`, error will be raised.
     *
     */
    tuple_map = convert_tuples_by_name(RelationGetDescr(srcrel), RelationGetDescr(dstrel));

    snapshot = RegisterSnapshot(GetTransactionSnapshot());
    scan = table_beginscan(srcrel, snapshot, 0, NULL);
    elog(NOTICE, "start copy data");
    while (table_scan_getnextslot(scan, ForwardScanDirection, srcslot))
    {
        TupleTableSlot* insertslot;

        /* read one row from original table */
        slot_getallattrs(srcslot);

        if (tuple_map)
        {
            insertslot = execute_attr_map_slot(tuple_map->attrMap, srcslot, dstslot);
        }
        else
        {
            insertslot = srcslot;

            ExecClearTuple(insertslot);
            memcpy(insertslot->tts_values, srcslot->tts_values, sizeof(Datum) * srcslot->tts_nvalid);
            memcpy(insertslot->tts_isnull, srcslot->tts_isnull, sizeof(bool) * srcslot->tts_nvalid);
            ExecStoreVirtualTuple(insertslot);
        }
        
        table_tuple_insert(dstrel, insertslot, mycid, ti_options, bistate);
        /* pg_usleep(1000000); */

        /* fast cancel copying by Ctrl+C */
        CHECK_FOR_INTERRUPTS();
    }
    table_endscan(scan);
    UnregisterSnapshot(snapshot);
    elog(NOTICE, "end copy data");

    ExecDropSingleTupleTableSlot(srcslot);
    ExecDropSingleTupleTableSlot(dstslot);
    FreeBulkInsertState(bistate);
    table_finish_bulk_insert(dstrel, ti_options);
}

static void
do_merge_data(char* fromtbls[], int num_tbls, char* dsttbl);

PG_FUNCTION_INFO_V1(merge_data);
Datum merge_data(PG_FUNCTION_ARGS)
{
    ArrayType *arr1;
    char* dsttbl = NULL;
    int16 elmlen;
    bool elmbyval;
    char elmalign;
    int num_elems;
    Datum* elem_values;
    bool *elem_nulls;
#define MAX_INPUTS 10
    char* fromtbls[MAX_INPUTS];

    if (PG_ARGISNULL(0))
        elog(ERROR, "the first argument must not be null");

    arr1 = PG_GETARG_ARRAYTYPE_P(0);

    if (!PG_ARGISNULL(1))
    {
        text* t = PG_GETARG_TEXT_PP(1);
        dsttbl = text_to_cstring(t);
    }

    get_typlenbyvalalign(ARR_ELEMTYPE(arr1), &elmlen, &elmbyval, &elmalign);
    deconstruct_array(arr1, ARR_ELEMTYPE(arr1), elmlen, elmbyval, elmalign, &elem_values, &elem_nulls, &num_elems);

    if (num_elems < 2 || num_elems > MAX_INPUTS) {
        elog(ERROR, "array size of the first argument must be at least 2 to 10");
    }

    for (int i = 0; i < num_elems; i++)
    {
        if (elem_nulls[i])
        {
            elog(ERROR, "element %d is null", i);
        }
        else
        {
            text* elem_text = DatumGetTextPP(elem_values[i]);
            char* elem_str = text_to_cstring(elem_text);
            fromtbls[i] = elem_str;
            if (elem_str[0] == '\0')
            {
                elog(ERROR, "element %d is empty", i);
            }
        }
    }

    for (int i = 0; i < num_elems; i++)
    {
        for (int j = i + 1; j < num_elems; j++)
        {
            if (strcasecmp(fromtbls[i], fromtbls[j]) == 0)
            {
                elog(ERROR, "element %d and %d have the same name", i, j);
            }
        }
    }

    /* basic argument validation done */

    do_merge_data(fromtbls, num_elems, dsttbl);

    PG_RETURN_TEXT_P(cstring_to_text("DONE"));
}

static void
do_merge_data(char* fromtbls[], int num_tbls, char* dsttbl)
{
    /* list must be initialized to NULL for lappend function, otherwise runtime crash */
    List* rels = NIL;
    ListCell* lc;
    int i;
    int has_same_name = -1;
    Relation dstrel;

    for (i = 0; i < num_tbls; i++)
    {
        char* fromtbl = fromtbls[i];
        Relation rel = table_openrv(makeRangeVar(NULL, fromtbl, -1), AccessExclusiveLock);
        if (has_same_name == -1 && (dsttbl == NULL || strcasecmp(fromtbl, dsttbl) == 0))
        {
            has_same_name = i;
            dstrel = rel;
        }
        else
        {
            rels = lappend(rels, rel);
        }
    }

    if (has_same_name == -1)
    {
       dstrel = table_openrv(makeRangeVar(NULL, dsttbl, -1), AccessExclusiveLock);
    }


    foreach(lc, rels)
    {
        Relation rel = lfirst(lc);
        elog(INFO, "copying data from %s to %s", RelationGetRelationName(rel), RelationGetRelationName(dstrel));
        copy_data(rel, dstrel);
    }


    /* epilog: cleanup now */

    foreach(lc, rels)
    {
        Relation rel = lfirst(lc);
        table_close(rel, NoLock);
    }
    table_close(dstrel, NoLock);
}
