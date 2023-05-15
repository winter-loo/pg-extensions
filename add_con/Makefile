EXTENSION = add_con
MODULE_big = add_con
OBJS = add_con.o

DATA = add_con--1.0.sql

PG_CFLAGS = -g -O0 -Wall -Werror

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)

