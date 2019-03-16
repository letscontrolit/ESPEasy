/*
 *  Example of AS_BH1750 library usage.
 *  
 *  This example initalises the BH1750 object using the default
 *  adaptive auto-high-resolution mode and then makes 
 *  a light level reading about every second.
 *  (the sensitivity and resolution are automatically adapted 
 *  the current light conditions)
 *  
 *  Wiring:
 *  VCC-5v
 *  GND-GND
 *  SCL-SCL(analog pin 5)
 *  SDA-SDA(analog pin 4)
 *  ADD-NC or GND
 *
 *  Copyright (c) 2013 Alexander Schulz.  All right reserved.
 *  
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 * 
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <Wire.h>
#include <AS_BH1750.h>

AS_BH1750 sensor;

void setup(){
  Serial.begin(9600);
  delay(50);
  
  // for normal sensor resolution (1 lx resolution, 0-65535 lx, 120ms, no PowerDown) use: sensor.begin(RESOLUTION_NORMAL, false);
  // Initialize sensor. if sensor is not present, false is returned
  if(sensor.begin()) {
    Serial.println("Sensor initialized");
  } 
  else {
    Serial.println("Sensor not present");
  }
  
  /*
  // to check the sensor present
  if(sensor.isPresent()) {
    Serial.println("Sensor present");
  } 
  else {
    Serial.println("Sensor not present");
  }*/
}

void loop() {
  float lux = sensor.readLightLevel();
  Serial.print("Light level: ");
  Serial.print(lux);
  Serial.println(" lx");
  delay(1000);
}

