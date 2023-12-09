#include "postgres.h"
#include "fmgr.h"
#include "utils/builtins.h"
#include "utils/array.h"

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(inspect_array);
Datum inspect_array(PG_FUNCTION_ARGS)
{
	/* add a debug point below */
	AnyArrayType *a = PG_GETARG_ANY_ARRAY_P(0);
	Oid			element_type = AARR_ELEMTYPE(a);
	/* 43 is oid of proc `int4out` */
	char* d = (char *)OidFunctionCall1(43, Int32GetDatum(element_type));
    PG_RETURN_TEXT_P(cstring_to_text(d));
}