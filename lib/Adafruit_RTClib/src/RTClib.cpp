/**************************************************************************/
/*!
  @file     RTClib.cpp

  @mainpage Adafruit RTClib

  @section intro Introduction

  This is a fork of JeeLab's fantastic real time clock library for Arduino.

  For details on using this library with an RTC module like the DS1307, PCF8523,
  or DS3231, see the guide at:
  https://learn.adafruit.com/ds1307-real-time-clock-breakout-board-kit/overview

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  @section classes Available classes

  This library provides the following classes:

  - Classes for manipulating dates, times and durations:
    - DateTime represents a specific point in time; this is the data
      type used for setting and reading the supported RTCs
    - TimeSpan represents the length of a time interval
  - Interfacing specific RTC chips:
    - RTC_DS1307
    - RTC_DS3231
    - RTC_PCF8523
  - RTC emulated in software; do not expect much accuracy out of these:
    - RTC_Millis is based on `millis()`
    - RTC_Micros is based on `micros()`; its drift rate can be tuned by
      the user

  @section license License

  Original library by JeeLabs https://jeelabs.org/pub/docs/rtclib/, released to
  the public domain.

  This version: MIT (see LICENSE)
*/
/**************************************************************************/

#include "RTClib.h"
#include <Wire.h>

#ifdef __AVR__
#include <avr/pgmspace.h>
#elif defined(ESP8266)
#include <pgmspace.h>
#elif defined(ARDUINO_ARCH_SAMD)
// nothing special needed
#elif defined(ARDUINO_SAM_DUE)
#define PROGMEM
#define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#define Wire Wire1
#endif

#if (ARDUINO >= 100)
#define _I2C_WRITE write ///< Modern I2C write
#define _I2C_READ read   ///< Modern I2C read
#else
#include <WProgram.h>
#define _I2C_WRITE send   ///< Legacy I2C write
#define _I2C_READ receive ///< legacy I2C read
#endif

/**************************************************************************/
/*!
    @brief  Read a byte from an I2C register
    @param addr I2C address
    @param reg Register address
    @return Register value
*/
/**************************************************************************/
static uint8_t read_i2c_register(uint8_t addr, uint8_t reg,
                                 TwoWire *wireInstance) {
  wireInstance->beginTransmission(addr);
  wireInstance->_I2C_WRITE((byte)reg);
  wireInstance->endTransmission();

  wireInstance->requestFrom(addr, (byte)1);
  return wireInstance->_I2C_READ();
}

/**************************************************************************/
/*!
    @brief  Write a byte to an I2C register
    @param addr I2C address
    @param reg Register address
    @param val Value to write
*/
/**************************************************************************/
static void write_i2c_register(uint8_t addr, uint8_t reg, uint8_t val,
                               TwoWire *wireInstance) {
  wireInstance->beginTransmission(addr);
  wireInstance->_I2C_WRITE((byte)reg);
  wireInstance->_I2C_WRITE((byte)val);
  wireInstance->endTransmission();
}

/**************************************************************************/
// utility code, some of this could be exposed in the DateTime API if needed
/**************************************************************************/

/**
  Number of days in each month, from January to November. December is not
  needed. Omitting it avoids an incompatibility with Paul Stoffregen's Time
  library. C.f. https://github.com/adafruit/RTClib/issues/114
*/
const uint8_t daysInMonth[] PROGMEM = {31, 28, 31, 30, 31, 30,
                                       31, 31, 30, 31, 30};

/**************************************************************************/
/*!
    @brief  Given a date, return number of days since 2000/01/01,
            valid for 2000--2099
    @param y Year
    @param m Month
    @param d Day
    @return Number of days
*/
/**************************************************************************/
static uint16_t date2days(uint16_t y, uint8_t m, uint8_t d) {
  if (y >= 2000U)
    y -= 2000U;
  uint16_t days = d;
  for (uint8_t i = 1; i < m; ++i)
    days += pgm_read_byte(daysInMonth + i - 1);
  if (m > 2 && y % 4 == 0)
    ++days;
  return days + 365 * y + (y + 3) / 4 - 1;
}

/**************************************************************************/
/*!
    @brief  Given a number of days, hours, minutes, and seconds, return the
   total seconds
    @param days Days
    @param h Hours
    @param m Minutes
    @param s Seconds
    @return Number of seconds total
*/
/**************************************************************************/
static uint32_t time2ulong(uint16_t days, uint8_t h, uint8_t m, uint8_t s) {
  return ((days * 24UL + h) * 60 + m) * 60 + s;
}

/**************************************************************************/
/*!
    @brief  Constructor from
        [Unix time](https://en.wikipedia.org/wiki/Unix_time).

    This builds a DateTime from an integer specifying the number of seconds
    elapsed since the epoch: 1970-01-01 00:00:00. This number is analogous
    to Unix time, with two small differences:

     - The Unix epoch is specified to be at 00:00:00
       [UTC](https://en.wikipedia.org/wiki/Coordinated_Universal_Time),
       whereas this class has no notion of time zones. The epoch used in
       this class is then at 00:00:00 on whatever time zone the user chooses
       to use, ignoring changes in DST.

     - Unix time is conventionally represented with signed numbers, whereas
       this constructor takes an unsigned argument. Because of this, it does
       _not_ suffer from the
       [year 2038 problem](https://en.wikipedia.org/wiki/Year_2038_problem).

    If called without argument, it returns the earliest time representable
    by this class: 2000-01-01 00:00:00.

    @see The `unixtime()` method is the converse of this constructor.

    @param t Time elapsed in seconds since 1970-01-01 00:00:00.
*/
/**************************************************************************/
DateTime::DateTime(uint32_t t) {
  t -= SECONDS_FROM_1970_TO_2000; // bring to 2000 timestamp from 1970

  ss = t % 60;
  t /= 60;
  mm = t % 60;
  t /= 60;
  hh = t % 24;
  uint16_t days = t / 24;
  uint8_t leap;
  for (yOff = 0;; ++yOff) {
    leap = yOff % 4 == 0;
    if (days < 365U + leap)
      break;
    days -= 365 + leap;
  }
  for (m = 1; m < 12; ++m) {
    uint8_t daysPerMonth = pgm_read_byte(daysInMonth + m - 1);
    if (leap && m == 2)
      ++daysPerMonth;
    if (days < daysPerMonth)
      break;
    days -= daysPerMonth;
  }
  d = days + 1;
}

/**************************************************************************/
/*!
    @brief  Constructor from (year, month, day, hour, minute, second).
    @warning If the provided parameters are not valid (e.g. 31 February),
           the constructed DateTime will be invalid.
    @see   The `isValid()` method can be used to test whether the
           constructed DateTime is valid.
    @param year Either the full year (range: 2000--2099) or the offset from
        year 2000 (range: 0--99).
    @param month Month number (1--12).
    @param day Day of the month (1--31).
    @param hour,min,sec Hour (0--23), minute (0--59) and second (0--59).
*/
/**************************************************************************/
DateTime::DateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour,
                   uint8_t min, uint8_t sec) {
  if (year >= 2000U)
    year -= 2000U;
  yOff = year;
  m = month;
  d = day;
  hh = hour;
  mm = min;
  ss = sec;
}

/**************************************************************************/
/*!
    @brief  Copy constructor.
    @param copy DateTime to copy.
*/
/**************************************************************************/
DateTime::DateTime(const DateTime &copy)
    : yOff(copy.yOff), m(copy.m), d(copy.d), hh(copy.hh), mm(copy.mm),
      ss(copy.ss) {}

/**************************************************************************/
/*!
    @brief  Convert a string containing two digits to uint8_t, e.g. "09" returns
   9
    @param p Pointer to a string containing two digits
*/
/**************************************************************************/
static uint8_t conv2d(const char *p) {
  uint8_t v = 0;
  if ('0' <= *p && *p <= '9')
    v = *p - '0';
  return 10 * v + *++p - '0';
}

/**************************************************************************/
/*!
    @brief  Constructor for generating the build time.

    This constructor expects its parameters to be strings in the format
    generated by the compiler's preprocessor macros `__DATE__` and
    `__TIME__`. Usage:

    ```
    DateTime buildTime(__DATE__, __TIME__);
    ```

    @note The `F()` macro can be used to reduce the RAM footprint, see
        the next constructor.

    @param date Date string, e.g. "Apr 16 2020".
    @param time Time string, e.g. "18:34:56".
*/
/**************************************************************************/
DateTime::DateTime(const char *date, const char *time) {
  yOff = conv2d(date + 9);
  // Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec
  switch (date[0]) {
  case 'J':
    m = (date[1] == 'a') ? 1 : ((date[2] == 'n') ? 6 : 7);
    break;
  case 'F':
    m = 2;
    break;
  case 'A':
    m = date[2] == 'r' ? 4 : 8;
    break;
  case 'M':
    m = date[2] == 'r' ? 3 : 5;
    break;
  case 'S':
    m = 9;
    break;
  case 'O':
    m = 10;
    break;
  case 'N':
    m = 11;
    break;
  case 'D':
    m = 12;
    break;
  }
  d = conv2d(date + 4);
  hh = conv2d(time);
  mm = conv2d(time + 3);
  ss = conv2d(time + 6);
}

