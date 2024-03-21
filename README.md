# Postgres Foreign Data Wrapper for SheetReader!

This Foreign Data Wrapper gives Postgresql access to SheetReader, which is a fast Excel sheet reader. For the used Postgresql version, take a look at the XDB Postgresql Docker Image.

## Compilation and Installation

To compile and install locally, just run the Makefile. It got tested with Postgresql Server version 13.

For developing and testing purposes, or to get a docker container with Postgresql 13 and the FDW, run the compilecommand.sh script. 
This script builds the image from ./docker/, starts it, copies over all relevant files and runs the installation there.
The Docker Container stays online afterwards and will be used in subsequent compilations and is ready to get inspected and used otherwise.

For now, the Docker Image being build is does not contain the fdw, but it is installed in a container instance afterwards. 
To have a container in a docker-compose with the fdw, one can match a service container name with the container name specified in the compilecommand.
After starting the compose, the compilescript will notice the container name and use the already running container from the compose file.

## Docker Testing Environment

The Docker Image used for testing and developing is based on the XDB Postgresql Image. Additionally, G++ and GDB are installed.

