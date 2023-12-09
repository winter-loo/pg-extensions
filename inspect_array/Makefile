EXTENSION = inspect_array
MODULE_big = inspect_array
OBJS = inspect_array.o

DATA = inspect_array--1.0.sql

ifdef DEBUG
	PG_CFLAGS = -g -O0 -Wall -Werror
endif

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)