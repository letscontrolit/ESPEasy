#include "../Helpers/ESPEasy_time.h"

#include "../../ESPEasy_common.h"

#include "../../_Plugin_Helper.h"

#include "../CustomBuild/CompiletimeDefines.h"

#include "../DataStructs/NTP_packet.h"
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

#ifdef USES_ESPEASY_NOW
# include "../Globals/ESPEasy_now_handler.h"
#endif // ifdef USES_ESPEASY_NOW


#include "../Helpers/Convert.h"
#include "../Helpers/Hardware.h"
#include "../Helpers/Hardware_I2C.h"
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
  uint32_t unixtime = 0;

  if (ExtRTC_get(unixtime)) {
    setExternalTimeSource(unixtime, timeSource_t::External_RTC_time_source);
    firstCall = false;
    return;
  }
#endif // if FEATURE_EXT_RTC

  if (firstCall && (RTC.lastSysTime != 0) && (RTC.deepSleepState != 1)) {
    firstCall = false;

    // Check to see if we have some kind of believable timestamp
    // It should not be before the build time
    // Still it makes sense to restore RTC time to get some kind of continuous logging when no time source is available.
    // ToDo TD-er: Fix this when time travel appears to be possible
    setExternalTimeSource(RTC.lastSysTime + getUptime_in_sec(),
                          RTC.lastSysTime < get_build_unixtime()
        ? timeSource_t::No_time_source
        : timeSource_t::Restore_RTC_time_source);
    initTime();
  }
}

bool ESPEasy_time::setExternalTimeSource_withTimeWander(
  double       new_time,
  timeSource_t new_timeSource,
  int32_t      wander,
  uint8_t      unitnr)
{
  if ((lastSyncTime_ms != 0) && (new_timeSource == _timeSource)) {
    if (new_timeSource != timeSource_t::Manual_set) {
      // Update from the same type of time source, except when manually adjusting the time
      if (timePassedSince(lastSyncTime_ms) < EXT_TIME_SOURCE_MIN_UPDATE_INTERVAL_MSEC) {
        return false;
      }
    }
  }

  if ((_timeSource < new_timeSource) &&
      (new_timeSource != timeSource_t::No_time_source) &&
      (new_timeSource != timeSource_t::Manual_set))
  {
    if (wander < 0) {
      wander = computeExpectedWander(new_timeSource);
    }

    // New time source is potentially worse than the current one.
    if (computeExpectedWander(_timeSource, timePassedSince(lastSyncTime_ms)) <
        static_cast<uint32_t>(wander)) {
      return false;
    }
  }

  if ((new_timeSource == timeSource_t::No_time_source) ||
      (new_timeSource == timeSource_t::Manual_set) ||
      (new_time > get_build_unixtime())) {
    if (externalUnixTime_offset_usec == 0) {
      const int64_t cur_system_Unixtime_usec = getMicros64() + unixTime_usec_uptime_offset;
      const int64_t new_Unixtime_usec        = new_time * 1000000.0;
      externalUnixTime_offset_usec = new_Unixtime_usec - cur_system_Unixtime_usec;
    }
    extTimeSource       = new_timeSource;
    lastSyncTime_ms     = millis();
    timeSource_p2p_unit = unitnr;
#ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      addLogMove(LOG_LEVEL_INFO, strformat(
                   F("Time : Set Ext. Time Source: %s time: %.3f offset: %s"),
                   String(toString(new_timeSource)).c_str(),
                   new_time,
                   secondsToDayHourMinuteSecond_ms(externalUnixTime_offset_usec).c_str()));
    }
#endif // ifndef BUILD_NO_DEBUG

    initTime();
  }
  return true;
}

bool ESPEasy_time::setExternalTimeSource(double new_time, timeSource_t new_timeSource, uint8_t unitnr) {
  return setExternalTimeSource_withTimeWander(
    new_time,
    new_timeSource,
    computeExpectedWander(new_timeSource),
    unitnr);
}

