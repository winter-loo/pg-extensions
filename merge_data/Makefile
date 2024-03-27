EXTENSION = merge_data
MODULE_big = merge_data
OBJS = merge_data.o

DATA = merge_data--1.0.sql

ifdef DEBUG
	PG_CFLAGS = -g -O0 -Wall -Werror
endif

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)