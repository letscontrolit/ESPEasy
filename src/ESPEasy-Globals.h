#ifndef ESPEASY_GLOBALS_H_
#define ESPEASY_GLOBALS_H_

#ifndef CORE_POST_2_5_0
  #define STR_HELPER(x) #x
  #define STR(x) STR_HELPER(x)
#endif

#ifdef __GCC__
#pragma GCC system_header
#endif

/*
    To modify the stock configuration without changing this repo file :
    - define USE_CUSTOM_H as a build flags. ie : export PLATFORMIO_BUILD_FLAGS="'-DUSE_CUSTOM_H'"
    - add a "Custom.h" file in this folder.

*/
#ifdef USE_CUSTOM_H
// make the compiler show a warning to confirm that this file is inlcuded
#warning "**** Using Settings from Custom.h File ***"
#include "Custom.h"
#endif



#include "ESPEasy_common.h"
#include "ESPEasy_fdwdecl.h"

#include "src/DataStructs/ESPEasyLimits.h"
#include "ESPEasy_plugindefs.h"
#include "src/Globals/Device.h"
#include "src/Globals/Settings.h"
#include "src/Globals/ESPEasy_time.h"







//#include <FS.h>



//enable reporting status to ESPEasy developers.
//this informs us of crashes and stability issues.
// not finished yet!
// #define FEATURE_REPORTING

//Select which plugin sets you want to build.
//These are normally automaticly set via the Platformio build environment.
//If you use ArduinoIDE you might need to uncomment some of them, depending on your needs
//If you dont select any, a version with a minimal number of plugins will be biult for 512k versions.
//(512k is NOT finsihed or tested yet as of v2.0.0-dev6)

//build all the normal stable plugins (on by default)
#define PLUGIN_BUILD_NORMAL

//build all plugins that are in test stadium
//#define PLUGIN_BUILD_TESTING

//build all plugins that still are being developed and are broken or incomplete
//#define PLUGIN_BUILD_DEV

//add this if you want SD support (add 10k flash)
//#define FEATURE_SD




// ********************************************************************************
//   DO NOT CHANGE ANYTHING BELOW THIS LINE
// ********************************************************************************


#define CMD_REBOOT                         89
#define CMD_WIFI_DISCONNECT               135



/*
// TODO TD-er: Declare global variables as extern and construct them in the .cpp.
// Move all other defines in this file to separate .h files
// This file should only have the "extern" declared global variables so it can be included where they are needed.
//
// For a very good tutorial on how C++ handles global variables, see:
//    https://www.fluentcpp.com/2019/07/23/how-to-define-a-global-constant-in-cpp/
// For more information about the discussion which lead to this big change:
//    https://github.com/letscontrolit/ESPEasy/issues/2621#issuecomment-533673956
*/


#include "src/DataStructs/NotificationSettingsStruct.h"
#include "src/DataStructs/NotificationStruct.h"

#ifdef USES_NOTIFIER
extern NotificationStruct Notification[NPLUGIN_MAX];
#endif








#include "ESPEasy_Log.h"
#include "ESPEasyTimeTypes.h"
#include "StringProviderTypes.h"
#include "ESPeasySerial.h"
#include "ESPEasy_fdwdecl.h"
#include "WebServer_fwddecl.h"



#define FS_NO_GLOBALS
#if defined(ESP8266)
  #include "core_version.h"
  #define NODE_TYPE_ID      NODE_TYPE_ID_ESP_EASYM_STD
  #define FILE_CONFIG       "config.dat"
  #define FILE_SECURITY     "security.dat"
  #define FILE_NOTIFICATION "notification.dat"
  #define FILE_RULES        "rules1.txt"
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
  #include <DNSServer.h>
  #include <Servo.h>
  #ifndef LWIP_OPEN_SRC
  #define LWIP_OPEN_SRC
  #endif
  #include "lwip/opt.h"
  #include "lwip/udp.h"
  #include "lwip/igmp.h"
  #include "include/UdpContext.h"
  #include "limits.h"
  extern "C" {
   #include "user_interface.h"
  }
  extern "C" {
  #include "spi_flash.h"
  }
  #ifdef CORE_POST_2_6_0
    extern "C" uint32_t _FS_start;
    extern "C" uint32_t _FS_end;
    extern "C" uint32_t _FS_page;
    extern "C" uint32_t _FS_block;
  #else
    extern "C" uint32_t _SPIFFS_start;
    extern "C" uint32_t _SPIFFS_end;
    extern "C" uint32_t _SPIFFS_page;
    extern "C" uint32_t _SPIFFS_block;
  #endif

  #ifdef FEATURE_MDNS
    #include <ESP8266mDNS.h>
  #endif
  #define SMALLEST_OTA_IMAGE 276848 // smallest known 2-step OTA image
  #define MAX_SKETCH_SIZE 1044464   // 1020 kB - 16 bytes
  #define PIN_D_MAX        16