uint32_t ESPEasy_time::getUptime_in_sec() const {
  return getMicros64() / 1000000ull;
}

uint32_t ESPEasy_time::getUnixTime() const
{
  const uint64_t unixtime_usec = getMicros64() + unixTime_usec_uptime_offset;

  return static_cast<uint32_t>(unixtime_usec / 1000000ull);
}

uint32_t ESPEasy_time::getUnixTime(uint32_t& unix_time_frac) const
{
  return systemMicros_to_Unixtime(getMicros64(), unix_time_frac);
}

int64_t ESPEasy_time::Unixtime_to_systemMicros(const uint32_t& unix_time_sec, uint32_t unix_time_frac) const
{
  const int64_t res =
    (static_cast<int64_t>(unix_time_sec) * 1000000ll) +
    unix_time_frac_to_micros(unix_time_frac);

  if (unixTime_usec_uptime_offset == 0) {
    // Time has not been set
    return res;
  }
  return res - unixTime_usec_uptime_offset;
}

uint32_t ESPEasy_time::systemMicros_to_Unixtime(const int64_t& systemMicros, uint32_t& unix_time_frac) const
{
  return micros_to_sec_time_frac(systemMicros + unixTime_usec_uptime_offset, unix_time_frac);
}

uint32_t ESPEasy_time::systemMicros_to_Localtime(const int64_t& systemMicros, uint32_t& unix_time_frac) const
{
  return time_zone.toLocal(systemMicros_to_Unixtime(systemMicros, unix_time_frac));
}

void ESPEasy_time::initTime()
{
  nextSyncTime = 0;
  now_();
}

unsigned long ESPEasy_time::getLocalUnixTime() const
{
  return time_zone.toLocal(getUnixTime());
}

unsigned long ESPEasy_time::getLocalUnixTime(uint32_t& unix_time_frac) const
{
  return time_zone.toLocal(getUnixTime(unix_time_frac));
}

