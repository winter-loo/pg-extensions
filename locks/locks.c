#include "postgres.h"
#include "fmgr.h"
#include "storage/lwlock.h"
#include "utils/builtins.h"

PG_MODULE_MAGIC;

void _PG_init(void) {
  /* ereport(INFO, (errmsg("...loading extension locks..."))); */
}

PG_FUNCTION_INFO_V1(locks);
Datum locks(PG_FUNCTION_ARGS)
{
    LWLockAcquire(AutoFileLock, LW_EXCLUSIVE);
    ereport(ERROR, errmsg("test lwlock cleanup on rollback implicitly"));
    PG_RETURN_TEXT_P(cstring_to_text("Ok"));
}
