//
//    FILE: AS5600_outmode_analog_100.ino
//  AUTHOR: Rob Tillaart
// PURPOSE: experimental demo
//     URL: https://github.com/RobTillaart/AS5600
//
//  connect the OUT pin to the analog port of the processor
//
//  The AS5600L does not support analog OUT.


#include "AS5600.h"


AS5600 as5600;   //  use default Wire


void setup()
{
  Serial.begin(115200);
  Serial.println(__FILE__);
  Serial.print("AS5600_LIB_VERSION: ");
  Serial.println(AS5600_LIB_VERSION);

  Wire.begin();

  as5600.begin(4);  //  set direction pin.
  as5600.setDirection(AS5600_CLOCK_WISE);  //  default, just be explicit.
  as5600.setOutputMode(AS5600_OUTMODE_ANALOG_100);
}


void loop()
{
  Serial.print(millis());
  Serial.print("\t");
  Serial.print(as5600.readAngle());
  Serial.print("\t");
  Serial.println(analogRead(A0));

  delay(1000);
}


//  -- END OF FILE --
