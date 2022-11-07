#include <Arduino.h>

#include <ComponentObject.h>
#include <RangeSensor.h>
#include <SparkFun_VL53L1X.h>
#include <vl53l1x_class.h>
#include <vl53l1_error_codes.h>

/*
 Calling distance offset calibration for the laser based VL53L1X
 By: Armin Joachimsmeyer
 for SparkFun Electronics
 Date: February 20th, 2020
 License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

 SparkFun labored with love to create this code. Feel like supporting open source hardware?
 Buy a board from SparkFun! https://www.sparkfun.com/products/14667

 This example calls the offset calibration for a VL53L1X sensor and stores the value to EEPROM.

 Are you getting weird readings? Be sure the vacuum tape has been removed from the sensor.
 */

#include <Wire.h>
#include "SparkFun_VL53L1X.h" //Click here to get the library: http://librarymanager/All#SparkFun_VL53L1X

#define VERSION_EXAMPLE "1.0"

#define EEPROM_ADDRESS_FOR_OFFSET (E2END - 2) // last 2 bytes of EEPROM

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
#if defined(__AVR_ATmega32U4__)
  while (!Serial)
    ; //delay for Leonardo, but this loops forever for Maple Serial
#endif
#if defined(SERIAL_USB)
  delay(2000); // To be able to connect Serial monitor after reset and before first printout
#endif
  // Just to know which program is running on my Arduino
  Serial.println(F("START " __FILE__ "\r\nVersion " VERSION_EXAMPLE " from  " __DATE__));

#if !defined(ESP32) && !defined(ARDUINO_SAM_DUE) && !defined(__SAM3X8E__)
  tone(TONE_PIN, 1000, 100);
  delay(200);
#endif

  Serial.println();
  Serial.println("*****************************************************************************************************");
  Serial.println("                                    Offset calibration");
  Serial.println("Place a light grey (17 % gray) target at a distance of 140mm in front of the VL53L1X sensor.");
  Serial.println("The calibration will start 5 seconds after a distance below 10 cm was detected for 1 second.");
  Serial.println("Use the resulting offset distance as parameter for the setOffset() function called after begin().");
  Serial.println("*****************************************************************************************************");
  Serial.println();

#ifdef AVR
  Serial.print("Old offset value read from EEPROM=");
  Serial.print((int16_t)eeprom_read_word((uint16_t *)EEPROM_ADDRESS_FOR_OFFSET));
  Serial.println(" mm");
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
  distanceSensor.setTimingBudgetInMs(50);

  int tLowDistanceCount = 0;
  while (tLowDistanceCount < 20)
  {
    while (!distanceSensor.checkForDataReady())
    {
      delay(1);
    }
    if (distanceSensor.getDistance() < 100)
    {
      tLowDistanceCount++;
    }
    else
    {
      tLowDistanceCount = 0;
    }
    distanceSensor.clearInterrupt();
  }

  Serial.println("Distance below 10cm detected for more than a second, start offset calibration in 5 seconds");
#if !defined(ESP32) && !defined(ARDUINO_SAM_DUE) && !defined(__SAM3X8E__)
  tone(TONE_PIN, 1000, 100);
  delay(1000);
  tone(TONE_PIN, 1000, 100);
  delay(1000);
  tone(TONE_PIN, 1000, 100);
  delay(1000);
  tone(TONE_PIN, 1000, 100);
  delay(1000);
  tone(TONE_PIN, 1000, 900);
  delay(1000);
#else
  delay(5000);
#endif

  /*
     * Place a target, 17 % gray, at a distance of 140 mm from the sensor and call the VL53L1X_CalibrateOffset (dev, 140, &offset) function.
     * The calibration may take a few seconds. The offset correction is applied to the sensor at the end of calibration.
     *
     * The calibration function takes 50 measurements and then takes the difference between the target distance
     * and the average distance and then calls setOffset() with this value. Thats all. No magic.
     */
  distanceSensor.calibrateOffset(140);
  Serial.print("Result of offset calibration. RealDistance - MeasuredDistance=");
  Serial.print(distanceSensor.getOffset());
  Serial.print(" mm");
  Serial.println();

#ifdef AVR
  eeprom_write_word((uint16_t *)EEPROM_ADDRESS_FOR_OFFSET, distanceSensor.getOffset());
  Serial.println("Offset value written to end of EEPROM.");
#endif

  // measure periodically. Intermeasurement period must be >/= timing budget.
  distanceSensor.setIntermeasurementPeriod(100);
  distanceSensor.startRanging(); // Start once

#if !defined(ESP32) && !defined(ARDUINO_SAM_DUE) && !defined(__SAM3X8E__)
  tone(TONE_PIN, 1000, 900);
#endif
}

void loop(void)
{
}
