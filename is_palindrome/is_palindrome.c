#include "postgres.h"
#include "fmgr.h"
#include "utils/builtins.h"
#include <string.h>

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(is_palindrome);

Datum is_palindrome(PG_FUNCTION_ARGS)
{
    bool answer = true;
    text* input_str = PG_GETARG_TEXT_P(0);
    char* arg_str;
    int i = 0, j = 0;

    arg_str = text_to_cstring(input_str);

    j = strlen(arg_str) - 1;

    for (i = 0; i < j; i++, j--) {
        if (arg_str[i] != arg_str[j]) {
            answer = false;
            break;
        }
    }

    PG_RETURN_BOOL(answer);
}
