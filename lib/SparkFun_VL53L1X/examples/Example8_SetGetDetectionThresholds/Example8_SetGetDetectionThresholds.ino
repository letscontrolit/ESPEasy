/*
  Reading distance from the laser based VL53L1X
  By: Nathan Seidle
  Revised by: Andy England and Ricardo Ramos
  SparkFun Electronics
  Date: January 21st, 2022
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

  SparkFun labored with love to create this code. Feel like supporting open source hardware? 
  Buy a board from SparkFun! https://www.sparkfun.com/products/14667

  This example sets high and low thresholds for detection.

  Are you getting weird readings? Be sure the vacuum tape has been removed from the sensor.
*/

#include <Wire.h>
#include "SparkFun_VL53L1X.h" //Click here to get the library: http://librarymanager/All#SparkFun_VL53L1X

//Optional interrupt and shutdown pins.
#define SHUTDOWN_PIN 2
#define INTERRUPT_PIN 3

SFEVL53L1X distanceSensor;
//Uncomment the following line to use the optional shutdown and interrupt pins.
//SFEVL53L1X distanceSensor(Wire, SHUTDOWN_PIN, INTERRUPT_PIN);

void setup(void)
{
  Wire.begin();

  Serial.begin(115200);
  Serial.println("VL53L1X Qwiic Test");

  if (distanceSensor.begin() != 0) //Begin returns 0 on a good init
  {
    Serial.println("Sensor failed to begin. Please check wiring. Freezing...");
    while (1)
      ;
  }
  Serial.println("Sensor online!");

  DetectionConfig dc;               // struct instance which holds the detection configuration
  dc.IntOnNoTarget = 1;              // No longer used - just use 1 per ST
  dc.distanceMode = DISTANCE_SHORT; // short distance mode
  dc.thresholdHigh = 300;           // high threshold of 300 mm
  dc.thresholdLow = 70;             // low threshold of 70 mm
  dc.windowMode = WINDOW_IN;        // will measure and trigger interrrupt when measurement fall between 70 and 300 mm

  if(distanceSensor.setThresholdConfig(&dc) == true)
  {
    Serial.println("Thresholds programmed. Reading configuration back...");
    dc = {};
    if(distanceSensor.getThresholdConfig(&dc) == true)
    {
      Serial.print("IntOnNoTarget: ");
      Serial.println(dc.IntOnNoTarget);
      Serial.print("Distance Mode: ");

      if(dc.distanceMode == DISTANCE_SHORT)
        Serial.println("DISTANCE_SHORT (0)");
      else
        Serial.println("DISTANCE_LONG (1)");
      
      Serial.print("Threshold High: ");
      Serial.println(dc.thresholdHigh);
      Serial.print("Threshold Low: ");
      Serial.println(dc.thresholdLow);
      Serial.print("Window mode: ");
      switch (dc.windowMode)
      {
      case WINDOW_BELOW:
        Serial.println("WINDOW_BELOW (0)");
        break;
      
      case WINDOW_ABOVE:
        Serial.println("WINDOW_ABOVE (1)");
        break;

      case WINDOW_OUT:
        Serial.println("WINDOW_OUT (2)");
        break;

      case WINDOW_IN:
        Serial.println("WINDOW_IN (3)");
        break;

      default:
        break;
      }
    }
  }
  else
  {
    Serial.println("Could not set threshold configuration.");
  }
  Serial.println("Starting measurements in 2 seconds.");
  delay(2000);
  Serial.println("Measurements started!");

}

void loop(void)
{
  distanceSensor.startRanging(); //Write configuration bytes to initiate measurement

  while (!distanceSensor.checkForDataReady())
  {
    delay(1);
  }
  int distance = distanceSensor.getDistance(); //Get the result of the measurement from the sensor
  distanceSensor.clearInterrupt();

  distanceSensor.stopRanging();

  Serial.print("Distance(mm): ");
  Serial.print(distance);

  float distanceInches = distance * 0.0393701;
  float distanceFeet = distanceInches / 12.0;

  Serial.print("\tDistance(ft): ");
  Serial.print(distanceFeet, 2);

  Serial.println();
}
