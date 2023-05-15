#!/bin/sh -e

make
make install

psql -c "drop extension if exists get_list;create extension get_list;" mydb