/**************************************************************************/
/*!
    @brief  Memory friendly constructor for generating the build time.

    This version is intended to save RAM by keeping the date and time
    strings in program memory. Use it with the `F()` macro:

    ```
    DateTime buildTime(F(__DATE__), F(__TIME__));
    ```

    @param date Date PROGMEM string, e.g. F("Apr 16 2020").
    @param time Time PROGMEM string, e.g. F("18:34:56").
*/
/**************************************************************************/
DateTime::DateTime(const __FlashStringHelper *date,
                   const __FlashStringHelper *time) {
  char buff[11];
  memcpy_P(buff, date, 11);
  yOff = conv2d(buff + 9);
  // Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec
  switch (buff[0]) {
  case 'J':
    m = (buff[1] == 'a') ? 1 : ((buff[2] == 'n') ? 6 : 7);
    break;
  case 'F':
    m = 2;
    break;
  case 'A':
    m = buff[2] == 'r' ? 4 : 8;
    break;
  case 'M':
    m = buff[2] == 'r' ? 3 : 5;
    break;
  case 'S':
    m = 9;
    break;
  case 'O':
    m = 10;
    break;
  case 'N':
    m = 11;
    break;
  case 'D':
    m = 12;
    break;
  }
  d = conv2d(buff + 4);
  memcpy_P(buff, time, 8);
  hh = conv2d(buff);
  mm = conv2d(buff + 3);
  ss = conv2d(buff + 6);
}

/**************************************************************************/
/*!
    @brief  Constructor for creating a DateTime from an ISO8601 date string.

    This constructor expects its parameters to be a string in the
    https://en.wikipedia.org/wiki/ISO_8601 format, e.g:

    "2020-06-25T15:29:37"

    Usage:

    ```
    DateTime dt("2020-06-25T15:29:37");
    ```

    @note The year must be > 2000, as only the yOff is considered.

    @param iso8601dateTime
           A dateTime string in iso8601 format,
           e.g. "2020-06-25T15:29:37".

*/
/**************************************************************************/
DateTime::DateTime(const char *iso8601dateTime) {
  char ref[] = "2000-01-01T00:00:00";
  memcpy(ref, iso8601dateTime, min(strlen(ref), strlen(iso8601dateTime)));
  yOff = conv2d(ref + 2);
  m = conv2d(ref + 5);
  d = conv2d(ref + 8);
  hh = conv2d(ref + 11);
  mm = conv2d(ref + 14);
  ss = conv2d(ref + 17);
}

/**************************************************************************/
/*!
    @brief  Check whether this DateTime is valid.
    @return true if valid, false if not.
*/
/**************************************************************************/
bool DateTime::isValid() const {
  if (yOff >= 100)
    return false;
  DateTime other(unixtime());
  return yOff == other.yOff && m == other.m && d == other.d && hh == other.hh &&
         mm == other.mm && ss == other.ss;
}

/**************************************************************************/
/*!
    @brief  Writes the DateTime as a string in a user-defined format.

    The _buffer_ parameter should be initialized by the caller with a string
    specifying the requested format. This format string may contain any of
    the following specifiers:

    | specifier | output                                                 |
    |-----------|--------------------------------------------------------|
    | YYYY      | the year as a 4-digit number (2000--2099)              |
    | YY        | the year as a 2-digit number (00--99)                  |
    | MM        | the month as a 2-digit number (01--12)                 |
    | MMM       | the abbreviated English month name ("Jan"--"Dec")      |
    | DD        | the day as a 2-digit number (01--31)                   |
    | DDD       | the abbreviated English day of the week ("Mon"--"Sun") |
    | AP        | either "AM" or "PM"                                    |
    | ap        | either "am" or "pm"                                    |
    | hh        | the hour as a 2-digit number (00--23 or 01--12)        |
    | mm        | the minute as a 2-digit number (00--59)                |
    | ss        | the second as a 2-digit number (00--59)                |

    If either "AP" or "ap" is used, the "hh" specifier uses 12-hour mode
    (range: 01--12). Otherwise it works in 24-hour mode (range: 00--23).

    The specifiers within _buffer_ will be overwritten with the appropriate
    values from the DateTime. Any characters not belonging to one of the
    above specifiers are left as-is.

    __Example__: The format "DDD, DD MMM YYYY hh:mm:ss" generates an output
    of the form "Thu, 16 Apr 2020 18:34:56.

    @see The `timestamp()` method provides similar functionnality, but it
        returns a `String` object and supports a limited choice of
        predefined formats.

    @param[in,out] buffer Array of `char` for holding the format description
        and the formatted DateTime. Before calling this method, the buffer
        should be initialized by the user with the format string. The method
        will overwrite the buffer with the formatted date and/or time.

    @return A pointer to the provided buffer. This is returned for
        convenience, in order to enable idioms such as
        `Serial.println(now.toString(buffer));`
*/
/**************************************************************************/

char *DateTime::toString(char *buffer) {
  uint8_t apTag =
      (strstr(buffer, "ap") != nullptr) || (strstr(buffer, "AP") != nullptr);
  uint8_t hourReformatted = 0, isPM = false;
  if (apTag) {     // 12 Hour Mode
    if (hh == 0) { // midnight
      isPM = false;
      hourReformatted = 12;
    } else if (hh == 12) { // noon
      isPM = true;
      hourReformatted = 12;
    } else if (hh < 12) { // morning
      isPM = false;
      hourReformatted = hh;
    } else { // 1 o'clock or after
      isPM = true;
      hourReformatted = hh - 12;
    }
  }

  for (size_t i = 0; i < strlen(buffer) - 1; i++) {
    if (buffer[i] == 'h' && buffer[i + 1] == 'h') {
      if (!apTag) { // 24 Hour Mode
        buffer[i] = '0' + hh / 10;
        buffer[i + 1] = '0' + hh % 10;
      } else { // 12 Hour Mode
        buffer[i] = '0' + hourReformatted / 10;
        buffer[i + 1] = '0' + hourReformatted % 10;
      }
    }
    if (buffer[i] == 'm' && buffer[i + 1] == 'm') {
      buffer[i] = '0' + mm / 10;
      buffer[i + 1] = '0' + mm % 10;
    }
    if (buffer[i] == 's' && buffer[i + 1] == 's') {
      buffer[i] = '0' + ss / 10;
      buffer[i + 1] = '0' + ss % 10;
    }
    if (buffer[i] == 'D' && buffer[i + 1] == 'D' && buffer[i + 2] == 'D') {
      static PROGMEM const char day_names[] = "SunMonTueWedThuFriSat";
      const char *p = &day_names[3 * dayOfTheWeek()];
      buffer[i] = pgm_read_byte(p);
      buffer[i + 1] = pgm_read_byte(p + 1);
      buffer[i + 2] = pgm_read_byte(p + 2);
    } else if (buffer[i] == 'D' && buffer[i + 1] == 'D') {
      buffer[i] = '0' + d / 10;
      buffer[i + 1] = '0' + d % 10;
    }
    if (buffer[i] == 'M' && buffer[i + 1] == 'M' && buffer[i + 2] == 'M') {
      static PROGMEM const char month_names[] =
          "JanFebMarAprMayJunJulAugSepOctNovDec";
      const char *p = &month_names[3 * (m - 1)];
      buffer[i] = pgm_read_byte(p);
      buffer[i + 1] = pgm_read_byte(p + 1);
      buffer[i + 2] = pgm_read_byte(p + 2);
    } else if (buffer[i] == 'M' && buffer[i + 1] == 'M') {
      buffer[i] = '0' + m / 10;
      buffer[i + 1] = '0' + m % 10;
    }
    if (buffer[i] == 'Y' && buffer[i + 1] == 'Y' && buffer[i + 2] == 'Y' &&
        buffer[i + 3] == 'Y') {
      buffer[i] = '2';
      buffer[i + 1] = '0';
      buffer[i + 2] = '0' + (yOff / 10) % 10;
      buffer[i + 3] = '0' + yOff % 10;
    } else if (buffer[i] == 'Y' && buffer[i + 1] == 'Y') {
      buffer[i] = '0' + (yOff / 10) % 10;
      buffer[i + 1] = '0' + yOff % 10;
    }
    if (buffer[i] == 'A' && buffer[i + 1] == 'P') {
      if (isPM) {
        buffer[i] = 'P';
        buffer[i + 1] = 'M';
      } else {
        buffer[i] = 'A';
        buffer[i + 1] = 'M';
      }
    } else if (buffer[i] == 'a' && buffer[i + 1] == 'p') {
      if (isPM) {
        buffer[i] = 'p';
        buffer[i + 1] = 'm';
      } else {
        buffer[i] = 'a';
        buffer[i + 1] = 'm';
      }
    }
  }
  return buffer;
}

