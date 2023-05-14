#!/bin/sh -e

make
make install

psql -c "drop extension array_sum;create extension array_sum;" mydb