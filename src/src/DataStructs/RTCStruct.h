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
  RTCStruct() = default;

  RTCStruct(const RTCStruct& other) = delete;

  RTCStruct& operator=(const RTCStruct&) = default; 

  void init();

  void clearLastWiFi();

  bool lastWiFi_set() const;


  byte ID1 = 0;
  byte ID2 = 0;
  byte lastWiFiChannel = 0;
  byte factoryResetCounter = 0;
  byte deepSleepState = 0;
  byte bootFailedCount = 0;
  byte flashDayCounter = 0;
  byte lastWiFiSettingsIndex = 0;
  unsigned long flashCounter = 0;
  unsigned long bootCounter = 0;
  unsigned long lastMixedSchedulerId = 0;
  uint8_t lastBSSID[6] = { 0 };
  byte unused1 = 0;  // Force alignment to 4 bytes
  byte unused2 = 0;
  unsigned long lastSysTime = 0;
};


#endif // DATASTRUCTS_RTC_STRUCTS_H
