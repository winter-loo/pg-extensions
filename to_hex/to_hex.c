#include "postgres.h"
#include "fmgr.h"
#include "utils/builtins.h"
#include "port/pg_bswap.h"

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(to_hex);
void _PG_init(void);

void _PG_init(void)
{
  ereport(INFO, (errmsg("...loading extension to_hex...")));
}

Datum to_hex(PG_FUNCTION_ARGS)
{
  union
  {
    float8 f;
    int64 i;
  } swap;
  char *buf;
  int64 ni;
  bool to_net_order = false;

  swap.f = PG_GETARG_FLOAT8(0);

  to_net_order = PG_GETARG_BOOL(1);

  ni = to_net_order ? pg_hton64(swap.i) : swap.i;

  buf = psprintf("%0lX", ni);
  PG_RETURN_TEXT_P(cstring_to_text(buf));
}
