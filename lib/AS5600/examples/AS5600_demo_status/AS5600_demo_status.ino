//
//    FILE: AS5600_demo_status.ino
//  AUTHOR: Rob Tillaart
// PURPOSE: demo
//     URL: https://github.com/RobTillaart/AS5600
//
//  Examples may use AS5600 or AS5600L devices.
//  Check if your sensor matches the one used in the example.
//  Optionally adjust the code.


#include "AS5600.h"


AS5600L as5600(0x40);   //  use default Wire


void setup()
{
  Serial.begin(115200);
  Serial.println(__FILE__);
  Serial.print("AS5600_LIB_VERSION: ");
  Serial.println(AS5600_LIB_VERSION);

  Wire.begin();

  as5600.begin(4);  //  set direction pin.
  as5600.setDirection(AS5600_CLOCK_WISE);  //  default, just be explicit.
}


void loop()
{
  Serial.print("STATUS:\t ");
  Serial.println(as5600.readStatus(), HEX);
  Serial.print("CONFIG:\t ");
  Serial.println(as5600.getConfigure(), HEX);
  Serial.print("  GAIN:\t ");
  Serial.println(as5600.readAGC(), HEX);
  Serial.print("MAGNET:\t ");
  Serial.println(as5600.readMagnitude(), HEX);
  Serial.print("DETECT:\t ");
  Serial.println(as5600.detectMagnet(), HEX);
  Serial.print("M HIGH:\t ");
  Serial.println(as5600.magnetTooStrong(), HEX);
  Serial.print("M  LOW:\t ");
  Serial.println(as5600.magnetTooWeak(), HEX);
  Serial.println();

  delay(1000);
}


//  -- END OF FILE --
