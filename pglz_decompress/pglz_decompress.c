#include "postgres.h"
#include "fmgr.h"
#include "utils/builtins.h"
#include "common/pg_lzcompress.h"

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(lz_decompress);
Datum lz_decompress(PG_FUNCTION_ARGS)
{
    bytea* source = PG_GETARG_BYTEA_P(0);
	int32 rawsize = PG_GETARG_INT32(1);

	char* str = VARDATA(source);
	int32 len = VARSIZE(source) - VARHDRSZ;
	char* buffer = palloc(rawsize);
	pglz_decompress(str, len, buffer, rawsize, false);

    PG_RETURN_TEXT_P(cstring_to_text(buffer));
}
