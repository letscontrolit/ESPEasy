/* Using DS3231 (or other supported RTC) with a custom TwoWire instance
 *
 * If using a microcontroller which supports additional i2c ports,
 * such as the SAMD21's SERCOMX, a user can define a custom i2c bus
 * to use with an RTC.
 * This example builds the custom i2c bus using SERCOM0 and leverages the "wiring_private.h" APIs
 * 
 * Connecting the device:
 * VCC and GND of RTC should be connected to some power source
 * SDA, SCL of RTC should be connected to the custom SDA and SCL pins.
 * In this particular example we are using a Nano 33 IoT and routing 
 * the custom Wire instance over pins 6 (SDA) and 5 (SCL)
 *
 * This example will work with Arduino Zero, any Arduino MKR board based on SAMD21, Nano 33 IoT
 * and any board by Adafruit, Sparkfun, Seeed Studio based on the same microcontroller
 * 
 */
#include <Wire.h>
#include "wiring_private.h"
#include <RTClib.h>

/* Defining the custom TwoWire instance for SAMD21 */
TwoWire myWire(&sercom0, 6, 5);  // Create the new wire instance assigning it to pin 0 and 1
extern "C"{
  void SERCOM0_Handler(void);

  void SERCOM0_Handler(void) {

    myWire.onService();

  }
}

/* Creating a new DS3231 object */
RTC_DS3231 myRTC;

String daysNames[] = {
  "Sunday",
  "Monday",
  "Tuesday",
  "Wednesday",
  "Thursday",
  "Friday",
  "Saturday"
};
String monthsNames[] = {
  "-",
  "January",
  "February",
  "March",
  "April",
  "May",
  "June",
  "July",
  "August",
  "September",
  "October",
  "November",
  "December"
};

void setup() {
  Serial.begin(57600);
  Serial.println("start");
  
  unsigned long setupStartTime = millis();
  /*** Waiting for Serial to be ready or timeout ***/
  while(!Serial && millis() - setupStartTime < 3000);

  /* 
   * Initialising pins 6 and 5 to be routed to the SERCOM0 pads 0 and 1 in order
   * to be used as SDA and SCL. Without this step the periphearl won't be patched through
  */ 
  pinPeripheral(6, PIO_SERCOM_ALT);  // PAD[0]   //Assign SDA function to pin 0
  pinPeripheral(5, PIO_SERCOM_ALT);  // PAD[1]   //Assign SCL function to pin 1

  /* We now pass our custom TwoWire object to the RTC instance */
  myRTC.begin(&myWire);
  
  /* 
   * From this moment on every operation on the RTC will work as expected
   * But the i2c bus being used will be the one we manually created using SERCOM0
  */

  /* 
   * Creating a Date object with 
   * YEAR, MONTH, DAY (2021, January, 1) 
   * HOUR, MINUTE, SECONDS (0, 0, 0)
   * Midnight of January 1st, 2021
  */
  DateTime newDT = DateTime(2021, 1, 1, 0, 0, 0);

  /* Pushing that date/time to the RTC */
  myRTC.adjust(newDT);
  Serial.println("setup done");
}

void loop() {
  /* creating a temporary date/time object to store the data coming from the RTC */
  DateTime dt = myRTC.now();

  /* printing that data to the Serial port in a meaningful format */ 
  Serial.println("************");
  Serial.print(daysNames[dt.dayOfTheWeek()]);
  Serial.print(" ");
  Serial.print(monthsNames[dt.month()]);
  Serial.print(" ");
  Serial.print(dt.day());
  Serial.print(", ");
  Serial.println(dt.year());
  Serial.print(dt.hour());
  Serial.print(":");
  Serial.print(dt.minute());
  Serial.print(":");
  Serial.println(dt.second());
  /* Delays are bad, but let's not flood the Serial for this silly example */
  delay(500);
}


