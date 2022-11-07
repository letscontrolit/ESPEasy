/*
  Reading distance from the laser based VL53L1X
  By: Nathan Seidle
  SparkFun Electronics
  Date: April 4th, 2018
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

  SparkFun labored with love to create this code. Feel like supporting open source hardware? 
  Buy a board from SparkFun! https://www.sparkfun.com/products/14667

  This example demonstrates how to read and average distance, the measurement status, and the signal rate.

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

//Store distance readings to get rolling average
#define HISTORY_SIZE 10
int history[HISTORY_SIZE];
byte historySpot;

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

  for (int x = 0; x < HISTORY_SIZE; x++)
    history[x] = 0;
}

void loop(void)
{
  long startTime = millis();
  distanceSensor.startRanging(); //Write configuration block of 135 bytes to setup a measurement
  while (!distanceSensor.checkForDataReady())
  {
    delay(1);
  }
  int distance = distanceSensor.getDistance(); //Get the result of the measurement from the sensor
  distanceSensor.clearInterrupt();
  distanceSensor.stopRanging();
  long endTime = millis();

  Serial.print("Distance(mm): ");
  Serial.print(distance);

  history[historySpot] = distance;
  if (++historySpot == HISTORY_SIZE)
    historySpot = 0;

  long avgDistance = 0;
  for (int x = 0; x < HISTORY_SIZE; x++)
    avgDistance += history[x];

  avgDistance /= HISTORY_SIZE;
  Serial.print("\tavgDistance: ");
  Serial.print(avgDistance);

  float distanceInches = avgDistance * 0.0393701;
  float distanceFeet = distanceInches / 12;

  Serial.print("\tavgDistance(ft): ");
  Serial.print(distanceFeet, 2);

  int signalRate = distanceSensor.getSignalRate();
  Serial.print("\tSignal rate: ");
  Serial.print(signalRate);

  byte rangeStatus = distanceSensor.getRangeStatus();
  Serial.print("\tRange Status: ");

  //Make it human readable
  switch (rangeStatus)
  {
  case 0:
    Serial.print("Good");
    break;
  case 1:
    Serial.print("Sigma fail");
    break;
  case 2:
    Serial.print("Signal fail");
    break;
  case 7:
    Serial.print("Wrapped target fail");
    break;
  default:
    Serial.print("Unknown: ");
    Serial.print(rangeStatus);
    break;
  }

  Serial.print("\tHz: ");
  Serial.print(1000.0 / (float)(endTime - startTime), 2);

  Serial.println();
}
