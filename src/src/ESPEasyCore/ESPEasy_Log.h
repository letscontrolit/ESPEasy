#ifndef ESPEASYCORE_ESPEASY_LOG_H
#define ESPEASYCORE_ESPEASY_LOG_H


#include "../../ESPEasy_common.h"

#define LOG_LEVEL_NONE                      0
#define LOG_LEVEL_ERROR                     1
#define LOG_LEVEL_INFO                      2
#define LOG_LEVEL_DEBUG                     3
#define LOG_LEVEL_DEBUG_MORE                4
#define LOG_LEVEL_DEBUG_DEV                 9 // use for testing/debugging only, not for regular use
#define LOG_LEVEL_NRELEMENTS                5 // Update this and getLogLevelDisplayString() when new log levels are added

#define LOG_TO_SERIAL         1
#define LOG_TO_SYSLOG         2
#define LOG_TO_WEBLOG         3
#define LOG_TO_SDCARD         4


/********************************************************************************************\
  Logging
  \*********************************************************************************************/
void initLog();

const __FlashStringHelper * getLogLevelDisplayString(int logLevel);

const __FlashStringHelper * getLogLevelDisplayStringFromIndex(byte index, int& logLevel);

void disableSerialLog();

void setLogLevelFor(byte destination, byte logLevel);

void updateLogLevelCache();

bool loglevelActiveFor(byte logLevel);

byte getSerialLogLevel();

byte getWebLogLevel();

bool loglevelActiveFor(byte destination, byte logLevel);


bool loglevelActive(byte logLevel, byte logLevelSettings);

//#ifdef LIMIT_BUILD_SIZE
// Macro does add to the build size, but does take more resources as the string may need resources to create
void addLog(byte loglevel, const __FlashStringHelper *str);
void addLog(byte logLevel, const char *line);
void addLog(byte loglevel, const String& string);
//#else
// Do this in a template to prevent casting to String when not needed.
//#define addLog(L,S) if (loglevelActiveFor(L)) { addToLog(L,S); }
//#endif

void addToLog(byte loglevel, const __FlashStringHelper *str);

void addToLog(byte loglevel, const String& string);

void addToLog(byte logLevel, const char *line);


#endif 