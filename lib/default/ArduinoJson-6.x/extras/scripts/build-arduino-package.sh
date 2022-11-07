#!/bin/bash

TAG=$(git describe)
OUTPUT="ArduinoJson-$TAG.zip"

cd $(dirname $0)/../../..

# remove existing file
rm -f $OUTPUT

# create zip
7z a $OUTPUT \
    -xr!.vs \
	ArduinoJson/CHANGELOG.md \
	ArduinoJson/examples \
	ArduinoJson/src \
	ArduinoJson/keywords.txt \
	ArduinoJson/library.properties \
	ArduinoJson/LICENSE.md \
	ArduinoJson/README.md \
	ArduinoJson/ArduinoJson.h
