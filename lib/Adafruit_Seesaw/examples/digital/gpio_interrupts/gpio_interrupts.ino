/*
 * This example shows how set GPIO interrupts on a seesaw.
 */

#include "Adafruit_seesaw.h"

//connect the interrupt pin on the seesaw (pin 8 on samd09 breakout) to this pin on your arduino
#define INT_PIN 3

//the interrupt will fire when this pin on the seesaw changes state 
#define SCAN_PIN 9

Adafruit_seesaw ss;

uint32_t mask = ((uint32_t)0b1 << SCAN_PIN);

void setup() {
  Serial.begin(115200);
  
  while (!Serial) delay(10);   // wait until serial port is opened
  
  if(!ss.begin()){
    Serial.println(F("seesaw not found!"));
    while(1) delay(10);
  }
  
  Serial.println(F("seesaw started OK!"));

  ss.pinModeBulk(mask, INPUT_PULLUP);
  ss.setGPIOInterrupts(mask, 1);
}

void loop() {
  if(!digitalRead(INT_PIN)){
    Serial.print(F("Interrupt fired! pin state: "));
    Serial.println(ss.digitalRead(SCAN_PIN));
  }
}
