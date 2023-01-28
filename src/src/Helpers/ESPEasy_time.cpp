#include "../Helpers/ESPEasy_time.h"

#include "../../ESPEasy_common.h"

#include "../CustomBuild/CompiletimeDefines.h"

#include "../DataStructs/TimingStats.h"
#include "../DataTypes/TimeSource.h"

#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../ESPEasyCore/ESPEasyNetwork.h"

#include "../Globals/EventQueue.h"
#include "../Globals/NetworkState.h"
#include "../Globals/Nodes.h"
#include "../Globals/RTC.h"
#include "../Globals/Settings.h"
#include "../Globals/TimeZone.h"

#include "../Helpers/Convert.h"
#include "../Helpers/Hardware.h"
#include "../Helpers/Misc.h"
#include "../Helpers/Networking.h"
#include "../Helpers/Numerical.h"
#include "../Helpers/StringConverter.h"

#include "../Helpers/ESPEasy_time_calc.h"

#include <time.h>

#if FEATURE_EXT_RTC
# include <RTClib.h>
#endif // if FEATURE_EXT_RTC


ESPEasy_time::ESPEasy_time() {
  memset(&local_tm, 0, sizeof(tm));
  memset(&tsRise,   0, sizeof(tm));
  memset(&tsSet,    0, sizeof(tm));
  memset(&sunRise,  0, sizeof(tm));
  memset(&sunSet,   0, sizeof(tm));
}

struct tm ESPEasy_time::addSeconds(const struct tm& ts, int seconds, bool toLocalTime, bool fromLocalTime) const {
  unsigned long time = makeTime(ts);

  if (fromLocalTime) {
    time = time_zone.fromLocal(time);
  }

  time += seconds;

  if (toLocalTime) {
    time = time_zone.toLocal(time);
  }
  struct tm result;

  breakTime(time, result);
  return result;
}

void ESPEasy_time::restoreFromRTC()
{
  static bool firstCall = true;

#if FEATURE_EXT_RTC
  uint32_t    unixtime  = 0;
  if (ExtRTC_get(unixtime)) {
    setExternalTimeSource(unixtime, timeSource_t::External_RTC_time_source);
    firstCall = false;
    return;
  }
#endif

  if (firstCall && (RTC.lastSysTime != 0) && (RTC.deepSleepState != 1)) {
    firstCall = false;

    // Check to see if we have some kind of believable timestamp
    // It should not be before the build time
    // Still it makes sense to restore RTC time to get some kind of continuous logging when no time source is available.
    // ToDo TD-er: Fix this when time travel appears to be possible
    setExternalTimeSource(RTC.lastSysTime,
                          RTC.lastSysTime < get_build_unixtime()
        ? timeSource_t::No_time_source
        : timeSource_t::Restore_RTC_time_source);

    // Do not add the current uptime as offset. This will be done when calling now()
    lastSyncTime_ms = 0;
    initTime();
  }
}

void ESPEasy_time::setExternalTimeSource(double time, timeSource_t new_timeSource, uint8_t unitnr) {
  if ((new_timeSource == timeSource) && (new_timeSource != timeSource_t::Manual_set)) {
    // Update from the same type of time source, except when manually adjusting the time
    if (timePassedSince(lastSyncTime_ms) < EXT_TIME_SOURCE_MIN_UPDATE_INTERVAL_MSEC) {
      return;
    }
  }

  if ((timeSource < new_timeSource) &&
      (new_timeSource != timeSource_t::No_time_source) &&
      (new_timeSource != timeSource_t::Manual_set))
  {
    // New time source is potentially worse than the current one.
    if (computeExpectedWander(timeSource, lastSyncTime_ms) <
        computeExpectedWander(new_timeSource, millis())) { return; }
  }

  if ((new_timeSource == timeSource_t::No_time_source) ||
      (new_timeSource == timeSource_t::Manual_set) ||
      (time > get_build_unixtime())) {
#ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = F("Time : Set Ext. Time Source: ");
      log += toString(new_timeSource);
      log += F(" time: ");
      log += static_cast<uint32_t>(time);
      addLogMove(LOG_LEVEL_INFO, log);
    }
#endif // ifndef BUILD_NO_DEBUG
    extTimeSource       = new_timeSource;
    externalUnixTime_d  = time;
    lastSyncTime_ms     = millis();
    timeSource_p2p_unit = unitnr;
    initTime();
  }
}

