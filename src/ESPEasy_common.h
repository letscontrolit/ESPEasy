#ifndef ESPEASY_COMMON_H
#define ESPEASY_COMMON_H

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

#include <Arduino.h>
// User configuration
#include "include/ESPEasy_config.h"
#include "./src/CustomBuild/ESPEasyDefaults.h"


#ifdef USE_SECOND_HEAP
  #include <umm_malloc/umm_heap_select.h>
#endif





#ifdef ESP8266
  # if !defined(ARDUINO_ESP8266_RELEASE_2_4_0) && !defined(ARDUINO_ESP8266_RELEASE_2_3_0)
    #  define SUPPORT_ARP
  # endif
#endif

#ifdef ESP32
# define SUPPORT_ARP
#endif

//#include "src/DataStructs/NodeStruct.h"
#include "src/DataTypes/NodeTypeID.h"
#include "src/Globals/RamTracker.h"
#include "src/ESPEasyCore/ESPEasy_Log.h"
#include "src/Helpers/ESPEasy_math.h"

#if defined(ESP8266)

  #include <core_version.h>
  #define NODE_TYPE_ID      NODE_TYPE_ID_ESP_EASYM_STD
  #include <lwip/init.h>
  #ifndef LWIP_VERSION_MAJOR
    #error
  #endif
  #if LWIP_VERSION_MAJOR == 2
  //  #include <lwip/priv/tcp_priv.h>
  #else
    #include <lwip/tcp_impl.h>
  #endif
  #include <ESP8266WiFi.h>
  //#include <ESP8266Ping.h>
  #ifndef LWIP_OPEN_SRC
  #define LWIP_OPEN_SRC
  #endif
  #include <lwip/opt.h>
  #include <lwip/udp.h>
  #include <lwip/igmp.h>
  #include <include/UdpContext.h>
  #include <limits.h>
  extern "C" {
   #include <user_interface.h>
  }

  #define SMALLEST_OTA_IMAGE 276848 // smallest known 2-step OTA image
  #define MAX_SKETCH_SIZE 1044464   // 1020 kB - 16 bytes
#endif
#if defined(ESP32)
  #include <WiFi.h>

  #ifdef ESP32S2
    #define NODE_TYPE_ID                        NODE_TYPE_ID_ESP_EASY32S2_STD
  #elif defined(ESP32C3)
    #define NODE_TYPE_ID                        NODE_TYPE_ID_ESP_EASY32C3_STD
  #else
    #define NODE_TYPE_ID                        NODE_TYPE_ID_ESP_EASY32_STD
  #endif
//  #include <WiFi.h>
//  #include  "esp32_ping.h"

  #ifdef ESP32S2
    #include <esp32s2/rom/rtc.h>
  #else
   #if ESP_IDF_VERSION_MAJOR > 3
    #include <esp32/rom/rtc.h>
   #else
    #include <rom/rtc.h>
   #endif
  #endif
  
  #include <esp_wifi.h> // Needed to call ESP-IDF functions like esp_wifi_....
#endif



#include <WiFiUdp.h>
#include <Wire.h>
#include <SPI.h>


extern const String EMPTY_STRING;



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