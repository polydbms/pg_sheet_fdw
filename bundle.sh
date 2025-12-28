#!/bin/bash

set -e

echo "Creating PGXN bundle with submodules..."
echo "This will validate META.json and create a release zip file."
echo ""

# Step 1: Validate META.json using pgxn-bundle
echo "Validating META.json..."
VALIDATION_OUTPUT=$(docker run --rm \
  -v $(pwd):/repo \
  -w /repo \
  pgxn/pgxn-tools \
  pgxn-bundle 2>&1 || true)

if echo "$VALIDATION_OUTPUT" | grep -q "META.json is OK"; then
  echo "✓ META.json is valid"
else
  echo "✗ META.json validation failed"
  echo "$VALIDATION_OUTPUT"
  exit 1
fi
echo ""

# Step 2: Get version from META.json
VERSION=$(grep -oP '"version":\s*"\K[^"]+' META.json | head -n 1)
BUNDLE_NAME="pg_sheet_fdw-${VERSION}"
ZIP_FILE="${BUNDLE_NAME}.zip"

# Remove old bundle if it exists
rm -f "$ZIP_FILE"

echo "Creating bundle: ${ZIP_FILE}"
echo "Including submodule contents..."
echo ""

# Step 3: Create a temporary directory for the bundle
TEMP_DIR=$(mktemp -d)
trap "rm -rf $TEMP_DIR" EXIT

# Step 4: Archive the main repository
git archive --format=tar --prefix="${BUNDLE_NAME}/" HEAD | tar -x -C "$TEMP_DIR"

# Step 5: Archive each submodule and add to the bundle
git submodule foreach --quiet 'git archive --format=tar --prefix='"${BUNDLE_NAME}"'/$path/ HEAD | tar -x -C '"$TEMP_DIR"

# Step 6: Create the zip file
cd "$TEMP_DIR"
zip -r -q "$OLDPWD/$ZIP_FILE" "${BUNDLE_NAME}/"
cd "$OLDPWD"

echo "Bundle contents (first 50 entries):"
unzip -l "$ZIP_FILE" | head -n 50

echo ""
echo "Verifying submodule inclusion..."
SUBMODULE_FILES=$(unzip -l "$ZIP_FILE" | grep -c "submodules/sheetreader/src/" || echo "0")
if [ "$SUBMODULE_FILES" -gt 0 ]; then
  echo "✓ Submodule files included: $SUBMODULE_FILES files"
else
  echo "✗ Warning: No submodule files found in bundle"
fi

echo ""
echo "✓ Bundle created successfully!"
echo ""
echo "The bundle zip file is ready for upload to PGXN:"
ls -lh "$ZIP_FILE"
