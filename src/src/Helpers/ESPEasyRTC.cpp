#include "../Helpers/ESPEasyRTC.h"

#include "../Globals/RTC.h"
#include "../DataStructs/RTCStruct.h"
#include "../DataStructs/RTCCacheStruct.h"
#include "../DataStructs/RTC_cache_handler_struct.h"
#include "../DataStructs/TimingStats.h"
#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../Globals/Plugins.h"
#include "../Globals/RuntimeData.h"
#include "../Helpers/CRC_functions.h"
#include "../../ESPEasy_common.h"

#ifdef ESP8266
#include <user_interface.h>
#endif

#ifdef ESP32
#include <soc/rtc.h>
#endif

/*********************************************************************************************\
* RTC memory stored values
\*********************************************************************************************/

// ESP8266 RTC layout:
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
// 122  UserVar checksum:  RTC_BASE_USERVAR + UserVar.getNrElements()
// 128  Cache (C016) metadata  4 blocks
// 132  Cache (C016) data  6 blocks per sample => max 10 samples



// ESP32 has 2 types of RTC memory:
// RTC SLOW:
//   - 8 kB which can be accessed by both CPU cores and ULP core
//   - Persistent even after reset (not power loss)
//   - Needs RTC_NOINIT_ATTR attribute
// RTC FAST:
//   - 8 kB, only accessed by the "PRO_CPU" 
//   - Persistent during sleep, but not after reset
//   - Needs RTC_DATA_ATTR attribute

// Important to realize:
// Since allocation to RTC memory on ESP32 is done by the compiler, there is no
// guarantee the addresses will be the same after a recompile.
// Thus the data stored in RTC may not survive flashing a new build.

// Structs stored in RTC SLOW:
//   - RTCStruct to keep information on reboot reason, last used WiFi, etc.
//   - UserVar   to keep task values persistent just like on ESP8266





//#define RTC_STRUCT_DEBUG



#ifdef ESP32
constexpr size_t UserVar_nrelements = VARS_PER_TASK * TASKS_MAX;


// Since the global UserVar and RTC objects are defined "extern", they cannot be located in the RTC memory.
// Thus we have to keep a copy here.
RTC_NOINIT_ATTR RTCStruct RTC_tmp;
RTC_NOINIT_ATTR float UserVar_RTC[UserVar_nrelements];
RTC_NOINIT_ATTR uint32_t UserVar_checksum;
#endif


/********************************************************************************************\
   Save RTC struct to RTC memory
 \*********************************************************************************************/
bool saveToRTC()
{
  // ESP8266 has the RTC struct stored in memory which we must actively fetch
  // ESP32 can use a compiler flag to mark a struct to be located in RTC_SLOW memory
  #if defined(ESP32)
  RTC_tmp = RTC;
  return true;
  #else // if defined(ESP32)

  START_TIMER
  if (!system_rtc_mem_write(RTC_BASE_STRUCT, (byte *)&RTC, sizeof(RTC)) || !readFromRTC())
  {
      # ifdef RTC_STRUCT_DEBUG
    addLog(LOG_LEVEL_ERROR, F("RTC  : Error while writing to RTC"));
      # endif // ifdef RTC_STRUCT_DEBUG
    return false;
  }
  else
  {
    STOP_TIMER(SAVE_TO_RTC);
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

  for (size_t i = 0; i < UserVar.getNrElements(); ++i) {
    UserVar[i] = 0.0f;
  }
  saveUserVarToRTC();
}

/********************************************************************************************\
   Read RTC struct from RTC memory
 \*********************************************************************************************/
bool readFromRTC()
{
  // ESP8266 has the RTC struct stored in memory which we must actively fetch
  // ESP32 can use a compiler flag to mark a struct to be located in RTC_SLOW memory
  #ifdef ESP32
  RTC = RTC_tmp;
  #endif
  #ifdef ESP8266
  if (!system_rtc_mem_read(RTC_BASE_STRUCT, (byte *)&RTC, sizeof(RTC))) {
    return false;
  }
  #endif
  return RTC.ID1 == 0xAA && RTC.ID2 == 0x55;
}

/********************************************************************************************\
   Save values to RTC memory
 \*********************************************************************************************/
bool saveUserVarToRTC()
{
  // ESP8266 has the RTC struct stored in memory which we must actively fetch
  // ESP32   Uses a temp structure which is mapped to the RTC address range.
  #if defined(ESP32)
  for (size_t i = 0; i < UserVar_nrelements; ++i) {
    UserVar_RTC[i] = UserVar[i];
  }
  UserVar_checksum = calc_CRC32((byte *)(&UserVar[0]), UserVar_nrelements * sizeof(float)); 
  return true;
  #endif

  #ifdef ESP8266
  // addLog(LOG_LEVEL_DEBUG, F("RTCMEM: saveUserVarToRTC"));
  byte    *buffer = UserVar.get();
  size_t   size   = UserVar.getNrElements() * sizeof(float);
  uint32_t sum    = calc_CRC32(buffer, size);
  bool  ret    = system_rtc_mem_write(RTC_BASE_USERVAR, buffer, size);
  ret &= system_rtc_mem_write(RTC_BASE_USERVAR + (size >> 2), (byte *)&sum, 4);
  return ret;
  #endif
}

/********************************************************************************************\
   Read RTC struct from RTC memory
 \*********************************************************************************************/
bool readUserVarFromRTC()
{
  // ESP8266 has the RTC struct stored in memory which we must actively fetch
  // ESP32   Uses a temp structure which is mapped to the RTC address range.
  #if defined(ESP32)
  if (calc_CRC32((byte *)(&UserVar_RTC[0]), UserVar_nrelements * sizeof(float)) == UserVar_checksum) {
    for (size_t i = 0; i < UserVar_nrelements; ++i) {
      UserVar[i] = UserVar_RTC[i];
    }
    return true;
  }
  return false;
  #endif

  #ifdef ESP8266
  // addLog(LOG_LEVEL_DEBUG, F("RTCMEM: readUserVarFromRTC"));
  byte    *buffer = UserVar.get();
  size_t   size   = UserVar.getNrElements() * sizeof(float);
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
  #endif 
}