unsigned long ESPEasy_time::now_() {
  bool timeSynced = false;

  if (nextSyncTime <= getUptime_in_sec()) {
    // nextSyncTime is in seconds
    double unixTime_d = -1.0;

    bool updatedTime = false;

    if ((externalUnixTime_offset_usec != 0) &&
        (extTimeSource != timeSource_t::No_time_source)) {
      unixTime_d = getMicros64() +
                   unixTime_usec_uptime_offset +
                   externalUnixTime_offset_usec;
      unixTime_d /= 1000000.0;

      syncInterval = EXT_TIME_SOURCE_MIN_UPDATE_INTERVAL_SEC;
      updatedTime  = true;
      _timeSource  = extTimeSource;
    } else {
      if (!isExternalTimeSource(_timeSource)
          || (timePassedSince(lastSyncTime_ms) > static_cast<long>(1000 * syncInterval)))
      {
        externalUnixTime_offset_usec = 0;

        // FIXME TD-er: calls to set external timesource should be done via the scheduler
        // Those should then also call setExternalTimeSource,
        // which determines whether the newly set time is an improvement
        if (getNtpTime(unixTime_d)) {
          updatedTime = true;
        } else {
        #if FEATURE_ESPEASY_P2P

          if (!updatedTime) {
            double  tmp_unixtime_d{};
            int32_t wander{};
            const timeSource_t tmp_timeSource = Nodes.getUnixTime(tmp_unixtime_d, wander, timeSource_p2p_unit);

            if (tmp_timeSource != timeSource_t::No_time_source) {
              // Nodes.getUnixTime does compensate for any delay since the timestamp was received from the p2p node
              // thus this can be used here without further compensation.

              // FIXME TD-er: Should check if this is a better time source compared to what we already have, using time wander

              unixTime_d   = tmp_unixtime_d;
              _timeSource  = tmp_timeSource;
              updatedTime  = true;
              syncInterval = EXT_TIME_SOURCE_MIN_UPDATE_INTERVAL_SEC;
            }
          }
        #endif // if FEATURE_ESPEASY_P2P

        #if FEATURE_EXT_RTC
          uint32_t tmp_unixtime = 0;

          if (!updatedTime &&
              (_timeSource > timeSource_t::External_RTC_time_source) && // No need to set from ext RTC more than once.
              ExtRTC_get(tmp_unixtime)) {
            unixTime_d   = tmp_unixtime;
            _timeSource  = timeSource_t::External_RTC_time_source;
            updatedTime  = true;
            syncInterval = 120; // Allow sync in 2 minutes to see if we get some better options from p2p nodes.
          }
        #endif // if FEATURE_EXT_RTC

          if (updatedTime && (externalUnixTime_offset_usec == 0)) {
            const int64_t cur_system_Unixtime_usec = getMicros64() + unixTime_usec_uptime_offset;
            const int64_t new_Unixtime_usec        = unixTime_d * 1000000.0;
            externalUnixTime_offset_usec = new_Unixtime_usec - cur_system_Unixtime_usec;
          }
        }
      }
    }

    // Clear the external time source so it has to be set again with updated values.
    extTimeSource = timeSource_t::No_time_source;

    if ((_timeSource != timeSource_t::ESPEASY_p2p_UDP) &&
        (_timeSource != timeSource_t::ESP_now_peer))
    {
      timeSource_p2p_unit = 0;
    }

    if (updatedTime) {
      START_TIMER;
      unixTime_usec_uptime_offset += externalUnixTime_offset_usec;

      constexpr int64_t ten_sec_in_usec = 10 * 1000000ll;

      if ((lastTimeWanderCalculation_ms != 0) &&
          statusNTPInitialized &&
          (std::abs(externalUnixTime_offset_usec) < ten_sec_in_usec)) {
        // Clock instability in ppm
        timeWander =
          static_cast<float>(externalUnixTime_offset_usec) /
          static_cast<float>(timePassedSince(lastTimeWanderCalculation_ms));
        timeWander *= 1000.0f;
      }

      lastTimeWanderCalculation_ms = millis();
      timeSynced                   = true;

      #if FEATURE_PLUGIN_STATS

      // GMT	Wed Jan 01 2020 00:00:00 GMT+0000
      constexpr int64_t unixTime_20200101_usec = 1577836800ll * 1000000ll;

      if (!statusNTPInitialized && (externalUnixTime_offset_usec > unixTime_20200101_usec)) {
        if (getUnixTime() > get_build_unixtime()) {
          // Update recorded plugin stats timestamps
          for (taskIndex_t taskIndex = 0; taskIndex < TASKS_MAX; taskIndex++)
          {
            PluginTaskData_base *taskData = getPluginTaskDataBaseClassOnly(taskIndex);

            if (taskData != nullptr) {
              taskData->processTimeSet(externalUnixTime_offset_usec / 1000000.0);
            }
          }
        }
      }

      #endif // if FEATURE_PLUGIN_STATS

      #if FEATURE_EXT_RTC

      // External RTC only stores with second resolution.
      // Thus to limit the error to +/- 500 ms, round the sysTime instead of just casting it.
      ExtRTC_set(static_cast<uint32_t>(unixTime_d + 0.5));
      #endif // if FEATURE_EXT_RTC
      {
        const unsigned long abs_time_offset_ms = std::abs(externalUnixTime_offset_usec) / 1000ll;

        if (_timeSource == timeSource_t::NTP_time_source) {
          // May need to lessen the load on the NTP servers, randomize the sync interval
          if (abs_time_offset_ms < 1000) {
            // offset is less than 1 second, so we consider it a regular time sync.
            if (abs_time_offset_ms < 100) {
              // Good clock stability, use 5 - 6 hour interval
              syncInterval = HwRandom(18000, 21600);
            } else {
              // Dynamic interval between 30 minutes ... 5 hours.
              syncInterval = 1800000 / abs_time_offset_ms;
            }
          } else {
            syncInterval = 3600;
          }

          if (syncInterval <= 3600) {
            syncInterval = HwRandom(3600, 4000);
          }
        } else if (_timeSource == timeSource_t::No_time_source) {
          syncInterval = 60;
        } else {
          syncInterval = 3600;
        }
      }

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        String log = F("Time set to ");
        #if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
        log += doubleToString(unixTime_d, 3);
        #else // if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
        log += static_cast<uint32_t>(unixTime_d);
        #endif // if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE

        if (std::abs(externalUnixTime_offset_usec / 1000000ll) < 86400ll) {
          // Only useful to show adjustment if it is less than a day.
          log += strformat(
            F(" Time adjusted by %d msec. Wander: %.3f ppm Source: "),
            static_cast<int32_t>(externalUnixTime_offset_usec / 1000ll),
            timeWander);
          log += toString(_timeSource);
        }
        addLogMove(LOG_LEVEL_INFO, log);
      }

      time_zone.applyTimeZone(unixTime_d);
      lastSyncTime_ms = millis();
      nextSyncTime    = getUptime_in_sec() + syncInterval;

      if (isExternalTimeSource(_timeSource)) {
        #ifdef USES_ESPEASY_NOW
        ESPEasy_now_handler.sendNTPbroadcast();
        #endif // ifdef USES_ESPEASY_NOW
      }
      STOP_TIMER(SYSTIME_UPDATED);
      externalUnixTime_offset_usec = 0;
    }
  }
  RTC.lastSysTime = getUnixTime();
  uint32_t localSystime = time_zone.toLocal(RTC.lastSysTime);
  breakTime(localSystime, local_tm);

  calcSunRiseAndSet(timeSynced);

  if (timeSynced) {
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      addLog(LOG_LEVEL_INFO, strformat(
               F("Local time: %s"),
               getDateTimeString('-', ':', ' ').c_str()));
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
  now_();

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
  switch (_timeSource) {
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
      break;
  }
  return getUnixTime() > get_build_unixtime();
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
    nextSyncTime = getUptime_in_sec() + HwRandom(20, 60);
  } else  {
    // Have to do a lookup each time, since the NTP pool always returns another IP
    const String ntpServerName = strformat(
      F("%d.pool.ntp.org"), HwRandom(0, 3));
    resolveHostByName(ntpServerName.c_str(), timeServerIP);
    log += ntpServerName;

    // When pool host fails, retry can be much sooner
    nextSyncTime = getUptime_in_sec() + HwRandom(5, 20);
    useNTPpool   = true;
  }

  log += F(" (");
  log += formatIP(timeServerIP);
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

  NTP_packet ntp_packet;

  log += F(" queried");
