/*
*
* Search for an EEPROM.
* This sketch iterate the eight possible I2C addresses and 
* checks if an EEPROM is found.
*
* Written by Christian Paul, 2014-11-24
* 
* 
*/

// include libraries
#include <Wire.h>

// setup
void setup() {
  Serial.begin(115200);
  Serial.println("AT24CX search");
  Serial.println("-------------------------");
  
  Wire.begin();
  int i2c = 0x50;
  for (int i=0; i<8; i++) {
    Serial.print("Search at [");
    Serial.print(i2c, HEX);
    Serial.print("]: ");
    Wire.beginTransmission(i2c);
    int result = Wire.endTransmission();
    if (result==0)
      Serial.println("FOUND!");
    else
      Serial.println("not found");
    i2c++;
  }

}

// main loop
void loop() {}

