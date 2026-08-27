/*******************************************************************************
  Strandtest 7 Segment
  Noiasca HT16K33

  This is a strandtest to show if wiring of your HT16K33 display is correct

  by noiasca
  Sketch Version 2020-06-05
  
  On startup you should see some text on your display
  In the loop all printable characters will be shown.

 *******************************************************************************/
#include <Wire.h>                                                    // HT16K33 uses I2C, include the Wire Library
#include "NoiascaHt16k33.h"                                          // include the noiasca HT16K33 library - download from //http://werner.rothschopf.net/

//Noiasca_ht16k33_hw_7 display = Noiasca_ht16k33_hw_7();             // object for 7 segments, 8 digits
Noiasca_ht16k33_hw_7_4_c display = Noiasca_ht16k33_hw_7_4_c();       // object for 7 segments, 4 visbible digits, colon digit. E.g. Adafruit 0.56" 4-Digit 7-Segment Display w/I2C Backpack 
//Noiasca_ht16k33_hw_14 display = Noiasca_ht16k33_hw_14();           // object for 14 segments - 8 digits
//Noiasca_ht16k33_hw_14_4 display = Noiasca_ht16k33_hw_14_4();       // object for 14 segments - 4 digits

const uint8_t i2cAddress = 0x70;                                     // the I2C address of the first module
const uint8_t numOfDevices = 1;                                      // how many modules have you installed on the I2C Bus
const uint16_t wait = 500;                                           // wait milliseconds between demos


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println(F("\n01 strandtest 7segment"));
  Wire.begin();                                                      // start the I2C interface
  display.begin(i2cAddress, numOfDevices);                           // I2C address of first display, total number of devices

  // display.setColonDigit();                                        // activates a separate digit for a colon manually. But better to use the Noiasca_ht16k33_hw_7_4_c class if necessary.
  Serial.println(F("ON Test"));
  display.clear();                     // clear display
  display.print(F("ALFABETADELT"));    // if the text is to long or your display (chain) you will only see the last letters
  delay(wait);
}


void loop() {
  // put your main code here, to run repeatedly:
  Serial.println(F("Printable characters"));
  for (uint8_t counter = 32; counter < 128; counter++) {
    display.clear();
    display.print(counter);            // print the numerical ANSI code of the character
    display.write(counter);            // print the character for the ANSI code
    delay(wait);
  }
}