uint32_t ESPEasy_time::getUnixTime() const
{
  return static_cast<uint32_t>(sysTime);
}

uint32_t ESPEasy_time::getUnixTime(uint32_t& unix_time_frac) const
{
  const uint32_t seconds(getUnixTime());
  double tmp(sysTime);

  tmp           -= seconds;
  tmp           *= 4294967295.0;
  unix_time_frac = tmp;
  return seconds;
}

void ESPEasy_time::initTime()
{
  nextSyncTime = 0;
  now();
}

unsigned long ESPEasy_time::now() {
  // calculate number of seconds passed since last call to now()
  bool timeSynced        = false;
  const long msec_passed = timePassedSince(prevMillis);

  sysTime    += static_cast<double>(msec_passed) / 1000.0;
  prevMillis += msec_passed;

  if (nextSyncTime <= sysTime) {
    // nextSyncTime & sysTime are in seconds
    double unixTime_d = -1.0;

    bool updatedTime = false;

    if (externalUnixTime_d > 0.0) {
      unixTime_d = externalUnixTime_d;

      // Correct for the delay between the last received external time and applying it
      unixTime_d        += (timePassedSince(lastSyncTime_ms) / 1000.0);
      externalUnixTime_d = -1.0;
      syncInterval       = EXT_TIME_SOURCE_MIN_UPDATE_INTERVAL_SEC;
      updatedTime        = true;
      timeSource         = extTimeSource;
    }

    if (!isExternalTimeSource(timeSource)
        || (extTimeSource <= timeSource)
        || (timePassedSince(lastSyncTime_ms) > static_cast<long>(1000 * syncInterval))) {
      if (getNtpTime(unixTime_d)) {
        updatedTime = true;
      } else {
        #if FEATURE_ESPEASY_P2P
        double tmp_unixtime_d;

        if (!updatedTime && Nodes.getUnixTime(tmp_unixtime_d, timeSource_p2p_unit)) {
          unixTime_d   = tmp_unixtime_d;
          timeSource   = timeSource_t::ESPEASY_p2p_UDP;
          updatedTime  = true;
          syncInterval = EXT_TIME_SOURCE_MIN_UPDATE_INTERVAL_SEC;
        }
        #endif // if FEATURE_ESPEASY_P2P

        #if FEATURE_EXT_RTC
        uint32_t tmp_unixtime = 0;
        if (!updatedTime &&
            (timeSource > timeSource_t::External_RTC_time_source) && // No need to set from ext RTC more than once.
            ExtRTC_get(tmp_unixtime)) {
          unixTime_d   = tmp_unixtime;
          timeSource   = timeSource_t::External_RTC_time_source;
          updatedTime  = true;
          syncInterval = 120; // Allow sync in 2 minutes to see if we get some better options from p2p nodes.
        }
        #endif
      }
    }

    // Clear the external time source so it has to be set again with updated values.
    extTimeSource = timeSource_t::No_time_source;

    if (timeSource != timeSource_t::ESPEASY_p2p_UDP) { timeSource_p2p_unit = 0; }

    if (updatedTime) {
      START_TIMER;
      const double time_offset = unixTime_d - sysTime - (timePassedSince(prevMillis) / 1000.0);

      if (statusNTPInitialized && (time_offset < 1.0)) {
        // Clock instability in ppm
        timeWander  = ((time_offset * 1000000.0) / timePassedSince(lastTimeWanderCalculation_ms));
        timeWander *= 1000.0f;
      }

      prevMillis                   = millis(); // restart counting from now (thanks to Korman for this fix)
      lastTimeWanderCalculation_ms = prevMillis;

      timeSynced = true;

      sysTime = unixTime_d;

      #if FEATURE_EXT_RTC
      // External RTC only stores with second resolution.
      // Thus to limit the error to +/- 500 ms, round the sysTime instead of just casting it.
      ExtRTC_set(round(sysTime));
      #endif
      {
        const unsigned long abs_time_offset_ms = std::abs(time_offset) * 1000;

        if (timeSource == timeSource_t::NTP_time_source) {
          // May need to lessen the load on the NTP servers, randomize the sync interval
          if (abs_time_offset_ms < 1000) {
            // offset is less than 1 second, so we consider it a regular time sync.
            if (abs_time_offset_ms < 100) {
              // Good clock stability, use 5 - 6 hour interval
              syncInterval = random(18000, 21600);
            } else {
              // Dynamic interval between 30 minutes ... 5 hours.
              syncInterval = 1800000 / abs_time_offset_ms;
            }
          } else {
            syncInterval = 3600;
          }

          if (syncInterval <= 3600) {
            syncInterval = random(3600, 4000);
          }
        } else if (timeSource == timeSource_t::No_time_source) {
          syncInterval = 60;
        } else {
          syncInterval = 3600;
        }
      }

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        String log = F("Time set to ");
        log += doubleToString(unixTime_d, 3);

        if ((-86400 < time_offset) && (time_offset < 86400)) {
          // Only useful to show adjustment if it is less than a day.
          log += F(" Time adjusted by ");
          log += doubleToString(time_offset * 1000.0);
          log += F(" msec. Wander: ");
          log += doubleToString(timeWander, 1);
          log += F(" ppm");
          log += F(" Source: ");
          log += toString(timeSource);
        }
        addLogMove(LOG_LEVEL_INFO, log);
      }

      time_zone.applyTimeZone(unixTime_d);
      lastSyncTime_ms = millis();
      nextSyncTime    = (uint32_t)unixTime_d + syncInterval;

      if (isExternalTimeSource(timeSource)) {
        #ifdef USES_ESPEASY_NOW
        ESPEasy_now_handler.sendNTPbroadcast();
        #endif // ifdef USES_ESPEASY_NOW
      }
      STOP_TIMER(SYSTIME_UPDATED);
    }
  }
  RTC.lastSysTime = static_cast<unsigned long>(sysTime);
  uint32_t localSystime = time_zone.toLocal(sysTime);
  breakTime(localSystime, local_tm);

  if (timeSynced) {
    calcSunRiseAndSet();

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = F("Local time: ");
      log += getDateTimeString('-', ':', ' ');
      addLogMove(LOG_LEVEL_INFO, log);
    }
    {
      // Notify plugins the time has been set.
      String dummy;
      PluginCall(PLUGIN_TIME_CHANGE, 0, dummy);
    }

    if (Settings.UseRules) {
      if (statusNTPInitialized) {
        eventQueue.add(F("Time#Set"));
      } else {
        eventQueue.add(F("Time#Initialized"));
      }
    }
    statusNTPInitialized = true; // @giig1967g: setting system variable %isntp%
  }
  return (unsigned long)localSystime;
}

