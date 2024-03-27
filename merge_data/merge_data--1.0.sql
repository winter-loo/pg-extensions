CREATE OR REPLACE FUNCTION merge_data(tbls TEXT[], totbl TEXT default null)
RETURNS text
AS 'MODULE_PATHNAME', 'merge_data'
LANGUAGE C;
