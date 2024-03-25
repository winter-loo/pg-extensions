#include "postgres.h"
#include "fmgr.h"
#include "utils/builtins.h"
#include "nodes/primnodes.h"
#include "nodes/parsenodes.h"
#include "nodes/plannodes.h"
#include "nodes/makefuncs.h"
#include "tcop/utility.h"

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(as_table);
Datum as_table(PG_FUNCTION_ARGS)
{
    text* tn1 = PG_GETARG_TEXT_PP(0);
    text* tn2 = PG_GETARG_TEXT_PP(1);
    char* tblname1 = text_to_cstring(tn1);
    char* tblname2 = text_to_cstring(tn2);

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

    /*
     * Indexes will be inherited on "attach new partitions" stage, after data
     * moving.
     */
    tlc->options = CREATE_TABLE_LIKE_ALL & ~CREATE_TABLE_LIKE_INDEXES;
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

    PG_RETURN_TEXT_P(cstring_to_text("DONE"));
}
