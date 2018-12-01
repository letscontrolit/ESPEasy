
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
#include <time.h>

struct tm tm;
uint32_t syncInterval = 3600;  // time sync will be attempted after this many seconds
double sysTime = 0.0; // Use high resolution double to get better sync between nodes when using NTP
uint32_t prevMillis = 0;
uint32_t nextSyncTime = 0;
struct tm tsRise, tsSet;
struct tm sunRise;
struct tm sunSet;

byte PrevMinutes = 0;

float sunDeclination(int doy) {
  // Declination of the sun in radians
  // Formula 2008 by Arnold(at)Barmettler.com, fit to 20 years of average declinations (2008-2027)
  return 0.409526325277017 * sin(0.0169060504029192 * (doy - 80.0856919827619));
}

float diurnalArc(float dec, float lat) {
  // Duration of the half sun path in hours (time from sunrise to the highest level in the south)
  float rad = 0.0174532925; // = pi/180.0
  float height = -50.0 / 60.0 * rad;
  float latRad = lat * rad;
  return 12.0 * acos((sin(height) - sin(latRad) * sin(dec)) / (cos(latRad) * cos(dec))) / 3.1415926536;
}

float equationOfTime(int doy) {
  // Difference between apparent and mean solar time
  // Formula 2008 by Arnold(at)Barmettler.com, fit to 20 years of average equation of time (2008-2027)
  return -0.170869921174742 * sin(0.0336997028793971 * doy + 0.465419984181394) - 0.129890681040717 * sin(0.0178674832556871 * doy - 0.167936777524864);
}

int dayOfYear(int year, int month, int day) {
  // Algorithm borrowed from DateToOrdinal by Ritchie Lawrence, www.commandline.co.uk
  int z = 14 - month;
  z /= 12;
  int y = year + 4800 - z;
  int m = month + 12 * z - 3;
  int j = 153 * m + 2;
  j = j / 5 + day + y * 365 + y / 4 - y / 100 + y / 400 - 32045;
  y = year + 4799;
  int k = y * 365 + y / 4 - y / 100 + y / 400 - 31738;
  return j - k + 1;
}

void calcSunRiseAndSet() {
  int doy = dayOfYear(tm.tm_year, tm.tm_mon, tm.tm_mday);
  float eqt = equationOfTime(doy);
  float dec = sunDeclination(doy);
  float da = diurnalArc(dec, Settings.Latitude);
  float rise = 12 - da - eqt;
  float set = 12 + da - eqt;
  tsRise.tm_hour = (int)rise;
  tsRise.tm_min = (rise - (int)rise) * 60.0;
  tsSet.tm_hour = (int)set;
  tsSet.tm_min = (set - (int)set) * 60.0;
  tsRise.tm_mday = tsSet.tm_mday = tm.tm_mday;
  tsRise.tm_mon = tsSet.tm_mon = tm.tm_mon;
  tsRise.tm_year = tsSet.tm_year = tm.tm_year;
  // Now apply the longitude
  int secOffset_longitude = -1.0 * (Settings.Longitude / 15.0) * 3600;
  tsSet = addSeconds(tsSet, secOffset_longitude, false);
  tsRise = addSeconds(tsRise, secOffset_longitude, false);

  breakTime(toLocal(makeTime(tsRise)), sunRise);
  breakTime(toLocal(makeTime(tsSet)), sunSet);
}


struct tm getSunRise(int secOffset) {
  return addSeconds(tsRise, secOffset, true);
}

struct tm getSunSet(int secOffset) {
  return addSeconds(tsSet, secOffset, true);
}

struct tm addSeconds(const struct tm& ts, int seconds, bool toLocalTime) {
  unsigned long time = makeTime(ts);
  time += seconds;
  if (toLocalTime) {
    time = toLocal(time);
  }
  struct tm result;
  breakTime(time, result);
  return result;
}

