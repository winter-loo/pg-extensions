CREATE OR REPLACE FUNCTION shared_add(int DEFAULT 1)
    RETURNS int
    AS 'add_con', 'shared_add'
    LANGUAGE C
    STRICT;

CREATE OR REPLACE FUNCTION priv_add(int DEFAULT 1)
    RETURNS int
    AS 'add_con', 'priv_add'
    LANGUAGE C
    STRICT;

CREATE OR REPLACE FUNCTION shared_reset()
    RETURNS VOID
    AS 'add_con', 'shared_reset'
    LANGUAGE C
    STRICT;

-- usage:
-- select add_con(), add_priv();
-- select shared_reset();