/*******************************************************************************
  Development tests
  Noiasca HT16K33

  This is a sketch I'm using during development and regression testing

  by noiasca
  Sketch Version 2020-06-05

 *******************************************************************************/
#include <Wire.h>                                                    // HT16K33 uses I2C, include the Wire Library
#include "NoiascaHt16k33.h"                                          // include the noiasca HT16K33 library - download from http://werner.rothschopf.net/

Noiasca_ht16k33_hw_7 display = Noiasca_ht16k33_hw_7();               // 7 segments, 8 digits
//Noiasca_ht16k33_hw_7_4_c display = Noiasca_ht16k33_hw_7_4_c();    // 7 Segment, 4 visbible digits, colon digit
//Noiasca_ht16k33_hw_14 display = Noiasca_ht16k33_hw_14();           // object for 14 segments - 8 digits
//Noiasca_ht16k33_hw_14_4 display = Noiasca_ht16k33_hw_14_4();         // object for 14 segments - 4 digits
//Noiasca_ht16k33_hw_14_ext display = Noiasca_ht16k33_hw_14_ext();         // object for 14 segments - 4 digits

const uint16_t wait = 500;                                           // wait milliseconds between demos

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println(F("\n90 development tests"));
  Wire.begin();                                                      // start the I2C interface
  Wire.setClock(400000);                         // optional: activate I2C fast mode. If it is to fast for other I2C devices. deactivate this row.

  /*
    display7.begin(0x70);
    display7.clear();
    display7.print(F("1.234"));
    //display7.showColon(1);            // show colon on 7 segment display
    //display7.blinkRate(HT16K33_BLINK_HALFHZ);
    delay(wait);
    display7.clear();
    display7.print(F("8888"));
  */

  display.begin(0x71);                 // I2C address of first display, optional total number of devices
  display.isConnected() ? Serial.println(F("I: display connected")) : Serial.println(F("E: display error"));

  Serial.println(F("ON Test"));
  display.clear();
  display.print(F("ALFABETADELT"));    // if the text is to long or your display (chain) you will only see the last letters
  delay(wait); delay(wait);

  Serial.println(F("Set brightness"));
  display.setBrightness(5);            // set brightness from 0 to 15

  Serial.println(F("all segments of first 4 digits on first device"));
  display.clear();
  display.write(127);
  display.write(127);
  display.write(127);
  display.write(127);
  delay(wait);

  /*
    Serial.println(F("Print a number"));
    display.clear();
    display.print(1234);
    delay(wait);

    Serial.println(F("Blink display"));
    display.clear();
    display.blinkRate(HT16K33_BLINK_2HZ);
    display.print(F("BLIN"));
    delay(wait); delay(wait); delay(wait);
    display.blinkRate(HT16K33_BLINK_OFF);

    Serial.println(F("Print a float"));
    display.clear();
    display.print(5.67);
    delay(wait);

    Serial.println(F("Print text with dots"));
    display.clear();
    display.print(F("5.6.7.8."));
    delay(wait);

    //display7.showColon(0);

  */
}


void loop() {
  // put your main code here, to run repeatedly:
  testOnOff();
}


void testOnOff()
{
  //display.home(); //4438 422
  display.clear(); // 4438 422
  Serial.println(F("test On Off"));
  Serial.println(F("off"));
  display.off();
  display.print(F("off "));   // will not be shown on the display
  delay(wait);

  Serial.println(F("new"));
  display.println(F("new ")); // will not be shown on the display
  delay(wait);

  Serial.println(F("On"));
  display.on();               // this will show the last text new
  //display.print(F("On  "));
  delay(wait * 2);
}



void testBrightness()
{
  Serial.println(F("test brightness"));
  for (byte i = 0; i < 16; i++)
  {
    Serial.println(i);
    display.setBrightness(i);            // set brightness from 0 to 15
    delay(wait);
  }
}

void testSet()
{
  // just a set of several tests
  char data[10]; // we need a small buffer to store a temp result

  Serial.println(F("display fixtext F and a float right aligned"));
  display.clear();
  for (float counter = -9; counter < 10;  counter = counter + 1.3)
  {
    int16_t myint = counter * 10;
    sprintf(data, "F%2d.%d", myint / 10, abs(myint) % 10);  // we split the output into two separte integers
    display.print(data);
    Serial.println(data);
    delay(wait);
  }

  /*
    Serial.println(F("display fixtext F and a float right aligned on systems like ESP8266 or ESP32 - will NOT work on Arduino UNO/MEGA/NANO ..."));
    display.clear();
    for (float counter = -9; counter < 10;  counter = counter + 1.3)
    {
    sprintf(data, "F%3.1f", counter);   // 3 total digits (including dot!) + 1 after dot
    display.print(data);
    Serial.println(data);
    delay(wait);
    }
  */

  Serial.println(F("display the Fixtext A and an integeger right aligned"));
  for (int8_t counter = 123; counter > -12;  counter--) {
    sprintf(data, "A%3d", counter);
    display.print(data);
    delay(wait);
  }

  Serial.println(F("Printable characters"));
  for (uint8_t counter = 32; counter < 128; counter++) {
    display.clear();
    display.print(counter);
    display.write(counter);
    delay(wait);
  }

  Serial.println(F("display numbers on display"));
  display.clear();
  for (uint16_t counter = 0; counter < 1000; counter++) {
    float myvalue = counter;
    myvalue = myvalue / 10;
    Serial.print(F("\ncounter=")); Serial.println(counter);
    Serial.print(F("myvalue=")); Serial.println(myvalue);
    //display.clear();
    display.setCursor(0);
    display.print(myvalue, 1);
    delay(500);
  }
}
