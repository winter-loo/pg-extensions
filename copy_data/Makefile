EXTENSION = copy_data
MODULE_big = copy_data
OBJS = copy_data.o

DATA = copy_data--1.0.sql

ifdef DEBUG
	PG_CFLAGS = -g -O0 -Wall -Werror
endif

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)