bool ESPEasy_time::reportNewMinute()
{
  now();

  int cur_min = local_tm.tm_min;

  if (!systemTimePresent()) {
    // Use millis() to compute some "minute"
    cur_min = (millis() / 60000) % 60;
  }

  if (cur_min == PrevMinutes)
  {
    return false;
  }
  PrevMinutes = cur_min;
  return true;
}

bool ESPEasy_time::systemTimePresent() const {
  switch (timeSource) {
    case timeSource_t::No_time_source:
    case timeSource_t::Restore_RTC_time_source:
      break;
    case timeSource_t::External_RTC_time_source:
    case timeSource_t::GPS_time_source:
    case timeSource_t::GPS_PPS_time_source:
    case timeSource_t::ESP_now_peer:
    case timeSource_t::ESPEASY_p2p_UDP:
    case timeSource_t::Manual_set:
      return true;
    case timeSource_t::NTP_time_source:
      return getUnixTime() > get_build_unixtime();
  }
  return nextSyncTime > 0 || externalUnixTime_d > get_build_unixtime();
}

bool ESPEasy_time::getNtpTime(double& unixTime_d)
{
  if (!Settings.UseNTP() || !NetworkConnected(10)) {
    return false;
  }

  if (lastNTPSyncTime_ms != 0) {
    if (timePassedSince(lastNTPSyncTime_ms) < static_cast<long>(1000 * syncInterval)) {
      // Make sure not to flood the NTP servers with requests.
      return false;
    }
  }
  START_TIMER;
  IPAddress timeServerIP;
  String    log = F("NTP  : NTP host ");

  bool useNTPpool = false;

  if (Settings.NTPHost[0] != 0) {
    resolveHostByName(Settings.NTPHost, timeServerIP);
    log += Settings.NTPHost;

    // When single set host fails, retry again in 20 seconds
    nextSyncTime = sysTime + random(20, 60);
  } else  {
    // Have to do a lookup each time, since the NTP pool always returns another IP
    String ntpServerName = String(random(0, 3));
    ntpServerName += F(".pool.ntp.org");
    resolveHostByName(ntpServerName.c_str(), timeServerIP);
    log += ntpServerName;

    // When pool host fails, retry can be much sooner
    nextSyncTime = sysTime + random(5, 20);
    useNTPpool   = true;
  }

  log += F(" (");
  log += timeServerIP.toString();
  log += ')';

  if (!hostReachable(timeServerIP)) {
    log += F(" unreachable");
    addLogMove(LOG_LEVEL_INFO, log);
    STOP_TIMER(NTP_FAIL);
    return false;
  }

  WiFiUDP udp;

  if (!beginWiFiUDP_randomPort(udp)) {
    return false;
  }

  const int NTP_PACKET_SIZE = 48;          // NTP time is in the first 48 bytes of message
  uint8_t   packetBuffer[NTP_PACKET_SIZE]; // buffer to hold incoming & outgoing packets

  log += F(" queried");
#ifndef BUILD_NO_DEBUG
  addLog(LOG_LEVEL_DEBUG_MORE, log);
#endif // ifndef BUILD_NO_DEBUG

  while (udp.parsePacket() > 0) { // discard any previously received packets
  }
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  packetBuffer[0]  = 0b11100011; // LI, Version, Mode
  packetBuffer[1]  = 0;          // Stratum, or type of clock
  packetBuffer[2]  = 6;          // Polling Interval
  packetBuffer[3]  = 0xEC;       // Peer Clock Precision
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;

  FeedSW_watchdog();

  if (udp.beginPacket(timeServerIP, 123) == 0) { // NTP requests are to port 123
    FeedSW_watchdog();
    udp.stop();
    STOP_TIMER(NTP_FAIL);
    return false;
  }
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();


  uint32_t beginWait = millis();

  while (!timeOutReached(beginWait + 1000)) {
    int size       = udp.parsePacket();
    int remotePort = udp.remotePort();

    if ((size >= NTP_PACKET_SIZE) && (remotePort == 123)) {
      udp.read(packetBuffer, NTP_PACKET_SIZE); // read packet into the buffer

      if ((packetBuffer[0] & 0b11000000) == 0b11000000) {
        // Leap-Indicator: unknown (clock unsynchronized)
        // See: https://github.com/letscontrolit/ESPEasy/issues/2886#issuecomment-586656384
        if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
          String log = F("NTP  : NTP host (");
          log += timeServerIP.toString();
          log += F(") unsynchronized");
          addLogMove(LOG_LEVEL_ERROR, log);
        }

        if (!useNTPpool) {
          // Does not make sense to try it very often if a single host is used which is not synchronized.
          nextSyncTime = sysTime + 120;
        }
        udp.stop();
        STOP_TIMER(NTP_FAIL);
        return false;
      }

      // For more detailed info on improving accuracy, see:
      // https://github.com/lettier/ntpclient/issues/4#issuecomment-360703503
      // For now, we simply use half the reply time as delay compensation.

      unsigned long secsSince1900;

      // convert four bytes starting at location 40 to a long integer
      // TX time is used here.
      secsSince1900  = (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];

      if (secsSince1900 == 0) {
        // No time stamp received

        if (!useNTPpool) {
          // Retry again in a minute.
          nextSyncTime = sysTime + 60;
        }
        udp.stop();
        STOP_TIMER(NTP_FAIL);
        return false;
      }
      uint32_t txTm = secsSince1900 - 2208988800UL;

      unsigned long txTm_f;
      txTm_f  = (unsigned long)packetBuffer[44] << 24;
      txTm_f |= (unsigned long)packetBuffer[45] << 16;
      txTm_f |= (unsigned long)packetBuffer[46] << 8;
      txTm_f |= (unsigned long)packetBuffer[47];

      // Convert seconds to double
      unixTime_d = static_cast<double>(txTm);

      // Add fractional part.
      unixTime_d += (static_cast<double>(txTm_f) / 4294967295.0);

      long total_delay = timePassedSince(beginWait);
      lastSyncTime_ms = millis();

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
        log += doubleToString(fractpart, 3);
        log += F(" seconds");
        addLogMove(LOG_LEVEL_INFO, log);
      }
      udp.stop();
      timeSource         = timeSource_t::NTP_time_source;
      lastNTPSyncTime_ms = millis();
      CheckRunningServices(); // FIXME TD-er: Sometimes services can only be started after NTP is successful
      STOP_TIMER(NTP_SUCCESS);
      return true;
    }
    delay(10);
  }

  // Timeout.
  if (!useNTPpool) {
    // Retry again in a minute.
    nextSyncTime = sysTime + 60;
  }

