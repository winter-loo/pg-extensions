-- select count_spi_exec('select * from foo', 100);
CREATE OR REPLACE FUNCTION count_spi_exec(sql_stmt text, rows_limit int)
    RETURNS int
    AS 'spi_exec', 'count_returned_rows'
    LANGUAGE C
    STRICT;
