EXTENSION = pglz_decompress
MODULE_big = pglz_decompress
OBJS = pglz_decompress.o

DATA = pglz_decompress--1.0.sql

ifdef DEBUG
	PG_CFLAGS = -g -O0 -Wall -Werror
endif

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)