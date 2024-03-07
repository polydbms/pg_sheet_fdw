

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

#include "utils/memutils.h"
#include "utils/builtins.h"
#include "utils/relcache.h"
#include "utils/date.h"
#include "utils/numeric.h"
#include "utils/rel.h"
#include "utils/array.h"
#include "utils/builtins.h"

#include "catalog/pg_foreign_server.h"
#include "catalog/pg_foreign_table.h"
#include "catalog/pg_user_mapping.h"
#include "catalog/pg_type.h"

#include "nodes/nodes.h"
#include "nodes/makefuncs.h"
#include "nodes/pg_list.h"


// Debug mode flag
#define DEBUG

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

// Custom c++ functions
extern int getTestInt();

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


#endif //pg_sheet_fdw_H