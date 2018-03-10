
/********************************************************************************************\
  Time zone
  \*********************************************************************************************/

// Borrowed code from Timezone: https://github.com/JChristensen/Timezon

TimeChangeRule m_dst;    // rule for start of dst or summer time for any year
TimeChangeRule m_std;    // rule for start of standard time for any year
uint32_t m_dstUTC = 0;       // dst start for given/current year, given in UTC
uint32_t m_stdUTC = 0;       // std time start for given/current year, given in UTC
uint32_t m_dstLoc = 0;       // dst start for given/current year, given in local time
uint32_t m_stdLoc = 0;       // std time start for given/current year, given in local time

/*
// Examples time zones
// Australia Eastern Time Zone (Sydney, Melbourne)
TimeChangeRule aEDT = {First, Sun, Oct, 2, 660};    // UTC + 11 hours
TimeChangeRule aEST = {First, Sun, Apr, 3, 600};    // UTC + 10 hours
setTimeZone(aEDT, aEST);

// Central European Time (Frankfurt, Paris)
TimeChangeRule CEST = {Last, Sun, Mar, 2, 120};     // Central European Summer Time
TimeChangeRule CET = {Last, Sun, Oct, 3, 60};       // Central European Standard Time
setTimeZone(CEST, CET);

// United Kingdom (London, Belfast)
TimeChangeRule BST = {Last, Sun, Mar, 1, 60};        // British Summer Time
TimeChangeRule GMT = {Last, Sun, Oct, 2, 0};         // Standard Time
setTimeZone(BST, GMT);

// UTC
TimeChangeRule utcRule = {Last, Sun, Mar, 1, 0};     // UTC
setTimeZone(utcRule, utcRule);

// US Eastern Time Zone (New York, Detroit)
TimeChangeRule usEDT = {Second, Sun, Mar, 2, -240};  // Eastern Daylight Time = UTC - 4 hours
TimeChangeRule usEST = {First, Sun, Nov, 2, -300};   // Eastern Standard Time = UTC - 5 hours
setTimeZone(usEDT, usEST);

// US Central Time Zone (Chicago, Houston)
TimeChangeRule usCDT = {Second, dowSunday, Mar, 2, -300};
TimeChangeRule usCST = {First, dowSunday, Nov, 2, -360};
setTimeZone(usCDT, usCST);

// US Mountain Time Zone (Denver, Salt Lake City)
TimeChangeRule usMDT = {Second, dowSunday, Mar, 2, -360};
TimeChangeRule usMST = {First, dowSunday, Nov, 2, -420};
setTimeZone(usMDT, usMST);

// Arizona is US Mountain Time Zone but does not use DST
setTimeZone(usMST, usMST);

// US Pacific Time Zone (Las Vegas, Los Angeles)
TimeChangeRule usPDT = {Second, dowSunday, Mar, 2, -420};
TimeChangeRule usPST = {First, dowSunday, Nov, 2, -480};
setTimeZone(usPDT, usPST);
*/

void getDefaultDst_flash_values(uint16_t& start, uint16_t& end) {
  // DST start: Last Sunday March    2am => 3am
  // DST end: 	Last Sunday October  3am => 2am
  TimeChangeRule CEST(Last, Sun, Mar, 2, Settings.TimeZone); // Summer Time
  TimeChangeRule CET(Last, Sun, Oct, 3, Settings.TimeZone);  // Standard Time
  start = CEST.toFlashStoredValue();
  end = CET.toFlashStoredValue();
}

void applyTimeZone(uint32_t curTime) {
  int dst_offset = Settings.DST ? 60 : 0;
  uint16_t tmpStart(Settings.DST_Start);
  uint16_t tmpEnd(Settings.DST_End);
  for (int i = 0; i < 2; ++i) {
    TimeChangeRule start(tmpStart, Settings.TimeZone + dst_offset); // Summer Time
    TimeChangeRule end(tmpEnd, Settings.TimeZone);               // Standard Time
    if (start.isValid() && end.isValid()) {
      setTimeZone(start, end, curTime);
      return;
    }
    getDefaultDst_flash_values(tmpStart, tmpEnd);
  }
}

void setTimeZone(const TimeChangeRule& dstStart, const TimeChangeRule& stdStart, uint32_t curTime) {
  m_dst = dstStart;
  m_std = stdStart;
  if (calcTimeChanges(year(curTime))) {
    logTimeZoneInfo();
  }
}

void logTimeZoneInfo() {
  String log = F("Current Time Zone: ");
  if (m_std.offset != m_dst.offset) {
    // Summer time
    log += F(" DST time start: ");
    if (m_dstLoc != 0) {
      timeStruct tmp;
      breakTime(m_dstLoc, tmp);
      log += getDateTimeString(tmp, '-', ':', ' ', false);
    }
    log += F(" offset: ");
    log += m_dst.offset;
    log += F(" min");
  }
  // Standard/Winter time.
  log += F("STD time start: ");
  if (m_stdLoc != 0) {
    timeStruct tmp;
    breakTime(m_stdLoc, tmp);
    log += getDateTimeString(tmp, '-', ':', ' ', false);
  }
  log += F(" offset: ");
  log += m_std.offset;
  log += F(" min");
  addLog(LOG_LEVEL_INFO, log);
}

