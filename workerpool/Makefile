EXTENSION = workerpool
MODULE_big = workerpool
OBJS = workerpool.o

DATA = workerpool--1.0.sql

ifdef DEBUG
	PG_CFLAGS = -g -O0 -Wall -Werror
endif

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)