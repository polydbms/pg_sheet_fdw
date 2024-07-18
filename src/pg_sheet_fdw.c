
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
    elog_debug("[%s]",__func__);

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
    char *filepath;
    char *sheetname;
    unsigned long batchSize = 0;
    int numberOfThreads = -1;
    int skipRows = 0;
    pg_sheet_fdwGetOptions(foreigntableid, &filepath, &sheetname, &batchSize, &numberOfThreads, &skipRows);
    unsigned long rowCount = registerExcelFileAndSheetAsTable(filepath, sheetname, foreigntableid, numberOfThreads);
    elog_debug("[%s] Got row count: %lu", __func__, rowCount);
    baserel->rows = (double) rowCount;

    // set baseline batchsize to have 101 batches, if batchsize not set and at least a size of 1000.
    if(!batchSize){
        batchSize = rowCount/100;
        if(batchSize < 1000) batchSize = 1000;
        elog_debug("[%s] BatchSize not set, using base batchSize of: %lu", __func__, batchSize);
    }

    // store rowcount for later usage
    List *fdw_private;
    Datum rowCountDate = UInt64GetDatum(rowCount);
    Datum batchSizeDate = UInt64GetDatum(batchSize);
    Datum skipRowsDate = Int32GetDatum(skipRows);
    fdw_private = list_make1((void *) rowCountDate);
    fdw_private = lappend(fdw_private, (void *) batchSizeDate);
    fdw_private = lappend(fdw_private, (void *) skipRowsDate);
    baserel->fdw_private = fdw_private;
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
    elog_debug("[%s]",__func__);

    Cost startup_cost = 0;
    Cost total_cost = baserel->rows;
    List *fdw_private = baserel->fdw_private;
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
    elog_debug("[%s]", __func__);

    // Just copied!!
    Index scan_relid = baserel->relid;
    scan_clauses = extract_actual_clauses(scan_clauses, false);
    List* fdw_private = baserel->fdw_private;
    return make_foreignscan(tlist, scan_clauses, scan_relid, NIL, fdw_private, NIL, NIL, NULL);
}

Datum pg_sheet_fdwConvertSheetNumericToPG(struct PGExcelCell* cell, Oid expectedType){
    long data;
    switch(expectedType){
        case (20): // bigint
            return Int64GetDatum((long) cell->data.real);
        case (21): // smallint
            data = (long) cell->data.real;
            if(data > 32767) data = 32767;
            else if(data < -32768) data = -32768;
            return Int16GetDatum(data);
        case (23): // integer
            data = (long) cell->data.real;
            if(data > 2147483647) data = 2147483647;
            else if(data < -2147483648) data = -2147483648;
            return Int32GetDatum(data);
        case (700): // real
            return Float4GetDatum((float4)cell->data.real);
        case (701): // double precision
            return Float8GetDatum((float8)cell->data.real);
        case (1700): // numeric/decimal
            return DirectFunctionCall1(float8_numeric, Float8GetDatum((float8)cell->data.real));
        default:
            ereport(ERROR,
                    (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
                            errmsg("[%s] unsupported data type!", __func__ )));
            return 0;
    }
}

