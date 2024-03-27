#include "postgres.h"
#include "access/heapam.h"
#include "access/table.h"
#include "fmgr.h"
#include "nodes/makefuncs.h"
#include "nodes/parsenodes.h"
#include "nodes/primnodes.h"
#include "utils/builtins.h"
#include "utils/snapmgr.h"
#include "utils/rel.h"
#include "tcop/utility.h"
#include "miscadmin.h"

PG_MODULE_MAGIC;

static void as_table(text* t1, text* t2)
{
    char* tblname1 = text_to_cstring(t1);
    char* tblname2 = text_to_cstring(t2);
    RangeVar* rtbl1 = makeRangeVar(NULL, tblname1, -1);
    RangeVar* rtbl2 = makeRangeVar(NULL, tblname2, -1);

    CreateStmt *createStmt;
    TableLikeClause *tlc;
    PlannedStmt *wrapper;

    createStmt = makeNode(CreateStmt);
    createStmt->relation = rtbl1;
    createStmt->tableElts = NIL;
    createStmt->inhRelations = NIL;
    createStmt->constraints = NIL;
    createStmt->options = NIL;
    createStmt->oncommit = ONCOMMIT_NOOP;
    createStmt->tablespacename = NULL;
    createStmt->if_not_exists = false;

    tlc = makeNode(TableLikeClause);
    tlc->relation = rtbl2;

    tlc->options = CREATE_TABLE_LIKE_ALL;
    tlc->relationOid = InvalidOid;
    createStmt->tableElts = lappend(createStmt->tableElts, tlc);

    /* Need to make a wrapper PlannedStmt. */
    wrapper = makeNode(PlannedStmt);
    wrapper->commandType = CMD_UTILITY;
    wrapper->canSetTag = false;
    wrapper->utilityStmt = (Node *) createStmt;
    wrapper->stmt_location = -1;
    wrapper->stmt_len = 0;

    ProcessUtility(wrapper, "internal query string", false,
        PROCESS_UTILITY_TOPLEVEL, NULL, NULL, NULL, NULL);
}

/*
 * Extracted from tablecmds.c
 */
PG_FUNCTION_INFO_V1(copy_data);
Datum copy_data(PG_FUNCTION_ARGS)
{
    /* copy data from t1 to t2 */
    text* t1 = PG_GETARG_TEXT_PP(0);
    text* t2 = PG_GETARG_TEXT_PP(1);
    bool  create_table = PG_GETARG_BOOL(2);

    RangeVar* rt1 = makeRangeVar(NULL, text_to_cstring(t1), -1);
    RangeVar* rt2 = makeRangeVar(NULL, text_to_cstring(t2), -1);
    Relation r1, r2;
    Snapshot snapshot;
    TableScanDesc scan;
    TupleTableSlot *srcslot, *dstslot;
    BulkInsertState bistate;
    int ti_options = 0;
    CommandId mycid = GetCurrentCommandId(true);
    TupleConversionMap* tuple_map;

    if (create_table)
    {
        /* create table t1 like t2 */
        as_table(t1, t2);
    }
    r1 = table_openrv(rt1, AccessExclusiveLock);
    r2 = table_openrv(rt2, AccessShareLock);

    bistate = GetBulkInsertState();
    dstslot = MakeSingleTupleTableSlot(RelationGetDescr(r2), table_slot_callbacks(r2));

    ExecStoreAllNullTuple(dstslot);

    srcslot = MakeSingleTupleTableSlot(RelationGetDescr(r1), table_slot_callbacks(r1));

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
    tuple_map = convert_tuples_by_name(RelationGetDescr(r1), RelationGetDescr(r2));

    snapshot = RegisterSnapshot(GetTransactionSnapshot());
    scan = table_beginscan(r1, snapshot, 0, NULL);
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
        
        table_tuple_insert(r2, insertslot, mycid, ti_options, bistate);
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
    table_finish_bulk_insert(r2, ti_options);

    
    table_close(r1, NoLock);
    table_close(r2, NoLock);

    PG_RETURN_TEXT_P(cstring_to_text("DONE"));
}
