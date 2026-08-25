/***************************************************

  Print Numbers aligned (without sprintf)

  This example shows how to print numbers right aligned without the RAM hungry sprintf

  The first function is a short introduction how to print with leading zeroes.
  The second function uses a similar concept but with trailing blanks.

  written by 
  noiasca

  Sktech Version 2020-06-06

 ****************************************************/
#include <Wire.h>                                                    // HT16K33 uses I2C, include the Wire Library
#include "NoiascaHt16k33.h"                                          // include the noiasca HT16K33 library - download from http://werner.rothschopf.net/

Noiasca_ht16k33_hw_7_4_c display = Noiasca_ht16k33_hw_7_4_c();       // object for 7 segments, 4 visbible digits, Colon digit. E.g. Adafruit 0.56" 4-Digit 7-Segment Display w/I2C Backpack
//Noiasca_ht16k33_hw_7 display = Noiasca_ht16k33_hw_7();             // object for 7 segments, 8 digits
//Noiasca_ht16k33_hw_14_4 display = Noiasca_ht16k33_hw_14_4();       // object for 14 segments - 4 digits

const uint16_t wait = 1000;                                          // wait milliseconds between demos

// Print a number Right aligned, position defines lowest digit. Pre-Zero
void printRightZeroedFrom0to3(int32_t value)
{
  display.print((value / 1000) % 10);
  display.print((value / 100) % 10);
  display.print((value / 10) % 10);
  display.print(value % 10);
}


// Print a number Right aligned
//    unused digits will be blanked
//    if the number doesn't fit into the given space (from startposition to endposition) display a filler
// Parameters: 
//    value = the number you want to print
//    startposition = first digit to be used
//    endposition = last digit to be used

void printRightBlank(int32_t value, uint8_t startposition, uint8_t endposition)
{
  uint8_t needed = neededDigits(value);
  uint8_t available = endposition - startposition + 1;
  display.setCursor(startposition);
  if (available - needed < 0)
  {
    Serial.print (value); Serial.println(F(" is to large to display on defined digits"));
    for (uint8_t i = 0; i < (available); i++)
    {
      display.print(F("o"));           // blank out the display
    }
  }
  else
  {
    display.setCursor(startposition);
    for (uint8_t i = 0; i < (available - needed); i++)
    {
      display.print(F(" "));           // print fillers on unused digits, deletes rests of old number
    }
    if (value < 0)                     // care about signed values
    {
      display.print(F("-"));
      value = value * -1;
    }
    display.print(value);
  }
}


// return the needed digits for a number
// Parameters: 
//    value = the number you want to analyze
// Return:
//    the digits of the given value
uint8_t neededDigits(int32_t value) {
  byte digit = 1;
  if (value >= 0) {
    value = -value;
    digit--;
  }
  switch (value) {
    case -2147483648 ... -1000000000:  digit++;
    case -999999999 ... -100000000:  digit++;
    case -99999999 ... -10000000:  digit++;
    case -9999999 ... -1000000:  digit++;
    case -999999 ... -100000:  digit++;
    case -99999 ... -10000:  digit++;
    case -9999 ... -1000:  digit++;
    case -999 ... -100:  digit++;
    case -99 ... -10:  digit++;
    case -9 ... 0:   digit++;
    default:  break;
  }
  return digit;
}


void setup() {
#ifndef __AVR_ATtiny85__
  Serial.begin(115200);
  Serial.println("Format Number Printing");
#endif
  Wire.begin();                                  // start the I2C interface
  Wire.setClock(400000);                         // optional: activate I2C fast mode. If it is to fast for other I2C devices. deactivate this row.
  display.begin(0x70);
  if (display.isConnected())                     // check if all HT16K33 are connected
    Serial.println(F("I: display connected"));
  else
    Serial.println(F("E: display error"));
  display.clear();
  printRightZeroedFrom0to3(123);                 // print a number right aligned using all digits from 0 to 3
  delay(wait);

  display.clear();
  printRightBlank(45, 1, 2);
  delay(wait);

  display.clear();
  printRightBlank(67, 0, 3);
  delay(wait);
}


void loop() {
  display.clear();
  for (int16_t counter = -110; counter < 1010; counter++) {
    printRightBlank(counter, 1, 3);              // print a number right aligned from digit 1 to 3
    delay(10);
  }
}
