#! /bin/bash
# This script starts the docker container, copies over all installation files and runs the compilation and installation.

# Specify the container and image name
CONTAINER_NAME="pg_sheet_fdw_test_environment"
IMAGE_NAME="pg_sheet_fdw"

echo "==========[ Updating Docker image $IMAGE_NAME."
cd docker && make && cd ..


# Check if the container is running, then delete it beforehand
if docker ps -q --filter "name=${CONTAINER_NAME}" | grep -q .; then
    echo "==========[ Container ${CONTAINER_NAME} is already running, stopping and deleting..."
    if docker stop ${CONTAINER_NAME}; then
                    echo "==========[ Container ${CONTAINER_NAME} stopped."
                    if docker container rm ${CONTAINER_NAME}; then
                                    echo "==========[ Container ${CONTAINER_NAME} deleted."
                                else
                                    echo "==========[ Failed to delete container ${CONTAINER_NAME}. Exiting..."
                                    exit 1
                                fi
                else
                    echo "==========[ Failed to stop container ${CONTAINER_NAME}. Exiting..."
                    exit 1
                fi
else
    if docker ps -a -q --filter "name=${CONTAINER_NAME}" | grep -q .; then
            # Container exists but is stopped, delete it
            if docker container rm ${CONTAINER_NAME}; then
                echo "==========[ Container ${CONTAINER_NAME} deleted."
            else
                echo "==========[ Failed to start container ${CONTAINER_NAME}. Exiting..."
                exit 1
            fi
    fi
fi

# now start new instance
if docker run -d --name ${CONTAINER_NAME} -v test-data:/data pg_sheet_fdw; then
    echo "==========[ Container ${CONTAINER_NAME} started."
else
    echo "==========[ Failed to start container ${CONTAINER_NAME}. Perhaps you haven't built the image yet? Exiting..."
    exit 1
fi

sleep 5

# Running tests
echo "==========[ Time for testing..."
docker exec -u 0 ${CONTAINER_NAME} bash -c 'chmod -R o+rwx /pg_sheet_fdw/test'
docker exec ${CONTAINER_NAME} bash -c 'cd /pg_sheet_fdw/test && ./test_fdw_runall.sh'

echo "==========[ Finished!"