CREATE OR REPLACE FUNCTION xid2num()
    RETURNS text
    AS 'xid2num', 'xid2num'
    LANGUAGE C
    STRICT;