#endif
#if defined(ESP32)

  // Temp fix for a missing core_version.h within ESP Arduino core. Wait until they actually have different releases
  #define ARDUINO_ESP8266_RELEASE "2_4_0"

  #define NODE_TYPE_ID                        NODE_TYPE_ID_ESP_EASY32_STD
  #define ICACHE_RAM_ATTR IRAM_ATTR
  #define FILE_CONFIG       "/config.dat"
  #define FILE_SECURITY     "/security.dat"
  #define FILE_NOTIFICATION "/notification.dat"
  #define FILE_RULES        "/rules1.txt"
  #include <WiFi.h>
//  #include  "esp32_ping.h"
  #ifdef USE_LITTLEFS
    #include "LittleFS.h"
  #else
    #include "SPIFFS.h"
  #endif
  #include <rom/rtc.h>
  #include "esp_wifi.h" // Needed to call ESP-IDF functions like esp_wifi_....
  #ifdef FEATURE_MDNS
    #include <ESPmDNS.h>
  #endif
  #define PIN_D_MAX        39
  extern int8_t ledChannelPin[16];
  #define MAX_SKETCH_SIZE 1900544   // 0x1d0000 look at partitions in csv file
#endif

#include <WiFiUdp.h>
#include <DNSServer.h>
#include <Wire.h>
#include <SPI.h>
#include <FS.h>
#ifdef FEATURE_SD
#include <SD.h>
#else
using namespace fs;
#endif
#include <base64.h>





enum gpio_direction {
  gpio_input,
  gpio_output,
  gpio_bidirectional
};


/*********************************************************************************************\
 * pinStatesStruct
\*********************************************************************************************/
/*
struct pinStatesStruct
{
  pinStatesStruct() : value(0), plugin(0), index(0), mode(0) {}
  uint16_t value;
  byte plugin;
  byte index;
  byte mode;
} pinStates[PINSTATE_TABLE_MAX];
*/



extern boolean printToWeb;
extern String printWebString;
extern boolean printToWebJSON;


//struct RTC_cache_handler_struct;


// FIXME TD-er: Must move this to some proper class (ESPEasy_Scheduler ?)
extern unsigned long timermqtt_interval;


extern unsigned long lastSend;
extern unsigned long lastWeb;
extern byte cmd_within_mainloop;
extern unsigned long wdcounter;
extern unsigned long timerAwakeFromDeepSleep;


#if FEATURE_ADC_VCC
extern float vcc;
#endif
#ifdef ESP8266
extern int lastADCvalue; // Keep track of last ADC value as it cannot be read while WiFi is connecting
#endif

extern boolean WebLoggedIn;
extern int WebLoggedInTimer;


extern String dummyString;  // FIXME @TD-er  This may take a lot of memory over time, since long-lived Strings only tend to grow.




extern bool shouldReboot;
extern bool firstLoop;

extern boolean activeRuleSets[RULESETS_MAX];

extern boolean UseRTOSMultitasking;


// void (*MainLoopCall_ptr)(void); //FIXME TD-er: No idea what this does.






// These wifi event functions must be in a .h-file because otherwise the preprocessor
// may not filter the ifdef checks properly.
// Also the functions use a lot of global defined variables, so include at the end of this file.
#include "ESPEasyWiFiEvent.h"
#define SPIFFS_CHECK(result, fname) if (!(result)) { return(FileError(__LINE__, fname)); }
#include "WebServer_Rules.h"




#endif /* ESPEASY_GLOBALS_H_ */
