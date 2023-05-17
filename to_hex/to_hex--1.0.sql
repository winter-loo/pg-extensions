-- first arg: the value needed to print in hexdecimal form
-- second arg: whether print in network byte order
CREATE OR REPLACE FUNCTION to_hex(float8, boolean DEFAULT false)
    RETURNS text
    AS 'to_hex'
    LANGUAGE C
    STRICT;