#ifndef BUILD_NO_DEBUG
  addLog(LOG_LEVEL_DEBUG_MORE, log);
#endif // ifndef BUILD_NO_DEBUG

  while (udp.parsePacket() > 0) { // discard any previously received packets
  }

  FeedSW_watchdog();

  if (udp.beginPacket(timeServerIP, 123) == 0) { // NTP requests are to port 123
    FeedSW_watchdog();
    udp.stop();
    STOP_TIMER(NTP_FAIL);
    return false;
  }
  constexpr int  NTP_packet_size = sizeof(NTP_packet);
  const uint64_t txMicros        = getMicros64() + unixTime_usec_uptime_offset;
  ntp_packet.setTxTimestamp(txMicros);
  udp.write(ntp_packet.data, NTP_packet_size);
  udp.endPacket();

  const uint32_t beginWait = millis();

#ifndef BUILD_NO_DEBUG
  addLog(LOG_LEVEL_DEBUG, concat(F("NTP  : before\n"), ntp_packet.toDebugString()));
#endif // ifndef BUILD_NO_DEBUG

  while (!timeOutReached(beginWait + 1000)) {
    const int size       = udp.parsePacket();
    const int remotePort = udp.remotePort();

    if (size >= NTP_packet_size) {
      if (remotePort != 123) {
#ifndef BUILD_NO_DEBUG
        addLog(LOG_LEVEL_DEBUG_MORE, concat(F("NTP  : Reply from wrong port: "), remotePort));
#endif // ifndef BUILD_NO_DEBUG
        udp.stop();
        STOP_TIMER(NTP_FAIL);
        return false;
      }
      udp.read(ntp_packet.data, NTP_packet_size); // read packet into the buffer
      const uint64_t receivedMicros = getMicros64();

      udp.stop();

#ifndef BUILD_NO_DEBUG
      addLog(LOG_LEVEL_DEBUG, concat(F("NTP  : after\n"), ntp_packet.toDebugString()));
#endif // ifndef BUILD_NO_DEBUG

      if (ntp_packet.isUnsynchronized()) {
        // Leap-Indicator: unknown (clock unsynchronized)
        // See: https://github.com/letscontrolit/ESPEasy/issues/2886#issuecomment-586656384
        if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
          addLog(LOG_LEVEL_ERROR, strformat(
                   F("NTP  : NTP host (%s) unsynchronized"),
                   formatIP(timeServerIP).c_str()));
        }

        if (!useNTPpool) {
          // Does not make sense to try it very often if a single host is used which is not synchronized.
          nextSyncTime = getUptime_in_sec() + 120;
        }
        STOP_TIMER(NTP_FAIL);
        return false;
      }

      // For more detailed info on improving accuracy, see:
      // https://github.com/lettier/ntpclient/issues/4#issuecomment-360703503

      int64_t offset_usec{};
      int64_t roundtripDelay_usec{};

      if (!ntp_packet.compute_usec(
            txMicros,
            receivedMicros + unixTime_usec_uptime_offset,
            offset_usec, roundtripDelay_usec))
      {
#ifndef BUILD_NO_DEBUG
        addLogMove(LOG_LEVEL_ERROR, strformat(
                     F("NTP  : NTP error: round-trip delay: %d [ms] offset: %s,\n  t0: %s,\n  t1: %s,\n  t2: %s,\n  t3: %s"),
                     static_cast<int32_t>(roundtripDelay_usec / 1000),
                     secondsToDayHourMinuteSecond_ms(offset_usec).c_str(),
                     doubleToString(ntp_packet.getReferenceTimestamp_usec() / 1000000.0, 3).c_str(),
                     doubleToString(ntp_packet.getOriginTimestamp_usec() / 1000000.0,    3).c_str(),
                     doubleToString(ntp_packet.getReceiveTimestamp_usec() / 1000000.0,   3).c_str(),
                     doubleToString(ntp_packet.getTransmitTimestamp_usec() / 1000000.0,  3).c_str()
                     ));
#else // ifndef BUILD_NO_DEBUG
        addLogMove(LOG_LEVEL_ERROR, strformat(
                     F("NTP  : NTP error: round-trip delay: %d [ms] offset: %s"),
                     static_cast<int32_t>(roundtripDelay_usec / 1000),
                     secondsToDayHourMinuteSecond_ms(offset_usec).c_str()
                     ));

#endif // ifndef BUILD_NO_DEBUG

        // Apparently this is not a valid packet
        // as the received timestamp is before the origin timestamp
        // or no valid timestamps from the NTP server.
        nextSyncTime = getUptime_in_sec() + 60;
        STOP_TIMER(NTP_FAIL);
        return false;
      }

      externalUnixTime_offset_usec = offset_usec;
      _timeSource                  = timeSource_t::NTP_time_source;
      lastSyncTime_ms              = millis();
      lastNTPSyncTime_ms           = lastSyncTime_ms;
      unixTime_d                   = getMicros64() +
                                     unixTime_usec_uptime_offset +
                                     externalUnixTime_offset_usec;
      unixTime_d /= 1000000.0;

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
#ifndef BUILD_NO_DEBUG
        addLogMove(LOG_LEVEL_INFO, strformat(
                     F("NTP  : NTP replied: delay %d ms round-trip delay: %u ms offset: %s,\n  t0: %s,\n  t1: %s,\n  t2: %s,\n  t3: %s"),
                     timePassedSince(beginWait),
                     static_cast<uint32_t>(roundtripDelay_usec / 1000),
                     secondsToDayHourMinuteSecond_ms(offset_usec).c_str(),
                     doubleToString(ntp_packet.getReferenceTimestamp_usec() / 1000000.0, 3).c_str(),
                     doubleToString(ntp_packet.getOriginTimestamp_usec() / 1000000.0,    3).c_str(),
                     doubleToString(ntp_packet.getReceiveTimestamp_usec() / 1000000.0,   3).c_str(),
                     doubleToString(ntp_packet.getTransmitTimestamp_usec() / 1000000.0,  3).c_str()
                     ));
#else // ifndef BUILD_NO_DEBUG
        addLogMove(LOG_LEVEL_INFO, strformat(
                     F("NTP  : NTP replied: delay %d ms round-trip delay: %u ms offset: %s"),
                     timePassedSince(beginWait),
                     static_cast<uint32_t>(roundtripDelay_usec / 1000),
                     secondsToDayHourMinuteSecond_ms(offset_usec).c_str()
                     ));
#endif // ifndef BUILD_NO_DEBUG
      }
      CheckRunningServices(); // FIXME TD-er: Sometimes services can only be started after NTP is successful
      STOP_TIMER(NTP_SUCCESS);
      return true;
    }
    delay(1);
  }

  // Timeout.
  if (!useNTPpool) {
    // Retry again in a minute.
    nextSyncTime = getUptime_in_sec() + 60;
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
  int    dif            = static_cast<int>((static_cast<int64_t>(getLocalUnixTime()) - static_cast<int64_t>(getUnixTime())) / 60); // Minutes
  char   valueString[6] = { 0 };
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

void ESPEasy_time::applyTimeZone()
{
  time_zone.applyTimeZone(getUnixTime());
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

  int32_t value;

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

void ESPEasy_time::calcSunRiseAndSet(bool timeSynced) {
  if (!timeSynced &&
      (tsSet.tm_mday == local_tm.tm_mday)) {
    // No need to recalculate if already calculated for this day
    return;
  }

  const int   doy  = dayOfYear(local_tm.tm_year, local_tm.tm_mon + 1, local_tm.tm_mday);
  const float eqt  = equationOfTime(doy);
  const float dec  = sunDeclination(doy);
  const float da   = diurnalArc(dec, Settings.Latitude);
  const float rise = 12 - da - eqt;
  const float set  = 12 + da - eqt;

  tsRise.tm_hour = rise;
  tsRise.tm_min  = (rise - static_cast<int>(rise)) * 60.0f;
  tsSet.tm_hour  = set;
  tsSet.tm_min   = (set - static_cast<int>(set)) * 60.0f;
  tsRise.tm_mday = tsSet.tm_mday = local_tm.tm_mday;
  tsRise.tm_mon  = tsSet.tm_mon = local_tm.tm_mon;
  tsRise.tm_year = tsSet.tm_year = local_tm.tm_year;

  // Now apply the longitude
  const int secOffset_longitude = -1.0f * (Settings.Longitude / 15.0f) * 3600;

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
    addLogMove(LOG_LEVEL_INFO, concat(
                 F("ExtRTC: Read external time source: "),
                 unixtime));
    return true;
  }
  addLog(LOG_LEVEL_ERROR, F("ExtRTC: Cannot get time from external time source"));
  return false;
}

#endif // if FEATURE_EXT_RTC

#if FEATURE_EXT_RTC
bool ESPEasy_time::ExtRTC_set(uint32_t unixtime)
{
  if (_timeSource >= timeSource_t::External_RTC_time_source) {
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

#endif // if FEATURE_EXT_RTC
