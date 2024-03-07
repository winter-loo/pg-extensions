#!/bin/sh -e

make install

psql -c "drop extension if exists xid2num;create extension xid2num;"
