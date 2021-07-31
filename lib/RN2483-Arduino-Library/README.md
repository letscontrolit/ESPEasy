[![Build Status](https://travis-ci.org/jpmeijers/RN2483-Arduino-Library.svg?branch=master)](https://travis-ci.org/jpmeijers/RN2483-Arduino-Library)

# RN2483-Arduino-Library
Arduino C++ code to communicate with a Microchip RN2483 and RN2903 module

# Installation
At the top of this website (https://github.com/jpmeijers/RN2483-Arduino-Library), choose Clone or download -> Download ZIP.

Follow the instructions on https://www.arduino.cc/en/Guide/Libraries to import this library into your Arduino IDE.

# Examples
Examples with this library are meant to be used to contribute to TTN Mapper (http://ttnmapper.org).

# Software Serial
This library supports hardware and software serial for communication with the RN2xx3 module. Se the examples for clarification how to do this.

When using hardware serial for the RN2xx3, but software serial for a chatty device like a GPS module, it can happen that the communication with the RN2xx3 is unsuccessful. This is due to the hardware serial receive interrupts being paused during the reception of a software serial character. When using 9600 baud for the gps, and 57600 for the RN2xx3, this effect is even wors. A workaround for this situation is to pause the software serial reception when running any LoRa/radio commands. Use: `softwareSerial.end()` to pause the software serial and `softwareSerial.begin(9600)` to start it again.

# License
All code in this repository falls under the Apache v2.0 license, unless otherwise stated in the header of the respective file.

   Copyright 2016 JP Meijers

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
