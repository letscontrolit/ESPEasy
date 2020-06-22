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
#include "src/DataStructs/EventQueue.h"
#include "src/Helpers/msecTimerHandlerStruct.h"
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
#include "I2CTypes.h"
#include <I2Cdev.h>


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


extern I2Cdev i2cdev;





// Setup DNS, only used if the ESP has no valid WiFi config
extern const byte DNS_PORT;
extern IPAddress apIP;
extern DNSServer dnsServer;
extern bool dnsServerActive;

//NTP status
extern bool statusNTPInitialized;

// udp protocol stuff (syslog, global sync, node info list, ntp time)
extern WiFiUDP portUDP;

// Ethernet Connectiopn status
#ifdef HAS_ETHERNET
extern uint8_t eth_wifi_mode;
extern bool eth_connected;
#endif















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

/********************************************************************************************\
  RTC_cache_struct
\*********************************************************************************************/
struct RTC_cache_struct
{
  uint32_t checksumData = 0;
  uint16_t readFileNr = 0;       // File number used to read from.
  uint16_t writeFileNr = 0;      // File number to write to.
  uint16_t readPos = 0;          // Read position in file based cache
  uint16_t writePos = 0;         // Write position in the RTC memory
  uint32_t checksumMetadata = 0;
};

struct RTC_cache_handler_struct;


/*********************************************************************************************\
 * rulesTimerStruct
\*********************************************************************************************/
struct rulesTimerStatus
{
  rulesTimerStatus() : timestamp(0), interval(0), paused(false) {}

  unsigned long timestamp;
  unsigned int interval; //interval in milliseconds
  bool paused;
};

extern rulesTimerStatus RulesTimer[RULES_TIMER_MAX];

extern msecTimerHandlerStruct msecTimerHandler;

extern unsigned long timer_gratuitous_arp_interval;
extern unsigned long timermqtt_interval;
extern unsigned long lastSend;
extern unsigned long lastWeb;
extern byte cmd_within_mainloop;
extern unsigned long wdcounter;
extern unsigned long timerAPoff;    // Timer to check whether the AP mode should be disabled (0 = disabled)
extern unsigned long timerAPstart;  // Timer to start AP mode, started when no valid network is detected.
extern unsigned long timerAwakeFromDeepSleep;
extern unsigned long last_system_event_run;

#if FEATURE_ADC_VCC
extern float vcc;
#endif
#ifdef ESP8266
extern int lastADCvalue; // Keep track of last ADC value as it cannot be read while WiFi is connecting
#endif

extern boolean WebLoggedIn;
extern int WebLoggedInTimer;


extern String dummyString;  // FIXME @TD-er  This may take a lot of memory over time, since long-lived Strings only tend to grow.

enum PluginPtrType {
  TaskPluginEnum,
  ControllerPluginEnum,
  NotificationPluginEnum
};
void schedule_event_timer(PluginPtrType ptr_type, byte Index, byte Function, struct EventStruct* event);
unsigned long createSystemEventMixedId(PluginPtrType ptr_type, byte Index, byte Function);
unsigned long createSystemEventMixedId(PluginPtrType ptr_type, uint16_t crc16);







extern bool webserverRunning;
extern bool webserver_init;


extern EventQueueStruct eventQueue;


extern bool shouldReboot;
extern bool firstLoop;

extern boolean activeRuleSets[RULESETS_MAX];

extern boolean UseRTOSMultitasking;


// void (*MainLoopCall_ptr)(void); //FIXME TD-er: No idea what this does.




#include "src/DataStructs/DeviceModel.h"

