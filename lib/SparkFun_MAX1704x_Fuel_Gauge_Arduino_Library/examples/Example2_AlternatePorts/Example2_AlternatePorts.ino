/******************************************************************************
Example2_AlternatePorts
By: Paul Clark
Date: October 23rd 2020

Based extensively on:
MAX17043_Simple_Serial.cpp
SparkFun MAX17043 Example Code
Jim Lindblom @ SparkFun Electronics
Original Creation Date: June 22, 2015

This file demonstrates the simple API of the SparkFun MAX17043 Arduino library using non-standard Wire and Serial ports.

This example will print the gauge's voltage and state-of-charge (SOC) readings
to serial (115200 baud)

This code is released under the MIT license.

Distributed as-is; no warranty is given.
******************************************************************************/

#include <Wire.h> // Needed for I2C

#include <SparkFun_MAX1704x_Fuel_Gauge_Arduino_Library.h> // Click here to get the library: http://librarymanager/All#SparkFun_MAX1704x_Fuel_Gauge_Arduino_Library

SFE_MAX1704X lipo; // Defaults to the MAX17043

//SFE_MAX1704X lipo(MAX1704X_MAX17043); // Create a MAX17043
//SFE_MAX1704X lipo(MAX1704X_MAX17044); // Create a MAX17044
//SFE_MAX1704X lipo(MAX1704X_MAX17048); // Create a MAX17048
//SFE_MAX1704X lipo(MAX1704X_MAX17049); // Create a MAX17049

// Define our non-standard ports:
#define mySerial Serial1
TwoWire myWire(0);

double voltage = 0; // Variable to keep track of LiPo voltage
double soc = 0; // Variable to keep track of LiPo state-of-charge (SOC)
bool alert; // Variable to keep track of whether alert has been triggered

void setup()
{
	mySerial.begin(115200); // Start serial, to output debug data
  while (!mySerial)
    ; //Wait for user to open terminal
  mySerial.println(F("MAX17043 Example"));

  myWire.begin();

  lipo.enableDebugging(mySerial); // Uncomment this line to enable helpful debug messages on non-standard serial

  // Set up the MAX17043 LiPo fuel gauge:
  if (lipo.begin(myWire) == false) // Connect to the MAX17043 using non-standard wire port
  {
    mySerial.println(F("MAX17043 not detected. Please check wiring. Freezing."));
    while (1)
      ;
  }

	// Quick start restarts the MAX17043 in hopes of getting a more accurate
	// guess for the SOC.
	lipo.quickStart();

	// We can set an interrupt to alert when the battery SoC gets too low.
	// We can alert at anywhere between 1% - 32%:
	lipo.setThreshold(20); // Set alert threshold to 20%.
}

void loop()
{
  // lipo.getVoltage() returns a voltage value (e.g. 3.93)
  voltage = lipo.getVoltage();
  // lipo.getSOC() returns the estimated state of charge (e.g. 79%)
  soc = lipo.getSOC();
  // lipo.getAlert() returns a 0 or 1 (0=alert not triggered)
  alert = lipo.getAlert();

  // Print the variables:
  mySerial.print("Voltage: ");
  mySerial.print(voltage);  // Print the battery voltage
  mySerial.println(" V");

  mySerial.print("Percentage: ");
  mySerial.print(soc); // Print the battery state of charge
  mySerial.println(" %");

  mySerial.print("Alert: ");
  mySerial.println(alert);
  mySerial.println();

  delay(500);
}