#ifndef BUILD_NO_DEBUG
  addLog(LOG_LEVEL_DEBUG_MORE, F("NTP  : No reply"));
#endif // ifndef BUILD_NO_DEBUG
  udp.stop();
  STOP_TIMER(NTP_FAIL);
  return false;
}

/**************************************************
* get the timezone-offset string in +/-0000 format
**************************************************/
String ESPEasy_time::getTimeZoneOffsetString() {
  int dif = static_cast<int>((static_cast<int64_t>(now()) - static_cast<int64_t>(getUnixTime())) / 60); // Minutes
  char valueString[6] = { 0 };
  String tzoffset;

  // Formatting the timezone-offset string as [+|-]HHMM
  if (dif < 0) {
    tzoffset += '-';
  } else {
    tzoffset += '+';
  }

  dif = abs(dif);
  sprintf_P(valueString, PSTR("%02d%02d"), dif / 60, dif % 60);
  tzoffset += String(valueString);
  return tzoffset;
}
/********************************************************************************************\
   Date/Time string formatters
 \*********************************************************************************************/
String ESPEasy_time::getDateString(char delimiter) const
{
  return formatDateString(local_tm, delimiter);
}

String ESPEasy_time::getTimeString(char delimiter, bool show_seconds /*=true*/, char hour_prefix /*='\0'*/) const
{
  return formatTimeString(local_tm, delimiter, false, show_seconds, hour_prefix);
}

