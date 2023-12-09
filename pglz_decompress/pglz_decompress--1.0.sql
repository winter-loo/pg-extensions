CREATE OR REPLACE FUNCTION pglz_decompress(bytea, int)
    RETURNS text
    AS 'pglz_decompress', 'lz_decompress'
    LANGUAGE C
    STRICT;
