#!/bin/sh -e

make
make install

psql -c "drop extension if exists pglz_decompress;create extension pglz_decompress;"