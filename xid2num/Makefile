EXTENSION = xid2num
MODULE_big = xid2num
OBJS = xid2num.o

DATA = xid2num--1.0.sql

ifdef DEBUG
	PG_CFLAGS = -g -O0 -Wall -Werror
endif

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)