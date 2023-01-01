/*
 * This example shows how read the ADC on a seesaw. 
 * The default ADC pins on the SAMD09 Breakout are 2, 3, and 4.
 */

#include "Adafruit_seesaw.h"

Adafruit_seesaw ss;
// on SAMD09, analog in can be 2, 3, or 4
// on Attiny8x7, analog in can be 0-3, 6, 7, 18-20
#define   ANALOGIN   2

void setup() {
  Serial.begin(115200);
  
  while (!Serial) delay(10);   // wait until serial port is opened
  
  if(!ss.begin()){
    Serial.println(F("seesaw not found!"));
    while(1) delay(10);
  }
  
  Serial.println(F("seesaw started OK!"));
}

void loop() {
  Serial.println(ss.analogRead(ANALOGIN));
  delay(50);  
}
