CREATE OR REPLACE FUNCTION copy_data(fromtbl text, totbl text, create_table boolean DEFAULT false)
RETURNS text
AS 'MODULE_PATHNAME', 'copy_data'
LANGUAGE C
STRICT;
