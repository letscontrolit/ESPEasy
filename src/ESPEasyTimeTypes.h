#ifndef ESPEASY_TIMETYPES_H_
#define ESPEASY_TIMETYPES_H_

#include <stdint.h>

struct  timeStruct {
  timeStruct() : Second(0), Minute(0), Hour(0), Wday(0), Day(0), Month(0), Year(0) {}
  uint8_t Second;
  uint8_t Minute;
  uint8_t Hour;
  uint8_t Wday;   // day of week, sunday is day 1
  uint8_t Day;
  uint8_t Month;
  uint8_t Year;   // offset from 1970;
};

// convenient constants for TimeChangeRules
enum week_t {Last=0, First, Second, Third, Fourth};
enum dow_t {Sun=1, Mon, Tue, Wed, Thu, Fri, Sat};
enum month_t {Jan=1, Feb, Mar, Apr, May, Jun, Jul, Aug, Sep, Oct, Nov, Dec};

// structure to describe rules for when daylight/summer time begins,
// or when standard time begins.
// For Daylight Saving Time Around the World, see:
// - https://www.timeanddate.com/time/dst/2018.html
// - https://en.wikipedia.org/wiki/Daylight_saving_time_by_country
struct TimeChangeRule {
    TimeChangeRule() :  week(0), dow(1), month(1), hour(0), offset(0) {}
    TimeChangeRule(uint8_t weeknr, uint8_t downr, uint8_t m, uint8_t h, uint16_t minutesoffset) :
        week(weeknr), dow(downr), month(m), hour(h), offset(minutesoffset) {}

    // Construct time change rule from stored values optimized for minimum space.
    TimeChangeRule(uint16_t flash_stored_value, int16_t minutesoffset) : offset(minutesoffset) {
      hour = flash_stored_value & 0x001f;
      month = (flash_stored_value >> 5) & 0x000f;
      dow = (flash_stored_value >> 9) & 0x0007;
      week = (flash_stored_value >> 12) & 0x0007;
    }

    uint16_t toFlashStoredValue() const {
      uint16_t value = hour;
      value = value | (month << 5);
      value = value | (dow << 9);
      value = value | (week << 12);
      return value;
    }

    bool isValid() const {
      return (week <= 4) && (dow != 0) && (dow <= 7) &&
             (month != 0) && (month <= 12) && (hour <= 23) &&
             (offset > -720) && (offset < 900); // UTC-12h ... UTC+14h + 1h DSToffset
    }

    uint8_t week;      // First, Second, Third, Fourth, or Last week of the month
    uint8_t dow;       // day of week, 1=Sun, 2=Mon, ... 7=Sat
    uint8_t month;     // 1=Jan, 2=Feb, ... 12=Dec
    uint8_t hour;      // 0-23
    int16_t offset;    // offset from UTC in minutes
};

// Forward declartions
void applyTimeZone(uint32_t curTime = 0);
void setTimeZone(const TimeChangeRule& dstStart, const TimeChangeRule& stdStart, uint32_t curTime = 0);
uint32_t calcTimeChangeForRule(const TimeChangeRule& r, int yr);
String getTimeString(char delimiter, bool show_seconds=true);
String getTimeString_ampm(char delimiter, bool show_seconds=true);
long timeDiff(unsigned long prev, unsigned long next);


#endif /* ESPEASY_TIMETYPES_H_ */
