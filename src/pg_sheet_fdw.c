
#include "pg_sheet_fdw.h"

// Postgresql Magic block!
#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

// Postgresql visible functions
PG_FUNCTION_INFO_V1(pg_sheet_fdw_handler);
PG_FUNCTION_INFO_V1(pg_sheet_fdw_validator);

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

/*
 * This function should update baserel->rows to be the expected number of rows returned by the table scan,
 * after accounting for the filtering done by the restriction quals.
 * The initial value of baserel->rows is just a constant default estimate, which should be replaced if at all possible.
 * The function may also choose to update baserel->width if it can compute a better estimate of the average result row width.
 * (The initial value is based on column data types and on column average-width values measured by the last ANALYZE.)
 * Also, this function may update baserel->tuples if it can compute a better estimate of the foreign table's total row count.
 * (The initial value is from pg_class.reltuples which represents the total row count seen by the last ANALYZE.)
 */
void pg_sheet_fdwGetForeignRelSize(PlannerInfo *root, RelOptInfo *baserel, Oid foreigntableid){
    baserel->rows = 2;
    elog_debug("%s",__func__);
    unsigned long test = registerExcelFileAndSheetAsTable("/pg_sheet_fdw/test/joel_test.xlsx", "encoding", 0);
    elog_debug("Got row count: %lu",test);
}

/*
 * This function must generate at least one access path (ForeignPath node) for a scan on the foreign table
 * and must call add_path to add each such path to baserel->pathlist.
 * It's recommended to use create_foreignscan_path to build the ForeignPath nodes.
 * The function can generate multiple access paths, e.g., a path which has valid pathkeys to represent a pre-sorted result.
 * Each access path must contain cost estimates,
 * and can contain any FDW-private information that is needed to identify the specific scan method intended.
 */
void pg_sheet_fdwGetForeignPaths(PlannerInfo *root, RelOptInfo *baserel, Oid foreigntableid){
    elog_debug("%s",__func__);

    // dummy values
    int dummycounter;
    Cost startup_cost = 25;
    Cost total_cost = 25;
    // dummy counter in fdw_private field
    dummycounter = 2;
    List *fdw_private = list_make1_int(dummycounter);
    add_path(baserel,(Path *) create_foreignscan_path(root, baserel, NULL, baserel->rows, startup_cost, total_cost, NIL, NULL, NULL, fdw_private));
}

/*
 * Create a ForeignScan plan node from the selected foreign access path. This is called at the end of query planning.
 * The parameters are as for GetForeignRelSize, plus the selected ForeignPath
 * (previously produced by GetForeignPaths, GetForeignJoinPaths, or GetForeignUpperPaths),
 * the target list to be emitted by the plan node, the restriction clauses to be enforced by the plan node,
 * and the outer subplan of the ForeignScan, which is used for rechecks performed by RecheckForeignScan.
 * (If the path is for a join rather than a base relation, foreigntableid is InvalidOid.)
 *
 * This function must create and return a ForeignScan plan node;
 * it's recommended to use make_foreignscan to build the ForeignScan node.
 */
ForeignScan* pg_sheet_fdwGetForeignPlan(PlannerInfo *root, RelOptInfo *baserel, Oid foreigntableid, ForeignPath *best_path, List *tlist, List *scan_clauses, Plan *outer_plan){
    elog_debug("%s",__func__);

    // Retain fdw_private information.
    List *fdw_private;
    fdw_private = best_path->fdw_private;

    // Just copied!!
    Index scan_relid = baserel->relid;
    scan_clauses = extract_actual_clauses(scan_clauses, false);
    return make_foreignscan(tlist, scan_clauses, scan_relid, NIL, fdw_private, NIL, NIL, NULL);
}

