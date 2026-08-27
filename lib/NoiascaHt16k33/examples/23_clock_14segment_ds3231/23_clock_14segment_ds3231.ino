/*******************************************************************************
  Clock example using 14 segment displays & DS3231 real-time clock.

  Needed hardware
  - one chains of two alphanumerical displays with 8 digts total of 16 digits.
  - a realtime clock DS3231

  The software:
  - the sketch uses only basic I2C communication to read the DS3231 time
  - no dedicated RTClib needed
  - the loop() must run without blocking code, otherwise blinking of seconds will not work
  - it simulates one row of the "Time Circuit Display" from Back to the Future.

  by noiasca
  Sketch Version 2020-06-06

 *******************************************************************************/
#include <Wire.h>                                          // needed for the DS3231 and HT16K33
#include "NoiascaHt16k33.h"                                // include the noiasca HT16K33 library - download from http://werner.rothschopf.net/


const uint8_t DS3231_I2C_ADDRESS = 0x68;                   // I2C address of the DS3231
Noiasca_ht16k33_hw_14 display = Noiasca_ht16k33_hw_14();   // 14 segment - Present time

// Define the clock style:
// 0 European continantial style   DD. MMM YYYY HH.MM
// 1 anglo-american style          MMM DD YYYY HH.MM
#define CLOCK_STYLE 0

void updatePresentTime()
{
  static uint16_t intervall = 60000;                       // we only check each n milliseconds the rtc
  static uint32_t previousMillis = -intervall;             // this initial value will force an update of the display on the first call
  const uint16_t intervallBlink = 1000;                    // blink frequency of time dot
  static uint32_t previousMillisBlink = 0;
  static byte previousMinute;                              // needed for check if new print is necessary
  static byte previousHour;                                // needed for blinking the time dot
  if (millis() - previousMillis > intervall)
  {
    //Serial.println(F("D46 check time"));
    byte second = 0, minute = 0, hour = 0, dayOfWeek, dayOfMonth = 1, month = 1, year = 0;
    readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);     // retrieve data from DS3231
    Serial.print(hour); Serial.print(F(":")); Serial.print (minute); Serial.print(F(":")); Serial.println(second);
    if (minute != previousMinute)
    {
      //Serial.println(F("D52 new minute"));
#if CLOCK_STYLE == 0
      // 0 Clock in European continantial style   DD. MMM YYYY HH.MM
      const char *monthName[] = {"JAN", "FEB", "MAR", "APR", "MAI", "JUN", "JUL", "AUG", "SEP", "OKT", "NOV", "DEZ"};   // German month names
      display.clear();
      if (dayOfMonth < 10) display.print(F("0"));
      display.print(dayOfMonth);
      display.print(F(". "));
      display.print(monthName[month - 1]);
      display.print(F(" "));
      display.print(2000 + year);
      display.print(F(" "));
      if (hour < 10) display.print(F("0"));
      display.print(hour);
      display.print(F("."));
      if (minute < 10) display.print(F("0"));
      display.print(minute);
#else
      // 1 Clock in anglo-american style          MMM DD YYYY HH.MM
      const char *monthName[] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};  // English month names
      display.clear();
      display.print(monthName[month - 1]);
      if (dayOfMonth < 10) display.print(F(" "));
      display.print(dayOfMonth);
      display.print(F(" "));
      display.print(2000 + year);
      display.print(F(" "));
      byte hourDisplay = hour;
      if (hour > 12) hourDisplay = hour - 12;
      if (hourDisplay < 10) display.print(F("0"));
      display.print(hourDisplay);
      display.setCursor(14);
      if (minute < 10) display.print(F("0"));
      display.print(minute);
#endif
      previousHour = hour;
      previousMinute = minute;
      //Serial.print(F("Temp ")); Serial.println(getDS3231temp()); // The DS3231 has a temperature compensation. You can retrive the temperature from the DS3231.
    }
    intervall = (60 - second) * 1000;  // adopt intervall depending on the time to next minute
    previousMillis = millis();
  }
#if CLOCK_STYLE == 0
  if (millis() - previousMillisBlink > intervallBlink)
  {
    // blink the dot between HH and MM
    display.setCursor(13);
    display.print(previousHour % 10);            // we need to reprint the Hour with or without a dot
    if ((millis() / intervallBlink ) % 2)
      display.print(F("."));
    previousMillisBlink = millis();
  }
#else
  if (millis() - previousMillisBlink > intervallBlink)
  {
    // blink a - between HH and MM
    display.setCursor(13);
    if ((millis() / intervallBlink ) % 2)
      display.print(F("-"));
    else
      display.print(F(" "));
    previousMillisBlink = millis();
  }
#endif
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println(F("\nTime Circuits Display"));
  Wire.begin();                                  // start the I2C interface
  Wire.setClock(400000);                         // optional: activate I2C fast mode. If it is to fast for other I2C devices. deactivate this row.
  display.begin(0x70, 2);                        // I2C adress of the first green display, 2 modules needed for 16 characters
  if (display.isConnected() == false)            // check if all HT16K33 are connected
    Serial.println(F("E: display error"));
  display.clear();
  //                  ss  mm  hh  w  dd  mm yy
  //setDS3231datetime(00, 56, 9, 7, 12, 10, 19); // set the time once
  Serial.println(F("ON Test"));
  display.print(F("  PRESENT TIME  "));
  delay(2000);
  Serial.println(F("now, display shows it's own animation..."));
}


void loop() {
  // put your main code here, to run repeatedly:
  updatePresentTime();
}
