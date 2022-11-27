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

inline int64_t timeDiff64(uint64_t prev, uint64_t next) {
  return ((int64_t) (next - prev));
}

// Compute the number of milliSeconds passed since timestamp given.
// N.B. value can be negative if the timestamp has not yet been reached.
inline long timePassedSince(const uint32_t& timestamp) {
  return timeDiff(timestamp, millis());
}

inline int64_t usecPassedSince(volatile uint64_t& timestamp) {
  return timeDiff64(timestamp, getMicros64());
}

inline int64_t usecPassedSince(const uint64_t& timestamp) {
  return timeDiff64(timestamp, getMicros64());
}

inline int64_t usecPassedSince(uint64_t& timestamp) { //-V669
  return timeDiff64(timestamp, getMicros64());
}

// Check if a certain timeout has been reached.
inline bool timeOutReached(unsigned long timer) {
  return timePassedSince(timer) >= 0;
}

inline bool usecTimeOutReached(const uint64_t& timer) {
  return usecPassedSince(timer) >= 0;
}



/********************************************************************************************\
   Unix Time computations
 \*********************************************************************************************/
bool isLeapYear(int year);

// Get number of days in a month.
// Month starts at 0 for January.
uint8_t getMonthDays(int year, uint8_t month);

uint32_t makeTime(const struct tm& tm);

void breakTime(unsigned long timeInput, struct tm& tm);

/********************************************************************************************\
   Unix Time formatting
 \*********************************************************************************************/

// Format given Date separated by the given delimiter
// date format example with '-' delimiter: 2016-12-31 (YYYY-MM-DD)
String formatDateString(const struct tm& ts, char delimiter);

// returns the current Time separated by the given delimiter
// time format example with ':' delimiter: 23:59:59 (HH:MM:SS)
String formatTimeString(const struct tm& ts, char delimiter, bool am_pm, bool show_seconds, char hour_prefix = '\0');

// returns the current Date and Time separated by the given delimiter
// if called like this: getDateTimeString('\0', '\0', '\0');
// it will give back this: 20161231235959  (YYYYMMDDHHMMSS)
String formatDateTimeString(const struct tm& ts, char dateDelimiter = '-', char timeDelimiter = ':',  char dateTimeDelimiter = ' ', bool am_pm = false);


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