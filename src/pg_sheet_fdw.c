
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
    elog_debug("%s",__func__);
    //TODO handle error cases
    char *filepath;
    char *sheetname;
    pg_sheet_fdwGetOptions(foreigntableid, &filepath, &sheetname);
    unsigned long test = registerExcelFileAndSheetAsTable(filepath, sheetname, 0);
    elog_debug("Got row count: %lu",test);
    baserel->rows = (double) test;
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

    Cost startup_cost = 25;
    Cost total_cost = 25;
    add_path(baserel,(Path *) create_foreignscan_path(root, baserel, NULL, baserel->rows, startup_cost, total_cost, NIL, NULL, NULL, NULL));
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

    // Just copied!!
    Index scan_relid = baserel->relid;
    scan_clauses = extract_actual_clauses(scan_clauses, false);
    return make_foreignscan(tlist, scan_clauses, scan_relid, NIL, NULL, NIL, NIL, NULL);
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

    TupleTableSlot *slot = node->ss.ss_ScanTupleSlot;
    TupleDesc tDescFromNode = node->ss.ss_currentRelation->rd_att;
    HeapTuple tuple;

    //TODO read the received excel strings into tuples
    //TODO replace id 0 with tableOID (where to get it here?)

    // set iterator to next excel row and read column count
    unsigned long columnCount = startNextRow(0);
    elog_debug("startNextRow column count: %lu", columnCount);
    // no more rows to read? finish
    if(columnCount == 0) return NULL;
    // prepare tuple data structure
    Datum *columns = (Datum *) palloc0(sizeof(Datum) * columnCount);
    bool *isnull = (bool *) palloc(sizeof(bool) * columnCount);
    // fill tuple with cells
    for(unsigned long i = 0; i < columnCount; i++){
        elog_debug("getting next cell!");
        struct PGExcelCell cell = getNextCell(0);
        elog_debug("got next cell!");
        switch (cell.type) {
            case T_STRING:
            case T_STRING_INLINE:
            case T_STRING_REF:
                elog_debug("Cell %lu with content: %s", i, cell.data.string);
                columns[i] = CStringGetTextDatum(cell.data.string);
                isnull[i] = false;
                free(cell.data.string);
                break;
            case T_BOOLEAN:
                elog_debug("Cell %lu with boolean: %d", i, cell.data.boolean);
                columns[i] = BoolGetDatum(cell.data.boolean);
                isnull[i] = false;
                break;
            case T_NUMERIC: //TODO convert number to pg number. Probably function needed. Trivial for integers, not trivial for the rest.
                elog_debug("Cell %lu with number: %f", i, cell.data.real);
                // for now assume bigint from postgres (64bit integer)
                columns[i] = Int64GetDatum((long) cell.data.real);
                isnull[i] = false;
                break;
            case T_DATE: //TODO function to convert between date formats.
                elog_debug("Cell %lu with date: %f", i, cell.data.real);
                // for now assume timestamp
                columns[i] = TimeADTGetDatum(cell.data.real);
                isnull[i] = false;
                break;
            default: // T_BLANK or T_ERROR
                elog_debug("Some Error in struct from getNextCell()");
                isnull[i] = true;
                break;
        }
    }
    tuple = heap_form_tuple(tDescFromNode, columns, isnull);
    ExecClearTuple(slot);
    elog_debug("Store tuple in TupleTableSlot");
    ExecStoreHeapTuple(tuple, slot, false);
    return slot;
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

/*
 * Fetches values from OPTIONS in Foreign Server and Table registration.
 */
static void
pg_sheet_fdwGetOptions(Oid foreigntableid, char **filepath, char **sheetname)
{
    elog_debug("%s",__func__);
    ForeignTable	*f_table;
    ForeignServer	*f_server;
    List		*options;
    ListCell	*lc;

    /*
     * Extract options from FDW objects.
     */
    f_table = GetForeignTable(foreigntableid);
    f_server = GetForeignServer(f_table->serverid);

    options = NIL;
    options = list_concat(options, f_table->options);
    options = list_concat(options, f_server->options);

    /* Loop through the options, and get the server/port */
    foreach(lc, options)
    {
        DefElem *def = (DefElem *) lfirst(lc);

        if (strcmp(def->defname, "filepath") == 0)
        {
            *filepath = defGetString(def);
            elog_debug("Got filepath with value: %s", *filepath);
        }

        if (strcmp(def->defname, "sheetname") == 0)
        {
            *sheetname = defGetString(def);
            elog_debug("Got sheetname with value: %s", *sheetname);
        }
    }
}
