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
    date       date,
    timestamp       timestamp,
    boolean       boolean
    ) SERVER dummy
    OPTIONS (filepath '/pg_sheet_fdw/test/datebool_test.xlsx', sheetname 'encoding', batchsize '5', numberofthreads '1');

-- Select everything
SELECT * FROM randomTestTable;

-- Drop anything
DROP FOREIGN TABLE IF EXISTS randomTestTable;
DROP SERVER IF EXISTS dummy;
DROP EXTENSION IF EXISTS pg_sheet_fdw;