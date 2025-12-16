#!/bin/bash
# This script starts the docker container, copies over all installation files and runs the compilation and installation.

run_test_env() {
    local PG_VERSION=$1
    local DOCKERFILE=$2
    local CONTAINER_NAME="pg_sheet_fdw_test_environment"
    local IMAGE_NAME="pg_sheet_fdw"

    if [ "$PG_VERSION" == "16" ]; then
        CONTAINER_NAME="${CONTAINER_NAME}_16"
        IMAGE_NAME="${IMAGE_NAME}_16"
    fi

    echo "==========[ Processing PostgreSQL $PG_VERSION Environment ]=========="
    echo "==========[ Updating Docker image $IMAGE_NAME using $DOCKERFILE."
    
    # Build image
    docker build -t $IMAGE_NAME -f docker/$DOCKERFILE .

    # Check if the container is running, then delete it beforehand
    if docker ps -q --filter "name=${CONTAINER_NAME}" | grep -q .; then
        echo "==========[ Container ${CONTAINER_NAME} is already running, stopping and deleting..."
        docker stop ${CONTAINER_NAME} > /dev/null
        docker container rm ${CONTAINER_NAME} > /dev/null
        echo "==========[ Container ${CONTAINER_NAME} deleted."
    else
        if docker ps -a -q --filter "name=${CONTAINER_NAME}" | grep -q .; then
            # Container exists but is stopped, delete it
            docker container rm ${CONTAINER_NAME} > /dev/null
            echo "==========[ Container ${CONTAINER_NAME} deleted."
        fi
    fi

    # now start new instance
    if docker run -d --name ${CONTAINER_NAME} -v test-data:/data $IMAGE_NAME; then
        echo "==========[ Container ${CONTAINER_NAME} started."
    else
        echo "==========[ Failed to start container ${CONTAINER_NAME}. Exiting..."
        exit 1
    fi

    sleep 5

    # Running tests
    echo "==========[ Time for testing PG$PG_VERSION..."
    docker exec -u 0 ${CONTAINER_NAME} bash -c 'chmod -R o+rwx /pg_sheet_fdw/test'
    docker exec ${CONTAINER_NAME} bash -c 'cd /pg_sheet_fdw/test && ./test_fdw_runall.sh'
    
    echo "==========[ Finished PG$PG_VERSION!"
    echo ""
}

# Run for PG13 (Default)
run_test_env "13" "Dockerfile"

# Run for PG16
run_test_env "16" "Dockerfile.pg16"