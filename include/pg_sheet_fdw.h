

#ifndef pg_sheet_fdw_H
#define pg_sheet_fdw_H


#include "postgres.h"
#include "fmgr.h"
#include "funcapi.h"
#include "foreign/fdwapi.h"
#include "foreign/foreign.h"
#include "nodes/nodes.h"
#include "optimizer/pathnode.h"
#include "optimizer/planmain.h"
#include "optimizer/restrictinfo.h"
#include "access/tupdesc.h"
#include "commands/defrem.h"

#include "utils/memutils.h"
#include "utils/builtins.h"
#include "utils/relcache.h"
#include "utils/date.h"
#include "utils/numeric.h"
#include "utils/rel.h"
#include "utils/array.h"
#include "utils/builtins.h"
#include "utils/int8.h"

#include "catalog/pg_foreign_server.h"
#include "catalog/pg_foreign_table.h"
#include "catalog/pg_user_mapping.h"
#include "catalog/pg_type.h"

#include "nodes/nodes.h"
#include "nodes/makefuncs.h"
#include "nodes/pg_list.h"


// Debug mode flag
//#define DEBUG

/* Macro to make conditional DEBUG more terse
 * Usage: elog(String); output can be found in console */
#ifdef DEBUG
#define elog_debug(...) elog(NOTICE, __VA_ARGS__)
#else
#define elog_debug(...) ((void) 0)
#endif


// Initialization functions
extern void _PG_init(void);

extern void _PG_fini(void);


// Custom c++ functions and structs
enum PGExcelCellType {
    T_NONE = 0, // blank cell
    T_NUMERIC = 1, // integer or double
    T_STRING_REF = 2, // we treat all string types like basic null terminated c strings
    T_STRING = 3,
    T_STRING_INLINE = 4,
    T_BOOLEAN = 5, // boolean is just int
    T_ERROR = 6,
    T_DATE = 7 // datetime value, already as unix timestamp (seconds since 1970), Excel stores as number of days since 1900
};

struct PGExcelCell {
    union {
        double real;
        unsigned long long stringIndex;
        unsigned char boolean;  // Using char (1 byte) for boolean in C
    } data;
    unsigned char type;
};

unsigned long registerExcelFileAndSheetAsTable(const char *pathToFile, const char *sheetName, unsigned int tableOID, int numberOfThreads);
unsigned long startNextRow(unsigned int tableOID);
struct PGExcelCell getNextCell(unsigned int tableOID);
struct PGExcelCell *getNextCellCast(unsigned int tableOID);
char* readStaticString(unsigned int tableOID, unsigned long long stringIndex);
char* readDynamicString(unsigned int tableOID, unsigned long long stringIndex);
void dropTable(unsigned int tableOID);

// FDW callback routines
void pg_sheet_fdwBeginForeignScan(ForeignScanState *node, int eflags);

TupleTableSlot *pg_sheet_fdwIterateForeignScan(ForeignScanState *node);

void pg_sheet_fdwReScanForeignScan(ForeignScanState *node);

void pg_sheet_fdwEndForeignScan(ForeignScanState *node);

void pg_sheet_fdwGetForeignRelSize(PlannerInfo *root, RelOptInfo *baserel, Oid foreigntableid);

void pg_sheet_fdwGetForeignPaths(PlannerInfo *root, RelOptInfo *baserel, Oid foreigntableid);

ForeignScan *
pg_sheet_fdwGetForeignPlan(PlannerInfo *root, RelOptInfo *baserel, Oid foreigntableid, ForeignPath *best_path,
                           List *tlist, List *scan_clauses, Plan *outer_plan);


// helper variables and structs
typedef struct {
    // Memory context for allocations
    MemoryContext context;
    // used as unique identifier in the parserInterface
    unsigned int tableID;
    int columnCount;
    int batchSize;
    unsigned long rowsLeft;
    unsigned long rowsPrefetched;
    unsigned long rowsRead;
    unsigned long batchIndex;
    // Used to check against received types from the ParserInterface
    Oid* expectedTypes;
    // memory for prefetched values
    Datum** cells;
    bool** isnull;
} pg_sheet_scanstate;

//helper functions
static void pg_sheet_fdwGetOptions(Oid foreigntableid, char **filepath, char **sheetname, unsigned long* batchSize, int* numberOfThreads);
Datum pg_sheet_fdwConvertSheetNumericToPG(struct PGExcelCell* cell, Oid expectedType);

#endif //pg_sheet_fdw_H