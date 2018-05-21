
/********************************************************************************************\
  Time stuff
  \*********************************************************************************************/

#define SECS_PER_MIN  (60UL)
#define SECS_PER_HOUR (3600UL)
#define SECS_PER_DAY  (SECS_PER_HOUR * 24UL)
#define DAYS_PER_WEEK (7UL)
#define SECS_PER_WEEK (SECS_PER_DAY * DAYS_PER_WEEK)
#define SECS_PER_YEAR (SECS_PER_WEEK * 52UL)
#define SECS_YR_2000  (946684800UL) // the time at the start of y2k
#define LEAP_YEAR(Y)     ( ((1970+Y)>0) && !((1970+Y)%4) && ( ((1970+Y)%100) || !((1970+Y)%400) ) )

timeStruct tm;
uint32_t syncInterval = 3600;  // time sync will be attempted after this many seconds
uint32_t sysTime = 0;
uint32_t prevMillis = 0;
uint32_t nextSyncTime = 0;

byte PrevMinutes = 0;

void breakTime(unsigned long timeInput, struct timeStruct &tm) {
  uint8_t year;
  uint8_t month, monthLength;
  uint32_t time;
  unsigned long days;
  const uint8_t monthDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

  time = (uint32_t)timeInput;
  tm.Second = time % 60;
  time /= 60; // now it is minutes
  tm.Minute = time % 60;
  time /= 60; // now it is hours
  tm.Hour = time % 24;
  time /= 24; // now it is days
  tm.Wday = ((time + 4) % 7) + 1;  // Sunday is day 1

  year = 0;
  days = 0;
  while ((unsigned)(days += (LEAP_YEAR(year) ? 366 : 365)) <= time) {
    year++;
  }
  tm.Year = year; // year is offset from 1970

  days -= LEAP_YEAR(year) ? 366 : 365;
  time  -= days; // now it is days in this year, starting at 0

  days = 0;
  month = 0;
  monthLength = 0;
  for (month = 0; month < 12; month++) {
    if (month == 1) { // february
      if (LEAP_YEAR(year)) {
        monthLength = 29;
      } else {
        monthLength = 28;
      }
    } else {
      monthLength = monthDays[month];
    }

    if (time >= monthLength) {
      time -= monthLength;
    } else {
      break;
    }
  }
  tm.Month = month + 1;  // jan is month 1
  tm.Day = time + 1;     // day of month
}

void setTime(unsigned long t) {
  sysTime = (uint32_t)t;
  applyTimeZone(t);
  nextSyncTime = (uint32_t)t + syncInterval;
  prevMillis = millis();  // restart counting from now (thanks to Korman for this fix)
  if (Settings.UseRules)
  {
    static bool firstUpdate = true;
    String event = firstUpdate ? F("Time#Initialized") : F("Time#Set");
    firstUpdate = false;
    rulesProcessing(event);
  }
}

uint32_t getUnixTime() {
  return sysTime;
}

unsigned long now() {
  // calculate number of seconds passed since last call to now()
  const long msec_passed = timePassedSince(prevMillis);
  const long seconds_passed = msec_passed / 1000;
  sysTime += seconds_passed;
  prevMillis += seconds_passed * 1000;
  if (nextSyncTime <= sysTime) {
    // nextSyncTime & sysTime are in seconds
    unsigned long  t = getNtpTime();
    if (t != 0) {
      setTime(t);
    }
  }
  uint32_t localSystime = toLocal(sysTime);
  breakTime(localSystime, tm);
  return (unsigned long)localSystime;
}

int year(unsigned long t) {
  timeStruct tmp;
  breakTime(t, tmp);
  return 1970 + tmp.Year;
}

int weekday(unsigned long t) {
  timeStruct tmp;
  breakTime(t, tmp);
  return tmp.Wday;
}



int year()
{
  return 1970 + tm.Year;
}

byte month()
{
	return tm.Month;
}

byte day()
{
	return tm.Day;
}

byte hour()
{
  return tm.Hour;
}

byte minute()
{
  return tm.Minute;
}

byte second()
{
	return tm.Second;
}

// day of week, sunday is day 1
int weekday()
{
  return tm.Wday;
}

String weekday_str()
{
  const int wday(weekday() - 1); // here: Count from Sunday = 0
  const String weekDays = F("SunMonTueWedThuFriSat");
  return weekDays.substring(wday * 3, wday * 3 + 3);
}