void breakTime(unsigned long timeInput, struct tm &tm) {
  uint8_t year;
  uint8_t month, monthLength;
  uint32_t time;
  unsigned long days;
  const uint8_t monthDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

  time = (uint32_t)timeInput;
  tm.tm_sec = time % 60;
  time /= 60; // now it is minutes
  tm.tm_min = time % 60;
  time /= 60; // now it is hours
  tm.tm_hour = time % 24;
  time /= 24; // now it is days
  tm.tm_wday = ((time + 4) % 7) + 1;  // Sunday is day 1

  year = 0;
  days = 0;
  while ((unsigned)(days += (LEAP_YEAR(year) ? 366 : 365)) <= time) {
    year++;
  }
  tm.tm_year = year; // year is offset from 1970

	days -= LEAP_YEAR(year) ? 366 : 365;
	time -= days; // now it is days in this year, starting at 0

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
  tm.tm_mon = month + 1;  // jan is month 1
  tm.tm_mday = time + 1;     // day of month
}

uint32_t getUnixTime()
{
	return static_cast<uint32_t>(sysTime);
}

int getSecOffset(const String& format) {
  int position_minus = format.indexOf('-');
  int position_plus = format.indexOf('+');
  if (position_minus == -1 && position_plus == -1)
    return 0;
  int sign_position = _max(position_minus, position_plus);
  int position_percent = format.indexOf('%', sign_position);
  if (position_percent == -1) {
    return 0;
  }
  String valueStr = getNumerical(format.substring(sign_position, position_percent), true);
  if (!isInt(valueStr)) return 0;
  int value = valueStr.toInt();
  switch (format.charAt(position_percent - 1)) {
    case 'm':
    case 'M':
      return value * 60;
    case 'h':
    case 'H':
      return value * 3600;
  }
  return value;
}

String getSunriseTimeString(char delimiter) {
  return getTimeString(sunRise, delimiter, false, false);
}

String getSunsetTimeString(char delimiter) {
  return getTimeString(sunSet, delimiter, false, false);
}

String getSunriseTimeString(char delimiter, int secOffset) {
  if (secOffset == 0)
    return getSunriseTimeString(delimiter);
  return getTimeString(getSunRise(secOffset), delimiter, false, false);
}

String getSunsetTimeString(char delimiter, int secOffset) {
  if (secOffset == 0)
    return getSunsetTimeString(delimiter);
  return getTimeString(getSunSet(secOffset), delimiter, false, false);
}


unsigned long now() {
  // calculate number of seconds passed since last call to now()
  bool timeSynced = false;
  const long msec_passed = timePassedSince(prevMillis);
	sysTime += static_cast<double>(msec_passed) / 1000.0;
	prevMillis += msec_passed;
	if (nextSyncTime <= sysTime) {
		// nextSyncTime & sysTime are in seconds
		double unixTime_d;
		if (getNtpTime(unixTime_d)) {
			prevMillis = millis();  // restart counting from now (thanks to Korman for this fix)
			timeSynced = true;
			sysTime = unixTime_d;

			applyTimeZone(unixTime_d);
			nextSyncTime = (uint32_t)unixTime_d + syncInterval;
		}
	}
	uint32_t localSystime = toLocal(sysTime);
	breakTime(localSystime, tm);
	if (timeSynced) {
		calcSunRiseAndSet();
		if (Settings.UseRules) {
			String event = statusNTPInitialized ? F("Time#Set") : F("Time#Initialized");
			rulesProcessing(event);
		}
		statusNTPInitialized = true; //@giig1967g: setting system variable %isntp%
	}
  return (unsigned long)localSystime;
}

int year(unsigned long t) {
  struct tm tmp;
  breakTime(t, tmp);
  return 1970 + tmp.tm_year;
}

int weekday(unsigned long t) {
  struct tm tmp;
  breakTime(t, tmp);
  return tmp.tm_wday;
}



int year()
{
  return 1970 + tm.tm_year;
}

byte month()
{
  return tm.tm_mon;
}

byte day()
{
  return tm.tm_mday;
}

byte hour()
{
  return tm.tm_hour;
}

byte minute()
{
  return tm.tm_min;
}

byte second()
{
  return tm.tm_sec;
}

// day of week, sunday is day 1
int weekday()
{
  return tm.tm_wday;
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
  if (tm.tm_min != PrevMinutes)
  {
    PluginCall(PLUGIN_CLOCK_IN, 0, dummyString);
    PrevMinutes = tm.tm_min;
    if (Settings.UseRules)
    {
      String event;
      event.reserve(21);
      event = F("Clock#Time=");
      event += weekday_str();
      event += ",";
      if (hour() < 10)
        event += '0';
      event += hour();
      event += ":";
      if (minute() < 10)
        event += '0';
      event += minute();
      rulesProcessing(event);
    }
  }
}


