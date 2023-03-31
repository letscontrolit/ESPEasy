#include "../Helpers/ESPEasy_time_calc.h"

#include <Arduino.h>
#include <limits.h>

#include "../Globals/ESPEasy_time.h"
#include "../Helpers/StringConverter.h"


#define SECS_PER_MIN  (60UL)
#define SECS_PER_HOUR (3600UL)
#define SECS_PER_DAY  (SECS_PER_HOUR * 24UL)


bool isLeapYear(int year) {
  return ((year > 0) && !(year % 4) && ((year % 100) || !(year % 400)));
}

uint8_t getMonthDays(int year, uint8_t month) {
  const uint8_t monthDays[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
  if (month == 1 && isLeapYear(year)) {
    return 29;
  }
  if (month > 11) {
    return 0;
  }
  return monthDays[month];
}

/********************************************************************************************\
   Unix Time computations
 \*********************************************************************************************/

uint32_t makeTime(const struct tm& tm) {
  // assemble time elements into uint32_t
  // note year argument is offset from 1970 (see macros in time.h to convert to other formats)
  // previous version used full four digit year (or digits since 2000),i.e. 2009 was 2009 or 9
  const int tm_year = tm.tm_year + 1900;

  // seconds from 1970 till 1 jan 00:00:00 of the given year
  // tm_year starts at 1900
  uint32_t seconds = 1577836800; // 01/01/2020 @ 12:00am (UTC)
  int year = 2020;
  if (tm_year < year) {
    // Just in case this function is called on old dates
    year = 1970;
    seconds = 0;
  }

  for (; year < tm_year; ++year) {
    seconds += SECS_PER_DAY * 365;
    if (isLeapYear(year)) {
      seconds += SECS_PER_DAY; // add extra days for leap years
    }
  }

  // add days for this year, months start from 0
  for (int i = 0; i < tm.tm_mon; i++) {
    seconds += SECS_PER_DAY * getMonthDays(tm_year, i);
  }
  seconds += (tm.tm_mday - 1) * SECS_PER_DAY;
  seconds += tm.tm_hour * SECS_PER_HOUR;
  seconds += tm.tm_min * SECS_PER_MIN;
  seconds += tm.tm_sec;
  return seconds;
}

void breakTime(unsigned long timeInput, struct tm& tm) {
  uint32_t time = (uint32_t)timeInput;
  tm.tm_sec  = time % 60;
  time      /= 60;                   // now it is minutes
  tm.tm_min  = time % 60;
  time      /= 60;                   // now it is hours
  tm.tm_hour = time % 24;
  time      /= 24;                   // now it is days
  tm.tm_wday = ((time + 4) % 7) + 1; // Sunday is day 1

  int      year = 1970;
  unsigned long days = 0;
  while ((unsigned)(days += (isLeapYear(year) ? 366 : 365)) <= time) {
    year++;
  }
  tm.tm_year = year - 1900; // tm_year starts at 1900

  days -= isLeapYear(year) ? 366 : 365;
  time -= days;      // now it is days in this year, starting at 0

  uint8_t month = 0;
  for (month = 0; month < 12; month++) {
    const uint8_t monthLength = getMonthDays(year, month);
    if (time >= monthLength) {
      time -= monthLength;
    } else {
      break;
    }
  }
  tm.tm_mon  = month;     // Jan is month 0
  tm.tm_mday = time + 1;  // day of month start at 1
}


String formatDateString(const struct tm& ts, char delimiter) {
  // time format example with ':' delimiter: 23:59:59 (HH:MM:SS)
  char DateString[20]; // 19 digits plus the null char
  const int year = 1900 + ts.tm_year;
  if (delimiter == '\0') {
    sprintf_P(DateString, PSTR("%4d%02d%02d"), year, ts.tm_mon + 1, ts.tm_mday);
  } else {
    sprintf_P(DateString, PSTR("%4d%c%02d%c%02d"), year, delimiter, ts.tm_mon + 1, delimiter, ts.tm_mday);
  }
  return DateString;
}


// returns the current Time separated by the given delimiter
// time format example with ':' delimiter: 23:59:59 (HH:MM:SS)
String formatTimeString(const struct tm& ts, char delimiter, bool am_pm, bool show_seconds, char hour_prefix /*='\0'*/)
{
  char TimeString[20]; // 19 digits plus the null char
  char hour_prefix_s[2] = { 0 };

  if (am_pm) {
    uint8_t hour(ts.tm_hour % 12);

    if (hour == 0) { hour = 12; }
    const char a_or_p = ts.tm_hour < 12 ? 'A' : 'P';
    if (hour < 10) { hour_prefix_s[0] = hour_prefix; }

    if (show_seconds) {
      if (delimiter == '\0') {
        sprintf_P(TimeString, PSTR("%s%d%02d%02d %cM"),
                  hour_prefix_s, hour, ts.tm_min, ts.tm_sec, a_or_p);
      } else {
        sprintf_P(TimeString, PSTR("%s%d%c%02d%c%02d %cM"),
                  hour_prefix_s, hour, delimiter, ts.tm_min, delimiter, ts.tm_sec, a_or_p);
      }
    } else {
      if (delimiter == '\0') {
        sprintf_P(TimeString, PSTR("%s%d%02d %cM"),
                  hour_prefix_s, hour, ts.tm_min, a_or_p);
      } else {
        sprintf_P(TimeString, PSTR("%s%d%c%02d %cM"),
                  hour_prefix_s, hour, delimiter, ts.tm_min, a_or_p);
      }
    }
  } else {
    if (show_seconds) {
      if (delimiter == '\0') {
        sprintf_P(TimeString, PSTR("%02d%02d%02d"),
                  ts.tm_hour, ts.tm_min, ts.tm_sec);
      } else {
        sprintf_P(TimeString, PSTR("%02d%c%02d%c%02d"),
                  ts.tm_hour, delimiter, ts.tm_min, delimiter, ts.tm_sec);
      }
    } else {
      if (ts.tm_hour < 10) { hour_prefix_s[0] = hour_prefix; }
      if (delimiter == '\0') {
        sprintf_P(TimeString, PSTR("%s%d%02d"),
                  hour_prefix_s, ts.tm_hour, ts.tm_min);
      } else {
        sprintf_P(TimeString, PSTR("%s%d%c%02d"),
                  hour_prefix_s, ts.tm_hour, delimiter, ts.tm_min);
      }
    }
  }
  return TimeString;
}


String formatDateTimeString(const struct tm& ts, char dateDelimiter, char timeDelimiter,  char dateTimeDelimiter, bool am_pm)
{
  // if called like this: getDateTimeString('\0', '\0', '\0');
  // it will give back this: 20161231235959  (YYYYMMDDHHMMSS)
  String ret = formatDateString(ts, dateDelimiter);

  if (dateTimeDelimiter != '\0') {
    ret += dateTimeDelimiter;
  }
  ret += formatTimeString(ts, timeDelimiter, am_pm, true);
  return ret;
}

/********************************************************************************************\
   Time computations for rules.
 \*********************************************************************************************/

String timeLong2String(unsigned long lngTime)
{
  unsigned long x = 0;
  String time;

  x = (lngTime >> 16) & 0xf;

  if (x == 0x0f) {
    x = 0;
  }
  String weekDays = F("AllSunMonTueWedThuFriSatWrkWkd");
  time  = weekDays.substring(x * 3, x * 3 + 3);
  time += ',';

  x = (lngTime >> 12) & 0xf;

  if (x == 0xf) {
    time += '*';
  }
  else if (x == 0xe) {
    time += '-';
  }
  else {
    time += x;
  }

  x = (lngTime >> 8) & 0xf;

  if (x == 0xf) {
    time += '*';
  }
  else if (x == 0xe) {
    time += '-';
  }
  else {
    time += x;
  }

  time += ':';

  x = (lngTime >> 4) & 0xf;

  if (x == 0xf) {
    time += '*';
  }
  else if (x == 0xe) {
    time += '-';
  }
  else {
    time += x;
  }

  x = (lngTime) & 0xf;

  if (x == 0xf) {
    time += '*';
  }
  else if (x == 0xe) {
    time += '-';
  }
  else {
    time += x;
  }

  return time;
}


unsigned long string2TimeLong(const String& str)
{
  // format 0000WWWWAAAABBBBCCCCDDDD
  // WWWW=weekday, AAAA=hours tens digit, BBBB=hours, CCCC=minutes tens digit DDDD=minutes

  char command[20];
  int  w, x, y;
  unsigned long a;
  {
    // Within a scope so the tmpString is only used for copy.
    String tmpString(str);
    tmpString.toLowerCase();
    tmpString.toCharArray(command, 20);
  }
  unsigned long lngTime = 0;
  String TmpStr1;

  if (GetArgv(command, TmpStr1, 1))
  {
    String day      = TmpStr1;
    String weekDays = F("allsunmontuewedthufrisatwrkwkd");
    y = weekDays.indexOf(TmpStr1) / 3;

    if (y == 0) {
      y = 0xf; // wildcard is 0xf
    }
    lngTime |= (unsigned long)y << 16;
  }

  if (GetArgv(command, TmpStr1, 2))
  {
    y = 0;

    for (x = TmpStr1.length() - 1; x >= 0; x--)
    {
      w = TmpStr1[x];

      if (isDigit(w) || (w == '*'))
      {
        a        = 0xffffffff  ^ (0xfUL << y); // create mask to clean nibble position y
        lngTime &= a;                          // maak nibble leeg

        if (w == '*') {
          lngTime |= (0xFUL << y);             // fill nibble with wildcard value
        }
        else {
          lngTime |= (w - '0') << y;           // fill nibble with token
        }
        y += 4;
      }
      else
      if (w == ':') {}
      else
      {
        break;
      }
    }
  }
  #undef TmpStr1Length
  return lngTime;
}



/********************************************************************************************\
   Match clock event
 \*********************************************************************************************/
bool matchClockEvent(unsigned long clockEvent, unsigned long clockSet)
{
  unsigned long Mask;

  for (uint8_t y = 0; y < 8; y++)
  {
    if (((clockSet >> (y * 4)) & 0xf) == 0xf)         // if nibble y has the wildcard value 0xf
    {
      Mask        = 0xffffffff  ^ (0xFUL << (y * 4)); // Mask to wipe nibble position y.
      clockEvent &= Mask;                             // clear nibble
      clockEvent |= (0xFUL << (y * 4));               // fill with wildcard value 0xf
    }
  }

  if (((clockSet >> (16)) & 0xf) == 0x8) {         // if weekday nibble has the wildcard value 0x8 (workdays)
    if (node_time.weekday() >= 2 && node_time.weekday() <= 6)         // and we have a working day today...
    {
      Mask        = 0xffffffff  ^ (0xFUL << (16)); // Mask to wipe nibble position.
      clockEvent &= Mask;                          // clear nibble
      clockEvent |= (0x8UL << (16));               // fill with wildcard value 0x8
    }
  }

  if (((clockSet >> (16)) & 0xf) == 0x9) {         // if weekday nibble has the wildcard value 0x9 (weekends)
    if (node_time.weekday() == 1 || node_time.weekday() == 7)          // and we have a weekend day today...
    {
      Mask        = 0xffffffff  ^ (0xFUL << (16)); // Mask to wipe nibble position.
      clockEvent &= Mask;                          // clear nibble
      clockEvent |= (0x9UL << (16));               // fill with wildcard value 0x9
    }
  }

  return (clockEvent == clockSet);
}
