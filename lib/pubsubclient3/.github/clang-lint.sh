#!/bin/bash
# Exit immediately if a command exits with a non-zero status.
set -e
# Enable the globstar shell option
shopt -s globstar
# Make sure we are inside the github workspace
cd $GITHUB_WORKSPACE
# Install clang-format (if not already installed)
#command -v clang-format >/dev/null 2>&1 || sudo apt-get -y install clang-format
# need Ubuntu clang-format version 19.1.1 (1ubuntu1~24.04.2)
# default Ubuntu clang-format version 18.1.3 (1ubuntu1) is not working
sudo apt-get -y install clang-format-19
# Check clang-format output
for f in **/*.{h,c,hpp,cpp,ino} ; do
    #if [ -f "$f" ] && [[ "$f" != "tests/"* ]]; then
    if [ -f "$f" ]; then
        echo "################################################################"
        echo "Checking file ${f}"
        diff $f <(clang-format-19 -assume-filename=main.cpp $f) 1>&2
    fi
done
