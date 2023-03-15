/*
  Self Test and Factory Reset
  By: Paul Clark
  Based on earlier code by: Nathan Seidle
  SparkFun Electronics
  Date: June 3rd, 2021
  License: MIT. See license file for more information but you can
  basically do whatever you want with this code.

  Feel like supporting open source hardware?
  Buy a board from SparkFun! https://www.sparkfun.com/products/18365

  This example prints the current CO2 level, relative humidity, and temperature in C.

  Hardware Connections:
  Attach RedBoard to computer using a USB cable.
  Connect SCD40/41 to RedBoard using Qwiic cable.
  Open Serial Monitor at 115200 baud.
*/

#include <Wire.h>

#include "SparkFun_SCD4x_Arduino_Library.h" //Click here to get the library: http://librarymanager/All#SparkFun_SCD4x
SCD4x mySensor;

void setup()
{
  Serial.begin(115200);
  Serial.println(F("SCD4x Example"));
  Wire.begin();

  //mySensor.enableDebugging(); // Uncomment this line to get helpful debug messages on Serial

  if (mySensor.begin() == false)
  {
    Serial.println(F("Sensor not detected. Please check wiring. Freezing..."));
    while (1)
      ;
  }

  //We need to stop periodic measurements before we can run the self test
  if (mySensor.stopPeriodicMeasurement() == true)
  {
    Serial.println(F("Periodic measurement is disabled!"));
  }  

  //Now we can run the self test:
  Serial.println(F("Starting the self-test. This will take 10 seconds to complete..."));

  bool success = mySensor.performSelfTest();

  Serial.print(F("The self test was "));
  if (success == false)
    Serial.print(F("not "));
  Serial.println(F("successful"));

  //We can do a factory reset if we want to completely reset the sensor
  Serial.println(F("Starting the factory reset. This will take 1200ms seconds to complete..."));

  success = mySensor.performFactoryReset();

  Serial.print(F("The factory reset was "));
  if (success == false)
    Serial.print(F("not "));
  Serial.println(F("successful"));

}

void loop()
{
  // Nothing to do here
}
