CREATE TYPE abc AS (a int, b int, c int);

CREATE OR REPLACE FUNCTION reverse_tuple(abc)
    RETURNS RECORD
    AS 'get_list', 'c_reverse_tuple'
    LANGUAGE C
    STRICT;

CREATE OR REPLACE FUNCTION permutation(abc)
    RETURNS TABLE(a int, b int, c int, x int)
    AS 'get_list', 'c_permutations_x'
    LANGUAGE C
    STRICT;