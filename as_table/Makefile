EXTENSION = as_table
MODULE_big = as_table
OBJS = as_table.o

DATA = as_table--1.0.sql

ifdef DEBUG
	PG_CFLAGS = -g -O0 -Wall -Werror
endif

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)