String ESPEasy_time::getTimeString_ampm(char delimiter, bool show_seconds /*=true*/, char hour_prefix /*='\0'*/) const
{
  return formatTimeString(local_tm, delimiter, true, show_seconds, hour_prefix);
}

String ESPEasy_time::getDateTimeString(char dateDelimiter, char timeDelimiter,  char dateTimeDelimiter) const {
  return formatDateTimeString(local_tm, dateDelimiter, timeDelimiter, dateTimeDelimiter, false);
}

String ESPEasy_time::getDateTimeString_ampm(char dateDelimiter, char timeDelimiter,  char dateTimeDelimiter) const {
  return formatDateTimeString(local_tm, dateDelimiter, timeDelimiter, dateTimeDelimiter, true);
}

/********************************************************************************************\
   Get current time/date
 \*********************************************************************************************/
int ESPEasy_time::year(unsigned long t)
{
  struct tm tmp;

  breakTime(t, tmp);
  return 1900 + tmp.tm_year;
}

int ESPEasy_time::weekday(unsigned long t)
{
  struct tm tmp;

  breakTime(t, tmp);
  return tmp.tm_wday;
}

String ESPEasy_time::weekday_str(int wday)
{
  const String weekDays = F("SunMonTueWedThuFriSat");

  return weekDays.substring(wday * 3, wday * 3 + 3);
}

String ESPEasy_time::weekday_str() const
{
  return weekday_str(weekday() - 1);
}

String ESPEasy_time::month_str(int month)
{
  const String months = F("JanFebMarAprMayJunJulAugSepOctNovDec");

  return months.substring(month * 3, month * 3 + 3);
}

