EXTENSION = hello_world
MODULE_big = hello_world
OBJS = hello_world.o

DATA = hello_world--1.0.sql

ifdef DEBUG
	PG_CFLAGS = -g -O0 -Wall -Werror
endif

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)