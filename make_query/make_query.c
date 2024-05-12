#include "postgres.h"
#include "fmgr.h"
#include "nodes/makefuncs.h"
#include "nodes/parsenodes.h"
#include "nodes/primnodes.h"
#include "nodes/print.h"
#include "parser/parse_node.h"
#include "parser/parse_relation.h"
#include "parser/parser.h"
#include "utils/builtins.h"
#include "utils/portal.h"
#include "tcop/cmdtag.h"
#include "tcop/pquery.h"
#include "tcop/tcopprot.h"
#include "tcop/dest.h"
#include "utils/snapshot.h"

PG_MODULE_MAGIC;

void _PG_init(void) {
  ereport(INFO, (errmsg("...loading extension make_query...")));
}

PG_FUNCTION_INFO_V1(make_query);
Datum make_query(PG_FUNCTION_ARGS)
{
    char *str = "Hello, world!";

    Query	   *qry = makeNode(Query);
    RangeTblEntry *rte;
    ParseState *pstate = make_parsestate(NULL);
    RangeVar* rel;
    FuncCall* fncall;
    TargetEntry* te;
    RangeTblRef* rtr;
    Var* arg;

    qry->commandType = CMD_SELECT;

    rel = makeRangeVar(NULL, "t", -1);
    addRangeTableEntry(pstate, rel, NULL, false, false);

    arg = makeVar(1, 1, 23, -1, 0, 0);
    fncall = makeFuncCall(SystemFuncName("sum"),
                          list_make1(arg),
                          COERCE_IMPLICIT_CAST,
                          -1);
	  te = makeTargetEntry((Expr *) fncall,
						   (AttrNumber) pstate->p_next_resno++,
						   "sum",
						   false);
    qry->targetList = lappend(qry->targetList, te);

    qry->rtable = pstate->p_rtable;
    qry->rteperminfos = pstate->p_rteperminfos;

		rtr = makeNode(RangeTblRef);
		rtr->rtindex = 1;
    pstate->p_joinlist = lappend(pstate->p_joinlist, rtr);
    qry->jointree = makeFromExpr(pstate->p_joinlist, NULL);

    {
        char* s = nodeToString(qry);
        elog_node_display(LOG, "parse tree", qry, false);
        pfree(s);
    }

    {
        Portal portal;
        DestReceiver* receiver;
        QueryCompletion qc;
        List* plantree_list;
        PlannedStmt * planstmt;
        planstmt = pg_plan_query(qry, NULL, CURSOR_OPT_PARALLEL_OK, NULL);
        plantree_list = lappend(plantree_list, planstmt);

        portal = CreatePortal("", true, true);
        /* Don't display the portal in pg_cursors */
        portal->visible = false;

        PortalDefineQuery(portal, NULL, NULL, CMDTAG_SELECT, plantree_list, NULL);
        PortalStart(portal, NULL, 0, InvalidSnapshot);

        receiver = CreateDestReceiver(DestDebug);

        PortalRun(portal, FETCH_ALL, true, true, receiver, receiver, &qc);

        receiver->rDestroy(receiver);

        PortalDrop(portal, false);
    }

    PG_RETURN_TEXT_P(cstring_to_text(str));
}
