#ifndef DATASTRUCTS_RTC_STRUCTS_H
#define DATASTRUCTS_RTC_STRUCTS_H

#include "../../ESPEasy_common.h"

// this offsets are in blocks, bytes = blocks * 4
#define RTC_BASE_STRUCT 64
#define RTC_BASE_USERVAR 74
#define RTC_BASE_CACHE 124

#define RTC_CACHE_DATA_SIZE 240
#define CACHE_FILE_MAX_SIZE 24000

/*********************************************************************************************\
 * RTCStruct
\*********************************************************************************************/
//max 40 bytes: ( 74 - 64 ) * 4
struct RTCStruct
{
  RTCStruct() : ID1(0), ID2(0), unused1(false), factoryResetCounter(0),
                deepSleepState(0), bootFailedCount(0), flashDayCounter(0),
                flashCounter(0), bootCounter(0), lastMixedSchedulerId(0) {}
  byte ID1;
  byte ID2;
  boolean unused1;
  byte factoryResetCounter;
  byte deepSleepState;
  byte bootFailedCount;
  byte flashDayCounter;
  unsigned long flashCounter;
  unsigned long bootCounter;
  unsigned long lastMixedSchedulerId;
};


#endif // DATASTRUCTS_RTC_STRUCTS_H
