#include "TimeESPeasy.h"

double    sysTime = 0.0;             // Use high resolution double to get better sync between nodes when using NTP

uint32_t makeTime(const struct tm& tm) {
  // assemble time elements into uint32_t
  // note year argument is offset from 1970 (see macros in time.h to convert to other formats)
  // previous version used full four digit year (or digits since 2000),i.e. 2009 was 2009 or 9
  const uint8_t monthDays[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
  int i;
  uint32_t seconds;

  // seconds from 1970 till 1 jan 00:00:00 of the given year
  seconds = tm.tm_year * (SECS_PER_DAY * 365);

  for (i = 0; i < tm.tm_year; i++) {
    if (LEAP_YEAR(i)) {
      seconds +=  SECS_PER_DAY; // add extra days for leap years
    }
  }

  // add days for this year, months start from 1
  for (i = 1; i < tm.tm_mon; i++) {
    if ((i == 2) && LEAP_YEAR(tm.tm_year)) {
      seconds += SECS_PER_DAY * 29;
    } else {
      seconds += SECS_PER_DAY * monthDays[i - 1]; // monthDay array starts from 0
    }
  }
  seconds += (tm.tm_mday - 1) * SECS_PER_DAY;
  seconds += tm.tm_hour * SECS_PER_HOUR;
  seconds += tm.tm_min * SECS_PER_MIN;
  seconds += tm.tm_sec;
  return (uint32_t)seconds;
}