// returns 0 if no new rows prefetched, 1 if at least one row prefetched
int pg_sheet_fdwPrefetchRows(pg_sheet_scanstate* state){
    // switch to memory context
    MemoryContext oldContext = MemoryContextSwitchTo(state->context);

    // debug print the state
    //elog_debug("[%s] state debug id: %u, column count: %u, rows left: %lu, rows prefetched: %lu, rows read: %lu, next batchIndex: %lu", __func__, state->tableID, state->columnCount, state->rowsLeft, state->rowsRead, state->rowsPrefetched, state->batchIndex );
    unsigned long prefetchCount = state->batchSize;
    if(state->rowsLeft < state->batchSize) prefetchCount = state->rowsLeft;
    //elog_debug("[%s] Pallocating %lu Bytes memory for %lu rows.", __func__, sizeof(Datum) * prefetchCount * state->columnCount, prefetchCount);
    // allocate new memory in the scan state at the next batch pointer.
    state->isnull[state->batchIndex] = (bool*) palloc(sizeof(bool) * prefetchCount * state->columnCount);
    state->cells[state->batchIndex] = (Datum*) palloc(sizeof(Datum) * prefetchCount * state->columnCount);

    // fetch amount of rows, check the received datatypes
    unsigned long rowsPrefetched = 0;
    for(; rowsPrefetched < prefetchCount; rowsPrefetched++){
        unsigned long columnCount = startNextRow(state->tableID);
        // no more rows to read? finish
        if(columnCount == 0) break;
        // columnCount valid?
        if(columnCount != state->columnCount){
            //elog_debug("[%s] invalid column count in sheet cell!", __func__);
            return 0;
        }
        // fill tuple with cells
        char *c;
        unsigned long currentRow = rowsPrefetched * state->columnCount;
        for(unsigned long i = 0; i < columnCount; i++){
            struct PGExcelCell* cell = getNextCellCast(state->tableID);
            Oid expectedType = state->expectedTypes[i];
            switch(expectedType){
                case (16):
                    if(cell->type != T_BOOLEAN) {
                        elog_debug("[%s] Mismatching type!", __func__);
                        state->isnull[state->batchIndex][currentRow+i] = true;
                        break;
                    }
//                    elog_debug("[%s] Cell %lu with boolean: %d", __func__, i, cell->data.boolean);
                    state->cells[state->batchIndex][currentRow+i] = BoolGetDatum(cell->data.boolean);
                    state->isnull[state->batchIndex][currentRow+i] = false;
                    break;
                case (20):
                case (21):
                case (23):
                case (700):
                case (701):
                case (1700):
                    if(cell->type != T_NUMERIC) {
                        elog_debug("[%s] Mismatching type!", __func__);
                        state->isnull[state->batchIndex][currentRow+i] = true;
                        break;
                    }
//                    elog_debug("[%s] Cell %lu with number: %f", __func__, i, cell->data.real);
                    state->cells[state->batchIndex][currentRow+i] = pg_sheet_fdwConvertSheetNumericToPG(cell, expectedType);
                    state->isnull[state->batchIndex][currentRow+i] = false;
                    break;
                case (1082): // date
                    if(cell->type != T_DATE) {
                        elog_debug("[%s] Mismatching type!", __func__);
                        state->isnull[state->batchIndex][currentRow+i] = true;
                        break;
                    }
//                    elog_debug("[%s] Cell %lu with date", __func__, i);
                    state->cells[state->batchIndex][currentRow+i] = DateADTGetDatum(DirectFunctionCall1(timestamp_date,(cell->data.real-946684800) * 1000000));
                    state->isnull[state->batchIndex][currentRow+i] = false;
                    break;
                case (1114): // timestamp
                    if(cell->type != T_DATE) {
                        elog_debug("[%s] Mismatching type!", __func__);
                        state->isnull[state->batchIndex][currentRow+i] = true;
                        break;
                    }
//                    elog_debug("[%s] Cell %lu with timestamp", __func__, i);
                    state->cells[state->batchIndex][currentRow+i] = TimeADTGetDatum((cell->data.real-946684800) * 1000000); // convert unix (seconds since 1970) to pg timestamp (microseconds since 2000)
                    state->isnull[state->batchIndex][currentRow+i] = false;
                    break;
                case (18):
                case (25):
                case (1042):
                case (1043):
                    if(cell->type == T_STRING || cell->type == T_STRING_INLINE){
                        //elog_debug("[%s] Got static string!", __func__);
                        c = readDynamicString(state->tableID, cell->data.stringIndex);
                    } else if(cell->type == T_STRING_REF) {
                        //elog_debug("[%s] Got dynamic string!", __func__);
                        c = readStaticString(state->tableID, cell->data.stringIndex);
                    } else {
                        elog_debug("[%s] Mismatching type!", __func__);
                        state->isnull[state->batchIndex][currentRow+i] = true;
                        break;
                    }
                    //elog_debug("[%s] Row %lu Cell %lu with string: %s with length: %lu", __func__, rowsPrefetched, i, c, strlen(c));
                    state->cells[state->batchIndex][currentRow+i] = CStringGetTextDatum(c);
                    state->isnull[state->batchIndex][currentRow+i] = false;
                    //elog_debug("[%s] Freeing string that got converted to a Datum!", __func__);
                    free(c);
                    break;
                default:
                    elog_debug("[%s] Unsupported datatype in Postgresql foreign table definition!", __func__);
            }
        }
    }
    //elog_debug("[%s] Fetched %lu rows", __func__, rowsPrefetched);

    // switch memory context back
    MemoryContextSwitchTo(oldContext);

    // set prefetch counter to amount and read counter to 0
    if(rowsPrefetched == 0) return 0;
    state->batchIndex = state->batchIndex +1;
    state->rowsRead = 0;
    state->rowsPrefetched = rowsPrefetched;
    state->rowsLeft = state->rowsLeft - rowsPrefetched;
    return 1;
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

    // build scan state and store memory context
    pg_sheet_scanstate *state = (pg_sheet_scanstate *) palloc(sizeof(pg_sheet_scanstate));

    // set memory context for prefetch allocations
    MemoryContext scanContext = AllocSetContextCreate(CurrentMemoryContext, "pg_sheet_allocationContext", ALLOCSET_DEFAULT_SIZES);
    MemoryContext oldContext = MemoryContextSwitchTo(scanContext);

    state->context = scanContext;

    // get total row count
    List* fdw_private = ((ForeignScan *)node->ss.ps.plan)->fdw_private;
    Datum rowCount = (Datum) linitial(fdw_private);
    Datum batchSize = (Datum) list_nth(fdw_private, 1);
    int skipRows = DatumGetInt32((Datum) list_nth(fdw_private, 2));
    state->rowsLeft = DatumGetUInt64(rowCount);
    state->batchSize = DatumGetUInt64(batchSize);

    elog_debug("[%s] Prepare scan for %lu rows", __func__, state->rowsLeft);

    // get table id (used for unique identifier in the parserinterface)
    state->tableID = node->ss.ss_currentRelation->rd_id;
    elog_debug("[%s] Foreign table Oid: %u",__func__, state->tableID);

    for(;skipRows>0;skipRows--){
        startNextRow(state->tableID);
    }

    // read number of columns and column types
    TupleDesc td = RelationGetDescr(node->ss.ss_currentRelation);
    state->columnCount = td->natts;
    elog_debug("[%s] Detected number of columns: %d",__func__, state->columnCount);
    state->expectedTypes = (Oid *) palloc(sizeof(Oid) * state->columnCount);
    for(int i=0; i < state->columnCount; i++){
        Form_pg_attribute attr = TupleDescAttr(td, i);
        state->expectedTypes[i] = attr->atttypid;
    }

    // pallocate memory for all the batch pointers
    unsigned long batchCount = (state->rowsLeft / state->batchSize +1);
    elog_debug("[%s] Pallocating %lu bytes for %lu batch pointers.", __func__ , batchCount  * sizeof(Datum*), batchCount);
    state->isnull = (bool**) palloc(batchCount  * sizeof(bool*));
    state->cells = (Datum**) palloc(batchCount  * sizeof(Datum*));

    // set counters
    state->rowsPrefetched = 0;
    state->rowsRead = 0;
    state->batchIndex = 0;

    // prefetch first batch
    elog_debug("[%s] Calling Prefetch function",__func__);
    int t = pg_sheet_fdwPrefetchRows(state);

    // store scan state pointer
    node->fdw_state = (void*) state;

    // switch back the memory context
    MemoryContextSwitchTo(oldContext);
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

    pg_sheet_scanstate *state = node->fdw_state;

    // need more rows prefetched?
    if(state->rowsRead >= state->rowsPrefetched){
        //elog_debug("[%s] No more rows left. Calling Prefetch function", __func__ );
        int t = pg_sheet_fdwPrefetchRows(state);
        if(!t){
            return NULL;
        }
    }

    // build tuple from already fetched rows and increment read counter
//    elog_debug("[%s] Storing tuple in TupleTableSlot from batch %lu at index %lu", __func__, state->batchIndex-1, state->rowsRead );
    TupleTableSlot *slot = node->ss.ss_ScanTupleSlot;
    ExecClearTuple(slot);
    // virtual tuple
    slot->tts_values = &(state->cells[state->batchIndex-1][state->rowsRead * state->columnCount]);
    slot->tts_isnull = &(state->isnull[state->batchIndex-1][state->rowsRead * state->columnCount]);
    ExecStoreVirtualTuple(slot);
    // heap tuple
//    Datum* values = &(state->cells[state->batchIndex-1][state->rowsRead * state->columnCount]);
//    bool* isnull = &(state->isnull[state->batchIndex-1][state->rowsRead * state->columnCount]);
//    HeapTuple tuple = heap_form_tuple(slot->tts_tupleDescriptor, values, isnull);
//    ExecStoreHeapTuple(tuple, slot, 0);

    state->rowsRead++;
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
 *
 * we release our palloced memory from our own memory context. it is probably hierarchically under the context
 * of the whole foreign table scan and will be released with it. But we can already release it here.
 */
void pg_sheet_fdwEndForeignScan(ForeignScanState *node){
    elog_debug("[%s] Dropping sheet in ParserInterface", __func__);
    pg_sheet_scanstate *state = node->fdw_state;
    dropTable(state->tableID);
    MemoryContextDelete(state->context);
}

/*
 * Read an options value and convert it to long. Throw error if it can't.
 */
int64
GetInt64Option(DefElem *def)
{
    int64 result;
    char *str_val = defGetString(def);

    if (!scanint8(str_val, true, &result))
        ereport(ERROR,
                (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
                        errmsg("invalid value for option \"%s\": \"%s\"",
                               def->defname, str_val)));

    return result;
}


/*
 * Fetches values from OPTIONS in Foreign Server and Table registration.
 */
static void
pg_sheet_fdwGetOptions(Oid foreigntableid, char **filepath, char **sheetname, unsigned long* batchSize, int* numberOfThreads, int* skipRows)
{
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

    bool foundSheet = false;
    bool foundPath = false;

    /* Loop through the options, and get the server/port */
    foreach(lc, options)
    {
        DefElem *def = (DefElem *) lfirst(lc);

        if (strcmp(def->defname, "filepath") == 0)
        {
            *filepath = defGetString(def);
            foundPath = true;
            elog_debug("[%s] Got filepath with value: %s", __func__, *filepath);
        }

        if (strcmp(def->defname, "sheetname") == 0)
        {
            *sheetname = defGetString(def);
            foundSheet = true;
            elog_debug("[%s] Got sheetname with value: %s", __func__, *sheetname);
        }

        if (strcmp(def->defname, "batchsize") == 0)
        {
            long custombatchsize = GetInt64Option(def);
            if(custombatchsize > 0) *batchSize = custombatchsize;
            elog_debug("[%s] Got batchsize with value: %lu", __func__, *batchSize);
        }

        if (strcmp(def->defname, "numberofthreads") == 0)
        {
            long customnumberofthreads = GetInt64Option(def);
            if(customnumberofthreads > 0 && customnumberofthreads <= 10) {
                *numberOfThreads = customnumberofthreads;
                elog_debug("[%s] Got number of threads with value: %d", __func__, *numberOfThreads);
            }
        }

        if (strcmp(def->defname, "skiprows") == 0){
            long customSkipRows = GetInt64Option(def);
            if(customSkipRows > 0 && customSkipRows <= 2147483648) {
                *skipRows = customSkipRows;
                elog_debug("[%s] Skipping %d rows", __func__, *skipRows);
            }
        }
    }

    if(!foundPath) ereport(ERROR,
                           (errcode(ERRCODE_FDW_OPTION_NAME_NOT_FOUND),
                                   errmsg("missing mandatory option \"filepath\"" )));
    if(!foundSheet){
        elog_debug("[%s] Found no sheetname, using default \"\"!", __func__, *sheetname);
        *sheetname = palloc(sizeof(char));
        const char *empty = "";
        strcpy(*sheetname, empty);
    }
}
