-- Create fdw extension
CREATE EXTENSION IF NOT EXISTS pg_sheet_fdw;

-- Create dummy server (does not exist)
-- The options could be important for the SheetReader
CREATE SERVER IF NOT EXISTS dummy
    FOREIGN DATA WRAPPER pg_sheet_fdw;

-- No User mapping needed as its just a SheetReader

-- We need a schema for postgres, so it knows, what data to expect.
-- For that we create a Foreign table.
CREATE FOREIGN TABLE IF NOT EXISTS randomTestTable(
    smallint       smallint,
    integer       integer,
    bigint       bigint,
    real       real,
    double_precision       double precision,
    numeric       numeric
    ) SERVER dummy
    OPTIONS (filepath '/pg_sheet_fdw/test/numeric_test.xlsx', sheetname 'encoding');

-- Select everything
SELECT * FROM randomTestTable;

-- Drop anything
DROP FOREIGN TABLE IF EXISTS randomTestTable;
DROP SERVER IF EXISTS dummy;
DROP EXTENSION IF EXISTS pg_sheet_fdw;