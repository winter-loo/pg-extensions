#include "postgres.h"
#include "fmgr.h"
#include "parser/parser.h"
#include "executor/executor.h"
#include "nodes/nodeFuncs.h"
#include "utils/lsyscache.h" /* getTypeOutputInfo */
#include "parser/parse_expr.h" /* transformExpr */

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(expreval);
Datum expreval(PG_FUNCTION_ARGS)
{
  char* string_expr = PG_GETARG_CSTRING(0);
  char* retval;
  StringInfoData sql;
  List* rawParseTrees;
  RawStmt* rawStmt;

  /* construct a select statement */
  initStringInfo(&sql);
  appendStringInfoString(&sql, "SELECT ");
  appendStringInfo(&sql, "%s", string_expr);

  rawParseTrees = raw_parser(sql.data, 0);
  if (list_length(rawParseTrees) != 1) {
    elog(ERROR, "invalid expr: %s", sql.data);   
  }

  rawStmt = (RawStmt *)linitial(rawParseTrees);
  {
    char* ast = nodeToString(rawStmt);
    fprintf(stderr, "%s\n", ast);
  }

  {
    SelectStmt* selectStmt = (SelectStmt*)rawStmt->stmt;
    ResTarget* target = (ResTarget*)linitial(selectStmt->targetList);
    A_Expr* expr = (A_Expr*)target->val;
    Node* expr2 = NULL;

    /*
     * before we call `ExecInitExpr`, we must transform the raw expr first
     */
    {
      ParseState *pstate = make_parsestate(NULL);
      expr2 = transformExpr(pstate, (Node*)expr, EXPR_KIND_SELECT_TARGET);
      free_parsestate(pstate);
    }

    {
      /*
       * core expression evaluation API
       */
      ExprState* exprState = ExecInitExpr((Expr*)expr2, NULL);
      /* ExprContext* econtext = CreateExprContext(NULL); */
      ExprContext* econtext = CreateStandaloneExprContext();
      Oid typoid, typOutput;
      bool typIsVarlena;
      bool isNull;
      Datum result = ExecEvalExpr(exprState, econtext, &isNull);
      FreeExprContext(econtext, false);

      /* get expression data type */
      typoid = exprType((Node*)expr2);
      /* get the oid of its output function */
      getTypeOutputInfo(typoid, &typOutput, &typIsVarlena);
      /* invokes the output function */
      retval = (char *)OidOutputFunctionCall(typOutput, result);
    }
  }

  PG_RETURN_CSTRING(retval);
}
