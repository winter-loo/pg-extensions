EXTENSION = poly_add
MODULE_big = poly_add
OBJS = poly_add.o

DATA = poly_add--1.0.sql

PG_CFLAGS = -Wall -Werror -g -O0

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)

