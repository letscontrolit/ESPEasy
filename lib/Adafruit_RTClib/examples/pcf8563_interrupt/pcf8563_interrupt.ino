// Date and time functions using a PCF8563 RTC connected via I2C and Wire lib
#include "RTClib.h"

RTC_PCF8563 rtc;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// use D2 for INT0; attach to CLKOUT pin on RTC
const uint8_t INT_PIN = 2; 

// flag to update serial; set in interrupt callback
volatile uint8_t tick_tock = 1;

// INT0 interrupt callback; update tick_tock flag
void set_tick_tock(void) {
  tick_tock = 1;
}

void setup () {
  Serial.begin(115200);

#ifndef ESP8266
  while (!Serial); // wait for serial port to connect. Needed for native USB
#endif

  
  pinMode(INT_PIN, INPUT);        // set up interrupt pin
  digitalWrite(INT_PIN, HIGH);    // turn on pullup resistors
  // attach interrupt to set_tick_tock callback on rising edge of INT0
  attachInterrupt(digitalPinToInterrupt(INT_PIN), set_tick_tock, RISING);

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    abort();
  }

  if (rtc.lostPower()) {
    Serial.println("RTC is NOT initialized, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
    //
    // Note: allow 2 seconds after inserting battery or applying external power
    // without battery before calling adjust(). This gives the PCF8523's
    // crystal oscillator time to stabilize. If you call adjust() very quickly
    // after the RTC is powered, lostPower() may still return true.
  }


  
  // When time needs to be re-set on a previously configured device, the
  // following line sets the RTC to the date & time this sketch was compiled
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // This line sets the RTC with an explicit date & time, for example to set
  // January 21, 2014 at 3am you would call:
  // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));

  // When the RTC was stopped and stays connected to the battery, it has
  // to be restarted by clearing the STOP bit. Let's do this to ensure
  // the RTC is running.
  rtc.start();
    
  // turn on 1Hz clock out, used as INT0 for serial update every second
  rtc.writeSqwPinMode(PCF8563_SquareWave1Hz);
}

void loop () {

  // check if time display should be output
  if(tick_tock) {
 
    DateTime now = rtc.now();

    char time_format[] = "hh:mm:ss AP";
    char date_format[] = "MM/DD/YYYY";

    Serial.println(now.toString(time_format));
    Serial.println(now.toString(date_format));
    Serial.println();
 
    tick_tock = 0;

  }

}
