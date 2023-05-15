#!/bin/sh -e

make
make install

psql -c "drop extension get_list;create extension get_list;" mydb