struct GpioFactorySettingsStruct {
  GpioFactorySettingsStruct(DeviceModel model = DeviceModel_default) {
    for (int i = 0; i < 4; ++i) {
      button[i] = -1;
      relais[i] = -1;
    }
    switch (model) {
      case DeviceModel_Sonoff_Basic:
      case DeviceModel_Sonoff_TH1x:
      case DeviceModel_Sonoff_S2x:
      case DeviceModel_Sonoff_TouchT1:
      case DeviceModel_Sonoff_POWr2:
        button[0] = 0;   // Single Button
        relais[0] = 12;  // Red Led and Relay (0 = Off, 1 = On)
        status_led = 13; // Green/Blue Led (0 = On, 1 = Off)
        i2c_sda = -1;
        i2c_scl = -1;
        break;
      case DeviceModel_Sonoff_POW:
        button[0] = 0;   // Single Button
        relais[0] = 12;  // Red Led and Relay (0 = Off, 1 = On)
        status_led = 15; // Blue Led (0 = On, 1 = Off)
        i2c_sda = -1;
        i2c_scl = -1;    // GPIO5 conflicts with HLW8012 Sel output
        break;
      case DeviceModel_Sonoff_TouchT2:
        button[0] = 0;   // Button 1
        button[1] = 9;   // Button 2
        relais[0] = 12;  // Led and Relay1 (0 = Off, 1 = On)
        relais[1] = 4;   // Led and Relay2 (0 = Off, 1 = On)
        status_led = 13; // Blue Led (0 = On, 1 = Off)
        i2c_sda = -1;    // GPIO4 conflicts with GPIO_REL3
        i2c_scl = -1;    // GPIO5 conflicts with GPIO_REL2
        break;
      case DeviceModel_Sonoff_TouchT3:
        button[0] = 0;   // Button 1
        button[1] = 10;  // Button 2
        button[2] = 9;   // Button 3
        relais[0] = 12;  // Led and Relay1 (0 = Off, 1 = On)
        relais[1] = 5;   // Led and Relay2 (0 = Off, 1 = On)
        relais[2] = 4;   // Led and Relay3 (0 = Off, 1 = On)
        status_led = 13; // Blue Led (0 = On, 1 = Off)
        i2c_sda = -1;    // GPIO4 conflicts with GPIO_REL3
        i2c_scl = -1;    // GPIO5 conflicts with GPIO_REL2
        break;

      case DeviceModel_Sonoff_4ch:
        button[0] = 0;   // Button 1
        button[1] = 9;   // Button 2
        button[2] = 10;  // Button 3
        button[3] = 14;  // Button 4
        relais[0] = 12;  // Red Led and Relay1 (0 = Off, 1 = On)
        relais[1] = 5;   // Red Led and Relay2 (0 = Off, 1 = On)
        relais[2] = 4;   // Red Led and Relay3 (0 = Off, 1 = On)
        relais[3] = 15;  // Red Led and Relay4 (0 = Off, 1 = On)
        status_led = 13; // Blue Led (0 = On, 1 = Off)
        i2c_sda = -1;    // GPIO4 conflicts with GPIO_REL3
        i2c_scl = -1;    // GPIO5 conflicts with GPIO_REL2
        break;
      case DeviceModel_Shelly1:
        button[0] = 5;   // Single Button
        relais[0] = 4;   // Red Led and Relay (0 = Off, 1 = On)
        status_led = 15; // Blue Led (0 = On, 1 = Off)
        i2c_sda = -1;    // GPIO4 conflicts with relay control.
        i2c_scl = -1;    // GPIO5 conflicts with SW input
        break;
      case DeviceModel_ShellyPLUG_S:
        button[0] = 13;  // Single Button
        relais[0] = 15;  // Red Led and Relay (0 = Off, 1 = On)
        status_led = 2;  // Blue Led (0 = On, 1 = Off)
        i2c_sda = -1;    // GPIO4 conflicts with relay control.
        i2c_scl = -1;    // GPIO5 conflicts with SW input
        break;
      case DeviceMode_Olimex_ESP32_PoE:
        button[0] = 34;    // DUT1 Button
        relais[0] = -1;    // No LED's or relays on board
        status_led = -1;
        i2c_sda = 4;
        i2c_scl = 5;
        eth_power = 12;
        eth_clock_mode = 3;
        eth_wifi_mode = 1;
        break;

      // case DeviceModel_default: break;
      default: break;
    }
  }

  int8_t button[4];
  int8_t relais[4];
  int8_t status_led = DEFAULT_PIN_STATUS_LED;
  int8_t i2c_sda = DEFAULT_PIN_I2C_SDA;
  int8_t i2c_scl = DEFAULT_PIN_I2C_SCL;
  int8_t eth_phyaddr = DEFAULT_ETH_PHY_ADDR;
  int8_t eth_phytype = DEFAULT_ETH_PHY_TYPE;
  int8_t eth_mdc = DEFAULT_ETH_PIN_MDC;
  int8_t eth_mdio = DEFAULT_ETH_PIN_MDIO;
  int8_t eth_power = DEFAULT_ETH_PIN_POWER;
  int8_t eth_clock_mode = DEFAULT_ETH_CLOCK_MODE;
  int8_t eth_wifi_mode = DEFAULT_ETH_WIFI_MODE;
};

void addPredefinedPlugins(const GpioFactorySettingsStruct& gpio_settings);
void addPredefinedRules(const GpioFactorySettingsStruct& gpio_settings);



// These wifi event functions must be in a .h-file because otherwise the preprocessor
// may not filter the ifdef checks properly.
// Also the functions use a lot of global defined variables, so include at the end of this file.
#include "ESPEasyWiFiEvent.h"
#define SPIFFS_CHECK(result, fname) if (!(result)) { return(FileError(__LINE__, fname)); }
#include "WebServer_Rules.h"




#endif /* ESPEASY_GLOBALS_H_ */
