CREATE OR REPLACE FUNCTION eval(cstring)
RETURNS cstring
AS 'MODULE_PATHNAME', 'expreval'
LANGUAGE C
STRICT;
