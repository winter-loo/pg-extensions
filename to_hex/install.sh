#!/bin/sh -e

make
make install

psql -c "drop extension if exists to_hex;create extension to_hex;" mydb