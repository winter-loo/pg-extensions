#!/bin/sh -e

make
make install

psql -c "drop extension if exists add_con;create extension add_con;" mydb