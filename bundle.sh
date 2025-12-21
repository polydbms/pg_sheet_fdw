#!/bin/bash

echo "Creating PGXN bundle..."
echo "This will validate META.json and create a release zip file."
echo ""

# Run pgxn-bundle in Docker container
docker run --rm \
  -v $(pwd):/repo \
  -w /repo \
  pgxn/pgxn-tools \
  pgxn-bundle || true

echo ""

# Check if bundle was created
if [ -f pg_sheet_fdw-*.zip ] || [ -f pg_sheet_fdw-0.1.0.zip ]; then
  echo "✓ Bundle created successfully!"
  echo ""
  echo "The bundle zip file is ready for upload to PGXN:"
  ls -lh pg_sheet_fdw-*.zip 2>/dev/null
else
  echo "✗ Bundle creation failed. Check the errors above."
  exit 1
fi
