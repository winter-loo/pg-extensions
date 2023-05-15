EXTENSION = count_trigger
MODULE_big = count_trigger
OBJS = count_trigger.o

DATA = count_trigger--1.0.sql

PG_CFLAGS = -g -O0 -Wall -Werror

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)

