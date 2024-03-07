#! /bin/bash
# This script starts the docker container, copies over all installation files and runs the compilation and installation.

# Specify the container name
CONTAINER_NAME="pg_sheet_fdw_test_environment"

# Check if the container is running
if docker ps -q --filter "name=${CONTAINER_NAME}" | grep -q .; then
    echo "Container ${CONTAINER_NAME} is already running."
else
    if docker ps -a -q --filter "name=${CONTAINER_NAME}" | grep -q .; then
            # Container exists but is stopped, start it
            if docker start ${CONTAINER_NAME}; then
                echo "Container ${CONTAINER_NAME} started."
            else
                echo "Failed to start container ${CONTAINER_NAME}. Exiting..."
                exit 1
            fi
        else
            # Container doesn't exist, create and start a new one
            if docker run -d --name ${CONTAINER_NAME} pg_sheet_fdw; then
                echo "Container ${CONTAINER_NAME} started."
            else
                echo "Failed to start container ${CONTAINER_NAME}. Perhaps you haven't built the image yet? Exiting..."
                exit 1
            fi
        fi
fi

# Replace relevant files
echo "Replacing relevant files."
docker exec -u 0 ${CONTAINER_NAME} rm -rf /pg_sheet_fdw
docker exec -u 0 ${CONTAINER_NAME} mkdir /pg_sheet_fdw
docker cp pg_sheet_fdw.control ${CONTAINER_NAME}:/pg_sheet_fdw/
docker cp pg_sheet_fdw--0.1.sql ${CONTAINER_NAME}:/pg_sheet_fdw/
docker cp Makefile ${CONTAINER_NAME}:/pg_sheet_fdw/
docker cp ./src ${CONTAINER_NAME}:/pg_sheet_fdw/src
docker cp ./include ${CONTAINER_NAME}:/pg_sheet_fdw/include
docker cp ./submodules ${CONTAINER_NAME}:/pg_sheet_fdw/submodules
docker cp ./test ${CONTAINER_NAME}:/pg_sheet_fdw/test

# Run installation
echo "Installing pg_sheet_fdw."
docker exec -u 0 ${CONTAINER_NAME} bash -c 'cd /pg_sheet_fdw && make USE_PGXS=1 install'

# Running tests
echo "Testing time."
docker exec -u 0 ${CONTAINER_NAME} bash -c 'chmod -R +x /pg_sheet_fdw/test'
docker exec ${CONTAINER_NAME} bash -c 'cd /pg_sheet_fdw/test && ./test_fdw_runall.sh'

echo "Finished!"