void initTime()
{
  nextSyncTime = 0;
  now();
}

void checkTime()
{
  now();
  if (tm.Minute != PrevMinutes)
  {
    PluginCall(PLUGIN_CLOCK_IN, 0, dummyString);
    PrevMinutes = tm.Minute;
    if (Settings.UseRules)
    {
      String event;
      event.reserve(21);
      event = F("Clock#Time=");
      event += weekday_str();
      event += ",";
      if (hour() < 10)
        event += "0";
      event += hour();
      event += ":";
      if (minute() < 10)
        event += "0";
      event += minute();
      rulesProcessing(event);
    }
  }
}


unsigned long getNtpTime()
{
  if (!Settings.UseNTP || !WiFiConnected(10)) {
    return 0;
  }
  IPAddress timeServerIP;
  String log = F("NTP  : NTP host ");
  if (Settings.NTPHost[0] != 0) {
    WiFi.hostByName(Settings.NTPHost, timeServerIP);
    log += Settings.NTPHost;
    // When single set host fails, retry again in a minute
    nextSyncTime = sysTime + 20;
  }
  else {
    // Have to do a lookup eacht time, since the NTP pool always returns another IP
    String ntpServerName = String(random(0, 3));
    ntpServerName += F(".pool.ntp.org");
    WiFi.hostByName(ntpServerName.c_str(), timeServerIP);
    log += ntpServerName;
    // When pool host fails, retry can be much sooner
    nextSyncTime = sysTime + 5;
  }

  log += F(" (");
  log += timeServerIP.toString();
  log += F(")");

  if (!hostReachable(timeServerIP)) {
    log += F(" unreachable");
    addLog(LOG_LEVEL_INFO, log);
    return 0;
  }

  WiFiUDP udp;
  udp.begin(123);

  const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
  byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

  log += F(" queried");
  addLog(LOG_LEVEL_DEBUG_MORE, log);

  while (udp.parsePacket() > 0) ; // discard any previously received packets

  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  udp.beginPacket(timeServerIP, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();

  uint32_t beginWait = millis();
  while (!timeOutReached(beginWait + 1000)) {
    int size = udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      log = F("NTP  : NTP replied: ");
      log += timePassedSince(beginWait);
      log += F(" mSec");
      addLog(LOG_LEVEL_DEBUG_MORE, log);
      return secsSince1900 - 2208988800UL;
    }
    delay(10);
  }
  log = F("NTP  : No reply");
  addLog(LOG_LEVEL_DEBUG_MORE, log);
  return 0;
}



/********************************************************************************************\
  Unsigned long Timer timeOut check
\*********************************************************************************************/

// Return the time difference as a signed value, taking into account the timers may overflow.
// Returned timediff is between -24.9 days and +24.9 days.
// Returned value is positive when "next" is after "prev"
long timeDiff(const unsigned long prev, const unsigned long next)
{
  long signed_diff = 0;
  // To cast a value to a signed long, the difference may not exceed half the ULONG_MAX
  const unsigned long half_max_unsigned_long = 2147483647u; // = 2^31 -1
  if (next >= prev) {
    const unsigned long diff = next - prev;
    if (diff <= half_max_unsigned_long) {
      // Normal situation, just return the difference.
      // Difference is a positive value.
      signed_diff = static_cast<long>(diff);
    } else {
      // prev has overflow, return a negative difference value
      signed_diff = static_cast<long>((ULONG_MAX - next) + prev + 1u);
      signed_diff = -1 * signed_diff;
    }
  } else {
    // next < prev
    const unsigned long diff = prev - next;
    if (diff <= half_max_unsigned_long) {
      // Normal situation, return a negative difference value
      signed_diff = static_cast<long>(diff);
      signed_diff = -1 * signed_diff;
    } else {
      // next has overflow, return a positive difference value
      signed_diff = static_cast<long>((ULONG_MAX - prev) + next + 1u);
    }
  }
  return signed_diff;
}

// Compute the number of milliSeconds passed since timestamp given.
// N.B. value can be negative if the timestamp has not yet been reached.
long timePassedSince(unsigned long timestamp) {
  return timeDiff(timestamp, millis());
}

