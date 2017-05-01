/**
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    Copyright 2016 Ratthanan Nalintasnai
**/

// Include library into the sketch
#include <AM2320.h>

// Create an instance of sensor
AM2320 sensor;

void setup() {
  // enable serial communication
  Serial.begin(115200);
  // call sensor.begin() to initialize the library
  sensor.begin();
}

void loop() {

  // sensor.measure() returns boolean value
  // - true indicates measurement is completed and success
  // - false indicates that either sensor is not ready or crc validation failed
  //   use getErrorCode() to check for cause of error.
  if (sensor.measure()) {
    Serial.print("Temperature: ");
    Serial.println(sensor.getTemperature());
    Serial.print("Humidity: ");
    Serial.println(sensor.getHumidity());
  }
  else {  // error has occured
    int errorCode = sensor.getErrorCode();
    switch (errorCode) {
      case 1: Serial.println("ERR: Sensor is offline"); break;
      case 2: Serial.println("ERR: CRC validation failed."); break;
    }    
  }

  delay(500);
}