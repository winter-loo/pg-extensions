#!/bin/sh -e

make
make install

psql -c "drop extension if exists hello_world;create extension hello_world;" mydb