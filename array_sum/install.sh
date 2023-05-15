#!/bin/sh -e

make
make install

psql -c "drop extension if exists array_sum;create extension array_sum;" mydb