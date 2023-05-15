#include "postgres.h"
#include "fmgr.h"
#include "utils/builtins.h"
#include "catalog/pg_type.h"
#include "utils/numeric.h"
#include "utils/array.h"
#include "utils/lsyscache.h"

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(poly_add);
PG_FUNCTION_INFO_V1(make_array);

// add two integers, numerics, strings
Datum poly_add(PG_FUNCTION_ARGS)
{
    Oid elem_type = get_fn_expr_argtype(fcinfo->flinfo, 0);

    switch (elem_type)
    {
    case INT4OID:
    {
        int a = PG_GETARG_INT32(0);
        int b = PG_GETARG_INT32(1);
        PG_RETURN_INT64(a + b);
    }
    case INT8OID:
    {
        int64 a = PG_GETARG_INT64(0);
        int64 b = PG_GETARG_INT64(1);
        PG_RETURN_INT64(a + b);
    }
    case FLOAT4OID:
    {
        float a = PG_GETARG_FLOAT4(0);
        float b = PG_GETARG_FLOAT4(1);
        PG_RETURN_FLOAT8(a + b);
    }
    case FLOAT8OID:
    {
        double a = PG_GETARG_FLOAT8(0);
        double b = PG_GETARG_FLOAT8(1);
        PG_RETURN_FLOAT8(a + b);
    }
    break;
    case NUMERICOID:
    {
        Numeric a = PG_GETARG_NUMERIC(0);
        Numeric b = PG_GETARG_NUMERIC(1);
        bool have_error = false;
        Numeric s = numeric_add_opt_error(a, b, &have_error);
        if (have_error)
        {
            elog(ERROR, "invalid input");
        }
        PG_RETURN_NUMERIC(s);
    }
    case BPCHAROID:
    case VARCHAROID:
    case TEXTOID:
    {
        text *a = PG_GETARG_TEXT_P(0);
        text *b = PG_GETARG_TEXT_P(1);
        char *a_cstr = text_to_cstring(a);
        size_t a_len = strlen(a_cstr);
        char *b_cstr = text_to_cstring(b);
        size_t b_len = strlen(b_cstr);
        size_t tot_len = a_len + b_len + 1;
        char *buf = palloc0(tot_len);
        strcat(buf, a_cstr);
        strcat(buf + a_len, b_cstr);
        PG_RETURN_TEXT_P(cstring_to_text(buf));
    }
    break;

    default:
        elog(ERROR, "not supported type: %d", elem_type);
        break;
    }

    PG_RETURN_NULL();
}

Datum make_array(PG_FUNCTION_ARGS)
{
    ArrayType *result;
    Oid element_type = get_fn_expr_argtype(fcinfo->flinfo, 0);
    Datum element;
    bool isnull;
    int16 typlen;
    bool typbyval;
    char typalign;
    int ndims;
    int dims[MAXDIM];
    int lbs[MAXDIM];

    if (!OidIsValid(element_type))
        elog(ERROR, "could not determine data type of input");

    /* get the provided element, being careful in case it's NULL */
    isnull = PG_ARGISNULL(0);
    if (isnull)
        element = (Datum)0;
    else
        element = PG_GETARG_DATUM(0);

    /* we have one dimension */
    ndims = 1;
    /* and one element */
    dims[0] = 1;
    /* and lower bound is 1 */
    lbs[0] = 1;

    /* get required info about the element type */
    get_typlenbyvalalign(element_type, &typlen, &typbyval, &typalign);

    /* now build the array */
    result = construct_md_array(&element, &isnull, ndims, dims, lbs,
                                element_type, typlen, typbyval, typalign);

    PG_RETURN_ARRAYTYPE_P(result);
}