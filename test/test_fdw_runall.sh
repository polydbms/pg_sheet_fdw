#!/bin/bash

# This script runs all sql tests in this directory.
actual_output=output.txt

# Loop through each SQL file in the directory
for file in ./*.sql; do
    # Extract the file name without extension
    filename=$(basename -- "$file")
    filename_no_ext="${filename%.*}"

    echo "==========[ Executing ${filename_no_ext}.sql..."

    # Run the SQL file using psql
    psql --echo-errors -v ON_ERROR_STOP=on -f "${file}" > "$actual_output"
    expected_output="${filename_no_ext}_solution.txt"
    cat "${expected_output}"
    echo "==========[ /\ Expected | Actual \/"
    cat "${actual_output}"

    # Check if the output matches the expected output
    if diff -q "$actual_output" "$expected_output" > /dev/null; then
        echo "==========[ Successfully executed ${filename_no_ext}.sql"
    else
        echo "==========[ Error executing ${filename_no_ext}.sql"
        exit 1
    fi
done
