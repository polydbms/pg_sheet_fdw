#!/bin/bash
set -e

echo "Running PGXN tests using pgxn/pgxn-tools image..."
echo ""

# Run pg-build-test directly using pgxn/pgxn-tools
docker run --rm --dns 8.8.8.8 \
  -v $(pwd):/repo \
  -w /repo \
  pgxn/pgxn-tools \
  bash -c "ln -s /repo /pg_sheet_fdw && pg-start 13 && pg-build-test"

echo ""
echo "✓ Tests completed successfully!"
