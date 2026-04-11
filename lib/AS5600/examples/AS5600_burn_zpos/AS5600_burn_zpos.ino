//
//    FILE: AS5600_burn_zpos.ino
//  AUTHOR: Rob Tillaart
// PURPOSE: demo (not tested yet - see issue #38)
//     URL: https://github.com/RobTillaart/AS5600
//
//  WARNING
//  As burning the angle can only be done three times this sketch has to be used with care.
//
//  You need to
//  - read the datasheet so you understand what you do
//  - read issue #38 to understand the discussion that lead to this sketch
//  - uncomment burnAngle() in AS5600.h and AS5600.cpp.
//  - adjust settings and MaxAngle in burn_zpos() function below ==> line 77++
//  - uncomment line 105 of this sketch
//
//  Examples may use AS5600 or AS5600L devices.
//  Check if your sensor matches the one used in the example.
//  Optionally adjust the code.


#include "AS5600.h"


AS5600 as5600;   //  use default Wire
//  AS5600L as5600;


void setup()
{
  Serial.begin(115200);
  Serial.println(__FILE__);
  Serial.print("AS5600_LIB_VERSION: ");
  Serial.println(AS5600_LIB_VERSION);

  Wire.begin();

  as5600.begin(4);  //  set direction pin.
  as5600.setDirection(AS5600_CLOCK_WISE);  //  default, just be explicit.

  if (as5600.isConnected())
  {
    Serial.println("Connected");
  }
  else
  {
    Serial.println("Failed to connect. Check wires and reboot.");
    //while (1);
  }


  Serial.println("\nWARNING  WARNING  WARNING  WARNING  WARNING  WARNING\n");
  Serial.println("This sketch will burn settings to your AS5600.");
  Serial.println("Adjust the settings in the sketch to your needs.");
  Serial.println("Press any key to continue.");
  Serial.println("\nWARNING  WARNING  WARNING  WARNING  WARNING  WARNING\n\n");
  while (Serial.available()) Serial.read();
  while (!Serial.available());
  Serial.read();


  while (Serial.available()) Serial.read();
  Serial.print("Are you sure to burn zpos? [Y for Yes]");
  while (!Serial.available());
  char c = Serial.read();
  if (c == 'Y')
  {
    burn_zpos();
  }

  Serial.println("\nDone..");
}


void loop()
{
}


void burn_zpos()
{
  //  ADJUST ZPOS
  const uint16_t ZPOS = 0;

  bool OK = true;
  OK = OK && as5600.setZPosition(ZPOS);
  OK = OK && (ZPOS == as5600.getZPosition());
  if (OK == false)
  {
    Serial.println("\nERROR: in settings, burn_zpos() cancelled.");
    return;
  }

  Serial.println();
  Serial.println("burning in 5 seconds");
  delay(1000);
  Serial.println("burning in 4 seconds");
  delay(1000);
  Serial.println("burning in 3 seconds");
  delay(1000);
  Serial.println("burning in 2 seconds");
  delay(1000);
  Serial.println("burning in 1 seconds");
  delay(1000);
  Serial.print("burning ...");
  delay(1000);
  //  uncomment next line
  //  as5600.burnAngle();
  Serial.println(" done.");
  Serial.println("Reboot AS5600 to use the new settings.");
}



//  -- END OF FILE --
