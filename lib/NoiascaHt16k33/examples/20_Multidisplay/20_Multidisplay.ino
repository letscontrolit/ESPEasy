/*******************************************************************************
  Multidisplay
  Noiasca HT16K33

  You can easily handle more than one display in different configuration with one Library

  The example will handle:
  - One Adafruit 4 Digit Display
  - A chain of two alphanumerical displays with 4 digts each for a total of 8 digits.
  - One single 4 digit alphanumeric display

  Based on "Blink Without Delay" this sketch uses NO DELAYS in the loop(). The loop() runs non blocking code!

  by noiasca
  Sketch Version 2020-09-19

 *******************************************************************************/
#include <Wire.h>                                                    // HT16K33 uses I2C, include the Wire Library
#include "NoiascaHt16k33.h"                                          // include the noiasca HT16K33 library - download from http://werner.rothschopf.net/

Noiasca_ht16k33_hw_7_4_c display7 = Noiasca_ht16k33_hw_7_4_c();      // 7 Segment, 4 visbible digits, colon digit
Noiasca_ht16k33_hw_14_4 displayA = Noiasca_ht16k33_hw_14_4();        // 14 segments, 4 digits
Noiasca_ht16k33_hw_14_4 displayB = Noiasca_ht16k33_hw_14_4();        // 14 segments, 4 digits

// just a simple counter
void tick_animation7()
{
  const uint16_t intervall = 100;              // intervall of updates
  static uint32_t previousMillis;              // is static, because we need the value the next time when we call the function again, WITHOUT the need of a global variable
  static uint16_t counter;                     // static, as we want to keep the counter from to call without a global variable
  if (millis() - previousMillis > intervall)   // check, if it is time to do something
  {
    display7.setCursor(0);                     // goto position 0
    display7.print(counter);                   // print the counter
    counter++;                                 // increase coutner for the next call
    if (counter > 9999)
    { counter = 0;                             // care about the rollover
      display7.clear();                        // clear display
    }
    previousMillis = millis();                 // remember the current time
  }
}

// show messages on display
void tick_animationA()
{
  const uint16_t intervall = 1500;
  static uint32_t previousMillis;
  static uint16_t counter;
  //                       12345678    12345678    12345678    12345678    12345678
  const char *myText[] = {"Lets see", "what is ", "new to  ", " this   ", "Example.",
                          "You can ", "add more", "Messages", "in the  ", "Sketch. ",
                          "Just see", "how many", "  Bytes ", "RAM are ", "Free.   ",
                          "Update  ", " Modify ", "  or    ", " Delete ", "Messages",
                          "Have fun", "REGARDS ", " noiasca"                              // no comma after last message
                         };
  if (millis() - previousMillis > intervall)
  {
    const uint8_t messages = sizeof(myText) / sizeof(myText[0]); // this will count the definied messages, so we don't have to define the number of messages manually ;-)
    // Serial.print (counter); Serial.print(F(" ")); Serial.println(myText[counter]);
    displayA.clear();
    displayA.print(myText[counter]);
    displayA.write(counter);
    counter++;
    if (counter >= messages) counter = 0;
    previousMillis = millis();
  }
}

// show all printable characters
void tick_animationB()
{
  const uint16_t intervall = 700;
  static uint32_t previousMillis;
  static uint16_t counter = 32;
  if (millis() - previousMillis > intervall)
  {
    displayB.clear();
    displayB.print(counter);
    displayB.write(counter);
    counter++;
    if (counter > 127) counter = 32;
    previousMillis = millis();
  }
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println(F("\nMultidisplay: 3 independent Display consisting of 4 HT16K33"));
  Wire.begin();                        // start the I2C interface
  Wire.setClock(400000);               // optional: activate I2C fast mode. If it is to fast for other I2C devices. deactivate this row.
  display7.begin(0x70);                // I2C adress of first display, total number of devices
  displayA.begin(0x71, 2);             // I2C adress of first display, two devices, means, we are using 0x71 and 0x72 as "one" display
  displayB.begin(0x73);                // I2C adress of first display, total number of devices

  if (display7.isConnected() == false)                     // check if all HT16K33 are connected
    Serial.println(F("E: display7 error"));
  if (displayA.isConnected() == false)                     // check if all HT16K33 are connected
    Serial.println(F("E: displayA error"));
  if (display7.isConnected() == false)                     // check if all HT16K33 are connected
    Serial.println(F("E: displayB error"));

  Serial.println(F("ON Test"));
  display7.print(F("1.234"));
  displayA.print(F("2 Module"));
  displayB.print(F("ABCD"));
  delay(2000);

  display7.clear();
  displayA.clear();
  displayB.clear();
  Serial.println(F("now, each Display shows it's own animation..."));
}


void loop() {
  // put your main code here, to run repeatedly:
  tick_animation7();
  tick_animationA();
  tick_animationB();
}