/**************************************************************************/
/*!
      @brief  Return the hour in 12-hour format.
      @return Hour (1--12).
*/
/**************************************************************************/
uint8_t DateTime::twelveHour() const {
  if (hh == 0 || hh == 12) { // midnight or noon
    return 12;
  } else if (hh > 12) { // 1 o'clock or later
    return hh - 12;
  } else { // morning
    return hh;
  }
}

/**************************************************************************/
/*!
    @brief  Return the day of the week.
    @return Day of week as an integer from 0 (Sunday) to 6 (Saturday).
*/
/**************************************************************************/
uint8_t DateTime::dayOfTheWeek() const {
  uint16_t day = date2days(yOff, m, d);
  return (day + 6) % 7; // Jan 1, 2000 is a Saturday, i.e. returns 6
}

/**************************************************************************/
/*!
    @brief  Return Unix time: seconds since 1 Jan 1970.

    @see The `DateTime::DateTime(uint32_t)` constructor is the converse of
        this method.

    @return Number of seconds since 1970-01-01 00:00:00.
*/
/**************************************************************************/
uint32_t DateTime::unixtime(void) const {
  uint32_t t;
  uint16_t days = date2days(yOff, m, d);
  t = time2ulong(days, hh, mm, ss);
  t += SECONDS_FROM_1970_TO_2000; // seconds from 1970 to 2000

  return t;
}

/**************************************************************************/
/*!
    @brief  Convert the DateTime to seconds since 1 Jan 2000

    The result can be converted back to a DateTime with:

    ```cpp
    DateTime(SECONDS_FROM_1970_TO_2000 + value)
    ```

    @return Number of seconds since 2000-01-01 00:00:00.
*/
/**************************************************************************/
uint32_t DateTime::secondstime(void) const {
  uint32_t t;
  uint16_t days = date2days(yOff, m, d);
  t = time2ulong(days, hh, mm, ss);
  return t;
}

/**************************************************************************/
/*!
    @brief  Add a TimeSpan to the DateTime object
    @param span TimeSpan object
    @return New DateTime object with span added to it.
*/
/**************************************************************************/
DateTime DateTime::operator+(const TimeSpan &span) {
  return DateTime(unixtime() + span.totalseconds());
}

/**************************************************************************/
/*!
    @brief  Subtract a TimeSpan from the DateTime object
    @param span TimeSpan object
    @return New DateTime object with span subtracted from it.
*/
/**************************************************************************/
DateTime DateTime::operator-(const TimeSpan &span) {
  return DateTime(unixtime() - span.totalseconds());
}

/**************************************************************************/
/*!
    @brief  Subtract one DateTime from another

    @note Since a TimeSpan cannot be negative, the subtracted DateTime
        should be less (earlier) than or equal to the one it is
        subtracted from.

    @param right The DateTime object to subtract from self (the left object)
    @return TimeSpan of the difference between DateTimes.
*/
/**************************************************************************/
TimeSpan DateTime::operator-(const DateTime &right) {
  return TimeSpan(unixtime() - right.unixtime());
}

/**************************************************************************/
/*!
    @author Anton Rieutskyi
    @brief  Test if one DateTime is less (earlier) than another.
    @warning if one or both DateTime objects are invalid, returned value is
        meaningless
    @see use `isValid()` method to check if DateTime object is valid
    @param right Comparison DateTime object
    @return True if the left DateTime is earlier than the right one,
        false otherwise.
*/
/**************************************************************************/
bool DateTime::operator<(const DateTime &right) const {
  return (yOff + 2000U < right.year() ||
          (yOff + 2000U == right.year() &&
           (m < right.month() ||
            (m == right.month() &&
             (d < right.day() ||
              (d == right.day() &&
               (hh < right.hour() ||
                (hh == right.hour() &&
                 (mm < right.minute() ||
                  (mm == right.minute() && ss < right.second()))))))))));
}

/**************************************************************************/
/*!
    @author Anton Rieutskyi
    @brief  Test if two DateTime objects are equal.
    @warning if one or both DateTime objects are invalid, returned value is
        meaningless
    @see use `isValid()` method to check if DateTime object is valid
    @param right Comparison DateTime object
    @return True if both DateTime objects are the same, false otherwise.
*/
/**************************************************************************/
bool DateTime::operator==(const DateTime &right) const {
  return (right.year() == yOff + 2000U && right.month() == m &&
          right.day() == d && right.hour() == hh && right.minute() == mm &&
          right.second() == ss);
}

/**************************************************************************/
/*!
    @brief  Return a ISO 8601 timestamp as a `String` object.

    The generated timestamp conforms to one of the predefined, ISO
    8601-compatible formats for representing the date (if _opt_ is
    `TIMESTAMP_DATE`), the time (`TIMESTAMP_TIME`), or both
    (`TIMESTAMP_FULL`).

    @see The `toString()` method provides more general string formatting.

    @param opt Format of the timestamp
    @return Timestamp string, e.g. "2020-04-16T18:34:56".
*/
/**************************************************************************/
String DateTime::timestamp(timestampOpt opt) {
  char buffer[25]; // large enough for any DateTime, including invalid ones

  // Generate timestamp according to opt
  switch (opt) {
  case TIMESTAMP_TIME:
    // Only time
    sprintf(buffer, "%02d:%02d:%02d", hh, mm, ss);
    break;
  case TIMESTAMP_DATE:
    // Only date
    sprintf(buffer, "%u-%02d-%02d", 2000U + yOff, m, d);
    break;
  default:
    // Full
    sprintf(buffer, "%u-%02d-%02dT%02d:%02d:%02d", 2000U + yOff, m, d, hh, mm,
            ss);
  }
  return String(buffer);
}

/**************************************************************************/
/*!
    @brief  Create a new TimeSpan object in seconds
    @param seconds Number of seconds
*/
/**************************************************************************/
TimeSpan::TimeSpan(int32_t seconds) : _seconds(seconds) {}

/**************************************************************************/
/*!
    @brief  Create a new TimeSpan object using a number of
   days/hours/minutes/seconds e.g. Make a TimeSpan of 3 hours and 45 minutes:
   new TimeSpan(0, 3, 45, 0);
    @param days Number of days
    @param hours Number of hours
    @param minutes Number of minutes
    @param seconds Number of seconds
*/
/**************************************************************************/
TimeSpan::TimeSpan(int16_t days, int8_t hours, int8_t minutes, int8_t seconds)
    : _seconds((int32_t)days * 86400L + (int32_t)hours * 3600 +
               (int32_t)minutes * 60 + seconds) {}

/**************************************************************************/
/*!
    @brief  Copy constructor, make a new TimeSpan using an existing one
    @param copy The TimeSpan to copy
*/
/**************************************************************************/
TimeSpan::TimeSpan(const TimeSpan &copy) : _seconds(copy._seconds) {}

/**************************************************************************/
/*!
    @brief  Add two TimeSpans
    @param right TimeSpan to add
    @return New TimeSpan object, sum of left and right
*/
/**************************************************************************/
TimeSpan TimeSpan::operator+(const TimeSpan &right) {
  return TimeSpan(_seconds + right._seconds);
}

/**************************************************************************/
/*!
    @brief  Subtract a TimeSpan
    @param right TimeSpan to subtract
    @return New TimeSpan object, right subtracted from left
*/
/**************************************************************************/
TimeSpan TimeSpan::operator-(const TimeSpan &right) {
  return TimeSpan(_seconds - right._seconds);
}

/**************************************************************************/
/*!
    @brief  Convert a binary coded decimal value to binary. RTC stores time/date
   values as BCD.
    @param val BCD value
    @return Binary value
*/
/**************************************************************************/
static uint8_t bcd2bin(uint8_t val) { return val - 6 * (val >> 4); }

/**************************************************************************/
/*!
    @brief  Convert a binary value to BCD format for the RTC registers
    @param val Binary value
    @return BCD value
*/
/**************************************************************************/
static uint8_t bin2bcd(uint8_t val) { return val + 6 * (val / 10); }

/**************************************************************************/
/*!
    @brief  Start I2C for the DS1307 and test succesful connection
    @return True if Wire can find DS1307 or false otherwise.
*/
/**************************************************************************/

boolean RTC_DS1307::begin(TwoWire *wireInstance) {
  RTCWireBus = wireInstance;
  //RTCWireBus->begin();
  RTCWireBus->beginTransmission(DS1307_ADDRESS);
  if (RTCWireBus->endTransmission() == 0)
    return true;
  return false;
}

