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
enum week_t {Last, First, Second, Third, Fourth};
enum dow_t {Sun=1, Mon, Tue, Wed, Thu, Fri, Sat};
enum month_t {Jan=1, Feb, Mar, Apr, May, Jun, Jul, Aug, Sep, Oct, Nov, Dec};

// structure to describe rules for when daylight/summer time begins,
// or when standard time begins.
// For Daylight Saving Time Around the World, see:
// - https://www.timeanddate.com/time/dst/2018.html
// - https://en.wikipedia.org/wiki/Daylight_saving_time_by_country
struct TimeChangeRule {
    TimeChangeRule() :  week(0), dow(0), month(0), hour(0), offset(0) {}
    TimeChangeRule(uint8_t weeknr, uint8_t downr, uint8_t m, uint8_t h, int minutesoffset) :
        week(weeknr), dow(downr), month(m), hour(h), offset(minutesoffset) {}

    uint8_t week;      // First, Second, Third, Fourth, or Last week of the month
    uint8_t dow;       // day of week, 1=Sun, 2=Mon, ... 7=Sat
    uint8_t month;     // 1=Jan, 2=Feb, ... 12=Dec
    uint8_t hour;      // 0-23
    int offset;        // offset from UTC in minutes
};

// Forward declartions
void applyTimeZone(uint32_t curTime = 0);
void setTimeZone(const TimeChangeRule& dstStart, const TimeChangeRule& stdStart, uint32_t curTime = 0);
uint32_t calcTimeChangeForRule(const TimeChangeRule& r, int yr);
