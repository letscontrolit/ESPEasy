/*******************************************************************************
  03_Strandtest_diagnostics
  Noiasca HT16K33

  If you have problems to get your LED modules working try this sketch.
  It will activate (most) of the connected LEDs.

  The example will handle:
  - test all available adresses
  - write the hex adress on each found module
  - switch on all remaining segments/LEDs
  - loop through a simple animation (set brightness)

  by noiasca
  Sketch Version 2020-06-08

 *******************************************************************************/
#include <Wire.h>                                                    // HT16K33 uses I2C, include the Wire Library
#include "NoiascaHt16k33.h"                                          // include the noiasca HT16K33 library - download from http://werner.rothschopf.net/

const byte startAddr = 0x70;                                         // lowest addresss of the HT16K33 is 0x70
const byte noOfModules = 8;                                          // test all 8 modules
const char allSegments = 127;                                        // character 127 will activate "all segments"
Noiasca_ht16k33_hw_14 display[noOfModules];                          // let's assume 8 separate HT16K33


// just a simple animation
void tick_animation()
{
  const uint16_t intervall = 500;              // intervall of updates
  static uint32_t previousMillis;              // is static, because we need the value the next time when we call the function again, WITHOUT the need of a global variable
  static uint16_t brightness;                  // static, as we want to keep the value from to call without a global variable
  if (millis() - previousMillis > intervall)   // check, if it is time to do something
  {
    for (auto &currentDisplay : display)
      currentDisplay.setBrightness(brightness);
    brightness++;
    if (brightness > 15) brightness = 0;
    previousMillis = millis();
  }
}


void setup() {
  Serial.begin(115200);
  Serial.println(F("\n03 Strandtest diagnostics"));
  Wire.begin();                          // start the I2C interface
  //Wire.setClock(400000);               // optional: activate I2C fast mode. If it is to fast for other I2C devices. deactivate this row.
  byte foundModules = 0;
  Serial.println(F("I: search for available HT16K33"));
  for (byte i = 0; i < noOfModules; i++)
  {
    if (display[i].begin(startAddr + i) == false)  // no error
    {
      Serial.print(F("I: display found 0x")); Serial.println((startAddr + i), HEX);
      display[i].clear();
      display[i].print((startAddr + i), HEX);      // print the hex address of the modul on the first two digits
      for (byte j = 0; j < 6; j++)
        display[i].write(allSegments);             // activate all segments of the remaining 6 digits
      foundModules++;
    }
    else
    {
      Serial.print(F("E: no display at 0x")); Serial.println((0x70 + i), HEX);
    }
  }
  Serial.print(F("I: found ")); Serial.print(foundModules); Serial.println(F(" modules on I2C bus"));
  Serial.println(F("start loop"));
}


void loop() {
  tick_animation();
}
