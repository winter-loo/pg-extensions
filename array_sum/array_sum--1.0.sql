CREATE FUNCTION sum_two_int(int, int)
    RETURNS int
    AS 'array_sum', 'sum_two_int'
-- you need to check null if using STRICT mode
LANGUAGE C;

CREATE FUNCTION array_sum(int[])
    RETURNS bigint
    AS 'array_sum', 'array_sum'
LANGUAGE C STRICT;

CREATE FUNCTION sum_some(VARIADIC int[])
    RETURNS bigint
    AS 'array_sum', 'array_sum'
LANGUAGE C STRICT;

