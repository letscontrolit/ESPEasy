/***************************************************

  12 Sevenseg
  
  This example is a variant of Adafruit Backpack "sevenseg" example to compare the two libraries

  original sketch
  Written by Limor Fried/Ladyada for Adafruit Industries.
  BSD license, all text above must be included in any redistribution

  this adopted Sketch
  written by noiasca

  Sktech Version 2020-06-06
  
 ****************************************************/
#include <Wire.h>                                                    // HT16K33 uses I2C, include the Wire Library
#include "NoiascaHt16k33.h"                                          // include the noiasca HT16K33 library - download from http://werner.rothschopf.net/
Noiasca_ht16k33_hw_7_4_c matrix = Noiasca_ht16k33_hw_7_4_c();        // object for 7 segments, 4 visbible digits, colon digit. e.g. Adafruit 0.56" 4-Digit 7-Segment Display w/I2C Backpack


void setup() {
#ifndef __AVR_ATtiny85__
  Serial.begin(115200);
  Serial.println("7 Segment Backpack Test");
#endif
  Wire.begin();                                                      // start the I2C interface
  matrix.begin(0x70);
}


void loop() {
  // try to print a number thats too long
  matrix.print(10000, DEC);
  delay(500);
  matrix.clear();

  // print a hex number
  matrix.print(0xBEEF, HEX);
  delay(500);
  matrix.clear();

  // print a floating point
  matrix.print(12.34);
  delay(500);
  matrix.clear();

  // print with print/println
  for (uint16_t counter = 0; counter < 9999; counter++) {
    matrix.println(counter);
    delay(10);
    matrix.setCursor(0); // as we are counting upwards, no need for matrix.clear()
  }

  // method #2 - draw each digit
  uint16_t blinkcounter = 0;
  boolean drawDots = false;

  for (uint16_t counter = 0; counter < 9999; counter ++) {
    matrix.print(counter / 1000);
    matrix.print((counter / 100) % 10);
    matrix.showColon(drawDots);
    matrix.print((counter / 10) % 10);
    matrix.print(counter % 10);

    blinkcounter += 50;
    if (blinkcounter < 500) {
      drawDots = false;
    } else if (blinkcounter < 1000) {
      drawDots = true;
    } else {
      blinkcounter = 0;
    }
    delay(10);
    matrix.setCursor(0);
  }
}
