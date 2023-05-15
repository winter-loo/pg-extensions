#!/bin/sh -e

make
make install

psql -c "drop extension if exists count_trigger;create extension count_trigger;" mydb