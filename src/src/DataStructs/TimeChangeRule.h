#ifndef DATASTRUCTS_TIMECHANGERULE_H
#define DATASTRUCTS_TIMECHANGERULE_H

#include <Arduino.h>

// structure to describe rules for when daylight/summer time begins,
// or when standard time begins.
// For Daylight Saving Time Around the World, see:
// - https://www.timeanddate.com/time/dst/2018.html
// - https://en.wikipedia.org/wiki/Daylight_saving_time_by_country
struct TimeChangeRule {

  // convenient constants for TimeChangeRules
  enum week_t { Last = 0, First, Second, Third, Fourth };
  enum dow_t { Sun = 1, Mon, Tue, Wed, Thu, Fri, Sat };
  enum month_t { Jan = 1, Feb, Mar, Apr, May, Jun, Jul, Aug, Sep, Oct, Nov, Dec };



  TimeChangeRule();

  TimeChangeRule(uint8_t weeknr, uint8_t downr, uint8_t m, uint8_t h, int16_t minutesoffset);

  // Construct time change rule from stored values optimized for minimum space.
  TimeChangeRule(uint16_t flash_stored_value, int16_t minutesoffset);

  uint16_t toFlashStoredValue() const;

  bool isValid() const;

  uint8_t week;                               // First, Second, Third, Fourth, or Last week of the month
  uint8_t dow;                                // day of week, 1=Sun, 2=Mon, ... 7=Sat
  uint8_t month;                              // 1=Jan, 2=Feb, ... 12=Dec
  uint8_t hour;                               // 0-23
  int16_t offset;                             // offset from UTC in minutes
};

#endif // DATASTRUCTS_TIMECHANGERULE_H