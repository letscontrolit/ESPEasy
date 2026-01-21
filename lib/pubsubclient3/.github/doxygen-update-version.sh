#!/bin/bash
# Update the Doxyfile PROJECT_NUMBER with the version from library.properties
# Exit immediately if a command exits with a non-zero status.
set -e
# Make sure we are inside the github workspace
cd $GITHUB_WORKSPACE
version=`grep "version=" library.properties | cut -d "=" -f 2-`
echo "Update current version in Doxyfile: $version"
sed -i "s/PROJECT_NUMBER.*/PROJECT_NUMBER         = \"v$version\"/" Doxyfile
