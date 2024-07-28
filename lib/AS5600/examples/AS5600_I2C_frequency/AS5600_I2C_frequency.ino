//
//    FILE: AS5600_I2C_frequency.ino
//  AUTHOR: Rob Tillaart
// PURPOSE: demo
//     URL: https://github.com/RobTillaart/AS5600
//
//  Examples may use AS5600 or AS5600L devices.
//  Check if your sensor matches the one used in the example.
//  Optionally adjust the code.


#include "AS5600.h"


AS5600L as5600;   //  use default Wire

uint32_t clk = 0;

uint32_t start, stop;


void setup()
{
  Serial.begin(115200);
  Serial.println(__FILE__);
  Serial.print("AS5600_LIB_VERSION: ");
  Serial.println(AS5600_LIB_VERSION);

  Wire.begin();

  as5600.begin(4);  //  set direction pin.
  //  as5600.setAddress(0x40);  //  AS5600L only

  as5600.setDirection(AS5600_CLOCK_WISE);  //  default, just be explicit.
  int b = as5600.isConnected();
  Serial.print("Connect: ");
  Serial.println(b);
}


void loop()
{
  clk += 100000;
  if (clk > 800000) clk = 100000;
  Wire.setClock(clk);

  delay(10);
  start = micros();
  int angle = as5600.readAngle();
  stop = micros();

  Serial.print(clk);
  Serial.print("\t");
  Serial.print(stop - start);
  Serial.print("\t");
  Serial.print(angle);
  Serial.print("\t");
  Serial.println(as5600.rawAngle() * AS5600_RAW_TO_DEGREES);

  delay(1000);
}


//  -- END OF FILE --

