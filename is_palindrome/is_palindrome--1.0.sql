CREATE OR REPLACE FUNCTION is_palindrome(text)
    RETURNS boolean
    AS 'is_palindrome', 'is_palindrome'
    LANGUAGE C
    STRICT;
