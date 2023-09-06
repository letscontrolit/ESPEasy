#!/bin/bash

# Using https://github.com/ah01/h2md tool to generate documentation.

if [ ! -d "h2md/" ];
then
    git clone https://github.com/jnthas/h2md.git
    cd h2md
    npm install
    cd ..
fi

node h2md/h2md.js ../src/ImprovWiFiLibrary.h -o ImprovWiFiLibrary.md -p cpp -l -f
[ $? == 0 ] && echo "Done." 