/**************************************************************************/
/*!
    @brief  Is the DS1307 running? Check the Clock Halt bit in register 0
    @return 1 if the RTC is running, 0 if not
*/
/**************************************************************************/
uint8_t RTC_DS1307::isrunning(void) {
  RTCWireBus->beginTransmission(DS1307_ADDRESS);
  RTCWireBus->_I2C_WRITE((byte)0);
  RTCWireBus->endTransmission();

  RTCWireBus->requestFrom(DS1307_ADDRESS, 1);
  uint8_t ss = RTCWireBus->_I2C_READ();
  return !(ss >> 7);
}

/**************************************************************************/
/*!
    @brief  Set the date and time in the DS1307
    @param dt DateTime object containing the desired date/time
*/
/**************************************************************************/
void RTC_DS1307::adjust(const DateTime &dt) {
  RTCWireBus->beginTransmission(DS1307_ADDRESS);
  RTCWireBus->_I2C_WRITE((byte)0); // start at location 0
  RTCWireBus->_I2C_WRITE(bin2bcd(dt.second()));
  RTCWireBus->_I2C_WRITE(bin2bcd(dt.minute()));
  RTCWireBus->_I2C_WRITE(bin2bcd(dt.hour()));
  RTCWireBus->_I2C_WRITE(bin2bcd(0));
  RTCWireBus->_I2C_WRITE(bin2bcd(dt.day()));
  RTCWireBus->_I2C_WRITE(bin2bcd(dt.month()));
  RTCWireBus->_I2C_WRITE(bin2bcd(dt.year() - 2000U));
  RTCWireBus->endTransmission();
}

/**************************************************************************/
/*!
    @brief  Get the current date and time from the DS1307
    @return DateTime object containing the current date and time
*/
/**************************************************************************/
DateTime RTC_DS1307::now() {
  RTCWireBus->beginTransmission(DS1307_ADDRESS);
  RTCWireBus->_I2C_WRITE((byte)0);
  RTCWireBus->endTransmission();

  RTCWireBus->requestFrom(DS1307_ADDRESS, 7);
  uint8_t ss = bcd2bin(RTCWireBus->_I2C_READ() & 0x7F);
  uint8_t mm = bcd2bin(RTCWireBus->_I2C_READ());
  uint8_t hh = bcd2bin(RTCWireBus->_I2C_READ());
  RTCWireBus->_I2C_READ();
  uint8_t d = bcd2bin(RTCWireBus->_I2C_READ());
  uint8_t m = bcd2bin(RTCWireBus->_I2C_READ());
  uint16_t y = bcd2bin(RTCWireBus->_I2C_READ()) + 2000U;

  return DateTime(y, m, d, hh, mm, ss);
}

/**************************************************************************/
/*!
    @brief  Read the current mode of the SQW pin
    @return Mode as Ds1307SqwPinMode enum
*/
/**************************************************************************/
Ds1307SqwPinMode RTC_DS1307::readSqwPinMode() {
  int mode;

  RTCWireBus->beginTransmission(DS1307_ADDRESS);
  RTCWireBus->_I2C_WRITE(DS1307_CONTROL);
  RTCWireBus->endTransmission();

  RTCWireBus->requestFrom((uint8_t)DS1307_ADDRESS, (uint8_t)1);
  mode = RTCWireBus->_I2C_READ();

  mode &= 0x93;
  return static_cast<Ds1307SqwPinMode>(mode);
}

/**************************************************************************/
/*!
    @brief  Change the SQW pin mode
    @param mode The mode to use
*/
/**************************************************************************/
void RTC_DS1307::writeSqwPinMode(Ds1307SqwPinMode mode) {
  RTCWireBus->beginTransmission(DS1307_ADDRESS);
  RTCWireBus->_I2C_WRITE(DS1307_CONTROL);
  RTCWireBus->_I2C_WRITE(mode);
  RTCWireBus->endTransmission();
}

/**************************************************************************/
/*!
    @brief  Read data from the DS1307's NVRAM
    @param buf Pointer to a buffer to store the data - make sure it's large
   enough to hold size bytes
    @param size Number of bytes to read
    @param address Starting NVRAM address, from 0 to 55
*/
/**************************************************************************/
void RTC_DS1307::readnvram(uint8_t *buf, uint8_t size, uint8_t address) {
  int addrByte = DS1307_NVRAM + address;
  RTCWireBus->beginTransmission(DS1307_ADDRESS);
  RTCWireBus->_I2C_WRITE(addrByte);
  RTCWireBus->endTransmission();

  RTCWireBus->requestFrom((uint8_t)DS1307_ADDRESS, size);
  for (uint8_t pos = 0; pos < size; ++pos) {
    buf[pos] = RTCWireBus->_I2C_READ();
  }
}

/**************************************************************************/
/*!
    @brief  Write data to the DS1307 NVRAM
    @param address Starting NVRAM address, from 0 to 55
    @param buf Pointer to buffer containing the data to write
    @param size Number of bytes in buf to write to NVRAM
*/
/**************************************************************************/
void RTC_DS1307::writenvram(uint8_t address, uint8_t *buf, uint8_t size) {
  int addrByte = DS1307_NVRAM + address;
  RTCWireBus->beginTransmission(DS1307_ADDRESS);
  RTCWireBus->_I2C_WRITE(addrByte);
  for (uint8_t pos = 0; pos < size; ++pos) {
    RTCWireBus->_I2C_WRITE(buf[pos]);
  }
  RTCWireBus->endTransmission();
}

/**************************************************************************/
/*!
    @brief  Shortcut to read one byte from NVRAM
    @param address NVRAM address, 0 to 55
    @return The byte read from NVRAM
*/
/**************************************************************************/
uint8_t RTC_DS1307::readnvram(uint8_t address) {
  uint8_t data;
  readnvram(&data, 1, address);
  return data;
}

/**************************************************************************/
/*!
    @brief  Shortcut to write one byte to NVRAM
    @param address NVRAM address, 0 to 55
    @param data One byte to write
*/
/**************************************************************************/
void RTC_DS1307::writenvram(uint8_t address, uint8_t data) {
  writenvram(address, &data, 1);
}

/** Alignment between the millis() timescale and the Unix timescale. These
  two variables are updated on each call to now(), which prevents
  rollover issues. Note that lastMillis is **not** the millis() value
  of the last call to now(): it's the millis() value corresponding to
  the last **full second** of Unix time. */
uint32_t RTC_Millis::lastMillis;
uint32_t RTC_Millis::lastUnix;

/**************************************************************************/
/*!
    @brief  Set the current date/time of the RTC_Millis clock.
    @param dt DateTime object with the desired date and time
*/
/**************************************************************************/
void RTC_Millis::adjust(const DateTime &dt) {
  lastMillis = millis();
  lastUnix = dt.unixtime();
}

/**************************************************************************/
/*!
    @brief  Return a DateTime object containing the current date/time.
            Note that computing (millis() - lastMillis) is rollover-safe as long
            as this method is called at least once every 49.7 days.
    @return DateTime object containing current time
*/
/**************************************************************************/
DateTime RTC_Millis::now() {
  uint32_t elapsedSeconds = (millis() - lastMillis) / 1000;
  lastMillis += elapsedSeconds * 1000;
  lastUnix += elapsedSeconds;
  return lastUnix;
}

/** Number of microseconds reported by micros() per "true" (calibrated) second.
 */
uint32_t RTC_Micros::microsPerSecond = 1000000;

/** The timing logic is identical to RTC_Millis. */
uint32_t RTC_Micros::lastMicros;
uint32_t RTC_Micros::lastUnix;

/**************************************************************************/
/*!
    @brief  Set the current date/time of the RTC_Micros clock.
    @param dt DateTime object with the desired date and time
*/
/**************************************************************************/
void RTC_Micros::adjust(const DateTime &dt) {
  lastMicros = micros();
  lastUnix = dt.unixtime();
}

/**************************************************************************/
/*!
    @brief  Adjust the RTC_Micros clock to compensate for system clock drift
    @param ppm Adjustment to make
*/
/**************************************************************************/
// A positive adjustment makes the clock faster.
void RTC_Micros::adjustDrift(int ppm) { microsPerSecond = 1000000 - ppm; }

/**************************************************************************/
/*!
    @brief  Get the current date/time from the RTC_Micros clock.
    @return DateTime object containing the current date/time
*/
/**************************************************************************/
DateTime RTC_Micros::now() {
  uint32_t elapsedSeconds = (micros() - lastMicros) / microsPerSecond;
  lastMicros += elapsedSeconds * microsPerSecond;
  lastUnix += elapsedSeconds;
  return lastUnix;
}

/**************************************************************************/
/*!
    @brief  Start I2C for the PCF8523 and test succesful connection
    @return True if Wire can find PCF8523 or false otherwise.
*/
/**************************************************************************/

