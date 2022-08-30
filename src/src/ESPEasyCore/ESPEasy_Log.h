#ifndef ESPEASYCORE_ESPEASY_LOG_H
#define ESPEASYCORE_ESPEASY_LOG_H


#include "../../ESPEasy_common.h"

#define LOG_LEVEL_NONE                      0
#define LOG_LEVEL_ERROR                     1
#define LOG_LEVEL_INFO                      2
# ifndef BUILD_NO_DEBUG
#define LOG_LEVEL_DEBUG                     3
#define LOG_LEVEL_DEBUG_MORE                4
#define LOG_LEVEL_DEBUG_DEV                 9 // use for testing/debugging only, not for regular use
#define LOG_LEVEL_NRELEMENTS                5 // Update this and getLogLevelDisplayString() when new log levels are added
#else
#define LOG_LEVEL_NRELEMENTS                2 // Update this and getLogLevelDisplayString() when new log levels are added
#endif

#define LOG_TO_SERIAL         1
#define LOG_TO_SYSLOG         2
#define LOG_TO_WEBLOG         3
#define LOG_TO_SDCARD         4


// Move the log String so it does not have to be copied in the web log
#define addLogMove(L, S)  addToLogMove(L, std::move(S))

/********************************************************************************************\
  Logging
  \*********************************************************************************************/
void initLog();

const __FlashStringHelper * getLogLevelDisplayString(int logLevel);

const __FlashStringHelper * getLogLevelDisplayStringFromIndex(uint8_t index, int& logLevel);

void disableSerialLog();

void setLogLevelFor(uint8_t destination, uint8_t logLevel);

void updateLogLevelCache();

bool loglevelActiveFor(uint8_t logLevel);

uint8_t getSerialLogLevel();

uint8_t getWebLogLevel();

bool loglevelActiveFor(uint8_t destination, uint8_t logLevel);


void addLog(uint8_t logLevel, const __FlashStringHelper *str);
void addLog(uint8_t logLevel, const char *line);
void addLog(uint8_t logLevel, String&& string);

void addLog(uint8_t logLevel, const String& string);
void addToLogMove(uint8_t logLevel, String&& string);


#endif 