// Check if a certain timeout has been reached.
boolean timeOutReached(unsigned long timer)
{
  const long passed = timePassedSince(timer);
  return passed >= 0;
}

void setNextTimeInterval(unsigned long& timer, const unsigned long step) {
  timer += step;
  const long passed = timePassedSince(timer);
  if (passed < 0) {
    // Event has not yet happened, which is fine.
    return;
  }
  if (static_cast<unsigned long>(passed) > step) {
    // No need to keep running behind, start again.
    timer = millis() + step;
    return;
  }
  // Try to get in sync again.
  timer = millis() + (step - passed);
}


/********************************************************************************************\
  Convert  a 32 bit integer into a string like "Sun,12:30"
  \*********************************************************************************************/
String timeLong2String(unsigned long lngTime)
{
  unsigned long x = 0;
  String time = "";

  x = (lngTime >> 16) & 0xf;
  if (x == 0x0f)
    x = 0;
  String weekDays = F("AllSunMonTueWedThuFriSatWrkWkd");
  time = weekDays.substring(x * 3, x * 3 + 3);
  time += ",";

  x = (lngTime >> 12) & 0xf;
  if (x == 0xf)
    time += "*";
  else if (x == 0xe)
    time += "-";
  else
    time += x;

  x = (lngTime >> 8) & 0xf;
  if (x == 0xf)
    time += "*";
  else if (x == 0xe)
    time += "-";
  else
    time += x;

  time += ":";

  x = (lngTime >> 4) & 0xf;
  if (x == 0xf)
    time += "*";
  else if (x == 0xe)
    time += "-";
  else
    time += x;

  x = (lngTime) & 0xf;
  if (x == 0xf)
    time += "*";
  else if (x == 0xe)
    time += "-";
  else
    time += x;

  return time;
}

// returns the current Date separated by the given delimiter
// date format example with '-' delimiter: 2016-12-31 (YYYY-MM-DD)
String getDateString(const timeStruct& ts, char delimiter) {
  char DateString[20]; //19 digits plus the null char
  const int year = 1970 + ts.Year;
  sprintf_P(DateString, PSTR("%4d%c%02d%c%02d"), year, delimiter, ts.Month, delimiter, ts.Day);
  return DateString;
}

String getDateString(char delimiter)
{
  return getDateString(tm, delimiter);
}

// returns the current Date without delimiter
// date format example: 20161231 (YYYYMMDD)
String getDateString()
{
	return getDateString('\0');
}

// returns the current Time separated by the given delimiter
// time format example with ':' delimiter: 23:59:59 (HH:MM:SS)
String getTimeString(const timeStruct& ts, char delimiter, bool am_pm, bool show_seconds)
{
  char TimeString[20]; //19 digits plus the null char
  if (am_pm) {
    uint8_t hour(ts.Hour % 12);
    if (hour == 0) { hour = 12; }
    const char a_or_p = ts.Hour < 12 ? 'A' : 'P';
    if (show_seconds) {
      sprintf_P(TimeString, PSTR("%d%c%02d%c%02d %cM"),
        hour, delimiter, ts.Minute, delimiter, ts.Second, a_or_p);
    } else {
      sprintf_P(TimeString, PSTR("%d%c%02d %cM"),
        hour, delimiter, ts.Minute, a_or_p);
    }
  } else {
    if (show_seconds) {
      sprintf_P(TimeString, PSTR("%02d%c%02d%c%02d"),
        ts.Hour, delimiter, ts.Minute, delimiter, ts.Second);
    } else {
      sprintf_P(TimeString, PSTR("%d%c%02d"),
        ts.Hour, delimiter, ts.Minute);
    }
  }
  return TimeString;
}

String getTimeString(char delimiter, bool show_seconds /*=true*/)
{
  return getTimeString(tm, delimiter, false, show_seconds);
}

String getTimeString_ampm(char delimiter, bool show_seconds /*=true*/)
{
  return getTimeString(tm, delimiter, true, show_seconds);
}

// returns the current Time without delimiter
// time format example: 235959 (HHMMSS)
String getTimeString()
{
	return getTimeString('\0');
}

String getTimeString_ampm()
{
	return getTimeString_ampm('\0');
}

