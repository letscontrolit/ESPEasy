//
//    FILE: AS5600_resetCumulativeCounter.ino
//  AUTHOR: Daniel-Frenkel, (slightly modified by Rob Tillaart)
// PURPOSE: demo - see issue #30
//     URL: https://github.com/RobTillaart/AS5600
//
//  Examples may use AS5600 or AS5600L devices.
//  Check if your sensor matches the one used in the example.
//  Optionally adjust the code.


#include "AS5600.h"


AS5600L as5600;   //  use default Wire


void setup()
{
  Serial.begin(115200);
  Serial.println(__FILE__);
  Serial.print("AS5600_LIB_VERSION: ");
  Serial.println(AS5600_LIB_VERSION);

  Wire.begin(14, 15);        //  ESP32

  as5600.begin();
  as5600.setAddress(0x40);   //  AS5600L has address

  as5600.setDirection(AS5600_CLOCK_WISE);  //  default, just be explicit.
  delay(1000);
  as5600.resetCumulativePosition(777);
}


void loop()
{
  static uint32_t lastTime = 0;

  //  set initial position
  as5600.getCumulativePosition();

  //  update every 100 ms
  if (millis() - lastTime >= 100)
  {
    lastTime = millis();
    Serial.println(as5600.getCumulativePosition());
  }
}


//  -- END OF FILE --