String ESPEasy_time::month_str() const
{
  return month_str(month() - 1);
}

/********************************************************************************************\
   Sunrise/Sunset calculations
 \*********************************************************************************************/
int ESPEasy_time::getSecOffset(const String& format) {
  int position_minus = format.indexOf('-');
  int position_plus  = format.indexOf('+');

  if ((position_minus == -1) && (position_plus == -1)) {
    return 0;
  }
  int sign_position    = _max(position_minus, position_plus);
  int position_percent = format.indexOf('%', sign_position);

  if (position_percent == -1) {
    return 0;
  }

  int value;

  if (!validIntFromString(format.substring(sign_position, position_percent), value)) {
    return 0;
  }

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

String ESPEasy_time::getSunriseTimeString(char delimiter) const {
  return formatTimeString(sunRise, delimiter, false, false);
}

String ESPEasy_time::getSunsetTimeString(char delimiter) const {
  return formatTimeString(sunSet, delimiter, false, false);
}

String ESPEasy_time::getSunriseTimeString(char delimiter, int secOffset) const {
  if (secOffset == 0) {
    return getSunriseTimeString(delimiter);
  }
  return formatTimeString(getSunRise(secOffset), delimiter, false, false);
}

String ESPEasy_time::getSunsetTimeString(char delimiter, int secOffset) const {
  if (secOffset == 0) {
    return getSunsetTimeString(delimiter);
  }
  return formatTimeString(getSunSet(secOffset), delimiter, false, false);
}

float ESPEasy_time::sunDeclination(int doy) {
  // Declination of the sun in radians
  // Formula 2008 by Arnold(at)Barmettler.com, fit to 20 years of average declinations (2008-2027)
  return 0.409526325277017 * sin(0.0169060504029192 * (doy - 80.0856919827619));
}

float ESPEasy_time::diurnalArc(float dec, float lat) {
  // Duration of the half sun path in hours (time from sunrise to the highest level in the south)
  float rad    = 0.0174532925f; // = pi/180.0
  float height = -50.0f / 60.0f * rad;
  float latRad = lat * rad;

  return 12.0f * acos((sin(height) - sin(latRad) * sin(dec)) / (cos(latRad) * cos(dec))) / M_PI;
}

float ESPEasy_time::equationOfTime(int doy) {
  // Difference between apparent and mean solar time
  // Formula 2008 by Arnold(at)Barmettler.com, fit to 20 years of average equation of time (2008-2027)
  return -0.170869921174742 * sin(0.0336997028793971 * doy + 0.465419984181394) - 0.129890681040717 * sin(
    0.0178674832556871 * doy - 0.167936777524864);
}

int ESPEasy_time::dayOfYear(int year, int month, int day) {
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

void ESPEasy_time::calcSunRiseAndSet() {
  int   doy  = dayOfYear(local_tm.tm_year, local_tm.tm_mon + 1, local_tm.tm_mday);
  float eqt  = equationOfTime(doy);
  float dec  = sunDeclination(doy);
  float da   = diurnalArc(dec, Settings.Latitude);
  float rise = 12 - da - eqt;
  float set  = 12 + da - eqt;

  tsRise.tm_hour = rise;
  tsRise.tm_min  = (rise - static_cast<int>(rise)) * 60.0f;
  tsSet.tm_hour  = set;
  tsSet.tm_min   = (set - static_cast<int>(set)) * 60.0f;
  tsRise.tm_mday = tsSet.tm_mday = local_tm.tm_mday;
  tsRise.tm_mon  = tsSet.tm_mon = local_tm.tm_mon;
  tsRise.tm_year = tsSet.tm_year = local_tm.tm_year;

  // Now apply the longitude
  int secOffset_longitude = -1.0f * (Settings.Longitude / 15.0f) * 3600;

  tsSet  = addSeconds(tsSet, secOffset_longitude, false);
  tsRise = addSeconds(tsRise, secOffset_longitude, false);

  breakTime(time_zone.toLocal(makeTime(tsRise)), sunRise);
  breakTime(time_zone.toLocal(makeTime(tsSet)),  sunSet);
}

struct tm ESPEasy_time::getSunRise(int secOffset) const {
  return addSeconds(tsRise, secOffset, true);
}

struct tm ESPEasy_time::getSunSet(int secOffset) const {
  return addSeconds(tsSet, secOffset, true);
}

#if FEATURE_EXT_RTC
bool ESPEasy_time::ExtRTC_get(uint32_t& unixtime)
{
  unixtime = 0;

  switch (Settings.ExtTimeSource()) {
    case ExtTimeSource_e::None:
      return false;
    case ExtTimeSource_e::DS1307:
    {
      I2CSelect_Max100kHz_ClockSpeed(); // Only supports upto 100 kHz
      RTC_DS1307 rtc;

      if (!rtc.begin()) {
        // Not found
        break;
      }

      if (!rtc.isrunning()) {
        // not running
        break;
      }
      unixtime = rtc.now().unixtime();
      break;
    }
    case ExtTimeSource_e::DS3231:
    {
      RTC_DS3231 rtc;

      if (!rtc.begin()) {
        // Not found
        break;
      }

      if (rtc.lostPower()) {
        // Cannot get the time from the module
        break;
      }
      unixtime = rtc.now().unixtime();
      break;
    }

    case ExtTimeSource_e::PCF8523:
    {
      RTC_PCF8523 rtc;

      if (!rtc.begin()) {
        // Not found
        break;
      }

      if (rtc.lostPower() || !rtc.initialized() || !rtc.isrunning()) {
        // Cannot get the time from the module
        break;
      }
      unixtime = rtc.now().unixtime();
      break;
    }
    case ExtTimeSource_e::PCF8563:
    {
      RTC_PCF8563 rtc;

      if (!rtc.begin()) {
        // Not found
        break;
      }

      if (rtc.lostPower() || !rtc.isrunning()) {
        // Cannot get the time from the module
        break;
      }
      unixtime = rtc.now().unixtime();
      break;
    }
  }

  if (unixtime != 0) {
    String log = F("ExtRTC: Read external time source: ");
    log += unixtime;
    addLogMove(LOG_LEVEL_INFO, log);
    return true;
  }
  addLog(LOG_LEVEL_ERROR, F("ExtRTC: Cannot get time from external time source"));
  return false;
}
#endif

#if FEATURE_EXT_RTC
bool ESPEasy_time::ExtRTC_set(uint32_t unixtime)
{
  if (timeSource >= timeSource_t::External_RTC_time_source) {
    // Do not adjust the external RTC time if we already used it as a time source.
    // or the new time source is worse than the external RTC time souce.
    return true;
  }
  bool timeAdjusted = false;

  switch (Settings.ExtTimeSource()) {
    case ExtTimeSource_e::None:
      return false;
    case ExtTimeSource_e::DS1307:
    {
      I2CSelect_Max100kHz_ClockSpeed(); // Only supports upto 100 kHz
      RTC_DS1307 rtc;

      if (rtc.begin()) {
        rtc.adjust(DateTime(unixtime));
        timeAdjusted = true;
      }
      break;
    }
    case ExtTimeSource_e::DS3231:
    {
      RTC_DS3231 rtc;

      if (rtc.begin()) {
        rtc.adjust(DateTime(unixtime));
        timeAdjusted = true;
      }
      break;
    }

    case ExtTimeSource_e::PCF8523:
    {
      RTC_PCF8523 rtc;

      if (rtc.begin()) {
        rtc.adjust(DateTime(unixtime));
        rtc.start();
        timeAdjusted = true;
      }
      break;
    }
    case ExtTimeSource_e::PCF8563:
    {
      RTC_PCF8563 rtc;

      if (rtc.begin()) {
        rtc.adjust(DateTime(unixtime));
        rtc.start();
        timeAdjusted = true;
      }
      break;
    }
  }

  if (timeAdjusted) {
    addLogMove(LOG_LEVEL_INFO, concat(
                 F("ExtRTC: External time source set to: "),
                 unixtime));
    return true;
  }
  addLog(LOG_LEVEL_ERROR, F("ExtRTC: Cannot set time to external time source"));
  return false;
}
#endif