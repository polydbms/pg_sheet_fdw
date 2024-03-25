#!/bin/bash

# This script runs all sql tests in this directory.

# Loop through each SQL file in the directory
for file in ./*.sql; do
    # Extract the file name without extension
    filename=$(basename -- "$file")
    filename_no_ext="${filename%.*}"

    echo "==========[ Executing ${filename_no_ext}.sql..."

    # Run the SQL file using psql
    psql --echo-errors -v ON_ERROR_STOP=on -f "${file}"

    # Check if there are any error messages in the output
    if [ $? -eq 0 ]; then
        echo "==========[ Successfully executed ${filename_no_ext}.sql"
    else
        echo "==========[ Error executing ${filename_no_ext}.sql"
        exit 1
        break
    fi
done