boolean RTC_PCF8523::begin(TwoWire *wireInstance) {
  RTCWireBus = wireInstance;
  //RTCWireBus->begin();
  RTCWireBus->beginTransmission(PCF8523_ADDRESS);
  if (RTCWireBus->endTransmission() == 0)
    return true;
  return false;
}

/**************************************************************************/
/*!
    @brief  Check the status register Oscillator Stop flag to see if the PCF8523
   stopped due to power loss
    @details When battery or external power is first applied, the PCF8523's
   crystal oscillator takes up to 2s to stabilize. During this time adjust()
   cannot clear the 'OS' flag. See datasheet OS flag section for details.
    @return True if the bit is set (oscillator is or has stopped) and false only
   after the bit is cleared, for instance with adjust()
*/
/**************************************************************************/
boolean RTC_PCF8523::lostPower(void) {
  return (read_i2c_register(PCF8523_ADDRESS, PCF8523_STATUSREG, RTCWireBus) >>
          7);
}

/**************************************************************************/
/*!
    @brief  Check control register 3 to see if we've run adjust() yet (setting
   the date/time and battery switchover mode)
    @return True if the PCF8523 has been set up, false if not
*/
/**************************************************************************/
boolean RTC_PCF8523::initialized(void) {
  RTCWireBus->beginTransmission(PCF8523_ADDRESS);
  RTCWireBus->_I2C_WRITE((byte)PCF8523_CONTROL_3);
  RTCWireBus->endTransmission();

  RTCWireBus->requestFrom(PCF8523_ADDRESS, 1);
  uint8_t ss = RTCWireBus->_I2C_READ();
  return ((ss & 0xE0) != 0xE0); // 0xE0 = standby mode, set after power out
}

/**************************************************************************/
/*!
    @brief  Set the date and time, set battery switchover mode
    @param dt DateTime to set
*/
/**************************************************************************/
void RTC_PCF8523::adjust(const DateTime &dt) {
  RTCWireBus->beginTransmission(PCF8523_ADDRESS);
  RTCWireBus->_I2C_WRITE((byte)3); // start at location 3
  RTCWireBus->_I2C_WRITE(bin2bcd(dt.second()));
  RTCWireBus->_I2C_WRITE(bin2bcd(dt.minute()));
  RTCWireBus->_I2C_WRITE(bin2bcd(dt.hour()));
  RTCWireBus->_I2C_WRITE(bin2bcd(dt.day()));
  RTCWireBus->_I2C_WRITE(bin2bcd(0)); // skip weekdays
  RTCWireBus->_I2C_WRITE(bin2bcd(dt.month()));
  RTCWireBus->_I2C_WRITE(bin2bcd(dt.year() - 2000U));
  RTCWireBus->endTransmission();

  // set to battery switchover mode
  RTCWireBus->beginTransmission(PCF8523_ADDRESS);
  RTCWireBus->_I2C_WRITE((byte)PCF8523_CONTROL_3);
  RTCWireBus->_I2C_WRITE((byte)0x00);
  RTCWireBus->endTransmission();
}

/**************************************************************************/
/*!
    @brief  Get the current date/time
    @return DateTime object containing the current date/time
*/
/**************************************************************************/
DateTime RTC_PCF8523::now() {
  RTCWireBus->beginTransmission(PCF8523_ADDRESS);
  RTCWireBus->_I2C_WRITE((byte)3);
  RTCWireBus->endTransmission();

  RTCWireBus->requestFrom(PCF8523_ADDRESS, 7);
  uint8_t ss = bcd2bin(RTCWireBus->_I2C_READ() & 0x7F);
  uint8_t mm = bcd2bin(RTCWireBus->_I2C_READ());
  uint8_t hh = bcd2bin(RTCWireBus->_I2C_READ());
  uint8_t d = bcd2bin(RTCWireBus->_I2C_READ());
  RTCWireBus->_I2C_READ(); // skip 'weekdays'
  uint8_t m = bcd2bin(RTCWireBus->_I2C_READ());
  uint16_t y = bcd2bin(RTCWireBus->_I2C_READ()) + 2000U;

  return DateTime(y, m, d, hh, mm, ss);
}

/**************************************************************************/
/*!
    @brief  Resets the STOP bit in register Control_1
*/
/**************************************************************************/
void RTC_PCF8523::start(void) {
  uint8_t ctlreg =
      read_i2c_register(PCF8523_ADDRESS, PCF8523_CONTROL_1, RTCWireBus);
  if (ctlreg & (1 << 5)) {
    write_i2c_register(PCF8523_ADDRESS, PCF8523_CONTROL_1, ctlreg & ~(1 << 5),
                       RTCWireBus);
  }
}

/**************************************************************************/
/*!
    @brief  Sets the STOP bit in register Control_1
*/
/**************************************************************************/
void RTC_PCF8523::stop(void) {
  uint8_t ctlreg =
      read_i2c_register(PCF8523_ADDRESS, PCF8523_CONTROL_1, RTCWireBus);
  if (!(ctlreg & (1 << 5))) {
    write_i2c_register(PCF8523_ADDRESS, PCF8523_CONTROL_1, ctlreg | (1 << 5),
                       RTCWireBus);
  }
}

/**************************************************************************/
/*!
    @brief  Is the PCF8523 running? Check the STOP bit in register Control_1
    @return 1 if the RTC is running, 0 if not
*/
/**************************************************************************/
uint8_t RTC_PCF8523::isrunning() {
  uint8_t ctlreg =
      read_i2c_register(PCF8523_ADDRESS, PCF8523_CONTROL_1, RTCWireBus);
  return !((ctlreg >> 5) & 1);
}

/**************************************************************************/
/*!
    @brief  Read the mode of the INT/SQW pin on the PCF8523
    @return SQW pin mode as a #Pcf8523SqwPinMode enum
*/
/**************************************************************************/
Pcf8523SqwPinMode RTC_PCF8523::readSqwPinMode() {
  int mode;

  RTCWireBus->beginTransmission(PCF8523_ADDRESS);
  RTCWireBus->_I2C_WRITE(PCF8523_CLKOUTCONTROL);
  RTCWireBus->endTransmission();

  RTCWireBus->requestFrom((uint8_t)PCF8523_ADDRESS, (uint8_t)1);
  mode = RTCWireBus->_I2C_READ();

  mode >>= 3;
  mode &= 0x7;
  return static_cast<Pcf8523SqwPinMode>(mode);
}

/**************************************************************************/
/*!
    @brief  Set the INT/SQW pin mode on the PCF8523
    @param mode The mode to set, see the #Pcf8523SqwPinMode enum for options
*/
/**************************************************************************/
void RTC_PCF8523::writeSqwPinMode(Pcf8523SqwPinMode mode) {
  RTCWireBus->beginTransmission(PCF8523_ADDRESS);
  RTCWireBus->_I2C_WRITE(PCF8523_CLKOUTCONTROL);
  RTCWireBus->_I2C_WRITE(mode << 3); // disables other timers
  RTCWireBus->endTransmission();
}

/**************************************************************************/
/*!
    @brief  Enable the Second Timer (1Hz) Interrupt on the PCF8523.
    @details The INT/SQW pin will pull low for a brief pulse once per second.
*/
/**************************************************************************/
void RTC_PCF8523::enableSecondTimer() {
  // Leave compatible settings intact
  uint8_t ctlreg =
      read_i2c_register(PCF8523_ADDRESS, PCF8523_CONTROL_1, RTCWireBus);
  uint8_t clkreg =
      read_i2c_register(PCF8523_ADDRESS, PCF8523_CLKOUTCONTROL, RTCWireBus);

  // TAM pulse int. mode (shared with Timer A), CLKOUT (aka SQW) disabled
  write_i2c_register(PCF8523_ADDRESS, PCF8523_CLKOUTCONTROL, clkreg | 0xB8,
                     RTCWireBus);

  // SIE Second timer int. enable
  write_i2c_register(PCF8523_ADDRESS, PCF8523_CONTROL_1, ctlreg | (1 << 2),
                     RTCWireBus);
}

/**************************************************************************/
/*!
    @brief  Disable the Second Timer (1Hz) Interrupt on the PCF8523.
*/
/**************************************************************************/
void RTC_PCF8523::disableSecondTimer() {
  // Leave compatible settings intact
  uint8_t ctlreg =
      read_i2c_register(PCF8523_ADDRESS, PCF8523_CONTROL_1, RTCWireBus);

  // SIE Second timer int. disable
  write_i2c_register(PCF8523_ADDRESS, PCF8523_CONTROL_1, ctlreg & ~(1 << 2),
                     RTCWireBus);
}

