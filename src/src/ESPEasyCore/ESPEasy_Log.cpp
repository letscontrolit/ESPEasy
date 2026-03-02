#include "../ESPEasyCore/ESPEasy_Log.h"

#include "../DataStructs/LogBuffer.h"
#include "../ESPEasyCore/Serial.h"
#include "../Globals/Cache.h"
#include "../Globals/ESPEasy_Console.h"
#include "../../ESPEasy/net/Globals/ESPEasyWiFiEvent.h"
#include "../../ESPEasy/net/ESPEasyNetwork.h"
#include "../Globals/Logging.h"
#include "../Globals/Settings.h"
#include "../Helpers/Networking.h"
#include "../Helpers/StringConverter.h"


#define UPDATE_LOGLEVEL_ACTIVE_CACHE_INTERVAL 5000

/********************************************************************************************\
   Init critical variables for logging (important during initial factory reset stuff )
 \*********************************************************************************************/
void initLog()
{
  // make sure addLog doesnt do any stuff before initalisation of Settings is complete.
  Settings.UseSerial      = true;
  Settings.SyslogFacility = 0;

  #ifdef ESP32
  setLogLevelFor(LOG_TO_SYSLOG, 2);
  setLogLevelFor(LOG_TO_WEBLOG, 2);
  #else // ifdef ESP32
  setLogLevelFor(LOG_TO_SYSLOG, 0);
  setLogLevelFor(LOG_TO_WEBLOG, 0);
  #endif // ifdef ESP32


  setLogLevelFor(LOG_TO_SERIAL, 2); // logging during initialisation
#if FEATURE_SD
  setLogLevelFor(LOG_TO_SDCARD, 0);
#endif
}

/********************************************************************************************\
   Logging
 \*********************************************************************************************/
void disableSerialLog() {
  log_to_serial_disabled = true;
  setLogLevelFor(LOG_TO_SERIAL, 0);
}

void setLogLevelFor(LogDestination destination, uint8_t logLevel) {
  switch (destination)
  {
    case LOG_TO_SERIAL:
#if USES_ESPEASY_CONSOLE_FALLBACK_PORT
    case LOG_TO_SERIAL_EXTRA:
#endif

      if (!log_to_serial_disabled || (logLevel == 0)) {
        Settings.SerialLogLevel = logLevel;
      }
      break;
    case LOG_TO_SYSLOG: Settings.SyslogLevel = logLevel;
      break;
    case LOG_TO_WEBLOG: Settings.WebLogLevel = logLevel;
      break;
#if FEATURE_SD
    case LOG_TO_SDCARD: Settings.SDLogLevel = logLevel;
      break;
#endif // if FEATURE_SD
    default:
      break;
  }
  updateLogLevelCache();
}

void updateLogLevelCache() {
  uint8_t max_lvl = 0;

  // FIXME TD-er: Must add check whether SW serial may be using the same pins as Serial0
  const bool useSerial = Settings.UseSerial && !activeTaskUseSerial0();

  if (log_to_serial_disabled) {
    if (useSerial) {
      ESPEasy_Console.setDebugOutput(false);
    }
  } else {
    max_lvl = _max(max_lvl, Settings.SerialLogLevel);
#ifndef BUILD_NO_DEBUG

    if (useSerial && (Settings.SerialLogLevel >= LOG_LEVEL_DEBUG_MORE)) {
      ESPEasy_Console.setDebugOutput(true);
    }
#endif // ifndef BUILD_NO_DEBUG
  }
  max_lvl = _max(max_lvl, Settings.SyslogLevel);

  if (Logging.logActiveRead(LOG_TO_WEBLOG)) {
    max_lvl = _max(max_lvl, Settings.WebLogLevel);
  }
#if FEATURE_SD
  max_lvl = _max(max_lvl, Settings.SDLogLevel);
#endif
  highest_active_log_level = max_lvl;
}

bool loglevelActiveFor(uint8_t logLevel) {
  #ifdef ESP32

  if (xPortInIsrContext()) {
    // When called from an ISR, you should not send out logs.
    // Allocating memory from within an ISR is a big no-no.
    // Also long-time blocking like sending logs (especially to a syslog server)
    // is also really not a good idea from an ISR call.
    return false;
  }
  #endif // ifdef ESP32
  static uint32_t lastUpdateLogLevelCache = 0;

  if ((lastUpdateLogLevelCache == 0) ||
      (timePassedSince(lastUpdateLogLevelCache) > UPDATE_LOGLEVEL_ACTIVE_CACHE_INTERVAL))
  {
    lastUpdateLogLevelCache = millis();
    updateLogLevelCache();
  }
  return logLevel <= highest_active_log_level;
}

