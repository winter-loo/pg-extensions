-- extract page id from tuple id '(<page_id>,<tuple_offset>)'
CREATE OR REPLACE FUNCTION page(tid) RETURNS TEXT AS
$$
BEGIN
    RETURN split_part(split_part($1::text, ',', 1), '(', 2);
END;
$$ LANGUAGE plpgsql;