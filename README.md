# Postgres Foreign Data Wrapper for SheetReader!

This Foreign Data Wrapper gives Postgresql access to SheetReader, which is a fast Excel sheet reader. For the used Postgresql version, take a look at the XDB Postgresql Docker Image.

## Compilation and Installation

To compile and install locally in Postgresql Server 13 on Ubuntu, just run the Makefile.

To get a docker container with Postgresql 13 and the FDW, run the compilecommand.sh script. 
This script builds the image from ./docker/, starts it, copies over all relevant files and runs the installation there.
The Docker Container stays online afterwards and will be used in subsequent compilations and is ready to get inspected and used otherwise.

For now, the Docker Image being build does not contain the fdw! 
To get a container in a docker-compose with the fdw installed, do the following:
Run make inside the docker directory to build the base image. Include the image as a service in your docker-compose file. Postgres Server is installed, so you can set ports and so on.
Then start the docker-compose. Get the container name of your docker-compose service, that should get the fdw installed. Insert that name in the compilecommand.sh script for CONTAINER_NAME and run it.

## Docker Testing Environment

The Docker Image used for testing and developing is based on the XDB Postgresql Image. Additionally, G++ and GDB are installed.

