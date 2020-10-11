#include "ESPEasy_Log.h"
#include "src/Globals/Logging.h"
#include "src/Globals/ESPEasyWiFiEvent.h"
#include "src/Globals/Settings.h"
#include "src/DataStructs/LogStruct.h"

/********************************************************************************************\
  Init critical variables for logging (important during initial factory reset stuff )
  \*********************************************************************************************/
void initLog()
{
  //make sure addLog doesnt do any stuff before initalisation of Settings is complete.
  Settings.UseSerial=true;
  Settings.SyslogFacility=0;
  setLogLevelFor(LOG_TO_SYSLOG, 0);
  setLogLevelFor(LOG_TO_SERIAL, 2); //logging during initialisation
  setLogLevelFor(LOG_TO_WEBLOG, 2);
  setLogLevelFor(LOG_TO_SDCARD, 0);
}


/********************************************************************************************\
  Logging
  \*********************************************************************************************/
String getLogLevelDisplayString(int logLevel) {
  switch (logLevel) {
    case LOG_LEVEL_NONE:       return F("None");
    case LOG_LEVEL_ERROR:      return F("Error");
    case LOG_LEVEL_INFO:       return F("Info");
    case LOG_LEVEL_DEBUG:      return F("Debug");
    case LOG_LEVEL_DEBUG_MORE: return F("Debug More");
    case LOG_LEVEL_DEBUG_DEV:  return F("Debug dev");

    default:
      return "";
  }
}

String getLogLevelDisplayStringFromIndex(byte index, int& logLevel) {
  switch (index) {
    case 0: logLevel = LOG_LEVEL_ERROR;      break;
    case 1: logLevel = LOG_LEVEL_INFO;       break;
    case 2: logLevel = LOG_LEVEL_DEBUG;      break;
    case 3: logLevel = LOG_LEVEL_DEBUG_MORE; break;
    case 4: logLevel = LOG_LEVEL_DEBUG_DEV;  break;

    default: logLevel = -1; return "";
  }
  return getLogLevelDisplayString(logLevel);
}

void disableSerialLog() {
  log_to_serial_disabled = true;
  setLogLevelFor(LOG_TO_SERIAL, 0);
}

void setLogLevelFor(byte destination, byte logLevel) {
  switch (destination) {
    case LOG_TO_SERIAL:
      if (!log_to_serial_disabled || logLevel == 0)
        Settings.SerialLogLevel = logLevel; break;
    case LOG_TO_SYSLOG: Settings.SyslogLevel = logLevel;    break;
    case LOG_TO_WEBLOG: Settings.WebLogLevel = logLevel;    break;
    case LOG_TO_SDCARD: Settings.SDLogLevel = logLevel;     break;
    default:
      break;
  }
  updateLogLevelCache();
}

void updateLogLevelCache() {
  byte max_lvl = 0;
  if (log_to_serial_disabled) {
    if (Settings.UseSerial) {
      Serial.setDebugOutput(false);
    }
  } else {
    max_lvl = _max(max_lvl, Settings.SerialLogLevel);
#ifndef BUILD_NO_DEBUG
    if (Settings.UseSerial && Settings.SerialLogLevel >= LOG_LEVEL_DEBUG_MORE) {
      Serial.setDebugOutput(true);
    }
#endif
  }
  max_lvl = _max(max_lvl, Settings.SyslogLevel);
  if (Logging.logActiveRead()) {
    max_lvl = _max(max_lvl, Settings.WebLogLevel);
  }
#ifdef FEATURE_SD
  max_lvl = _max(max_lvl, Settings.SDLogLevel);
#endif
  highest_active_log_level = max_lvl;
}

bool loglevelActiveFor(byte logLevel) {
  return loglevelActive(logLevel, highest_active_log_level);
}

byte getSerialLogLevel() {
  if (log_to_serial_disabled || !Settings.UseSerial) return 0;
  if (!(bitRead(wifiStatus, ESPEASY_WIFI_SERVICES_INITIALIZED))){
    if (Settings.SerialLogLevel < LOG_LEVEL_INFO) {
      return LOG_LEVEL_INFO;
    }
  }
  return Settings.SerialLogLevel;
}

byte getWebLogLevel() {
  byte logLevelSettings = 0;
  if (Logging.logActiveRead()) {
    logLevelSettings = Settings.WebLogLevel;
  } else {
    if (Settings.WebLogLevel != 0) {
      updateLogLevelCache();
    }
  }
  return logLevelSettings;
}

bool loglevelActiveFor(byte destination, byte logLevel) {
  byte logLevelSettings = 0;
  switch (destination) {
    case LOG_TO_SERIAL: {
      logLevelSettings = getSerialLogLevel();
      break;
    }
    case LOG_TO_SYSLOG: {
      logLevelSettings = Settings.SyslogLevel;
      break;
    }
    case LOG_TO_WEBLOG: {
      logLevelSettings = getWebLogLevel();
      break;
    }
    case LOG_TO_SDCARD: {
      #ifdef FEATURE_SD
      logLevelSettings = Settings.SDLogLevel;
      #endif
      break;
    }
    default:
      return false;
  }
  return loglevelActive(logLevel, logLevelSettings);
}


bool loglevelActive(byte logLevel, byte logLevelSettings) {
  return (logLevel <= logLevelSettings);
}

void addToLog(byte loglevel, const String& string)
{
  addToLog(loglevel, string.c_str());
}

void addToLog(byte logLevel, const char *line)
{
  // Please note all functions called from here handling line must be PROGMEM aware.
  if (loglevelActiveFor(LOG_TO_SERIAL, logLevel)) {
    addToSerialBuffer(String(millis()).c_str());
    addToSerialBuffer(" : ");
    String loglevelDisplayString = getLogLevelDisplayString(logLevel);
    while (loglevelDisplayString.length() < 6) {
      loglevelDisplayString += ' ';
    }
    addToSerialBuffer(loglevelDisplayString.c_str());
    addToSerialBuffer(": ");
    addToSerialBuffer(line);
    addNewlineToSerialBuffer();
  }
  if (loglevelActiveFor(LOG_TO_SYSLOG, logLevel)) {
    syslog(logLevel, line);
  }
  if (loglevelActiveFor(LOG_TO_WEBLOG, logLevel)) {
    Logging.add(logLevel, line);
  }

#ifdef FEATURE_SD
  if (loglevelActiveFor(LOG_TO_SDCARD, logLevel)) {
    File logFile = SD.open("log.dat", FILE_WRITE);
    if (logFile) {
      const char* c = line;
      bool done = false;
      while (!done) {
        // Must use PROGMEM aware functions here to process line
        char ch = pgm_read_byte(c++);
        if (ch == '\0') {
          done = true;
        } else {
          logFile.print(ch);
        }
      }
      logFile.println();
    }
    logFile.close();
  }
#endif
}
