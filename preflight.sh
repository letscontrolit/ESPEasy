#!/bin/bash

# Check for documentation only changes.
# Source: https://github.com/dev-id/Magic-Spoiler/blob/8111a06ab6682e020169991d5e2aa4fa503d787f/preflight.sh

set -e
MAIN_BRANCH="mega"
MD=".md$"
DOCS="^docs/"

CHANGED_FILES=`git diff --name-only ${MAIN_BRANCH}...${TRAVIS_COMMIT}`
ONLY_READMES=True
DOCUMENTATION_CHANGED=False

for CHANGED_FILE in $CHANGED_FILES; do
  if ! [[ $CHANGED_FILE =~ $MD || $CHANGED_FILE =~ $DOCS ]]; then
    ONLY_READMES=False
#    break
  else
    DOCUMENTATION_CHANGED=True
  fi
done

#if [[ $ONLY_READMES == True ]]; then
#  echo "Only documentation files found, exiting."
#  travis_terminate 0
#  exit 1
#else
#  echo "Non-documentation files found, continuing with build."
#fi

if [[ $DOCUMENTATION_CHANGED == True ]]; then
  echo "Generate documentation."
  cd docs
  make html
fi
