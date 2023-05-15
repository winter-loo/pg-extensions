#!/bin/sh -e

make
make install

psql -c "drop extension if exists poly_add;create extension poly_add;" mydb