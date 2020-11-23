#ifndef HELPERS_ESPEASY_TIME_CALC_H
#define HELPERS_ESPEASY_TIME_CALC_H

#include <Arduino.h>


inline uint64_t getMicros64() {
  #ifdef ESP8266
  return micros64();
  #endif
  #ifdef ESP32
  return esp_timer_get_time();
  #endif
}


/********************************************************************************************\
   Simple time computations.
 \*********************************************************************************************/

// Return the time difference as a signed value, taking into account the timers may overflow.
// Returned timediff is between -24.9 days and +24.9 days.
// Returned value is positive when "next" is after "prev"
inline int32_t timeDiff(const unsigned long prev, const unsigned long next) {
  return ((int32_t) (next - prev));
}

// Compute the number of milliSeconds passed since timestamp given.
// N.B. value can be negative if the timestamp has not yet been reached.
inline long timePassedSince(unsigned long timestamp) {
  return timeDiff(timestamp, millis());
}

inline long usecPassedSince(unsigned long timestamp) {
  return timeDiff(timestamp, micros());
}

// Check if a certain timeout has been reached.
inline bool timeOutReached(unsigned long timer) {
  return timePassedSince(timer) >= 0;
}

inline bool usecTimeOutReached(unsigned long timer) {
  return usecPassedSince(timer) >= 0;
}


// Check if a certain timeout has been reached.
bool timeOutReached(unsigned long timer);

bool usecTimeOutReached(unsigned long timer);





/********************************************************************************************\
   Unix Time computations
 \*********************************************************************************************/
bool isLeapYear(int year);

uint32_t makeTime(const struct tm& tm);

/********************************************************************************************\
   Time computations for rules.
 \*********************************************************************************************/

// format 0000WWWWAAAABBBBCCCCDDDD
// WWWW=weekday, AAAA=hours tens digit, BBBB=hours, CCCC=minutes tens digit DDDD=minutes

// Convert a 32 bit integer into a string like "Sun,12:30"
String timeLong2String(unsigned long lngTime);

// Convert a string like "Sun,12:30" into a 32 bit integer
unsigned long string2TimeLong(const String& str);


/********************************************************************************************\
   Match clock event
 \*********************************************************************************************/
bool matchClockEvent(unsigned long clockEvent, unsigned long clockSet);


#endif // HELPERS_ESPEASY_TIME_CALC_H