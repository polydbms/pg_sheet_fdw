# Postgres Foreign Data Wrapper for SheetReader!

This Foreign Data Wrapper gives Postgresql access to SheetReader, which is a fast Excel sheet reader. For the used Postgresql version, take a look at the XDB Postgresql Docker Image.



## Installing on local Postgresql Server

To compile and install locally in Postgresql Server 13 on Ubuntu, just run the Makefile.

## Build Docker Image

To get a Docker Image with Postgresql Server 13 on Ubuntu and the Foreign Data Wrapper, head to the dockers directory and run make.
The resulting Image has the tag pg_sheet_fdw. The Docker Image is based on the XDB Postgresql Image.

## Docker Testing Environment

Running the compilecommand.sh builds the Docker Image, starts a container called pg_sheet_fdw_test_environment and performs tests on it.
Afterwards, the container is still running and can be further inspected and used for custom tests.

