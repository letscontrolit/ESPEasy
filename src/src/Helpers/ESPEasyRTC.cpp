#include "ESPEasyRTC.h"

#include "../Globals/RTC.h"
#include "../DataStructs/RTCStruct.h"
#include "../DataStructs/RTCCacheStruct.h"
#include "../DataStructs/RTC_cache_handler_struct.h"
#include "../Globals/Plugins.h"
#include "../Globals/RuntimeData.h"
#include "../Helpers/CRC_functions.h"

#ifdef ESP8266
#include <user_interface.h>
#endif

/*********************************************************************************************\
* RTC memory stored values
\*********************************************************************************************/

/*
   During deep sleep, only RTC still working, so maybe we need to save some user data in RTC memory.
   Only user data area can be used by user.
 |<--------system data--------->|<-----------------user data--------------->|
 | 256 bytes                    | 512 bytes                                 |
   Note:
   RTC memory is 4 bytes aligned for read and write operations.
   Address parameter refers to block number(4 bytes per block).
   So, if we want to access some data at the beginning of user data area,
   address: 256/4 = 64
   data   : data pointer
   size   : data length, byte

   Prototype:
    bool system_rtc_mem_read (
      uint32 src_addr,
      void * data,
      uint32 save_size
    )

    bool system_rtc_mem_write (
      uint32 des_addr,
      void * data,
      uint32 save_size
    )
 */

// RTC layout ESPeasy:
// these offsets are in blocks, bytes = blocks * 4
// 64   RTCStruct  max 40 bytes: ( 74 - 64 ) * 4
// 74   UserVar
// 122  UserVar checksum:  RTC_BASE_USERVAR + (sizeof(UserVar) / 4)
// 128  Cache (C016) metadata  4 blocks
// 132  Cache (C016) data  6 blocks per sample => max 10 samples

// #define RTC_STRUCT_DEBUG




/********************************************************************************************\
   Save RTC struct to RTC memory
 \*********************************************************************************************/
bool saveToRTC()
{
  #if defined(ESP32)
  return false;
  #else // if defined(ESP32)

  if (!system_rtc_mem_write(RTC_BASE_STRUCT, (byte *)&RTC, sizeof(RTC)) || !readFromRTC())
  {
      # ifdef RTC_STRUCT_DEBUG
    addLog(LOG_LEVEL_ERROR, F("RTC  : Error while writing to RTC"));
      # endif // ifdef RTC_STRUCT_DEBUG
    return false;
  }
  else
  {
    return true;
  }
  #endif // if defined(ESP32)
}

/********************************************************************************************\
   Initialize RTC memory
 \*********************************************************************************************/
void initRTC()
{
  RTC.init();
  saveToRTC();

  for (size_t i = 0; i < sizeof(UserVar) / sizeof(float); ++i) {
    UserVar[i] = 0.0f;
  }
  saveUserVarToRTC();
}

/********************************************************************************************\
   Read RTC struct from RTC memory
 \*********************************************************************************************/
bool readFromRTC()
{
  #if defined(ESP32)
  return false;
  #else // if defined(ESP32)

  if (!system_rtc_mem_read(RTC_BASE_STRUCT, (byte *)&RTC, sizeof(RTC))) {
    return false;
  }
  return RTC.ID1 == 0xAA && RTC.ID2 == 0x55;
  #endif // if defined(ESP32)
}

/********************************************************************************************\
   Save values to RTC memory
 \*********************************************************************************************/
bool saveUserVarToRTC()
{
  #if defined(ESP32)
  return false;
  #else // if defined(ESP32)

  // addLog(LOG_LEVEL_DEBUG, F("RTCMEM: saveUserVarToRTC"));
  byte    *buffer = (byte *)&UserVar;
  size_t   size   = sizeof(UserVar);
  uint32_t sum    = calc_CRC32(buffer, size);
  bool  ret    = system_rtc_mem_write(RTC_BASE_USERVAR, buffer, size);
  ret &= system_rtc_mem_write(RTC_BASE_USERVAR + (size >> 2), (byte *)&sum, 4);
  return ret;
  #endif // if defined(ESP32)
}

/********************************************************************************************\
   Read RTC struct from RTC memory
 \*********************************************************************************************/
bool readUserVarFromRTC()
{
  #if defined(ESP32)
  return false;
  #else // if defined(ESP32)

  // addLog(LOG_LEVEL_DEBUG, F("RTCMEM: readUserVarFromRTC"));
  byte    *buffer = (byte *)&UserVar;
  size_t   size   = sizeof(UserVar);
  bool  ret    = system_rtc_mem_read(RTC_BASE_USERVAR, buffer, size);
  uint32_t sumRAM = calc_CRC32(buffer, size);
  uint32_t sumRTC = 0;
  ret &= system_rtc_mem_read(RTC_BASE_USERVAR + (size >> 2), (byte *)&sumRTC, 4);

  if (!ret || (sumRTC != sumRAM))
  {
      # ifdef RTC_STRUCT_DEBUG
    addLog(LOG_LEVEL_ERROR, F("RTC  : Checksum error on reading RTC user var"));
      # endif // ifdef RTC_STRUCT_DEBUG
    memset(buffer, 0, size);
  }
  return ret;
  #endif // if defined(ESP32)
}