/**************************************************************************/
/*!
    @brief  Enable the Countdown Timer Interrupt on the PCF8523.
    @details The INT/SQW pin will be pulled low at the end of a specified
   countdown period ranging from 244 microseconds to 10.625 days.
    Uses PCF8523 Timer B. Any existing CLKOUT square wave, configured with
   writeSqwPinMode(), will halt. The interrupt low pulse width is adjustable
   from 3/64ths (default) to 14/64ths of a second.
    @param clkFreq One of the PCF8523's Timer Source Clock Frequencies.
   See the #PCF8523TimerClockFreq enum for options and associated time ranges.
    @param numPeriods The number of clkFreq periods (1-255) to count down.
    @param lowPulseWidth Optional: the length of time for the interrupt pin
   low pulse. See the #PCF8523TimerIntPulse enum for options.
*/
/**************************************************************************/
void RTC_PCF8523::enableCountdownTimer(PCF8523TimerClockFreq clkFreq,
                                       uint8_t numPeriods,
                                       uint8_t lowPulseWidth) {
  // Datasheet cautions against updating countdown value while it's running,
  // so disabling allows repeated calls with new values to set new countdowns
  disableCountdownTimer();

  // Leave compatible settings intact
  uint8_t ctlreg =
      read_i2c_register(PCF8523_ADDRESS, PCF8523_CONTROL_2, RTCWireBus);
  uint8_t clkreg =
      read_i2c_register(PCF8523_ADDRESS, PCF8523_CLKOUTCONTROL, RTCWireBus);

  // CTBIE Countdown Timer B Interrupt Enabled
  write_i2c_register(PCF8523_ADDRESS, PCF8523_CONTROL_2, ctlreg |= 0x01,
                     RTCWireBus);

  // Timer B source clock frequency, optionally int. low pulse width
  write_i2c_register(PCF8523_ADDRESS, PCF8523_TIMER_B_FRCTL,
                     lowPulseWidth << 4 | clkFreq, RTCWireBus);

  // Timer B value (number of source clock periods)
  write_i2c_register(PCF8523_ADDRESS, PCF8523_TIMER_B_VALUE, numPeriods,
                     RTCWireBus);

  // TBM Timer B pulse int. mode, CLKOUT (aka SQW) disabled, TBC start Timer B
  write_i2c_register(PCF8523_ADDRESS, PCF8523_CLKOUTCONTROL, clkreg | 0x79,
                     RTCWireBus);
}

/**************************************************************************/
/*!
    @overload
    @brief  Enable Countdown Timer using default interrupt low pulse width.
    @param clkFreq One of the PCF8523's Timer Source Clock Frequencies.
   See the #PCF8523TimerClockFreq enum for options and associated time ranges.
    @param numPeriods The number of clkFreq periods (1-255) to count down.
*/
/**************************************************************************/
void RTC_PCF8523::enableCountdownTimer(PCF8523TimerClockFreq clkFreq,
                                       uint8_t numPeriods) {
  enableCountdownTimer(clkFreq, numPeriods, 0);
}

/**************************************************************************/
/*!
    @brief  Disable the Countdown Timer Interrupt on the PCF8523.
    @details For simplicity, this function strictly disables Timer B by setting
   TBC to 0. The datasheet describes TBC as the Timer B on/off switch.
   Timer B is the only countdown timer implemented at this time.
   The following flags have no effect while TBC is off, they are *not* cleared:
      - TBM: Timer B will still be set to pulsed mode.
      - CTBIE: Timer B interrupt would be triggered if TBC were on.
      - CTBF: Timer B flag indicates that interrupt was triggered. Though
        typically used for non-pulsed mode, user may wish to query this later.
*/
/**************************************************************************/
void RTC_PCF8523::disableCountdownTimer() {
  // Leave compatible settings intact
  uint8_t clkreg =
      read_i2c_register(PCF8523_ADDRESS, PCF8523_CLKOUTCONTROL, RTCWireBus);

  // TBC disable to stop Timer B clock
  write_i2c_register(PCF8523_ADDRESS, PCF8523_CLKOUTCONTROL, ~1 & clkreg,
                     RTCWireBus);
}

/**************************************************************************/
/*!
    @brief  Stop all timers, clear their flags and settings on the PCF8523.
    @details This includes the Countdown Timer, Second Timer, and any CLKOUT
   square wave configured with writeSqwPinMode().
*/
/**************************************************************************/
void RTC_PCF8523::deconfigureAllTimers() {
  disableSecondTimer(); // Surgically clears CONTROL_1
  write_i2c_register(PCF8523_ADDRESS, PCF8523_CONTROL_2, 0, RTCWireBus);
  write_i2c_register(PCF8523_ADDRESS, PCF8523_CLKOUTCONTROL, 0, RTCWireBus);
  write_i2c_register(PCF8523_ADDRESS, PCF8523_TIMER_B_FRCTL, 0, RTCWireBus);
  write_i2c_register(PCF8523_ADDRESS, PCF8523_TIMER_B_VALUE, 0, RTCWireBus);
}

/**************************************************************************/
/*!
    @brief Compensate the drift of the RTC.
    @details This method sets the "offset" register of the PCF8523,
      which can be used to correct a previously measured drift rate.
      Two correction modes are available:

      - **PCF8523\_TwoHours**: Clock adjustments are performed on
        `offset` consecutive minutes every two hours. This is the most
        energy-efficient mode.

      - **PCF8523\_OneMinute**: Clock adjustments are performed on
        `offset` consecutive seconds every minute. Extra adjustments are
        performed on the last second of the minute is `abs(offset)>60`.

      The `offset` parameter sets the correction amount in units of
      roughly 4&nbsp;ppm. The exact unit depends on the selected mode:

      |  mode               | offset unit                            |
      |---------------------|----------------------------------------|
      | `PCF8523_TwoHours`  | 4.340 ppm = 0.375 s/day = 2.625 s/week |
      | `PCF8523_OneMinute` | 4.069 ppm = 0.352 s/day = 2.461 s/week |

      See the accompanying sketch pcf8523.ino for an example on how to
      use this method.

    @param mode Correction mode, either `PCF8523_TwoHours` or
      `PCF8523_OneMinute`.
    @param offset Correction amount, from -64 to +63. A positive offset
      makes the clock slower.
*/
/**************************************************************************/
void RTC_PCF8523::calibrate(Pcf8523OffsetMode mode, int8_t offset) {
  uint8_t reg = (uint8_t)offset & 0x7F;
  reg |= mode;

  RTCWireBus->beginTransmission(PCF8523_ADDRESS);
  RTCWireBus->_I2C_WRITE(PCF8523_OFFSET);
  RTCWireBus->_I2C_WRITE(reg);
  RTCWireBus->endTransmission();
}

// START RTC_PCF8563 implementation

/**************************************************************************/
/*!
    @brief  Start I2C for the PCF8563 and test succesful connection
    @return True if Wire can find PCF8563 or false otherwise.
*/
/**************************************************************************/

boolean RTC_PCF8563::begin(TwoWire *wireInstance) {
  RTCWireBus = wireInstance;
  //RTCWireBus->begin();
  RTCWireBus->beginTransmission(PCF8563_ADDRESS);
  if (RTCWireBus->endTransmission() == 0)
    return true;
  return false;
}

/**************************************************************************/
/*!
    @brief  Check the status of the VL bit in the VL_SECONDS register.
    @details The PCF8563 has an on-chip voltage-low detector. When VDD drops
     below Vlow, bit VL in the VL_seconds register is set to indicate that
     the integrity of the clock information is no longer guaranteed.
    @return True if the bit is set (VDD droped below Vlow) indicating that
    the clock integrity is not guaranteed and false only after the bit is
    cleared using adjust()
*/
/**************************************************************************/

boolean RTC_PCF8563::lostPower(void) {
  return (read_i2c_register(PCF8563_ADDRESS, PCF8563_VL_SECONDS, RTCWireBus) >>
          7);
}

/**************************************************************************/
/*!
    @brief  Set the date and time
    @param dt DateTime to set
*/
/**************************************************************************/
void RTC_PCF8563::adjust(const DateTime &dt) {

  RTCWireBus->beginTransmission(PCF8563_ADDRESS);
  RTCWireBus->_I2C_WRITE(PCF8563_VL_SECONDS); // start at location 2, VL_SECONDS
  RTCWireBus->_I2C_WRITE(bin2bcd(dt.second()));
  RTCWireBus->_I2C_WRITE(bin2bcd(dt.minute()));
  RTCWireBus->_I2C_WRITE(bin2bcd(dt.hour()));
  RTCWireBus->_I2C_WRITE(bin2bcd(dt.day()));
  RTCWireBus->_I2C_WRITE(bin2bcd(0)); // skip weekdays
  RTCWireBus->_I2C_WRITE(bin2bcd(dt.month()));
  RTCWireBus->_I2C_WRITE(bin2bcd(dt.year() - 2000));
  RTCWireBus->endTransmission();
}

/**************************************************************************/
/*!
    @brief  Get the current date/time
    @return DateTime object containing the current date/time
*/
/**************************************************************************/

