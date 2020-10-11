#ifndef ESPEASY_COMMON_H
#define ESPEASY_COMMON_H

// *****************************************************************************************
// For Arduino IDE users:
// When building using Custom.h, uncomment the next line:
//#define USE_CUSTOM_H
// *****************************************************************************************



#include <stddef.h>
namespace std
{
  using ::ptrdiff_t;
  using ::size_t;
}

#include <stdint.h>
#include <Arduino.h>
#include <string.h>

// User configuration
// Include Custom.h before ESPEasyDefaults.h. 
#ifdef USE_CUSTOM_H
#include "Custom.h"
#endif

#include "src/Globals/RamTracker.h"
#include "src/DataStructs/ESPEasyDefaults.h"

#ifdef USE_LITTLEFS
  #include <LittleFS.h>
  #define ESPEASY_FS LittleFS
#else 
  #ifdef ESP32
    #include <SPIFFS.h>
  #endif
  #define ESPEASY_FS SPIFFS
#endif

// Include custom first, then build info. (one may want to set BUILD_GIT for example)
#include "ESPEasy_buildinfo.h"

#include "define_plugin_sets.h"

#ifdef ESP32
#include <esp8266-compat.h>

#endif


#define ZERO_FILL(S)  memset((S), 0, sizeof(S))
#define ZERO_TERMINATE(S)  S[sizeof(S) - 1] = 0



String getUnknownString();



/******************************************************************************\
 * Detect core versions *******************************************************
\******************************************************************************/

#ifndef ESP32
  #if defined(ARDUINO_ESP8266_RELEASE_2_4_0) || defined(ARDUINO_ESP8266_RELEASE_2_4_1)  || defined(ARDUINO_ESP8266_RELEASE_2_4_2)
    #ifndef CORE_2_4_X
      #define CORE_2_4_X
    #endif
  #endif

  #if defined(ARDUINO_ESP8266_RELEASE_2_3_0) || defined(ARDUINO_ESP8266_RELEASE_2_4_0) || defined(ARDUINO_ESP8266_RELEASE_2_4_1)
    #ifndef CORE_PRE_2_4_2
      #define CORE_PRE_2_4_2
    #endif
  #endif

  #if defined(ARDUINO_ESP8266_RELEASE_2_3_0) || defined(CORE_2_4_X)
    #ifndef CORE_PRE_2_5_0
      #define CORE_PRE_2_5_0
    #endif
  #else
    #ifndef CORE_POST_2_5_0
      #define CORE_POST_2_5_0
    #endif
  #endif


  #ifdef FORCE_PRE_2_5_0
    #ifdef CORE_POST_2_5_0
      #undef CORE_POST_2_5_0
    #endif
  #endif
#endif // ESP32




// Enable FEATURE_ADC_VCC to measure supply voltage using the analog pin
// Please note that the TOUT pin has to be disconnected in this mode
// Use the "System Info" device to read the VCC value
#ifndef FEATURE_ADC_VCC
  #define FEATURE_ADC_VCC                  false
#endif

#ifndef ARDUINO_OTA_PORT
  #if defined(ESP32)
    #define ARDUINO_OTA_PORT  3232
  #else
    // Do not use port 8266 for OTA, since that's used for ESPeasy p2p
    #define ARDUINO_OTA_PORT  18266
  #endif
#endif

#if defined(ESP8266)
  //enable Arduino OTA updating.
  //Note: This adds around 10kb to the firmware size, and 1kb extra ram.
  // #define FEATURE_ARDUINO_OTA

  //enable mDNS mode (adds about 6kb ram and some bytes IRAM)
  // #define FEATURE_MDNS
#endif
#if defined(ESP32)
 //#define FEATURE_ARDUINO_OTA
 //#define FEATURE_MDNS
#endif

#endif // ESPEASY_COMMON_H