CREATE OR REPLACE FUNCTION poly_add(anyelement, anyelement)
    RETURNS anyelement
    AS 'poly_add', 'poly_add'
    LANGUAGE C
    STRICT;

CREATE OR REPLACE FUNCTION make_array(anyelement)
    RETURNS anyarray
    AS 'poly_add', 'make_array'
    LANGUAGE C IMMUTABLE;