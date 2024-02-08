#!/bin/bash

# This script runs all sql tests in this directory.

# Loop through each SQL file in the directory
for file in ./*.sql; do
    # Extract the file name without extension
    filename=$(basename -- "$file")
    filename_no_ext="${filename%.*}"

    echo "Executing ${filename_no_ext}.sql..."

    # Run the SQL file using psql
    psql -f "${file}"

    # Check if psql command succeeded
    if [ $? -eq 0 ]; then
        echo "Successfully executed ${filename_no_ext}.sql"
    else
        echo "Error executing ${filename_no_ext}.sql"
        # Exit the loop if an error occurs
        break
    fi
done