bool getNtpTime(double& unixTime_d)
{
	if (!Settings.UseNTP || !WiFiConnected(10)) {
		return false;
	}
	IPAddress timeServerIP;
	String log = F("NTP  : NTP host ");
	if (Settings.NTPHost[0] != 0) {
		resolveHostByName(Settings.NTPHost, timeServerIP);
		log += Settings.NTPHost;
		// When single set host fails, retry again in a minute
		nextSyncTime = sysTime + 20;
	}else  {
		// Have to do a lookup eacht time, since the NTP pool always returns another IP
		String ntpServerName = String(random(0, 3));
		ntpServerName += F(".pool.ntp.org");
		resolveHostByName(ntpServerName.c_str(), timeServerIP);
		log += ntpServerName;
		// When pool host fails, retry can be much sooner
		nextSyncTime = sysTime + 5;
	}

	log += " (";
	log += timeServerIP.toString();
	log += ')';

	if (!hostReachable(timeServerIP)) {
		log += F(" unreachable");
		addLog(LOG_LEVEL_INFO, log);
		return false;
	}

  WiFiUDP udp;
  if (!beginWiFiUDP_randomPort(udp))
    return 0;

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
  if (udp.beginPacket(timeServerIP, 123) == 0) { //NTP requests are to port 123
    udp.stop();
    return 0;
  }
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();



	uint32_t beginWait = millis();
	while (!timeOutReached(beginWait + 1000)) {
		int size = udp.parsePacket();
		int remotePort = udp.remotePort();
		if (size >= NTP_PACKET_SIZE && remotePort == 123) {
			udp.read(packetBuffer, NTP_PACKET_SIZE); // read packet into the buffer

			// For more detailed info on improving accuracy, see:
			// https://github.com/lettier/ntpclient/issues/4#issuecomment-360703503
			// For now, we simply use half the reply time as delay compensation.

			unsigned long secsSince1900;
			// convert four bytes starting at location 40 to a long integer
			// TX time is used here.
			secsSince1900 = (unsigned long)packetBuffer[40] << 24;
			secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
			secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
			secsSince1900 |= (unsigned long)packetBuffer[43];
			uint32_t txTm = secsSince1900 - 2208988800UL;

			unsigned long txTm_f;
			txTm_f = (unsigned long)packetBuffer[44] << 24;
			txTm_f |= (unsigned long)packetBuffer[45] << 16;
			txTm_f |= (unsigned long)packetBuffer[46] << 8;
			txTm_f |= (unsigned long)packetBuffer[47];
			// Convert seconds to double
			unixTime_d = static_cast<double>(txTm);
			// Add fractional part.
			unixTime_d += (static_cast<double>(txTm_f) / 4294967295.0);

			long total_delay = timePassedSince(beginWait);
			// compensate for the delay by adding half the total delay
			// N.B. unixTime_d is in seconds and delay in msec.
			double delay_compensation = static_cast<double>(total_delay) / 2000.0;
			unixTime_d += delay_compensation;

			if (loglevelActiveFor(LOG_LEVEL_INFO)) {
				String log = F("NTP  : NTP replied: delay ");
				log += total_delay;
				log += F(" mSec");
				log += F(" Accuracy increased by ");
				double fractpart, intpart;
				fractpart = modf(unixTime_d, &intpart);
				if (fractpart < delay_compensation) {
					// We gained more than 1 second in accuracy
					fractpart += 1.0;
				}
				log += String(fractpart, 3);
				log += F(" seconds");
				addLog(LOG_LEVEL_INFO, log);
			}
			udp.stop();

			return true;
		}
		delay(10);
	}
	addLog(LOG_LEVEL_DEBUG_MORE, F("NTP  : No reply"));
	udp.stop();
	return false;
}



/********************************************************************************************\
   Unsigned long Timer timeOut check
 \*********************************************************************************************/

