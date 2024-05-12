EXTENSION = make_query
MODULE_big = make_query
OBJS = make_query.o

DATA = make_query--1.0.sql

ifdef DEBUG
	PG_CFLAGS = -g -O0 -Wall -Werror
endif

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)