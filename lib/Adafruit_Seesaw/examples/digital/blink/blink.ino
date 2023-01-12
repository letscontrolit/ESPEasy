/*
 * This example shows how to blink a pin on a seesaw.
 * Attach the positive (longer lead) of the LED to pin 15 on the seesaw, and
 * the negative lead of the LED to ground through a 1k ohm resistor.
 */

#include "Adafruit_seesaw.h"

Adafruit_seesaw ss;

#define BLINK_PIN 15

void setup() {
  Serial.begin(115200);
  
  while (!Serial) delay(10);   // wait until serial port is opened
  
  if(!ss.begin()){
    Serial.println("seesaw not found!");
    while(1) delay(10);
  }
  
  Serial.println(F("seesaw started OK!"));

  ss.pinMode(BLINK_PIN, OUTPUT);
}

void loop() {
  ss.digitalWrite(BLINK_PIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(100);                       // wait for a second
  ss.digitalWrite(BLINK_PIN, LOW);    // turn the LED off by making the voltage LOW
  delay(100);  
}
