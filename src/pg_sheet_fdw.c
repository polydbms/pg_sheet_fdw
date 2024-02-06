
// Debug mode flag
#define DEBUG

/* Macro to make conditional DEBUG more terse
 * Usage: elog(String); output can be found in console */
#ifdef DEBUG
#define elog_debug(...) elog(WARNING, __VA_ARGS__)
#else
#define elog_debug(...) ((void) 0)
#endif

#include "pg_sheet_fdw.h"

// Postgresql Magic block!
PG_MODULE_MAGIC;

// Postgresql visible functions
PG_FUNCTION_INFO_V1(pg_sheet_fdw_handler);
PG_FUNCTION_INFO_V1(pg_sheet_fdw_validator);

// Initialization functions
extern void _PG_init(void);
extern void _PG_fini(void);

// FDW callback routines
static void pg_sheet_fdwBeginForeignScan(ForeignScanState *node, int eflags);
static TupleTableSlot *pg_sheet_fdwIterateForeignScan(ForeignScanState *node);
static void pg_sheet_fdwReScanForeignScan(ForeignScanState *node);
static void pg_sheet_fdwEndForeignScan(ForeignScanState *node);
static void pg_sheet_fdwGetForeignRelSize(PlannerInfo *root, RelOptInfo *baserel, Oid foreigntableid);
static void pg_sheet_fdwGetForeignPaths(PlannerInfo *root, RelOptInfo *baserel, Oid foreigntableid);
static ForeignScan* pg_sheet_fdwGetForeignPlan(PlannerInfo *root, RelOptInfo *baserel, Oid foreigntableid, ForeignPath *best_path, List *tlist, List *scan_clauses, Plan *outer_plan);

// Gets called as soon as the library is loaded into memory once.
// Here you can load global stuff needed for the FDW to work.
void _PG_init(void){
    elog_debug("Initialization of FDW!");
}

// Gets NEVER called!! (May change in postgres future)
void _PG_fini(void){
    elog_debug("deinitialization got called");
}

// The handler function just returns function pointers to all FDW functions
Datum pg_sheet_fdw_handler(PG_FUNCTION_ARGS){
    elog_debug("%s",__func__);

    FdwRoutine *fdwroutine = makeNode(FdwRoutine);

    fdwroutine->GetForeignRelSize = pg_sheet_fdwGetForeignRelSize;
    fdwroutine->GetForeignPaths = pg_sheet_fdwGetForeignPaths;
    fdwroutine->GetForeignPlan = pg_sheet_fdwGetForeignPlan;
    fdwroutine->BeginForeignScan = pg_sheet_fdwBeginForeignScan;
    fdwroutine->IterateForeignScan = pg_sheet_fdwIterateForeignScan;
    fdwroutine->ReScanForeignScan = pg_sheet_fdwReScanForeignScan;
    fdwroutine->EndForeignScan = pg_sheet_fdwEndForeignScan;

    PG_RETURN_POINTER(fdwroutine);
}