/*
 * Begin executing a foreign scan. This is called during executor startup.
 * It should perform any initialization needed before the scan can start,
 * but not start executing the actual scan (that should be done upon the first call to IterateForeignScan).
 * The ForeignScanState node has already been created, but its fdw_state field is still NULL.
 * Information about the table to scan is accessible through the ForeignScanState node
 * (in particular, from the underlying ForeignScan plan node, which contains any FDW-private information provided by GetForeignPlan).
 * eflags contains flag bits describing the executor's operating mode for this plan node.
 */
void pg_sheet_fdwBeginForeignScan(ForeignScanState *node, int eflags){
    elog_debug("%s",__func__);
}

/*
 * Fetch one row from the foreign source, returning it in a tuple table slot
 * (the node's ScanTupleSlot should be used for this purpose). Return NULL if no more rows are available.
 * The tuple table slot infrastructure allows either a physical or virtual tuple to be returned;
 * in most cases the latter choice is preferable from a performance standpoint.
 * Note that this is called in a short-lived memory context that will be reset between invocations.
 * Create a memory context in BeginForeignScan if you need longer-lived storage, or use the es_query_cxt of the node's EState.
 */
TupleTableSlot *pg_sheet_fdwIterateForeignScan(ForeignScanState *node){
    elog_debug("%s",__func__);

    unsigned long test = startNextRow(0);
    elog_debug("startNextRow column count: %lu", test);
    for(unsigned long i = 0; i < test; i++){
        elog_debug("getting next cell!");
        struct PGExcelCell cell = getNextCell(0);
        elog_debug("got next cell!");
        switch (cell.type) {
            case T_STRING:
            case T_STRING_INLINE:
            case T_STRING_REF:
                elog_debug("Cell %lu with content: %s", i, cell.data.string);
                free(cell.data.string);
                break;
            case T_BOOLEAN:
                elog_debug("Cell %lu with boolean: %d", i, cell.data.boolean);
                break;
            case T_NUMERIC:
                elog_debug("Cell %lu with number: %f", i, cell.data.real);
                break;
            case T_DATE:
                elog_debug("Cell %lu with date: %f", i, cell.data.real);
                break;
            default:
                elog_debug("Some Error in struct from getNextCell()");
                break;
        }
    }
    //TODO read the received excel strings into tuples
    TupleTableSlot *slot = node->ss.ss_ScanTupleSlot;
    TupleDesc tDescFromNode = node->ss.ss_currentRelation->rd_att;
    HeapTuple tuple;

    ForeignScan *plan = (ForeignScan *) node->ss.ps.plan;
    int dummycounter =  linitial_int(plan->fdw_private);
    elog_debug("dummycounter value: %d", dummycounter);
    if(dummycounter >0) {
        Datum *columns = (Datum *) palloc0(sizeof(Datum) * 3);
        bool *isnull = (bool *) palloc(sizeof(bool) * 3);
        char *testtext = (char *) palloc0(20);
        memcpy(testtext, "Test", 5);
        isnull[0] = false;
        isnull[1] = false;
        isnull[2] = false;
        columns[0] = Int32GetDatum(getTestInt());
        columns[1] = CStringGetTextDatum(testtext);
        memcpy(testtext, "Hallo", 6);
        columns[2] = CStringGetTextDatum(testtext);
        tuple = heap_form_tuple(tDescFromNode, columns, isnull);

        list_head(plan->fdw_private)->int_value--;

        ExecClearTuple(slot);
        elog_debug("Store tuple in TupleTableSlot");
        ExecStoreHeapTuple(tuple, slot, false);
        return slot;
    }
    else return NULL;
}

/*
 * Restart the scan from the beginning. Note that any parameters the scan depends on may have changed value,
 * so the new scan does not necessarily return exactly the same rows.
 */
void pg_sheet_fdwReScanForeignScan(ForeignScanState *node){elog_debug("%s",__func__);}

/*
 * End the scan and release resources. It is normally not important to release palloc'd memory,
 * but for example open files and connections to remote servers should be cleaned up.
 */
void pg_sheet_fdwEndForeignScan(ForeignScanState *node){
    elog_debug("%s", __func__);
}