uint8_t getSerialLogLevel() {
#if FEATURE_DEFINE_SERIAL_CONSOLE_PORT

  // FIXME TD-er: Must add check whether SW serial may be using the same pins as Serial0
  if (log_to_serial_disabled || !Settings.UseSerial) { return 0; }
#else // if FEATURE_DEFINE_SERIAL_CONSOLE_PORT

  if (log_to_serial_disabled || !Settings.UseSerial || activeTaskUseSerial0()) { return 0; }
#endif // if FEATURE_DEFINE_SERIAL_CONSOLE_PORT

  if (!ESPEasy::net::NetworkConnected()) {
    if (Settings.SerialLogLevel < LOG_LEVEL_INFO) {
      return LOG_LEVEL_INFO;
    }
  }
  return Settings.SerialLogLevel;
}

uint8_t getWebLogLevel() {
  if (Logging.logActiveRead(LOG_TO_WEBLOG)) {
    return Settings.WebLogLevel;
  }

  if (Settings.WebLogLevel != LOG_LEVEL_NONE) {
    updateLogLevelCache();
  }
  return LOG_LEVEL_NONE;
}

bool loglevelActiveFor(LogDestination destination, uint8_t logLevel) {
  #ifdef ESP32

  if (xPortInIsrContext()) {
    // When called from an ISR, you should not send out logs.
    // Allocating memory from within an ISR is a big no-no.
    // Also long-time blocking like sending logs (especially to a syslog server)
    // is also really not a good idea from an ISR call.
    return false;
  }
  #endif // ifdef ESP32

  if (!Logging.logActiveRead(destination)) { return false; }

  uint8_t logLevelSettings = 0;

  switch (destination)
  {
    case LOG_TO_SERIAL:
#if USES_ESPEASY_CONSOLE_FALLBACK_PORT
    case LOG_TO_SERIAL_EXTRA:
#endif
    {
      logLevelSettings = getSerialLogLevel();
      break;
    }
    case LOG_TO_SYSLOG:
    {
      if (logLevel == LOG_LEVEL_NONE) { return false; }
      logLevelSettings = Settings.SyslogLevel;
      break;
    }
    case LOG_TO_WEBLOG:
    {
      logLevelSettings = getWebLogLevel();

      if (logLevel == LOG_LEVEL_NONE) { return logLevelSettings != LOG_LEVEL_NONE; }
      break;
    }
#if FEATURE_SD
    case LOG_TO_SDCARD:
    {
      if (logLevel == LOG_LEVEL_NONE) { return false; }
      logLevelSettings = Settings.SDLogLevel;
      break;
    }
#endif // if FEATURE_SD
    default:
      return false;
  }
  return logLevel <= logLevelSettings;
}

void addLog(uint8_t logLevel, const __FlashStringHelper *str)
{
  #ifdef ESP32

  if (xPortInIsrContext()) {
    // When called from an ISR, you should not send out logs.
    // Allocating memory from within an ISR is a big no-no.
    // Also long-time blocking like sending logs (especially to a syslog server)
    // is also really not a good idea from an ISR call.
    return;
  }
  #endif // ifdef ESP32
  Logging.addLogEntry(LogEntry_t(logLevel, str));
}

void addLog(uint8_t logLevel, const char *line) {
  #ifdef ESP32

  if (xPortInIsrContext()) {
    // When called from an ISR, you should not send out logs.
    // Allocating memory from within an ISR is a big no-no.
    // Also long-time blocking like sending logs (especially to a syslog server)
    // is also really not a good idea from an ISR call.
    return;
  }
  #endif // ifdef ESP32
  Logging.addLogEntry(LogEntry_t(logLevel, line));
}

void addLog(uint8_t logLevel, String&& str) { addToLogMove(logLevel, std::move(str)); }

void addLog(uint8_t logLevel, const String& str)
{
  #ifdef ESP32

  if (xPortInIsrContext()) {
    // When called from an ISR, you should not send out logs.
    // Allocating memory from within an ISR is a big no-no.
    // Also long-time blocking like sending logs (especially to a syslog server)
    // is also really not a good idea from an ISR call.
    return;
  }
  #endif // ifdef ESP32
  Logging.addLogEntry(LogEntry_t(logLevel, str));
}

void addToLogMove(uint8_t logLevel, String&& str)
{
  #ifdef ESP32

  if (xPortInIsrContext()) {
    // When called from an ISR, you should not send out logs.
    // Allocating memory from within an ISR is a big no-no.
    // Also long-time blocking like sending logs (especially to a syslog server)
    // is also really not a good idea from an ISR call.
    return;
  }
  #endif // ifdef ESP32
  Logging.addLogEntry(LogEntry_t(logLevel, std::move(str)));
}
