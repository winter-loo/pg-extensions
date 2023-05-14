EXTENSION = array_sum
MODULE_big = array_sum
OBJS = array_sum.o

DATA = array_sum--1.0.sql

PG_CFLAGS = -g -O0

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)

