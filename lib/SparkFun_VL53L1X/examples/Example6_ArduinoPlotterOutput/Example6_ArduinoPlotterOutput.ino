/*
 Reading distance from the laser based VL53L1X
 By: Armin Joachimsmeyer
 for SparkFun Electronics
 Date: February 20th, 2020
 License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

 SparkFun labored with love to create this code. Feel like supporting open source hardware?
 Buy a board from SparkFun! https://www.sparkfun.com/products/14667

 This example prints the distance to an object to the Arduino Serial Plotter and generates a tone depending on distance.

 Are you getting weird readings? Be sure the vacuum tape has been removed from the sensor.
 */

#include <Wire.h>
#include "SparkFun_VL53L1X.h" //Click here to get the library: http://librarymanager/All#SparkFun_VL53L1X

//Optional interrupt and shutdown pins.
#define SHUTDOWN_PIN 2
#define INTERRUPT_PIN 3
#define TONE_PIN 11

SFEVL53L1X distanceSensor;
//Uncomment the following line to use the optional shutdown and interrupt pins.
//SFEVL53L1X distanceSensor(Wire, SHUTDOWN_PIN, INTERRUPT_PIN);

void setup(void)
{
  Wire.begin();

  // initialize the digital pin as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  while (!Serial)
    ; //delay for Leonardo
  // Just to know which program is running on my Arduino
  Serial.println(F("START " __FILE__));

#if !defined(ESP32) && !defined(ARDUINO_SAM_DUE) && !defined(__SAM3X8E__)
  // test beep for the connected speaker
  tone(TONE_PIN, 1000, 100);
  delay(200);
#endif

  if (distanceSensor.begin() != 0) //Begin returns 0 on a good init
  {
    Serial.println("Sensor failed to begin. Please check wiring. Freezing...");
    while (1)
      ;
  }
  Serial.println("Sensor online!");

  // Short mode max distance is limited to 1.3 m but has a better ambient immunity.
  // Above 1.3 meter error 4 is thrown (wrap around).
  distanceSensor.setDistanceModeShort();
  //distanceSensor.setDistanceModeLong(); // default

  // Print Legend
  Serial.println("Distance Signal-Rate/100 Ambient-Rate/100");

  /*
     * The minimum timing budget is 20 ms for the short distance mode and 33 ms for the medium and long distance modes.
     * Predefined values = 15, 20, 33, 50, 100(default), 200, 500.
     * This function must be called after SetDistanceMode.
     */
  distanceSensor.setTimingBudgetInMs(50);

  // measure periodically. Intermeasurement period must be >/= timing budget.
  distanceSensor.setIntermeasurementPeriod(100);
  distanceSensor.startRanging(); // Start once
}

void loop(void)
{
  while (!distanceSensor.checkForDataReady())
  {
    delay(1);
  }
  byte rangeStatus = distanceSensor.getRangeStatus();
  unsigned int distance = distanceSensor.getDistance(); //Get the result of the measurement from the sensor
  distanceSensor.clearInterrupt();

  /*
     * With signed int we get overflow at short distances.
     * With unsigned we get an overflow below around 2.5 cm.
     */
  unsigned int tSignalRate = distanceSensor.getSignalRate();
  unsigned int tAmbientRate = distanceSensor.getAmbientRate();

  if (rangeStatus == 0)
  {
#if !defined(ESP32) && !defined(ARDUINO_SAM_DUE) && !defined(__SAM3X8E__)
    tone(11, distance + 500);
#endif
  }
  else
  {
    // if tAmbientRate > tSignalRate we likely get a signal fail error condition
    // in Distance mode short we get error 4 (out of bounds) or 7 (wrap around) if the distance is greater than 1.3 meter.
    distance = rangeStatus;
#if !defined(ESP32) && !defined(ARDUINO_SAM_DUE) && !defined(__SAM3X8E__)
    noTone(11);
#endif
  }

  Serial.print(distance);
  Serial.print(' ');
  Serial.print(tSignalRate / 100);
  Serial.print(' ');
  Serial.println(tAmbientRate / 100);
}