// Return the time difference as a signed value, taking into account the timers may overflow.
// Returned timediff is between -24.9 days and +24.9 days.
// Returned value is positive when "next" is after "prev"
long timeDiff(const unsigned long prev, const unsigned long next)
{
  unsigned long start = ESP.getCycleCount();
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
  unsigned long end = ESP.getCycleCount();
  if (end > start) {
    ++timediff_calls;
    timediff_cpu_cycles_total += (end - start);
  }
  return signed_diff;
}

// Compute the number of milliSeconds passed since timestamp given.
// N.B. value can be negative if the timestamp has not yet been reached.
long timePassedSince(unsigned long timestamp) {
  return timeDiff(timestamp, millis());
}

long usecPassedSince(unsigned long timestamp) {
  return timeDiff(timestamp, micros());
}

// Check if a certain timeout has been reached.
boolean timeOutReached(unsigned long timer) {
  const long passed = timePassedSince(timer);
  return passed >= 0;
}

boolean usecTimeOutReached(unsigned long timer) {
  const long passed = usecPassedSince(timer);
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
		time += '-';
	else
		time += x;

	x = (lngTime >> 8) & 0xf;
	if (x == 0xf)
		time += "*";
	else if (x == 0xe)
		time += '-';
	else
		time += x;

	time += ":";

	x = (lngTime >> 4) & 0xf;
	if (x == 0xf)
		time += "*";
	else if (x == 0xe)
		time += '-';
	else
		time += x;

	x = (lngTime) & 0xf;
	if (x == 0xf)
		time += "*";
	else if (x == 0xe)
		time += '-';
	else
		time += x;

	return time;
}

// returns the current Date separated by the given delimiter
// date format example with '-' delimiter: 2016-12-31 (YYYY-MM-DD)
String getDateString(const struct tm& ts, char delimiter) {
  char DateString[20]; //19 digits plus the null char
  const int year = 1970 + ts.tm_year;
  sprintf_P(DateString, PSTR("%4d%c%02d%c%02d"), year, delimiter, ts.tm_mon, delimiter, ts.tm_mday);
  return DateString;
}

String getDateString(char delimiter)
{
	return getDateString(tm, delimiter);
}
String getDateString(const struct tm& ts)
{
  return getDateString(tm, ':');
}
// returns the current Date without delimiter
// date format example: 20161231 (YYYYMMDD)
String getDateString()
{
  return getDateString('\0');
}

// returns the current Time separated by the given delimiter
// time format example with ':' delimiter: 23:59:59 (HH:MM:SS)
String getTimeString(const struct tm& ts, char delimiter, bool am_pm, bool show_seconds)
{
  char TimeString[20]; //19 digits plus the null char
  if (am_pm) {
    uint8_t hour(ts.tm_hour % 12);
    if (hour == 0) { hour = 12; }
    const char a_or_p = ts.tm_hour < 12 ? 'A' : 'P';
    if (show_seconds) {
      sprintf_P(TimeString, PSTR("%d%c%02d%c%02d %cM"),
        hour, delimiter, ts.tm_min, delimiter, ts.tm_sec, a_or_p);
    } else {
      sprintf_P(TimeString, PSTR("%d%c%02d %cM"),
        hour, delimiter, ts.tm_min, a_or_p);
    }
  } else {
    if (show_seconds) {
      sprintf_P(TimeString, PSTR("%02d%c%02d%c%02d"),
        ts.tm_hour, delimiter, ts.tm_min, delimiter, ts.tm_sec);
    } else {
      sprintf_P(TimeString, PSTR("%d%c%02d"),
        ts.tm_hour, delimiter, ts.tm_min);
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
String getDateTimeString(const struct tm& ts, char dateDelimiter, char timeDelimiter,  char dateTimeDelimiter, bool am_pm)
{
  String ret = getDateString(ts, dateDelimiter);
  if (dateTimeDelimiter != '\0')
    ret += dateTimeDelimiter;
  ret += getTimeString(ts, timeDelimiter, am_pm, true);
  return ret;
}

String getDateTimeString(const struct tm& ts)
{
  return getDateTimeString(ts,'-', ':', ' ', false);
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
  int w, x, y;
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
    for (x = TmpStr1.length() - 1; x >= 0; x--)
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
  #undef TmpStr1Length
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