DateTime RTC_PCF8563::now() {
  RTCWireBus->beginTransmission(PCF8563_ADDRESS);
  RTCWireBus->_I2C_WRITE((byte)2);
  RTCWireBus->endTransmission();

  RTCWireBus->requestFrom(PCF8563_ADDRESS, 7);
  uint8_t ss = bcd2bin(RTCWireBus->_I2C_READ() & 0x7F);
  uint8_t mm = bcd2bin(RTCWireBus->_I2C_READ() & 0x7F);
  uint8_t hh = bcd2bin(RTCWireBus->_I2C_READ() & 0x3F);
  uint8_t d = bcd2bin(RTCWireBus->_I2C_READ() & 0x3F);
  RTCWireBus->_I2C_READ(); // skip 'weekdays'
  uint8_t m = bcd2bin(RTCWireBus->_I2C_READ() & 0x1F);
  uint16_t y = bcd2bin(RTCWireBus->_I2C_READ()) + 2000;

  return DateTime(y, m, d, hh, mm, ss);
}

/**************************************************************************/
/*!
    @brief  Resets the STOP bit in register Control_1
*/
/**************************************************************************/
void RTC_PCF8563::start(void) {
  uint8_t ctlreg =
      read_i2c_register(PCF8563_ADDRESS, PCF8563_CONTROL_1, RTCWireBus);
  if (ctlreg & (1 << 5)) {
    write_i2c_register(PCF8563_ADDRESS, PCF8563_CONTROL_1, ctlreg & ~(1 << 5),
                       RTCWireBus);
  }
}

/**************************************************************************/
/*!
    @brief  Sets the STOP bit in register Control_1
*/
/**************************************************************************/
void RTC_PCF8563::stop(void) {
  uint8_t ctlreg =
      read_i2c_register(PCF8563_ADDRESS, PCF8563_CONTROL_1, RTCWireBus);
  if (!(ctlreg & (1 << 5))) {
    write_i2c_register(PCF8523_ADDRESS, PCF8563_CONTROL_1, ctlreg | (1 << 5),
                       RTCWireBus);
  }
}

/**************************************************************************/
/*!
    @brief  Is the PCF8563 running? Check the STOP bit in register Control_1
    @return 1 if the RTC is running, 0 if not
*/
/**************************************************************************/
uint8_t RTC_PCF8563::isrunning() {
  uint8_t ctlreg =
      read_i2c_register(PCF8563_ADDRESS, PCF8563_CONTROL_1, RTCWireBus);
  return !((ctlreg >> 5) & 1);
}

/**************************************************************************/
/*!
    @brief  Read the mode of the CLKOUT pin on the PCF8563
    @return CLKOUT pin mode as a #Pcf8563SqwPinMode enum
*/
/**************************************************************************/
Pcf8563SqwPinMode RTC_PCF8563::readSqwPinMode() {

  int mode;

  RTCWireBus->beginTransmission(PCF8563_ADDRESS);
  RTCWireBus->_I2C_WRITE(PCF8563_CLKOUTCONTROL);
  RTCWireBus->endTransmission();

  RTCWireBus->requestFrom((uint8_t)PCF8563_ADDRESS, (uint8_t)1);
  mode = RTCWireBus->_I2C_READ();

  return static_cast<Pcf8563SqwPinMode>(mode & PCF8563_CLKOUT_MASK);
}

/**************************************************************************/
/*!
    @brief  Set the CLKOUT pin mode on the PCF8563
    @param mode The mode to set, see the #Pcf8563SqwPinMode enum for options
*/
/**************************************************************************/
void RTC_PCF8563::writeSqwPinMode(Pcf8563SqwPinMode mode) {

  RTCWireBus->beginTransmission(PCF8563_ADDRESS);
  RTCWireBus->_I2C_WRITE(PCF8563_CLKOUTCONTROL);
  RTCWireBus->_I2C_WRITE(mode);
  RTCWireBus->endTransmission();
}
// END RTC_PCF8563 implementation

/**************************************************************************/
/*!
    @brief  Convert the day of the week to a representation suitable for
            storing in the DS3231: from 1 (Monday) to 7 (Sunday).
    @param  d Day of the week as represented by the library:
            from 0 (Sunday) to 6 (Saturday).
*/
/**************************************************************************/
static uint8_t dowToDS3231(uint8_t d) { return d == 0 ? 7 : d; }

/**************************************************************************/
/*!
    @brief  Start I2C for the DS3231 and test succesful connection
    @return True if Wire can find DS3231 or false otherwise.
*/
/**************************************************************************/

boolean RTC_DS3231::begin(TwoWire *wireInstance) {
  RTCWireBus = wireInstance;
  //RTCWireBus->begin();
  RTCWireBus->beginTransmission(DS3231_ADDRESS);
  if (RTCWireBus->endTransmission() == 0)
    return true;
  return false;
}

/**************************************************************************/
/*!
    @brief  Check the status register Oscillator Stop Flag to see if the DS3231
   stopped due to power loss
    @return True if the bit is set (oscillator stopped) or false if it is
   running
*/
/**************************************************************************/
bool RTC_DS3231::lostPower(void) {
  return (read_i2c_register(DS3231_ADDRESS, DS3231_STATUSREG, RTCWireBus) >> 7);
}

/**************************************************************************/
/*!
    @brief  Set the date and flip the Oscillator Stop Flag
    @param dt DateTime object containing the date/time to set
*/
/**************************************************************************/
void RTC_DS3231::adjust(const DateTime &dt) {
  RTCWireBus->beginTransmission(DS3231_ADDRESS);
  RTCWireBus->_I2C_WRITE((byte)DS3231_TIME); // start at location 0
  RTCWireBus->_I2C_WRITE(bin2bcd(dt.second()));
  RTCWireBus->_I2C_WRITE(bin2bcd(dt.minute()));
  RTCWireBus->_I2C_WRITE(bin2bcd(dt.hour()));
  // The RTC must know the day of the week for the weekly alarms to work.
  RTCWireBus->_I2C_WRITE(bin2bcd(dowToDS3231(dt.dayOfTheWeek())));
  RTCWireBus->_I2C_WRITE(bin2bcd(dt.day()));
  RTCWireBus->_I2C_WRITE((bin2bcd(dt.month()) & 0x7f) | (dt.year()>=2000 ? 0x80 : 0x00)); // Need to add the century bit
  RTCWireBus->_I2C_WRITE(bin2bcd(dt.year() - 2000U));
  RTCWireBus->endTransmission();

  uint8_t statreg =
      read_i2c_register(DS3231_ADDRESS, DS3231_STATUSREG, RTCWireBus);
  statreg &= ~0x80; // flip OSF bit
  write_i2c_register(DS3231_ADDRESS, DS3231_STATUSREG, statreg, RTCWireBus);
}

/**************************************************************************/
/*!
    @brief  Get the current date/time
    @return DateTime object with the current date/time
*/
/**************************************************************************/
DateTime RTC_DS3231::now() {
  RTCWireBus->beginTransmission(DS3231_ADDRESS);
  RTCWireBus->_I2C_WRITE((byte)0);
  RTCWireBus->endTransmission();

  RTCWireBus->requestFrom(DS3231_ADDRESS, 7);
  uint8_t ss = bcd2bin(RTCWireBus->_I2C_READ() & 0x7F);
  uint8_t mm = bcd2bin(RTCWireBus->_I2C_READ());
  uint8_t hh_raw = RTCWireBus->_I2C_READ();
  uint8_t hh = 0;
  if (hh_raw & 0x40) {
    // 12h mode, first take the last 5 bits
    hh = bcd2bin(hh_raw & 0x1F);
    if (hh_raw & 0x20) {
      hh += 12;
    }
  } else {
    hh = bcd2bin(hh_raw);
  }
  RTCWireBus->_I2C_READ(); // skip day-of-week
  uint8_t d = bcd2bin(RTCWireBus->_I2C_READ());
  uint8_t m = bcd2bin(RTCWireBus->_I2C_READ() & 0x7F); // Skip century bit.
  uint16_t y = bcd2bin(RTCWireBus->_I2C_READ()) + 2000U;

  return DateTime(y, m, d, hh, mm, ss);
}

/**************************************************************************/
/*!
    @brief  Read the SQW pin mode
    @return Pin mode, see Ds3231SqwPinMode enum
*/
/**************************************************************************/
Ds3231SqwPinMode RTC_DS3231::readSqwPinMode() {
  int mode;

  RTCWireBus->beginTransmission(DS3231_ADDRESS);
  RTCWireBus->_I2C_WRITE(DS3231_CONTROL);
  RTCWireBus->endTransmission();

  RTCWireBus->requestFrom((uint8_t)DS3231_ADDRESS, (uint8_t)1);
  mode = RTCWireBus->_I2C_READ();

  mode &= 0x1C;
  if (mode & 0x04)
    mode = DS3231_OFF;
  return static_cast<Ds3231SqwPinMode>(mode);
}