uint32_t makeTime(const timeStruct &tm) {
  // assemble time elements into uint32_t
  // note year argument is offset from 1970 (see macros in time.h to convert to other formats)
  // previous version used full four digit year (or digits since 2000),i.e. 2009 was 2009 or 9
  const uint8_t monthDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  int i;
  uint32_t seconds;

  // seconds from 1970 till 1 jan 00:00:00 of the given year
  seconds= tm.Year*(SECS_PER_DAY * 365);
  for (i = 0; i < tm.Year; i++) {
    if (LEAP_YEAR(i)) {
      seconds +=  SECS_PER_DAY;   // add extra days for leap years
    }
  }

  // add days for this year, months start from 1
  for (i = 1; i < tm.Month; i++) {
    if ( (i == 2) && LEAP_YEAR(tm.Year)) {
      seconds += SECS_PER_DAY * 29;
    } else {
      seconds += SECS_PER_DAY * monthDays[i-1];  //monthDay array starts from 0
    }
  }
  seconds+= (tm.Day-1) * SECS_PER_DAY;
  seconds+= tm.Hour * SECS_PER_HOUR;
  seconds+= tm.Minute * SECS_PER_MIN;
  seconds+= tm.Second;
  return (uint32_t)seconds;
}


///*----------------------------------------------------------------------*
// * Convert the given time change rule to a uint32_t value                 *
// * for the given year.                                                  *
// *----------------------------------------------------------------------*/
uint32_t calcTimeChangeForRule(const TimeChangeRule& r, int yr)
{
    uint8_t m = r.month;     // temp copies of r.month and r.week
    uint8_t w = r.week;
    if (w == 0)              // is this a "Last week" rule?
    {
        if (++m > 12)        // yes, for "Last", go to the next month
        {
            m = 1;
            ++yr;
        }
        w = 1;               // and treat as first week of next month, subtract 7 days later
    }

    // calculate first day of the month, or for "Last" rules, first day of the next month
    timeStruct tm;
    tm.Hour = r.hour;
    tm.Minute = 0;
    tm.Second = 0;
    tm.Day = 1;
    tm.Month = m;
    tm.Year = yr - 1970;
    uint32_t t = makeTime(tm);

    // add offset from the first of the month to r.dow, and offset for the given week
    t += ( (r.dow - weekday(t) + 7) % 7 + (w - 1) * 7 ) * SECS_PER_DAY;
    // back up a week if this is a "Last" rule
    if (r.week == 0) t -= 7 * SECS_PER_DAY;
    return t;
}


/*----------------------------------------------------------------------*
 * Calculate the DST and standard time change points for the given      *
 * given year as local and UTC uint32_t values.                           *
 *----------------------------------------------------------------------*/
bool calcTimeChanges(int yr)
{
  uint32_t dstLoc = calcTimeChangeForRule(m_dst, yr);
  uint32_t stdLoc = calcTimeChangeForRule(m_std, yr);
  bool changed = (m_dstLoc != dstLoc) || (m_stdLoc != stdLoc);
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
uint32_t toLocal(uint32_t utc)
{
    // recalculate the time change points if needed
    if (year(utc) != year(m_dstUTC)) calcTimeChanges(year(utc));

    if (utcIsDST(utc))
        return utc + m_dst.offset * SECS_PER_MIN;
    else
        return utc + m_std.offset * SECS_PER_MIN;
}


/*----------------------------------------------------------------------*
 * Determine whether the given UTC uint32_t is within the DST interval    *
 * or the Standard time interval.                                       *
 *----------------------------------------------------------------------*/
bool utcIsDST(uint32_t utc)
{
    // recalculate the time change points if needed
    if (year(utc) != year(m_dstUTC)) calcTimeChanges(year(utc));

    if (m_stdUTC == m_dstUTC)       // daylight time not observed in this tz
        return false;
    else if (m_stdUTC > m_dstUTC)   // northern hemisphere
        return (utc >= m_dstUTC && utc < m_stdUTC);
    else                            // southern hemisphere
        return !(utc >= m_stdUTC && utc < m_dstUTC);
}

/*----------------------------------------------------------------------*
 * Determine whether the given Local uint32_t is within the DST interval  *
 * or the Standard time interval.                                       *
 *----------------------------------------------------------------------*/
bool locIsDST(uint32_t local)
{
    // recalculate the time change points if needed
    if (year(local) != year(m_dstLoc)) calcTimeChanges(year(local));

    if (m_stdUTC == m_dstUTC)       // daylight time not observed in this tz
        return false;
    else if (m_stdLoc > m_dstLoc)   // northern hemisphere
        return (local >= m_dstLoc && local < m_stdLoc);
    else                            // southern hemisphere
        return !(local >= m_stdLoc && local < m_dstLoc);
}
