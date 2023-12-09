#!/bin/sh -e

make
make install

psql -c "drop extension if exists inspect_array;create extension inspect_array;"