/**************************************************************************/
/*!
    @brief  Set the SQW pin mode
    @param mode Desired mode, see Ds3231SqwPinMode enum
*/
/**************************************************************************/
void RTC_DS3231::writeSqwPinMode(Ds3231SqwPinMode mode) {
  uint8_t ctrl;
  ctrl = read_i2c_register(DS3231_ADDRESS, DS3231_CONTROL, RTCWireBus);

  ctrl &= ~0x04; // turn off INTCON
  ctrl &= ~0x18; // set freq bits to 0

  ctrl |= mode;
  write_i2c_register(DS3231_ADDRESS, DS3231_CONTROL, ctrl, RTCWireBus);

  // Serial.println( read_i2c_register(DS3231_ADDRESS, DS3231_CONTROL,
  // RTCWireBus), HEX);
}

/**************************************************************************/
/*!
    @brief  Get the current temperature from the DS3231's temperature sensor
    @return Current temperature (float)
*/
/**************************************************************************/
float RTC_DS3231::getTemperature() {
  uint8_t lsb;
  int8_t msb;
  RTCWireBus->beginTransmission(DS3231_ADDRESS);
  RTCWireBus->_I2C_WRITE(DS3231_TEMPERATUREREG);
  RTCWireBus->endTransmission();

  RTCWireBus->requestFrom(DS3231_ADDRESS, 2);
  msb = RTCWireBus->_I2C_READ();
  lsb = RTCWireBus->_I2C_READ();

  //  Serial.print("msb=");
  //  Serial.print(msb,HEX);
  //  Serial.print(", lsb=");
  //  Serial.println(lsb,HEX);

  return (float)msb + (lsb >> 6) * 0.25f;
}

/**************************************************************************/
/*!
    @brief  Set alarm 1 for DS3231
        @param 	dt DateTime object
        @param 	alarm_mode Desired mode, see Ds3231Alarm1Mode enum
    @return False if control register is not set, otherwise true
*/
/**************************************************************************/
bool RTC_DS3231::setAlarm1(const DateTime &dt, Ds3231Alarm1Mode alarm_mode) {
  uint8_t ctrl = read_i2c_register(DS3231_ADDRESS, DS3231_CONTROL, RTCWireBus);
  if (!(ctrl & 0x04)) {
    return false;
  }

  uint8_t A1M1 = (alarm_mode & 0x01) << 7; // Seconds bit 7.
  uint8_t A1M2 = (alarm_mode & 0x02) << 6; // Minutes bit 7.
  uint8_t A1M3 = (alarm_mode & 0x04) << 5; // Hour bit 7.
  uint8_t A1M4 = (alarm_mode & 0x08) << 4; // Day/Date bit 7.
  uint8_t DY_DT = (alarm_mode & 0x10)
                  << 2; // Day/Date bit 6. Date when 0, day of week when 1.

  RTCWireBus->beginTransmission(DS3231_ADDRESS);
  RTCWireBus->_I2C_WRITE(DS3231_ALARM1);
  RTCWireBus->_I2C_WRITE(bin2bcd(dt.second()) | A1M1);
  RTCWireBus->_I2C_WRITE(bin2bcd(dt.minute()) | A1M2);
  RTCWireBus->_I2C_WRITE(bin2bcd(dt.hour()) | A1M3);
  if (DY_DT) {
    RTCWireBus->_I2C_WRITE(bin2bcd(dowToDS3231(dt.dayOfTheWeek())) | A1M4 |
                           DY_DT);
  } else {
    RTCWireBus->_I2C_WRITE(bin2bcd(dt.day()) | A1M4 | DY_DT);
  }
  RTCWireBus->endTransmission();

  ctrl |= 0x01; // AI1E
  write_i2c_register(DS3231_ADDRESS, DS3231_CONTROL, ctrl, RTCWireBus);
  return true;
}

/**************************************************************************/
/*!
    @brief  Set alarm 2 for DS3231
        @param 	dt DateTime object
        @param 	alarm_mode Desired mode, see Ds3231Alarm2Mode enum
    @return False if control register is not set, otherwise true
*/
/**************************************************************************/
bool RTC_DS3231::setAlarm2(const DateTime &dt, Ds3231Alarm2Mode alarm_mode) {
  uint8_t ctrl = read_i2c_register(DS3231_ADDRESS, DS3231_CONTROL, RTCWireBus);
  if (!(ctrl & 0x04)) {
    return false;
  }

  uint8_t A2M2 = (alarm_mode & 0x01) << 7; // Minutes bit 7.
  uint8_t A2M3 = (alarm_mode & 0x02) << 6; // Hour bit 7.
  uint8_t A2M4 = (alarm_mode & 0x04) << 5; // Day/Date bit 7.
  uint8_t DY_DT = (alarm_mode & 0x8)
                  << 3; // Day/Date bit 6. Date when 0, day of week when 1.

  RTCWireBus->beginTransmission(DS3231_ADDRESS);
  RTCWireBus->_I2C_WRITE(DS3231_ALARM2);
  RTCWireBus->_I2C_WRITE(bin2bcd(dt.minute()) | A2M2);
  RTCWireBus->_I2C_WRITE(bin2bcd(dt.hour()) | A2M3);
  if (DY_DT) {
    RTCWireBus->_I2C_WRITE(bin2bcd(dowToDS3231(dt.dayOfTheWeek())) | A2M4 |
                           DY_DT);
  } else {
    RTCWireBus->_I2C_WRITE(bin2bcd(dt.day()) | A2M4 | DY_DT);
  }
  RTCWireBus->endTransmission();

  ctrl |= 0x02; // AI2E
  write_i2c_register(DS3231_ADDRESS, DS3231_CONTROL, ctrl, RTCWireBus);
  return true;
}

/**************************************************************************/
/*!
    @brief  Disable alarm
        @param 	alarm_num Alarm number to disable
*/
/**************************************************************************/
void RTC_DS3231::disableAlarm(uint8_t alarm_num) {
  uint8_t ctrl = read_i2c_register(DS3231_ADDRESS, DS3231_CONTROL, RTCWireBus);
  ctrl &= ~(1 << (alarm_num - 1));
  write_i2c_register(DS3231_ADDRESS, DS3231_CONTROL, ctrl, RTCWireBus);
}

/**************************************************************************/
/*!
    @brief  Clear status of alarm
        @param 	alarm_num Alarm number to clear
*/
/**************************************************************************/
void RTC_DS3231::clearAlarm(uint8_t alarm_num) {
  uint8_t status =
      read_i2c_register(DS3231_ADDRESS, DS3231_STATUSREG, RTCWireBus);
  status &= ~(0x1 << (alarm_num - 1));
  write_i2c_register(DS3231_ADDRESS, DS3231_STATUSREG, status, RTCWireBus);
}

/**************************************************************************/
/*!
    @brief  Get status of alarm
        @param 	alarm_num Alarm number to check status of
        @return True if alarm has been fired otherwise false
*/
/**************************************************************************/
bool RTC_DS3231::alarmFired(uint8_t alarm_num) {
  uint8_t status =
      read_i2c_register(DS3231_ADDRESS, DS3231_STATUSREG, RTCWireBus);
  return (status >> (alarm_num - 1)) & 0x1;
}

/**************************************************************************/
/*!
    @brief  Enable 32KHz Output
    @details The 32kHz output is enabled by default. It requires an external
    pull-up resistor to function correctly
*/
/**************************************************************************/
void RTC_DS3231::enable32K(void) {
  uint8_t status =
      read_i2c_register(DS3231_ADDRESS, DS3231_STATUSREG, RTCWireBus);
  status |= (0x1 << 0x03);
  write_i2c_register(DS3231_ADDRESS, DS3231_STATUSREG, status, RTCWireBus);
  // Serial.println(read_i2c_register(DS3231_ADDRESS, DS3231_STATUSREG,
  // RTCWireBus), BIN);
}

/**************************************************************************/
/*!
    @brief  Disable 32KHz Output
*/
/**************************************************************************/
void RTC_DS3231::disable32K(void) {
  uint8_t status =
      read_i2c_register(DS3231_ADDRESS, DS3231_STATUSREG, RTCWireBus);
  status &= ~(0x1 << 0x03);
  write_i2c_register(DS3231_ADDRESS, DS3231_STATUSREG, status, RTCWireBus);
  // Serial.println(read_i2c_register(DS3231_ADDRESS, DS3231_STATUSREG,
  // RTCWireBus), BIN);
}

/**************************************************************************/
/*!
    @brief  Get status of 32KHz Output
    @return True if enabled otherwise false
*/
/**************************************************************************/
bool RTC_DS3231::isEnabled32K(void) {
  uint8_t status =
      read_i2c_register(DS3231_ADDRESS, DS3231_STATUSREG, RTCWireBus);
  return (status >> 0x03) & 0x1;
}
