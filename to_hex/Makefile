EXTENSION = to_hex
MODULE_big = to_hex
OBJS = to_hex.o

DATA = to_hex--1.0.sql

PG_CFLAGS = -g -O0 -Wall -Werror

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)

