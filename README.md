# PG_Sheet - A Postgres Foreign Data Wrapper for SheetReader

This Foreign Data Wrapper gives Postgresql access to SheetReader, which is a fast Excel sheet reader. It enables Postgresql to access local .xlsx files (Excel Sheets) as foreign tables. This code is tested with PostgreSQL Server 13, but probably works for newer versions too.

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Prerequisites

### Submodules

This repository depends on submodules! To pull the submodules, either clone this repository with the extra flag for submodules:
```
git clone --recurse-submodules <repository_url>
```
OR initialize the submodules afterward:
```
git submodule update --init
```

### Postgres Development Headers

This module needs access to Postgresql development code. A plain Postgresql installation is not sufficient! To install the needed dependency replace X with your version number and run:
```
sudo apt install postgresql-server-dev-X
```

## Installing on local Postgresql Server

Now to compile and install locally on Ubuntu, run:
```
make
make USE_PGXS=1 install
```
You eventually need elevated privileges.
These commands compile PG_Sheet and copie all relevant files into the respective folders of your local PostgreSQL Server installation.

## Development Scripts

This repository includes three bash scripts to help with development and testing:

### 1. `compile_In_Docker.sh`
Builds Docker images with PostgreSQL and pg_sheet_fdw installed, then runs tests.
- Creates Docker images for PostgreSQL 13 and 16
- Starts containers and runs test queries from `/test` directory
- Containers remain running for further inspection
- Useful for testing in isolated environments

```bash
./compile_In_Docker.sh
```

### 2. `local_pgxn_test.sh`
Runs PGXN-style tests locally using the official `pgxn/pgxn-tools` Docker image.
- Tests the extension on PostgreSQL 13
- Validates build and installation
- Runs regression tests
- Matches the GitHub Actions CI workflow

```bash
./local_pgxn_test.sh
```

### 3. `bundle.sh`
Creates a PGXN release bundle (zip file) for uploading to the official PGXN registry.
- Validates `META.json`
- Creates a release zip file (e.g., `pg_sheet_fdw-0.1.0.zip`)
- Excludes development files (CI/CD, Docker, etc.)
- Ready for upload to [PGXN Manager](https://manager.pgxn.org/)

```bash
./bundle.sh
```

> **Note**: Commit `.gitattributes` before running `bundle.sh` to ensure development files are properly excluded from the bundle.

## Test

In the /test directory are small Excel Sheets for testing. The script "/test/test_fdw_runall.sh" executes basic functioning tests on the local PostgreSQL Server. It calls `psql --echo-errors -v ON_ERROR_STOP=on -f ` on all sql test files. The command can be modified if local user credentials are needed. Also keep in mind, that the postgres user needs reading permission on all sheets.

## Usage

First, register the Foreign Data Wrapper as Extension and create a Server:
```
CREATE EXTENSION IF NOT EXISTS pg_sheet_fdw;
CREATE SERVER IF NOT EXISTS dummy FOREIGN DATA WRAPPER pg_sheet_fdw;
```
Second, create a Foreign Table on the registered Server. The table schema should match the datatypes in the Excel Sheet. In general, small deviations of datatypes are of no concern and are silently resolved. For example, using a smallint in the Foreign Table Schema but receiving a larger int from the Excel Sheet. In this case, the maximum smallint value is used.
Here is the Foreign Table of the string query test. The corresponding Server has to match the Server of the PG_Sheet Extension. Under the OPTIONS field, the Filepath and Sheetname of the Excel Sheet have to be supplied: 
```
CREATE FOREIGN TABLE IF NOT EXISTS randomTestTable(
    varchar1       varchar,
    varchar2       varchar,
    char       char
    ) SERVER dummy
    OPTIONS (filepath '/pg_sheet_fdw/test/string_test.xlsx', sheetname 'encoding');
```
The Extension can just be dropped if no longer needed:
```
DROP FOREIGN TABLE IF EXISTS randomTestTable;
DROP SERVER IF EXISTS dummy;
DROP EXTENSION IF EXISTS pg_sheet_fdw;
```
or
```
DROP EXTENSION IF EXISTS pg_sheet_fdw CASCADE;
```

### Options

| Name              | Description                                                                        |                     Default                     | Mandatory |
|:------------------|:-----------------------------------------------------------------------------------|:-----------------------------------------------:|:---------:|
| `filepath`        | Absolute path of the Excel file                                                    |                                                 |    yes    |
| `sheetname`       | Name of the Excel Sheet to read                                                    |               First Sheet in file               |    no     |
| `skiprows`        | Skips the first n rows. Useful for skipping header rows.                           |                        0                        |    no     |
| `numberofthreads` | Sets the number of Sheetreader worker threads. Does not influence the fdw threads. | Defaults to sane number based on current system |    no     |
| `batchsize`       | Sets the size of prefetch batches in the fdw.                                      |       Defaults to a size for 101 batches        |    no     |
