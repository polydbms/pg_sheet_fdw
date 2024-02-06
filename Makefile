EXTENSION = pg_sheet_fdw
MODULE_big = pg_sheet_fdw
MODULEDIR = pg_sheet_fdw
DATA = pg_sheet_fdw--0.1.sql
OBJS = pg_sheet_fdw.o #deparse.o
#HEADERS = warp_client.h

ifdef DEBUG
$(info $(shell echo "debug ist an"))
endif

PG_LIBS = -lpq
#SHLIB_LINK = -lwarp_client -lucp
#PG_LDFLAGS += -L/usr/lib/ucx/ -L/usr/lib/warpdrive/

#PG_CPPFLAGS = -I/usr/include -I.

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
#suppress warning with mixed declarations
$(OBJS): CFLAGS += $(PERMIT_DECLARATION_AFTER_STATEMENT)
