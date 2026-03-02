#pragma once


#include "../../ESPEasy_common.h"
#include "../DataTypes/LogLevels.h"

#include "../DataStructs/LogEntry.h"



// Move the log String so it does not have to be copied in the web log
#define addLogMove(L, S) addToLogMove(L, std::move(S))

enum LogDestination : uint8_t;

/********************************************************************************************\
   Logging
 \*********************************************************************************************/
void    initLog();


void    disableSerialLog();

void    setLogLevelFor(LogDestination destination,
                       uint8_t        logLevel);

void    updateLogLevelCache();

bool    loglevelActiveFor(uint8_t logLevel);

uint8_t getSerialLogLevel();

uint8_t getWebLogLevel();

bool    loglevelActiveFor(LogDestination destination,
                          uint8_t        logLevel);

void    addLog(uint8_t                    logLevel,
               const __FlashStringHelper *str);
void    addLog(uint8_t     logLevel,
               const char *line);
void    addLog(uint8_t  logLevel,
               String&& str);

void    addLog(uint8_t       logLevel,
               const String& str);

void    addToLogMove(uint8_t  logLevel,
                     String&& str);
