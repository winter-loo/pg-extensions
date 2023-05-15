#!/bin/sh -e

make
make install

psql -c "drop extension if exists is_palindrome;create extension is_palindrome;" mydb