CREATE OR REPLACE FUNCTION hello_world()
    RETURNS text
    AS 'hello_world', 'hello_world'
    LANGUAGE C
    STRICT;
