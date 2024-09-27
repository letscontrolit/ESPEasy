//
//    FILE: AS5600L_set_address.ino
//  AUTHOR: Rob Tillaart
// PURPOSE: demo
//     URL: https://github.com/RobTillaart/AS5600
//
//  Examples may use AS5600 or AS5600L devices.
//  Check if your sensor matches the one used in the example.
//  Optionally adjust the code.


#include "AS5600.h"


AS5600L ASL;   //  use default Wire
// AS5600 ASL;   //  use default Wire


void setup()
{
  Serial.begin(115200);
  Serial.println(__FILE__);
  Serial.print("AS5600_LIB_VERSION: ");
  Serial.println(AS5600_LIB_VERSION);

  Wire.begin();

  ASL.begin(4);  //  set direction pin.
  ASL.setDirection(AS5600_CLOCK_WISE);  //  default, just be explicit.
  int b = ASL.isConnected();
  Serial.print("Connect: ");
  Serial.println(b);

  Serial.print("ADDR: ");
  Serial.println(ASL.getAddress());

  ASL.setAddress(0x38);

  Serial.print("ADDR: ");
  Serial.println(ASL.getAddress());
}


void loop()
{
  //  Serial.print(millis());
  //  Serial.print("\t");
  Serial.print(ASL.readAngle());
  Serial.print("\t");
  Serial.println(ASL.rawAngle() * AS5600_RAW_TO_DEGREES);

  delay(1000);
}


//  -- END OF FILE --
