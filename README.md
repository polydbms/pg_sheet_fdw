# Postgres Foreign Data Wrapper for SheetReader!

This Foreign Data Wrapper gives Postgresql access to SheetReader, which is a fast Excel sheet reader. For the used Postgresql version, take a look at the XDB Postgresql Docker Image.

## Compilation and Installation

To compile and install locally, just run the Makefile.

For developing and testing purposes, first build the Docker Image by running make in ./docker/, then run the compilecommand.sh script. 
This script uses the image from ./docker/, copies over all relevant files and runs the installation there.
The Docker Container stays online afterwards and will be used in subsequent compilations and is ready to get inspected.

## Docker Testing Environment

The Docker Image used for testing and developing is based on the XDB Postgresql Image. Additionally, G++ and GDB are installed.

