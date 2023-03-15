/*
 * This example shows how to blink a pin on a seesaw.
 * It is written to use the built-in LED on the ATtiny817 breakout with seesaw.
 */

#include "Adafruit_seesaw.h"

Adafruit_seesaw ss;

#define BLINK_PIN 5

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
  ss.digitalWrite(BLINK_PIN, LOW);   // turn the LED on (the LED is tied low)
  delay(1000);                       // wait for a second
  ss.digitalWrite(BLINK_PIN, HIGH);    // turn the LED off
  delay(1000);  
}
