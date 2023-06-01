#ifndef ESPEASY_COMMON_H
#define ESPEASY_COMMON_H

#ifdef __cplusplus

// *****************************************************************************************
// For Arduino IDE users:
// When building using Custom.h, uncomment the next line:
//#define USE_CUSTOM_H
// *****************************************************************************************

/*
    To modify the stock configuration without changing this repo file :
    - define USE_CUSTOM_H as a build flags. ie : export PLATFORMIO_BUILD_FLAGS="'-DUSE_CUSTOM_H'"
    - add a "Custom.h" file in this folder.

*/

#include <Arduino.h> // See: https://github.com/esp8266/Arduino/issues/8922#issuecomment-1542301697
#include <cmath>


// User configuration
#include "include/ESPEasy_config.h"
#include "./src/CustomBuild/ESPEasyDefaults.h"


#ifdef USE_SECOND_HEAP
  #include <umm_malloc/umm_heap_select.h>
#endif


#if defined(ESP8266)
  # include <ESP8266WiFi.h>
#endif // if defined(ESP8266)
#if defined(ESP32)
  # include <WiFi.h>
#endif // if defined(ESP32)



#ifdef ESP8266
  # if !defined(ARDUINO_ESP8266_RELEASE_2_4_0) && !defined(ARDUINO_ESP8266_RELEASE_2_3_0)
    #  define SUPPORT_ARP
  # endif
#endif

#ifdef ESP32
# define SUPPORT_ARP
#endif

//#include "src/DataStructs/NodeStruct.h"
//#include "src/DataTypes/NodeTypeID.h"
#include "src/Globals/RamTracker.h"
#include "src/ESPEasyCore/ESPEasy_Log.h"
#include "src/Helpers/ESPEasy_math.h"

#if defined(ESP8266)

  #include <core_version.h>
  #include <lwip/init.h>
  #ifndef LWIP_VERSION_MAJOR
    #error
  #endif
  #if LWIP_VERSION_MAJOR == 2
  //  #include <lwip/priv/tcp_priv.h>
  #else
    #include <lwip/tcp_impl.h>
  #endif
//  #include <ESP8266WiFi.h>
  //#include <ESP8266Ping.h>
  #ifndef LWIP_OPEN_SRC
  #define LWIP_OPEN_SRC
  #endif
//  #include <lwip/opt.h>
//  #include <lwip/udp.h>
//  #include <lwip/igmp.h>
//  #include <include/UdpContext.h>
  #include <limits.h>
  /*
  extern "C" {
   #include <user_interface.h>
  }
  */

#endif
/*
#if defined(ESP32)
  #include <WiFi.h>

  #ifdef ESP32S2
    #include <esp32s2/rom/rtc.h>
  #elif defined(ESP32S3)
    #include <esp32s3/rom/rtc.h>
  #elif defined(ESP32C3)
    #include <esp32c3/rom/rtc.h>
  # elif defined(ESP32_CLASSIC)
    #if ESP_IDF_VERSION_MAJOR > 3
      #include <esp32/rom/rtc.h>
    #else
      #include <rom/rtc.h>
    #endif
  # else

    static_assert(false, "Implement processor architecture");

  #endif
//  #include <WiFi.h>
//  #include  "esp32_ping.h"

 
  #include <esp_wifi.h> // Needed to call ESP-IDF functions like esp_wifi_....
#endif
*/


//#include <WiFiUdp.h>
//#include <Wire.h>
//#include <SPI.h>


extern const String EMPTY_STRING;



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

#endif

#endif // ESPEASY_COMMON_H