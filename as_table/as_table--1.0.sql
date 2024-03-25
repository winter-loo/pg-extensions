CREATE OR REPLACE FUNCTION as_table(targettbl text, fromtbl text)
RETURNS text
AS 'MODULE_PATHNAME', 'as_table'
LANGUAGE C
STRICT;