// returns the current Date and Time separated by the given delimiter
// if called like this: getDateTimeString('\0', '\0', '\0');
// it will give back this: 20161231235959  (YYYYMMDDHHMMSS)
String getDateTimeString(const timeStruct& ts, char dateDelimiter, char timeDelimiter,  char dateTimeDelimiter, bool am_pm)
{
	String ret = getDateString(ts, dateDelimiter);
	if (dateTimeDelimiter != '\0')
		ret += dateTimeDelimiter;
	ret += getTimeString(ts, timeDelimiter, am_pm, true);
	return ret;
}

String getDateTimeString(char dateDelimiter, char timeDelimiter,  char dateTimeDelimiter) {
  return getDateTimeString(tm, dateDelimiter, timeDelimiter, dateTimeDelimiter, false);
}

String getDateTimeString_ampm(char dateDelimiter, char timeDelimiter,  char dateTimeDelimiter) {
  return getDateTimeString(tm, dateDelimiter, timeDelimiter, dateTimeDelimiter, true);
}

/********************************************************************************************\
  Convert a string like "Sun,12:30" into a 32 bit integer
  \*********************************************************************************************/
unsigned long string2TimeLong(const String &str)
{
  // format 0000WWWWAAAABBBBCCCCDDDD
  // WWWW=weekday, AAAA=hours tens digit, BBBB=hours, CCCC=minutes tens digit DDDD=minutes

  char command[20];
  char TmpStr1[10];
  int w, x, y;
  unsigned long a;
  {
    // Within a scope so the tmpString is only used for copy.
    String tmpString(str);
    tmpString.toLowerCase();
    tmpString.toCharArray(command, 20);
  }
  unsigned long lngTime = 0;

  if (GetArgv(command, TmpStr1, 1))
  {
    String day = TmpStr1;
    String weekDays = F("allsunmontuewedthufrisatwrkwkd");
    y = weekDays.indexOf(TmpStr1) / 3;
    if (y == 0)
      y = 0xf; // wildcard is 0xf
    lngTime |= (unsigned long)y << 16;
  }

  if (GetArgv(command, TmpStr1, 2))
  {
    y = 0;
    for (x = strlen(TmpStr1) - 1; x >= 0; x--)
    {
      w = TmpStr1[x];
      if ( (w >= '0' && w <= '9') || w == '*')
      {
        a = 0xffffffff  ^ (0xfUL << y); // create mask to clean nibble position y
        lngTime &= a; // maak nibble leeg
        if (w == '*')
          lngTime |= (0xFUL << y); // fill nibble with wildcard value
        else
          lngTime |= (w - '0') << y; // fill nibble with token
        y += 4;
      }
      else
        if (w == ':');
      else
      {
        break;
      }
    }
  }
  return lngTime;
}

/********************************************************************************************\
  Match clock event
  \*********************************************************************************************/
boolean matchClockEvent(unsigned long clockEvent, unsigned long clockSet)
{
  unsigned long Mask;
  for (byte y = 0; y < 8; y++)
  {
    if (((clockSet >> (y * 4)) & 0xf) == 0xf)  // if nibble y has the wildcard value 0xf
    {
      Mask = 0xffffffff  ^ (0xFUL << (y * 4)); // Mask to wipe nibble position y.
      clockEvent &= Mask;                      // clear nibble
      clockEvent |= (0xFUL << (y * 4));        // fill with wildcard value 0xf
    }
  }

  if (((clockSet >> (16)) & 0xf) == 0x8)     // if weekday nibble has the wildcard value 0x8 (workdays)
    if (weekday() >= 2 and weekday() <= 6)   // and we have a working day today...
    {
      Mask = 0xffffffff  ^ (0xFUL << (16));  // Mask to wipe nibble position.
      clockEvent &= Mask;                    // clear nibble
      clockEvent |= (0x8UL << (16));         // fill with wildcard value 0x8
    }

  if (((clockSet >> (16)) & 0xf) == 0x9)     // if weekday nibble has the wildcard value 0x9 (weekends)
    if (weekday() == 1 or weekday() == 7)    // and we have a weekend day today...
    {
      Mask = 0xffffffff  ^ (0xFUL << (16));  // Mask to wipe nibble position.
      clockEvent &= Mask;                    // clear nibble
      clockEvent |= (0x9UL << (16));         // fill with wildcard value 0x9
    }

  if (clockEvent == clockSet)
    return true;
  return false;
}
