#include "TimeChangeRule.h"



TimeChangeRule::TimeChangeRule() :  week(0), dow(1), month(1), hour(0), offset(0) {}

TimeChangeRule::TimeChangeRule(uint8_t weeknr, uint8_t downr, uint8_t m, uint8_t h, int16_t minutesoffset) :
  week(weeknr), dow(downr), month(m), hour(h), offset(minutesoffset) {}

// Construct time change rule from stored values optimized for minimum space.
TimeChangeRule::TimeChangeRule(uint16_t flash_stored_value, int16_t minutesoffset) : offset(minutesoffset) {
  hour  = flash_stored_value & 0x001f;
  month = (flash_stored_value >> 5) & 0x000f;
  dow   = (flash_stored_value >> 9) & 0x0007;
  week  = (flash_stored_value >> 12) & 0x0007;
}

uint16_t TimeChangeRule::toFlashStoredValue() const {
  uint16_t value = hour;

  value = value | (month << 5);
  value = value | (dow << 9);
  value = value | (week << 12);
  return value;
}

bool TimeChangeRule::isValid() const {
  return (week <= 4) && (dow != 0) && (dow <= 7) &&
          (month != 0) && (month <= 12) && (hour <= 23) &&
          (offset > -720) && (offset < 900); // UTC-12h ... UTC+14h + 1h DSToffset
}
