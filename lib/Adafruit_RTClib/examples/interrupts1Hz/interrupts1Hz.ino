/***********************************************************************

    Combining RTClib with the avr-libc timing functions
    ===================================================

The standard way of getting the current time and date with RTClib is to
call the now() method of the appropriate RTC class. This, however, is
somewhat slow, as it involves communicating with the RTC through the I2C
bus. An alternative, more lightweight method involves configuring the
RTC to deliver one pulse per second to an interrupt pin, and counting
the seconds in the interrupt handler. The timekeeping is then entirely
handled in the Arduino, with no I2C communication with the RTC other
than during the initialization phase.

On AVR-based Arduinos (Uno, Nano, Micro, ...), the Arduino core library
is built on top of avr-libc, which is an implementation of the standard
C library for the AVR platform. This library provides the standard C
functions for handling time:[1] time(), gmtime(), mktime(), etc. The
time() function is normally used to retrieve the current time from the
operating system, but since we have no operating system, the avr-libc
provides its own non-standard functions for implementing a time source:

 - set_system_time() initializes the library's idea of the current time
 - system_tick() steps the system time by one second.

This sketch demonstrates how to combine RTClib and avr-libc in order to
handle the timekeeping entirely on the Arduino from an interrupt
delivered by the RTC:

 - RTClib is used to configure the RTC and retrieve the initial time
 - avr-libc is used for regular timekeeping

This sketch only works on AVR-based Arduinos, as it relies on
non-standard functions provided by avr-libc.

[1] https://www.nongnu.org/avr-libc/user-manual/group__avr__time.html

***********************************************************************/

#include <RTClib.h>
#include <time.h>  // standard C timing functions

// Pin receiving the one-pulse-per-second signal from the RTC.
// This should be an interrupt-capable pin.
const uint8_t pin1pps = 2;

// We will use the PCF8523 RTC, which has the handy "Second Timer".
// Other RTCs could be used with their "square wave" output configured
// to 1 Hz.
RTC_PCF8523 rtc;

void setup() {

  // Initialize the serial port.
  Serial.begin(57600);
  while (!Serial); // wait for serial port to connect. Needed for native USB

  // Initialize the RTC.
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    abort();
  }
  if (!rtc.initialized() || rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  rtc.deconfigureAllTimers();  // undo previous configuration, if any

  // Initialize the system time from the RTC time. Both avr-libc and
  // DateTime::secondstime() use the start of year 2000 as their
  // reference "epoch".
  set_system_time(rtc.now().secondstime());

  // Keep the time in sync using the one-pulse-per-second output of the
  // RTC as an interrupt source and calling system_tick() from the
  // interrupt service routine.
  pinMode(pin1pps, INPUT_PULLUP);
  rtc.enableSecondTimer();
  attachInterrupt(digitalPinToInterrupt(pin1pps), system_tick, FALLING);
}

void loop() {

  // From here on, we only use the standard C timing functions.
  // time() returns the current time as a single number of type time_t,
  // this is the number of seconds elapsed since a reference "epoch".
  time_t now = time(nullptr);

  // gmtime() converts the time to a broken-down form (year, month...)
  // similar to the DateTime class. Unlike localtime(), it doesn't
  // attempt timezone conversions.
  struct tm *broken_down_time = gmtime(&now);

  // asctime() returns a textual representation of the date and time as
  // a C string (pointer to a character array). The format is similar to
  // the DateTime::toString() format "DDD MMM DD hh:mm:ss YYYY".
  Serial.println(asctime(broken_down_time));

  delay(1000);
}
