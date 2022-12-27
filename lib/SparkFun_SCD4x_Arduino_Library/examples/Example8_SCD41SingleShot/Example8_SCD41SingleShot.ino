/*
  SCD41 Low Power Single Shot
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

SCD4x mySensor(SCD4x_SENSOR_SCD41); // Tell the library we have a SCD41 connected

void setup()
{
  Serial.begin(115200);
  Serial.println(F("SCD41 Example"));
  Wire.begin();

  //mySensor.enableDebugging(); // Uncomment this line to get helpful debug messages on Serial

  if (mySensor.begin(false, true, false) == false) // Do not start periodic measurements
  //measBegin_________/     |     |
  //autoCalibrate__________/      |
  //skipStopPeriodicMeasurements_/
  {
    Serial.println(F("Sensor not detected. Please check wiring. Freezing..."));
    while (1)
      ;
  }

  //Let's call measureSingleShot to start the first conversion
  bool success = mySensor.measureSingleShot();
  if (success == false)
  {
    Serial.println(F("measureSingleShot failed. Are you sure you have a SCD41 connected? Freezing..."));
    while (1)
      ;    
  }
}

void loop()
{
  while (mySensor.readMeasurement() == false) // readMeasurement will return true when fresh data is available
  {
    Serial.print(F("."));
    delay(500);
  }

  Serial.println();

  Serial.print(F("CO2(ppm):"));
  Serial.print(mySensor.getCO2());

  Serial.print(F("\tTemperature(C):"));
  Serial.print(mySensor.getTemperature(), 1);

  Serial.print(F("\tHumidity(%RH):"));
  Serial.print(mySensor.getHumidity(), 1);

  Serial.println();
  
  mySensor.measureSingleShotRHTOnly(); // Request just the RH and the Temperature (should take 50ms)

  while (mySensor.readMeasurement() == false) // readMeasurement will return true when fresh data is available
  {
    Serial.print(F("."));
    delay(5);
  }

  Serial.println();

  Serial.print(F("Temperature(C):"));
  Serial.print(mySensor.getTemperature(), 1);

  Serial.print(F("\tHumidity(%RH):"));
  Serial.print(mySensor.getHumidity(), 1);

  Serial.println();
  
  mySensor.measureSingleShot(); // Request fresh data (should take 5 seconds)
}
