#include "ESPEasy_time_zone.h"

#include "../DataStructs/TimeChangeRule.h"
#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../Globals/ESPEasy_time.h"
#include "../Globals/Settings.h"

#include "ESPEasy_time_calc.h"

#include <time.h>


#define SECS_PER_MIN  (60UL)
#define SECS_PER_HOUR (3600UL)
#define SECS_PER_DAY  (SECS_PER_HOUR * 24UL)



void ESPEasy_time_zone::getDefaultDst_flash_values(uint16_t& start, uint16_t& end) {
  // DST start: Last Sunday March    2am => 3am
  // DST end:   Last Sunday October  3am => 2am
  TimeChangeRule CEST(TimeChangeRule::Last, TimeChangeRule::Sun, TimeChangeRule::Mar, 2, Settings.TimeZone); // Summer Time
  TimeChangeRule CET(TimeChangeRule::Last, TimeChangeRule::Sun, TimeChangeRule::Oct, 3, Settings.TimeZone);  // Standard Time

  start = CEST.toFlashStoredValue();
  end   = CET.toFlashStoredValue();
}

void ESPEasy_time_zone::applyTimeZone(uint32_t curTime) {
  int dst_offset = Settings.DST ? 60 : 0;
  uint16_t tmpStart(Settings.DST_Start);
  uint16_t tmpEnd(Settings.DST_End);

  for (int i = 0; i < 2; ++i) {
    TimeChangeRule start(tmpStart, Settings.TimeZone + dst_offset); // Summer Time
    TimeChangeRule end(tmpEnd, Settings.TimeZone);                  // Standard Time

    if (start.isValid() && end.isValid()) {
      setTimeZone(start, end, curTime);
      return;
    }
    getDefaultDst_flash_values(tmpStart, tmpEnd);
  }
}

void ESPEasy_time_zone::setTimeZone(const TimeChangeRule& dstStart, const TimeChangeRule& stdStart, uint32_t curTime) {
  m_dst = dstStart;
  m_std = stdStart;

  if (calcTimeChanges(ESPEasy_time::year(curTime))) {
    logTimeZoneInfo();
  }
}

void ESPEasy_time_zone::logTimeZoneInfo() {
  String log = F("Current Time Zone: ");

  if (m_std.offset != m_dst.offset) {
    // Summer time
    log += F(" DST time start: ");

    if (m_dstLoc != 0) {
      struct tm tmp;
      ESPEasy_time::breakTime(m_dstLoc, tmp);
      log += ESPEasy_time::getDateTimeString(tmp, '-', ':', ' ', false);
    }
    log += F(" offset: ");
    log += m_dst.offset;
    log += F(" min ");
  }

  // Standard/Winter time.
  log += F("STD time start: ");

  if (m_stdLoc != 0) {
    struct tm tmp;
    ESPEasy_time::breakTime(m_stdLoc, tmp);
    log += ESPEasy_time::getDateTimeString(tmp, '-', ':', ' ', false);
  }
  log += F(" offset: ");
  log += m_std.offset;
  log += F(" min");
  addLog(LOG_LEVEL_INFO, log);
}


///*----------------------------------------------------------------------*
// * Convert the given time change rule to a uint32_t value                 *
// * for the given year.                                                  *
// *----------------------------------------------------------------------*/
uint32_t ESPEasy_time_zone::calcTimeChangeForRule(const TimeChangeRule& r, int yr)
{
  uint8_t m = r.month; // temp copies of r.month and r.week
  uint8_t w = r.week;

  if (w == 0)          // is this a "Last week" rule?
  {
    if (++m > 12)      // yes, for "Last", go to the next month
    {
      m = 1;
      ++yr;
    }
    w = 1; // and treat as first week of next month, subtract 7 days later
  }

  // calculate first day of the month, or for "Last" rules, first day of the next month
  struct tm tm;
  tm.tm_hour = r.hour;
  tm.tm_min  = 0;
  tm.tm_sec  = 0;
  tm.tm_mday = 1;
  tm.tm_mon  = m;
  tm.tm_year = yr - 1970;
  uint32_t t = makeTime(tm);

  // add offset from the first of the month to r.dow, and offset for the given week
  t += ((r.dow - ESPEasy_time::weekday(t) + 7) % 7 + (w - 1) * 7) * SECS_PER_DAY;

  // back up a week if this is a "Last" rule
  if (r.week == 0) { t -= 7 * SECS_PER_DAY; }
  return t;
}

/*----------------------------------------------------------------------*
* Calculate the DST and standard time change points for the given      *
* given year as local and UTC uint32_t values.                           *
*----------------------------------------------------------------------*/
bool ESPEasy_time_zone::calcTimeChanges(int yr)
{
  uint32_t dstLoc  = calcTimeChangeForRule(m_dst, yr);
  uint32_t stdLoc  = calcTimeChangeForRule(m_std, yr);
  bool     changed = (m_dstLoc != dstLoc) || (m_stdLoc != stdLoc);

  m_dstLoc = dstLoc;
  m_stdLoc = stdLoc;
  m_dstUTC = m_dstLoc - m_std.offset * SECS_PER_MIN;
  m_stdUTC = m_stdLoc - m_dst.offset * SECS_PER_MIN;
  return changed;
}

/*----------------------------------------------------------------------*
* Convert the given UTC time to local time, standard or                *
* daylight time, as appropriate.                                       *
*----------------------------------------------------------------------*/
uint32_t ESPEasy_time_zone::toLocal(uint32_t utc)
{
  // recalculate the time change points if needed
  if (ESPEasy_time::year(utc) != ESPEasy_time::year(m_dstUTC)) { calcTimeChanges(ESPEasy_time::year(utc)); }

  if (utcIsDST(utc)) {
    return utc + m_dst.offset * SECS_PER_MIN;
  }
  else {
    return utc + m_std.offset * SECS_PER_MIN;
  }
}

/*----------------------------------------------------------------------*
* Determine whether the given UTC uint32_t is within the DST interval    *
* or the Standard time interval.                                       *
*----------------------------------------------------------------------*/
bool ESPEasy_time_zone::utcIsDST(uint32_t utc)
{
  // recalculate the time change points if needed
  if (ESPEasy_time::year(utc) != ESPEasy_time::year(m_dstUTC)) { calcTimeChanges(ESPEasy_time::year(utc)); }

  if (m_stdUTC == m_dstUTC) {     // daylight time not observed in this tz
    return false;
  }
  else if (m_stdUTC > m_dstUTC) { // northern hemisphere
    return utc >= m_dstUTC && utc < m_stdUTC;
  }
  else {                          // southern hemisphere
    return !(utc >= m_stdUTC && utc < m_dstUTC);
  }
}

/*----------------------------------------------------------------------*
* Determine whether the given Local uint32_t is within the DST interval  *
* or the Standard time interval.                                       *
*----------------------------------------------------------------------*/
bool ESPEasy_time_zone::locIsDST(uint32_t local)
{
  // recalculate the time change points if needed
  if (ESPEasy_time::year(local) != ESPEasy_time::year(m_dstLoc)) { calcTimeChanges(ESPEasy_time::year(local)); }

  if (m_stdUTC == m_dstUTC) {     // daylight time not observed in this tz
    return false;
  }
  else if (m_stdLoc > m_dstLoc) { // northern hemisphere
    return local >= m_dstLoc && local < m_stdLoc;
  }
  else {                          // southern hemisphere
    return !(local >= m_stdLoc && local < m_dstLoc);
  }
}


