#!/bin/bash
# Exit immediately if a command exits with a non-zero status.
set -e
# Enable the globstar shell option
shopt -s globstar
# Make sure we are inside the github workspace
cd $GITHUB_WORKSPACE
# Create directories
mkdir $HOME/Arduino
mkdir $HOME/Arduino/libraries
# Install Arduino IDE
export PATH=$PATH:$GITHUB_WORKSPACE/bin
curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh
arduino-cli config init
arduino-cli config add board_manager.additional_urls http://arduino.esp8266.com/stable/package_esp8266com_index.json
arduino-cli config set library.enable_unsafe_install true
arduino-cli core update-index
#arduino-cli core search
# Install Arduino cores
arduino-cli core install arduino:avr
arduino-cli core install esp8266:esp8266
arduino-cli core install esp32:esp32
#arduino-cli board listall
# Install Arduino libs
arduino-cli lib install Ethernet
arduino-cli lib install --git-url https://github.com/ennui2342/arduino-sram
# Link the project to the Arduino library
ln -s $GITHUB_WORKSPACE $HOME/Arduino/libraries/CI_Test_Library

# Compile all *.ino files for the Arduino
for f in **/*.ino ; do
    if [[ "$f" != *mqtt_esp8266.ino && "$f" != *mqtt_large_message.ino ]]; then
        echo "################################################################"
        echo "Arduino Uno compiling file ${f}"
        arduino-cli compile -b arduino:avr:uno $f
    fi
done

# Compile all *.ino files for the ESP32
for f in **/*.ino ; do
    if [[ "$f" != *"mqtt_stream.ino" ]]; then
        echo "################################################################"
        echo "ESP32 compiling file ${f}"
        arduino-cli compile -b esp32:esp32:esp32 $f
    fi
done

# Compile all *.ino files for the ESP8266
for f in **/*.ino ; do
    echo "################################################################"
    echo "ESP8266 compiling file ${f}"
    arduino-cli compile -b esp8266:esp8266:generic $f
done
