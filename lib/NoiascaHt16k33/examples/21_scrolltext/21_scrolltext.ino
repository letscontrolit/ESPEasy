/*******************************************************************************
  Scrolltext
  Noiasca HT16K33

  The extended class can scroll text (from left to right)

  The example will handle:
  - A chain of three alphanumerical displays with 4 digts each for a total of 12 digits.
  
  The loop() mut run without blocking code, otherwise the scrolling text will be blcoked also
  
  by noiasca
  Sketch Version 2022-02-28

 *******************************************************************************/

#include <Wire.h>                                                    // HT16K33 uses I2C, include the Wire Library
#include "NoiascaHt16k33.h"                                          // include the noiasca HT16K33 library - download from http://werner.rothschopf.net/

Noiasca_ht16k33_hw_14_ext display = Noiasca_ht16k33_hw_14_ext();     // 14 segment, extended class with scroll support

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println(F("\nScroll a text"));
  Wire.begin();                                  // start the I2C interface
  //Wire.setClock(400000);                       // optional: activate I2C fast mode. If it is to fast for other I2C devices deactivate this row.
  display.begin(0x70, 3);                        // I2C adress of first display, in total we use 3 displays
  if (display.isConnected() == false)            // check if all HT16K33 are connected
    Serial.println(F("E: display error"));
  display.setDigits(4);                          // if your modules are not using all 8 digits, reduce the used digits
  Serial.println(F("ON Test"));
  display.print(F("3 Modules on"));              // just some "on" test
  delay(2000);

  Serial.println(F("now, each display shows it's own animation..."));
  display.setScrolltext("THIS IS OUR FIRST SCROLLTEXT. LOOKS NICE. noiasca");  // this text will be scrolled
  
  // more examples how to scroll content:
  // display.setScrolltext("SHORTY");            // scroll text can be very short also
  // display.setScrolltext(1234567890);          // scroll a (long) number
  // display.setScrolltext(123.456);             // floats will be fixed to 2 decimals.

  // display.setScrollInterval(300);             // sets the scroll speed
  // display.setScrollWait(5000);                // sets the wait/timeout after all letters are scrolled to the display
}


void loop() {
  // put your main code here, to run repeatedly:
  display.scroll();          //needs to be called for updating the scroll text
}
