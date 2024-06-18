//
//    FILE: AS5600_demo.ino
//  AUTHOR: Rob Tillaart
// PURPOSE: demo
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

  Wire.begin();

  as5600.begin(4);  //  set direction pin.
  as5600.setDirection(AS5600_CLOCK_WISE);  //  default, just be explicit.
  int b = as5600.isConnected();
  Serial.print("Connect: ");
  Serial.println(b);
  delay(1000);
}


void loop()
{
  //  Serial.print(millis());
  //  Serial.print("\t");
  Serial.print(as5600.readAngle());
  Serial.print("\t");
  Serial.println(as5600.rawAngle());
  //  Serial.println(as5600.rawAngle() * AS5600_RAW_TO_DEGREES);

  delay(1000);
}


//  -- END OF FILE --
