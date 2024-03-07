EXTENSION = pg_sheet_fdw
MODULE_big = pg_sheet_fdw
DATA = pg_sheet_fdw--0.1.sql
OBJS = src/pg_sheet_fdw.o src/ParserInterface.o submodules/SheetReader-r/src/XlsxFile.o submodules/SheetReader-r/src/XlsxSheet.o submodules/SheetReader-r/src/miniz/miniz.o
PG_LIBS = -lpq
PG_CPPFLAGS = -I./include -I./submodules/SheetReader-r/src
PG_CFLAGS = -I./include

ifdef DEBUG
$(info $(shell echo "debug ist an"))
endif

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)

#suppress warning with mixed declarations
$(OBJS): CFLAGS += $(PERMIT_DECLARATION_AFTER_STATEMENT)

PG_SHEET_FDW_AFTER_COMPILATION_TESTS: