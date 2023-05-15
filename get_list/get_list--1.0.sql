CREATE TYPE abc AS (a int, b int, c int);
CREATE TYPE perm_abc AS (a int, b int, c int, x int);

-- select reverse_tuple((1,2,3));
CREATE OR REPLACE FUNCTION reverse_tuple(abc)
    RETURNS perm_abc
    AS 'get_list', 'c_reverse_tuple'
    LANGUAGE C
    STRICT;

CREATE OR REPLACE FUNCTION permutation(abc)
    RETURNS SETOF perm_abc
    AS 'get_list', 'c_permutations_x'
    LANGUAGE C
    STRICT;