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
    -- the symbol name can be ommited if this sql function name included in the object file
    AS 'add_con'
    LANGUAGE C
    STRICT;

CREATE OR REPLACE FUNCTION shared_get()
    RETURNS int
    AS 'add_con', 'shared_get'
    LANGUAGE C
    STRICT;

-- usage:
-- select add_con(), add_priv();
-- select shared_reset();
