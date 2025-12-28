# Docker Images for pg_sheet_fdw

This directory contains Docker configurations for building, testing, and verifying the pg_sheet_fdw extension across different PostgreSQL versions and installation methods.

## Dockerfiles

| File | PostgreSQL | Installation Method |
|------|------------|---------------------|
| `Dockerfile` | 13 | Build from local source |
| `Dockerfile.pg16` | 16 (recommended) | Build from local source |
| `Dockerfile.pgxn-test` | 16 | Install from PGXN registry |

**Note**: `Dockerfile.pgxn-test` tests real-world end-user installation via `pgxn install pg_sheet_fdw` from https://pgxn.org/dist/pg_sheet_fdw/

## Usage

### Using compile_In_Docker.sh

From the project root:

```bash
# Run all tests (PG13, PG16, and PGXN) - default
./compile_In_Docker.sh

# Run only PostgreSQL 13 tests (uses Dockerfile)
./compile_In_Docker.sh pg13

# Run only PostgreSQL 16 tests (uses Dockerfile.pg16)
./compile_In_Docker.sh pg16

# Run only PGXN installation test (uses Dockerfile.pgxn-test)
./compile_In_Docker.sh pgxn
```


The script builds Docker images, starts containers, runs the full test suite, and keeps containers running afterward.

### Using Makefile

```bash
cd docker
make
```

This builds the default Docker images (PG13 and PG16 from local source).
