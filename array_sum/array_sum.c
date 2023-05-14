#include "postgres.h"
#include "fmgr.h"
#include "utils/array.h"
/* INT4OID */
#include "catalog/pg_type.h"
/* get_typlenbyvalalign */
#include "utils/lsyscache.h"

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(sum_two_int);
PG_FUNCTION_INFO_V1(array_sum);

#define DLOG(fmt, ...) \
    ereport(NOTICE, (errcode(ERRCODE_WARNING), errmsg(fmt, __VA_ARGS__)))

/* pass by value */
Datum sum_two_int(PG_FUNCTION_ARGS)
{
    int a = PG_GETARG_INT32(0);
    int b = PG_GETARG_INT32(1);
    PG_RETURN_INT32(a + b);
}

/* pass by reference */
Datum array_sum(PG_FUNCTION_ARGS)
{
    ArrayType *arr = PG_GETARG_ARRAYTYPE_P(0);
    Datum *elems;
    int nelems;
    int16 elmlen;
    bool elmbyval;
    char elmalign;
    int i;
    int64_t sum = 0;
    int32_t value = 0;

    bool *nulls;

    Assert(ARR_ELEMTYPE(arr) == INT4OID);

    if (ARR_NDIM(arr) > 1)
    {
        ereport(ERROR, (errcode(ERRCODE_ARRAY_SUBSCRIPT_ERROR), errmsg("1-dimensional array needed")));
    }

    get_typlenbyvalalign(INT4OID, &elmlen, &elmbyval, &elmalign);

    deconstruct_array(arr,
                      INT4OID, elmlen, elmbyval, elmalign,
                      &elems, &nulls, &nelems);

    DLOG("item count is %d", nelems);

    // Calculate the sum of the array elements
    for (i = 0; i < nelems; i++)
    {
        if (nulls[i])
            continue;
        value = DatumGetInt32(elems[i]);
        DLOG("item %d is %d", i, value);
        sum += value;
    }

    PG_RETURN_INT64(sum);
}