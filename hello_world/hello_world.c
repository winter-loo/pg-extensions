#include "postgres.h"
#include "fmgr.h"
#include "utils/builtins.h"

PG_MODULE_MAGIC;

void _PG_init(void) {
  ereport(INFO, (errmsg("...loading extension hello_world...")));
}

PG_FUNCTION_INFO_V1(hello_world);
Datum hello_world(PG_FUNCTION_ARGS)
{
    char *str = "Hello, world!";
    PG_RETURN_TEXT_P(cstring_to_text(str));
}
