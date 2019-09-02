#ifndef ESPEASY_GLOBALS_H_
#define ESPEASY_GLOBALS_H_

#ifndef CORE_POST_2_5_0
  #define STR_HELPER(x) #x
  #define STR(x) STR_HELPER(x)
#endif

#ifdef __GCC__
#pragma GCC system_header
#endif

#include "ESPEasy_common.h"
#include "ESPEasy_fdwdecl.h"

#include "DataStructs/ESPEasyLimits.h"

#include <FS.h>

// ********************************************************************************
// Check struct sizes at compile time
// Usage:
//   struct foo
//   {
//     char bla[16];
//   };
//
//   check_size<foo, 8>();
// ********************************************************************************
template <typename ToCheck, std::size_t ExpectedSize, std::size_t RealSize = sizeof(ToCheck)>
void check_size() {
  static_assert(ExpectedSize == RealSize, "");
}


// User configuration
#include "DataStructs/ESPEasyDefaults.h"



// Enable FEATURE_ADC_VCC to measure supply voltage using the analog pin
// Please note that the TOUT pin has to be disconnected in this mode
// Use the "System Info" device to read the VCC value
#ifndef FEATURE_ADC_VCC
  #define FEATURE_ADC_VCC                  false
#endif

#if defined(ESP32)
#define ARDUINO_OTA_PORT  3232
#else
// Do not use port 8266 for OTA, since that's used for ESPeasy p2p
#define ARDUINO_OTA_PORT  18266
#endif

#if defined(ESP8266)
  //enable Arduino OTA updating.
  //Note: This adds around 10kb to the firmware size, and 1kb extra ram.
  // #define FEATURE_ARDUINO_OTA

  //enable mDNS mode (adds about 6kb ram and some bytes IRAM)
  // #define FEATURE_MDNS
#endif
#if defined(ESP32)
 #define FEATURE_ARDUINO_OTA
 //#define FEATURE_MDNS
#endif

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
#define ESP_PROJECT_PID           2016110801L

#if defined(ESP8266)
  #define VERSION                             2 // config file version (not ESPEasy version). increase if you make incompatible changes to config system.
#endif
#if defined(ESP32)
  #define VERSION                             3 // Change in config.dat mapping needs a full reset
#endif

#define BUILD                           20103 // git version 2.1.03
#if defined(ESP8266)
  #define BUILD_NOTES                 " - Mega"
#endif
#if defined(ESP32)
  #define BUILD_NOTES                 " - Mega32"
#endif

#ifndef BUILD_GIT
#define BUILD_GIT "(custom)"
#endif

#define MAX_FLASHWRITES_PER_DAY           100 // per 24 hour window
#define INPUT_COMMAND_SIZE                240 // Affects maximum command length in rules and other commands
// FIXME TD-er: INPUT_COMMAND_SIZE is also used in commands where simply a check for valid parameter is needed
// and some may need less memory. (which is stack allocated)

#define NODE_TYPE_ID_ESP_EASY_STD           1
#define NODE_TYPE_ID_RPI_EASY_STD           5  // https://github.com/enesbcs/rpieasy
#define NODE_TYPE_ID_ESP_EASYM_STD         17
#define NODE_TYPE_ID_ESP_EASY32_STD        33
#define NODE_TYPE_ID_ARDUINO_EASY_STD      65
#define NODE_TYPE_ID_NANO_EASY_STD         81

#define TIMER_20MSEC                        1
#define TIMER_100MSEC                       2
#define TIMER_1SEC                          3
#define TIMER_30SEC                         4
#define TIMER_MQTT                          5
#define TIMER_STATISTICS                    6
#define TIMER_GRATUITOUS_ARP                7
#define TIMER_MQTT_DELAY_QUEUE              8
#define TIMER_C001_DELAY_QUEUE              9
#define TIMER_C003_DELAY_QUEUE             10
#define TIMER_C004_DELAY_QUEUE             11
#define TIMER_C007_DELAY_QUEUE             12
#define TIMER_C008_DELAY_QUEUE             13
#define TIMER_C009_DELAY_QUEUE             14
#define TIMER_C010_DELAY_QUEUE             15
#define TIMER_C011_DELAY_QUEUE             16
#define TIMER_C012_DELAY_QUEUE             17
#define TIMER_C013_DELAY_QUEUE             18
#define TIMER_C014_DELAY_QUEUE             19
#define TIMER_C015_DELAY_QUEUE             20
#define TIMER_C016_DELAY_QUEUE             21
#define TIMER_C017_DELAY_QUEUE             22
#define TIMER_C018_DELAY_QUEUE             23
#define TIMER_C019_DELAY_QUEUE             24
#define TIMER_C020_DELAY_QUEUE             25

#define TIMING_STATS_THRESHOLD             100000
#define TIMER_GRATUITOUS_ARP_MAX           5000



#define PLUGIN_INIT_ALL                     1
#define PLUGIN_INIT                         2
#define PLUGIN_READ                         3 // This call can yield new data (when success = true) and then send to controllers
#define PLUGIN_ONCE_A_SECOND                4 // Called once a second
#define PLUGIN_TEN_PER_SECOND               5 // Called 10x per second (typical for checking new data instead of waiting)
#define PLUGIN_DEVICE_ADD                   6 // Called at boot for letting a plugin adding itself to list of available plugins/devices
#define PLUGIN_EVENTLIST_ADD                7 
#define PLUGIN_WEBFORM_SAVE                 8 // Call from web interface to save settings
#define PLUGIN_WEBFORM_LOAD                 9 // Call from web interface for presenting settings and status of plugin
#define PLUGIN_WEBFORM_SHOW_VALUES         10 // Call from devices overview page to format values in HTML
#define PLUGIN_GET_DEVICENAME              11 
#define PLUGIN_GET_DEVICEVALUENAMES        12
#define PLUGIN_WRITE                       13
#define PLUGIN_EVENT_OUT                   14
#define PLUGIN_WEBFORM_SHOW_CONFIG         15
#define PLUGIN_SERIAL_IN                   16
#define PLUGIN_UDP_IN                      17
#define PLUGIN_CLOCK_IN                    18
#define PLUGIN_TIMER_IN                    19
#define PLUGIN_FIFTY_PER_SECOND            20
#define PLUGIN_SET_CONFIG                  21
#define PLUGIN_GET_DEVICEGPIONAMES         22
#define PLUGIN_EXIT                        23
#define PLUGIN_GET_CONFIG                  24
#define PLUGIN_UNCONDITIONAL_POLL          25
#define PLUGIN_REQUEST                     26
#define PLUGIN_TIME_CHANGE                 27
#define PLUGIN_MONITOR                     28
#define PLUGIN_SET_DEFAULTS                29
#define PLUGIN_GET_PACKED_RAW_DATA         30 // Return all data in a compact binary format specific for that plugin.
                                              // Needs USES_PACKED_RAW_DATA


// Make sure the CPLUGIN_* does not overlap PLUGIN_*
#define CPLUGIN_PROTOCOL_ADD               41 // Called at boot for letting a controller adding itself to list of available controllers
#define CPLUGIN_PROTOCOL_TEMPLATE          42
#define CPLUGIN_PROTOCOL_SEND              43
#define CPLUGIN_PROTOCOL_RECV              44
#define CPLUGIN_GET_DEVICENAME             45
#define CPLUGIN_WEBFORM_SAVE               46
#define CPLUGIN_WEBFORM_LOAD               47
#define CPLUGIN_GET_PROTOCOL_DISPLAY_NAME  48
#define CPLUGIN_TASK_CHANGE_NOTIFICATION   49
#define CPLUGIN_INIT                       50
#define CPLUGIN_UDP_IN                     51 
#define CPLUGIN_FLUSH                      52 // Force offloading data stored in buffers, called before sleep/reboot

// new messages for autodiscover controller plugins (experimental) i.e. C014
#define CPLUGIN_GOT_CONNECTED              53 // call after connected to mqtt server to publich device autodicover features
#define CPLUGIN_GOT_INVALID                54 // should be called before major changes i.e. changing the device name to clean up data on the controller. !ToDo
#define CPLUGIN_INTERVAL                   55 // call every interval loop
#define CPLUGIN_ACKNOWLEDGE                56 // call for sending acknowledges !ToDo done by direct function call in PluginCall() for now.

#define CPLUGIN_WEBFORM_SHOW_HOST_CONFIG   57 // Used for showing host information for the controller.

#define CONTROLLER_USE_DNS                  1
#define CONTROLLER_HOSTNAME                 2
#define CONTROLLER_IP                       3 
#define CONTROLLER_PORT                     4
#define CONTROLLER_USER                     5
#define CONTROLLER_PASS                     6
#define CONTROLLER_MIN_SEND_INTERVAL        7
#define CONTROLLER_MAX_QUEUE_DEPTH          8
#define CONTROLLER_MAX_RETRIES              9
#define CONTROLLER_FULL_QUEUE_ACTION        10
#define CONTROLLER_CHECK_REPLY              12
#define CONTROLLER_SUBSCRIBE                13
#define CONTROLLER_PUBLISH                  14
#define CONTROLLER_LWT_TOPIC                15
#define CONTROLLER_LWT_CONNECT_MESSAGE      16
#define CONTROLLER_LWT_DISCONNECT_MESSAGE   17
#define CONTROLLER_TIMEOUT                  18
#define CONTROLLER_SAMPLE_SET_INITIATOR       19
#define CONTROLLER_ENABLED                  20  // Keep this as last, is used to loop over all parameters

#define NPLUGIN_PROTOCOL_ADD                1
#define NPLUGIN_GET_DEVICENAME              2
#define NPLUGIN_WEBFORM_SAVE                3
#define NPLUGIN_WEBFORM_LOAD                4
#define NPLUGIN_WRITE                       5
#define NPLUGIN_NOTIFY                      6
#define NPLUGIN_NOT_FOUND                 255


#define LOG_LEVEL_NONE                      0
#define LOG_LEVEL_ERROR                     1
#define LOG_LEVEL_INFO                      2
#define LOG_LEVEL_DEBUG                     3
#define LOG_LEVEL_DEBUG_MORE                4
#define LOG_LEVEL_DEBUG_DEV                 9 // use for testing/debugging only, not for regular use
#define LOG_LEVEL_NRELEMENTS                5 // Update this and getLogLevelDisplayString() when new log levels are added

#define CMD_REBOOT                         89
#define CMD_WIFI_DISCONNECT               135



#define PIN_MODE_UNDEFINED                  0
#define PIN_MODE_INPUT                      1
#define PIN_MODE_OUTPUT                     2
#define PIN_MODE_PWM                        3
#define PIN_MODE_SERVO                      4
#define PIN_MODE_INPUT_PULLUP               5
#define PIN_MODE_OFFLINE                    6

#define SEARCH_PIN_STATE                 true
#define NO_SEARCH_PIN_STATE             false

#define DEVICE_TYPE_SINGLE                  1  // connected through 1 datapin
#define DEVICE_TYPE_DUAL                    2  // connected through 2 datapins
#define DEVICE_TYPE_TRIPLE                  3  // connected through 3 datapins
#define DEVICE_TYPE_ANALOG                 10  // AIN/tout pin
#define DEVICE_TYPE_I2C                    20  // connected through I2C
#define DEVICE_TYPE_DUMMY                  99  // Dummy device, has no physical connection

#define SENSOR_TYPE_NONE                    0
#define SENSOR_TYPE_SINGLE                  1
#define SENSOR_TYPE_TEMP_HUM                2
#define SENSOR_TYPE_TEMP_BARO               3
#define SENSOR_TYPE_TEMP_HUM_BARO           4
#define SENSOR_TYPE_DUAL                    5
#define SENSOR_TYPE_TRIPLE                  6
#define SENSOR_TYPE_QUAD                    7
#define SENSOR_TYPE_TEMP_EMPTY_BARO         8
#define SENSOR_TYPE_SWITCH                 10
#define SENSOR_TYPE_DIMMER                 11
#define SENSOR_TYPE_LONG                   20
#define SENSOR_TYPE_WIND                   21
#define SENSOR_TYPE_STRING                 22

#define UNIT_NUMBER_MAX                  9999  // Stored in Settings.Unit
#define DOMOTICZ_MAX_IDX            999999999  // Looks like it is an unsigned int, so could be up to 4 bln.

#define VALUE_SOURCE_SYSTEM                 1
#define VALUE_SOURCE_SERIAL                 2
#define VALUE_SOURCE_HTTP                   3
#define VALUE_SOURCE_MQTT                   4
#define VALUE_SOURCE_UDP                    5
#define VALUE_SOURCE_WEB_FRONTEND           6

#define BOOT_CAUSE_MANUAL_REBOOT            0
#define BOOT_CAUSE_COLD_BOOT                1
#define BOOT_CAUSE_DEEP_SLEEP               2
#define BOOT_CAUSE_EXT_WD                  10

#define DAT_TASKS_DISTANCE               2048  // DAT_TASKS_SIZE + DAT_TASKS_CUSTOM_SIZE
#define DAT_TASKS_SIZE                   1024
#define DAT_TASKS_CUSTOM_OFFSET          1024  // Equal to DAT_TASKS_SIZE
#define DAT_TASKS_CUSTOM_SIZE            1024
#define DAT_CUSTOM_CONTROLLER_SIZE       1024
#define DAT_CONTROLLER_SIZE              1024
#define DAT_NOTIFICATION_SIZE            1024

#define DAT_BASIC_SETTINGS_SIZE          4096

#if defined(ESP8266)
  #define DAT_OFFSET_TASKS                 4096  // each task = 2k, (1024 basic + 1024 bytes custom), 12 max
  #define DAT_OFFSET_CONTROLLER           28672  // each controller = 1k, 4 max
  #define DAT_OFFSET_CUSTOM_CONTROLLER    32768  // each custom controller config = 1k, 4 max.
  #define CONFIG_FILE_SIZE                65536
#endif
#if defined(ESP32)
  #define DAT_OFFSET_CONTROLLER            8192  // each controller = 1k, 4 max
  #define DAT_OFFSET_CUSTOM_CONTROLLER    12288  // each custom controller config = 1k, 4 max.
  #define DAT_OFFSET_TASKS                32768  // each task = 2k, (1024 basic + 1024 bytes custom), 32 max
  #define CONFIG_FILE_SIZE               131072
#endif


/*
        To modify the stock configuration without changing this repo file :
    - define USE_CUSTOM_H as a build flags. ie : export PLATFORMIO_BUILD_FLAGS="'-DUSE_CUSTOM_H'"
        - add a "Custom.h" file in this folder.

*/
#ifdef USE_CUSTOM_H
#include "Custom.h"
#endif

#include "DataStructs/ESPEasyLimits.h"

#include "DataStructs/ESPEasy_EventStruct.h"
#include "DataStructs/SettingsType.h"
#include "DataStructs/CRCStruct.h"
#include "DataStructs/SecurityStruct.h"
#include "DataStructs/SettingsStruct.h"
#include "DataStructs/DeviceModel.h"
#include "DataStructs/FactoryDefaultPref.h"
#include "DataStructs/ControllerSettingsStruct.h"

CRCStruct CRCValues;
SecurityStruct SecuritySettings;
SettingsStruct Settings;
ResetFactoryDefaultPreference_struct ResetFactoryDefaultPreference;

#include "define_plugin_sets.h"
#include "WebStaticData.h"
#include "ESPEasyTimeTypes.h"
#include "StringProviderTypes.h"
#include "ESPeasySerial.h"
#include "ESPEasy_fdwdecl.h"
#include "WebServer_fwddecl.h"
#include "I2CTypes.h"
#include <I2Cdev.h>
#include <map>
#include <deque>


#define FS_NO_GLOBALS
#if defined(ESP8266)
  #include "core_version.h"
  #define NODE_TYPE_ID                        NODE_TYPE_ID_ESP_EASYM_STD
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
  #include <ESP8266WebServer.h>
  ESP8266WebServer WebServer(80);
  #include <DNSServer.h>
  #include <Servo.h>
  #include <ESP8266HTTPUpdateServer.h>
  ESP8266HTTPUpdateServer httpUpdater(true);
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
  #define MAX_SKETCH_SIZE 1044464
  #ifdef FEATURE_ARDUINO_OTA
    #include <ArduinoOTA.h>
    #include <ESP8266mDNS.h>
    bool ArduinoOTAtriggered=false;
  #endif
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
  #include <WebServer.h>
  #include "SPIFFS.h"
  #include <rom/rtc.h>
  #include "esp_wifi.h" // Needed to call ESP-IDF functions like esp_wifi_....
  WebServer WebServer(80);
  #ifdef FEATURE_MDNS
    #include <ESPmDNS.h>
  #endif
  #ifdef FEATURE_ARDUINO_OTA
    #include <ArduinoOTA.h>
    #include <ESPmDNS.h>
    bool ArduinoOTAtriggered=false;
  #endif
  #define PIN_D_MAX        39
  int8_t ledChannelPin[16];
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
#if FEATURE_ADC_VCC
ADC_MODE(ADC_VCC);
#endif


I2Cdev i2cdev;

#ifdef USES_MQTT
#include <PubSubClient.h>
// MQTT client
WiFiClient mqtt;
PubSubClient MQTTclient(mqtt);
bool MQTTclient_should_reconnect = true;
bool MQTTclient_connected = false;
int mqtt_reconnect_count = 0;
#endif //USES_MQTT

#ifdef USES_P037
// mqtt import status
bool P037_MQTTImport_connected = false;
#endif

#define ESPEASY_WIFI_DISCONNECTED            0
#define ESPEASY_WIFI_CONNECTED               1
#define ESPEASY_WIFI_GOT_IP                  2
#define ESPEASY_WIFI_SERVICES_INITIALIZED    4

#if defined(ESP32)
void WiFiEvent(system_event_id_t event, system_event_info_t info);
#else
WiFiEventHandler stationConnectedHandler;
WiFiEventHandler stationDisconnectedHandler;
WiFiEventHandler stationGotIpHandler;
WiFiEventHandler stationModeDHCPTimeoutHandler;
WiFiEventHandler APModeStationConnectedHandler;
WiFiEventHandler APModeStationDisconnectedHandler;
#endif

// Setup DNS, only used if the ESP has no valid WiFi config
const byte DNS_PORT = 53;
IPAddress apIP(DEFAULT_AP_IP);
DNSServer dnsServer;
bool dnsServerActive = false;

//NTP status
bool statusNTPInitialized = false;

// udp protocol stuff (syslog, global sync, node info list, ntp time)
WiFiUDP portUDP;

class TimingStats;





/*********************************************************************************************\
 * Custom Variables for usage in rules and http.
 * Syntax: %vX%
 * usage:
 * let,1,10
 * if %v1%=10 do ...
\*********************************************************************************************/
float         customFloatVar[CUSTOM_VARS_MAX];







/*********************************************************************************************\
 * NotificationSettingsStruct
\*********************************************************************************************/
struct NotificationSettingsStruct
{
  NotificationSettingsStruct() : Port(0), Pin1(0), Pin2(0) {
    ZERO_FILL(Server);
    ZERO_FILL(Domain);
    ZERO_FILL(Sender);
    ZERO_FILL(Receiver);
    ZERO_FILL(Subject);
    ZERO_FILL(Body);
    ZERO_FILL(User);
    ZERO_FILL(Pass);
  }

  void validate() {
    ZERO_TERMINATE(Server);
    ZERO_TERMINATE(Domain);
    ZERO_TERMINATE(Sender);
    ZERO_TERMINATE(Receiver);
    ZERO_TERMINATE(Subject);
    ZERO_TERMINATE(Body);
    ZERO_TERMINATE(User);
    ZERO_TERMINATE(Pass);
  }

  char          Server[65];
  unsigned int  Port;
  char          Domain[65];
  char          Sender[65];
  char          Receiver[65];
  char          Subject[129];
  char          Body[513];
  byte          Pin1;
  byte          Pin2;
  char          User[49];
  char          Pass[33];
  //its safe to extend this struct, up to 4096 bytes, default values in config are 0
};

typedef std::shared_ptr<NotificationSettingsStruct> NotificationSettingsStruct_ptr_type;
#define MakeNotificationSettings(T) NotificationSettingsStruct_ptr_type NotificationSettingsStruct_ptr(new NotificationSettingsStruct());\
                                    NotificationSettingsStruct& T = *NotificationSettingsStruct_ptr;


/*********************************************************************************************\
 * ExtraTaskSettingsStruct
\*********************************************************************************************/

// This is only used by some plugins to store extra settings like formula descriptions.
// These settings can only be active for one plugin, meaning they have to be loaded
// over and over again from flash when another active plugin uses these values.
//FIXME @TD-er: Should think of another mechanism to make this more efficient.
struct ExtraTaskSettingsStruct
{
  ExtraTaskSettingsStruct() : TaskIndex(TASKS_MAX) {
    clear();
  }

  void clear() {
    TaskIndex = TASKS_MAX;
    ZERO_FILL(TaskDeviceName);
    for (byte i = 0; i < VARS_PER_TASK; ++i) {
      TaskDeviceValueDecimals[i] = 2;
      ZERO_FILL(TaskDeviceFormula[i]);
      ZERO_FILL(TaskDeviceValueNames[i]);
    }
    for (byte i = 0; i < PLUGIN_EXTRACONFIGVAR_MAX; ++i) {
      TaskDevicePluginConfigLong[i] = 0;
      TaskDevicePluginConfig[i] = 0;
    }
  }

  void validate() {
    ZERO_TERMINATE(TaskDeviceName);
    for (byte i = 0; i < VARS_PER_TASK; ++i) {
      ZERO_TERMINATE(TaskDeviceFormula[i]);
      ZERO_TERMINATE(TaskDeviceValueNames[i]);
    }
  }

  bool checkUniqueValueNames() {
    for (int i = 0; i < (VARS_PER_TASK - 1); ++i) {
      for (int j = i; j < VARS_PER_TASK; ++j) {
        if (i != j && TaskDeviceValueNames[i][0] != 0) {
          if (strcasecmp(TaskDeviceValueNames[i], TaskDeviceValueNames[j]) == 0)
            return false;
        }
      }
    }
    return true;
  }

  void clearUnusedValueNames(byte usedVars) {
    for (byte i = usedVars; i < VARS_PER_TASK; ++i) {
      TaskDeviceValueDecimals[i] = 2;
      ZERO_FILL(TaskDeviceFormula[i]);
      ZERO_FILL(TaskDeviceValueNames[i]);
    }
  }

  bool checkInvalidCharInNames(const char* name) {
    int pos = 0;
    while (*(name+pos) != 0) {
      switch (*(name+pos)) {
        case ',':
        case ' ':
        case '#':
        case '[':
        case ']':
          return false;
      }
      ++pos;
    }
    return true;
  }

  bool checkInvalidCharInNames() {
    if (!checkInvalidCharInNames(&TaskDeviceName[0])) return false;
    for (int i = 0; i < (VARS_PER_TASK - 1); ++i) {
      if (!checkInvalidCharInNames(&TaskDeviceValueNames[i][0])) return false;
    }
    return true;
  }

  byte    TaskIndex;  // Always < TASKS_MAX
  char    TaskDeviceName[NAME_FORMULA_LENGTH_MAX + 1];
  char    TaskDeviceFormula[VARS_PER_TASK][NAME_FORMULA_LENGTH_MAX + 1];
  char    TaskDeviceValueNames[VARS_PER_TASK][NAME_FORMULA_LENGTH_MAX + 1];
  long    TaskDevicePluginConfigLong[PLUGIN_EXTRACONFIGVAR_MAX];
  byte    TaskDeviceValueDecimals[VARS_PER_TASK];
  int16_t TaskDevicePluginConfig[PLUGIN_EXTRACONFIGVAR_MAX];
} ExtraTaskSettings;



/*********************************************************************************************\
 * LogStruct
\*********************************************************************************************/
#define LOG_STRUCT_MESSAGE_SIZE 128
#ifdef ESP32
  #define LOG_STRUCT_MESSAGE_LINES 30
  #define LOG_BUFFER_EXPIRE         30000  // Time after which a buffered log item is considered expired.
#else
  #if defined(PLUGIN_BUILD_TESTING) || defined(PLUGIN_BUILD_DEV)
    #define LOG_STRUCT_MESSAGE_LINES 10
  #else
    #define LOG_STRUCT_MESSAGE_LINES 15
  #endif
  #define LOG_BUFFER_EXPIRE         5000  // Time after which a buffered log item is considered expired.
#endif

struct LogStruct {
    LogStruct() : write_idx(0), read_idx(0), lastReadTimeStamp(0) {
      for (int i = 0; i < LOG_STRUCT_MESSAGE_LINES; ++i) {
        timeStamp[i] = 0;
        log_level[i] = 0;
      }
    }

    void add(const byte loglevel, const char *line) {
      write_idx = (write_idx + 1) % LOG_STRUCT_MESSAGE_LINES;
      if (write_idx == read_idx) {
        // Buffer full, move read_idx to overwrite oldest entry.
        read_idx = (read_idx + 1) % LOG_STRUCT_MESSAGE_LINES;
      }
      timeStamp[write_idx] = millis();
      log_level[write_idx] = loglevel;
      unsigned linelength = strlen(line);
      if (linelength > LOG_STRUCT_MESSAGE_SIZE-1)
        linelength = LOG_STRUCT_MESSAGE_SIZE-1;
      Message[write_idx] = "";
      Message[write_idx].reserve(linelength);
      for (unsigned i = 0; i < linelength; ++i) {
        Message[write_idx] += *(line + i);
      }
    }

    // Read the next item and append it to the given string.
    // Returns whether new lines are available.
    bool get(String& output, const String& lineEnd) {
      lastReadTimeStamp = millis();
      if (!isEmpty()) {
        read_idx = (read_idx + 1) % LOG_STRUCT_MESSAGE_LINES;
        output += formatLine(read_idx, lineEnd);
      }
      return !isEmpty();
    }

    String get_logjson_formatted(bool& logLinesAvailable, unsigned long& timestamp) {
      lastReadTimeStamp = millis();
      logLinesAvailable = false;
      if (isEmpty()) {
        return "";
      }
      read_idx = (read_idx + 1) % LOG_STRUCT_MESSAGE_LINES;
      timestamp = timeStamp[read_idx];
      String output = logjson_formatLine(read_idx);
      if (isEmpty()) return output;
      output += ",\n";
      logLinesAvailable = true;
      return output;
    }

    bool isEmpty() {
      return (write_idx == read_idx);
    }

    bool logActiveRead() {
      clearExpiredEntries();
      return timePassedSince(lastReadTimeStamp) < LOG_BUFFER_EXPIRE;
    }

  private:
    String formatLine(int index, const String& lineEnd) {
      String output;
      output += timeStamp[index];
      output += " : ";
      output += Message[index];
      output += lineEnd;
      return output;
    }

    String logjson_formatLine(int index) {
      String output;
      output.reserve(LOG_STRUCT_MESSAGE_SIZE + 64);
      output = "{";
      output += to_json_object_value("timestamp", String(timeStamp[index]));
      output += ",\n";
      output += to_json_object_value("text",  Message[index]);
      output += ",\n";
      output += to_json_object_value("level", String(log_level[index]));
      output += "}";
      return output;
    }

    void clearExpiredEntries() {
      if (isEmpty()) {
        return;
      }
      if (timePassedSince(lastReadTimeStamp) > LOG_BUFFER_EXPIRE) {
        // Clear the entire log.
        // If web log is the only log active, it will not be checked again until it is read.
        for (read_idx = 0; read_idx < LOG_STRUCT_MESSAGE_LINES; ++read_idx) {
          Message[read_idx] = String(); // Free also the reserved memory.
          timeStamp[read_idx] = 0;
          log_level[read_idx] = 0;
        }
        read_idx = 0;
        write_idx = 0;
      }
    }

    String Message[LOG_STRUCT_MESSAGE_LINES];
    unsigned long timeStamp[LOG_STRUCT_MESSAGE_LINES];
    int write_idx;
    int read_idx;
    unsigned long lastReadTimeStamp;
    byte log_level[LOG_STRUCT_MESSAGE_LINES];

} Logging;

std::deque<char> serialWriteBuffer;

byte highest_active_log_level = 0;
bool log_to_serial_disabled = false;
// Do this in a template to prevent casting to String when not needed.
#define addLog(L,S) if (loglevelActiveFor(L)) { addToLog(L,S); }

/*********************************************************************************************\
 * DeviceStruct
\*********************************************************************************************/
struct DeviceStruct
{
  DeviceStruct() :
    Number(0), Type(0), VType(0), Ports(0), ValueCount(0),
    PullUpOption(false), InverseLogicOption(false), FormulaOption(false),
    Custom(false), SendDataOption(false), GlobalSyncOption(false),
    TimerOption(false), TimerOptional(false), DecimalsOnly(false) {}

  bool connectedToGPIOpins() {
    return (Type >= DEVICE_TYPE_SINGLE && Type <= DEVICE_TYPE_TRIPLE);
  }


  byte Number;  // Plugin ID number.   (PLUGIN_ID_xxx)
  byte Type;    // How the device is connected. e.g. DEVICE_TYPE_SINGLE => connected through 1 datapin
  byte VType;   // Type of value the plugin will return, used only for Domoticz
  byte Ports;   // Port to use when device has multiple I/O pins  (N.B. not used much)
  byte ValueCount;             // The number of output values of a plugin. The value should match the number of keys PLUGIN_VALUENAME1_xxx
  bool PullUpOption : 1;       // Allow to set internal pull-up resistors.
  bool InverseLogicOption : 1; // Allow to invert the boolean state (e.g. a switch)
  bool FormulaOption : 1;      // Allow to enter a formula to convert values during read. (not possible with Custom enabled)
  bool Custom : 1;
  bool SendDataOption : 1;     // Allow to send data to a controller.
  bool GlobalSyncOption : 1;   // No longer used. Was used for ESPeasy values sync between nodes
  bool TimerOption : 1;        // Allow to set the "Interval" timer for the plugin.
  bool TimerOptional : 1;      // When taskdevice timer is not set and not optional, use default "Interval" delay (Settings.Delay)
  bool DecimalsOnly;           // Allow to set the number of decimals (otherwise treated a 0 decimals)
};
typedef std::vector<DeviceStruct> DeviceVector;
DeviceVector Device;


/*********************************************************************************************\
 * ProtocolStruct
\*********************************************************************************************/
struct ProtocolStruct
{
  ProtocolStruct() :
    defaultPort(0), Number(0), usesMQTT(false), usesAccount(false), usesPassword(false),
    usesTemplate(false), usesID(false), Custom(false), usesHost(true), usesPort(true), 
    usesQueue(true), usesSampleSets(false) {}
  uint16_t defaultPort;
  byte Number;
  bool usesMQTT : 1;
  bool usesAccount : 1;
  bool usesPassword : 1;
  bool usesTemplate : 1;  // When set, the protocol will pre-load some templates like default MQTT topics
  bool usesID : 1;        // Whether a controller supports sending an IDX value sent along with plugin data
  bool Custom : 1;        // When set, the controller has to define all parameters on the controller setup page
  bool usesHost : 1;
  bool usesPort : 1;
  bool usesQueue : 1;
  bool usesSampleSets : 1;
};
typedef std::vector<ProtocolStruct> ProtocolVector;
ProtocolVector Protocol;


/*********************************************************************************************\
 * NotificationStruct
\*********************************************************************************************/
struct NotificationStruct
{
  NotificationStruct() :
    Number(0), usesMessaging(false), usesGPIO(0) {}
  byte Number;
  boolean usesMessaging;
  byte usesGPIO;
} Notification[NPLUGIN_MAX];


/*********************************************************************************************\
 * NodeStruct
\*********************************************************************************************/
struct NodeStruct
{
  NodeStruct() :
    build(0), age(0), nodeType(0)
    {
      for (byte i = 0; i < 4; ++i) ip[i] = 0;
    }
  String nodeName;
  IPAddress ip;
  uint16_t build;
  byte age;
  byte nodeType;
};
typedef std::map<byte, NodeStruct> NodesMap;
NodesMap Nodes;

/*********************************************************************************************\
 * systemTimerStruct
\*********************************************************************************************/
struct systemTimerStruct
{
  systemTimerStruct() :
    timer(0), Par1(0), Par2(0), Par3(0), Par4(0), Par5(0), TaskIndex(-1), plugin(0) {}

  unsigned long timer;
  int Par1;
  int Par2;
  int Par3;
  int Par4;
  int Par5;
  int16_t TaskIndex;
  byte plugin;
};
std::map<unsigned long, systemTimerStruct> systemTimers;

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
} RTC;

int deviceCount = -1;
int protocolCount = -1;
int notificationCount = -1;

boolean printToWeb = false;
String printWebString = "";
boolean printToWebJSON = false;

float UserVar[VARS_PER_TASK * TASKS_MAX];

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
  boolean paused;
} RulesTimer[RULES_TIMER_MAX];

msecTimerHandlerStruct msecTimerHandler;

unsigned long timer_gratuitous_arp_interval = 5000;
unsigned long timermqtt_interval = 250;
unsigned long lastSend = 0;
unsigned long lastWeb = 0;
byte cmd_within_mainloop = 0;
unsigned long connectionFailures = 0;
unsigned long wdcounter = 0;
unsigned long timerAPoff = 0;    // Timer to check whether the AP mode should be disabled (0 = disabled)
unsigned long timerAPstart = 0;  // Timer to start AP mode, started when no valid network is detected.
unsigned long timerAwakeFromDeepSleep = 0;
unsigned long last_system_event_run = 0;

#if FEATURE_ADC_VCC
float vcc = -1.0;
#endif

boolean WebLoggedIn = false;
int WebLoggedInTimer = 300;

boolean (*Plugin_ptr[PLUGIN_MAX])(byte, struct EventStruct*, String&);
std::vector<byte> Plugin_id;
std::vector<int> Task_id_to_Plugin_id;

bool (*CPlugin_ptr[CPLUGIN_MAX])(byte, struct EventStruct*, String&);
byte CPlugin_id[CPLUGIN_MAX];

boolean (*NPlugin_ptr[NPLUGIN_MAX])(byte, struct EventStruct*, String&);
byte NPlugin_id[NPLUGIN_MAX];

String dummyString = "";  // FIXME @TD-er  This may take a lot of memory over time, since long-lived Strings only tend to grow.

enum PluginPtrType {
  TaskPluginEnum,
  ControllerPluginEnum,
  NotificationPluginEnum,
  CommandTimerEnum
};
void schedule_event_timer(PluginPtrType ptr_type, byte Index, byte Function, struct EventStruct* event);
unsigned long createSystemEventMixedId(PluginPtrType ptr_type, byte Index, byte Function);
unsigned long createSystemEventMixedId(PluginPtrType ptr_type, uint16_t crc16);


byte lastBootCause = BOOT_CAUSE_MANUAL_REBOOT;
unsigned long lastMixedSchedulerId_beforereboot = 0;

#if defined(ESP32)
enum WiFiDisconnectReason
{
    WIFI_DISCONNECT_REASON_UNSPECIFIED              = 1,
    WIFI_DISCONNECT_REASON_AUTH_EXPIRE              = 2,
    WIFI_DISCONNECT_REASON_AUTH_LEAVE               = 3,
    WIFI_DISCONNECT_REASON_ASSOC_EXPIRE             = 4,
    WIFI_DISCONNECT_REASON_ASSOC_TOOMANY            = 5,
    WIFI_DISCONNECT_REASON_NOT_AUTHED               = 6,
    WIFI_DISCONNECT_REASON_NOT_ASSOCED              = 7,
    WIFI_DISCONNECT_REASON_ASSOC_LEAVE              = 8,
    WIFI_DISCONNECT_REASON_ASSOC_NOT_AUTHED         = 9,
    WIFI_DISCONNECT_REASON_DISASSOC_PWRCAP_BAD      = 10,  /* 11h */
    WIFI_DISCONNECT_REASON_DISASSOC_SUPCHAN_BAD     = 11,  /* 11h */
    WIFI_DISCONNECT_REASON_IE_INVALID               = 13,  /* 11i */
    WIFI_DISCONNECT_REASON_MIC_FAILURE              = 14,  /* 11i */
    WIFI_DISCONNECT_REASON_4WAY_HANDSHAKE_TIMEOUT   = 15,  /* 11i */
    WIFI_DISCONNECT_REASON_GROUP_KEY_UPDATE_TIMEOUT = 16,  /* 11i */
    WIFI_DISCONNECT_REASON_IE_IN_4WAY_DIFFERS       = 17,  /* 11i */
    WIFI_DISCONNECT_REASON_GROUP_CIPHER_INVALID     = 18,  /* 11i */
    WIFI_DISCONNECT_REASON_PAIRWISE_CIPHER_INVALID  = 19,  /* 11i */
    WIFI_DISCONNECT_REASON_AKMP_INVALID             = 20,  /* 11i */
    WIFI_DISCONNECT_REASON_UNSUPP_RSN_IE_VERSION    = 21,  /* 11i */
    WIFI_DISCONNECT_REASON_INVALID_RSN_IE_CAP       = 22,  /* 11i */
    WIFI_DISCONNECT_REASON_802_1X_AUTH_FAILED       = 23,  /* 11i */
    WIFI_DISCONNECT_REASON_CIPHER_SUITE_REJECTED    = 24,  /* 11i */

    WIFI_DISCONNECT_REASON_BEACON_TIMEOUT           = 200,
    WIFI_DISCONNECT_REASON_NO_AP_FOUND              = 201,
    WIFI_DISCONNECT_REASON_AUTH_FAIL                = 202,
    WIFI_DISCONNECT_REASON_ASSOC_FAIL               = 203,
    WIFI_DISCONNECT_REASON_HANDSHAKE_TIMEOUT        = 204
};
#endif


bool useStaticIP();

// WiFi related data
boolean wifiSetup = false;
boolean wifiSetupConnect = false;
uint8_t lastBSSID[6] = {0};
uint8_t wifiStatus = ESPEASY_WIFI_DISCONNECTED;
unsigned long last_wifi_connect_attempt_moment = 0;
unsigned int wifi_connect_attempt = 0;
int wifi_reconnects = -1; // First connection attempt is not a reconnect.
uint8_t lastWiFiSettings = 0;
String last_ssid;
bool bssid_changed = false;
uint8_t last_channel = 0;
WiFiDisconnectReason lastDisconnectReason = WIFI_DISCONNECT_REASON_UNSPECIFIED;
unsigned long lastConnectMoment = 0;
unsigned long lastDisconnectMoment = 0;
unsigned long lastGetIPmoment = 0;
unsigned long lastGetScanMoment = 0;
unsigned long lastConnectedDuration = 0;
bool intent_to_reboot = false;
uint8_t lastMacConnectedAPmode[6] = {0};
uint8_t lastMacDisconnectedAPmode[6] = {0};

//uint32_t scan_done_status = 0;
uint8_t  scan_done_number = 0;
//uint8_t  scan_done_scan_id = 0;

// Semaphore like booleans for processing data gathered from WiFi events.
volatile bool processedConnect = true;
volatile bool processedDisconnect = true;
volatile bool processedGotIP = true;
volatile bool processedDHCPTimeout = true;
volatile bool processedConnectAPmode = true;
volatile bool processedDisconnectAPmode = true;
volatile bool processedScanDone = true;
bool wifiConnectAttemptNeeded = true;

bool webserverRunning = false;
bool webserver_init = false;

unsigned long idle_msec_per_sec = 0;
unsigned long elapsed10ps = 0;
unsigned long elapsed10psU = 0;
unsigned long elapsed50ps = 0;
unsigned long loopCounter = 0;
unsigned long loopCounterLast = 0;
unsigned long loopCounterMax = 1;
unsigned long lastLoopStart = 0;
unsigned long shortestLoop = 10000000;
unsigned long longestLoop = 0;
unsigned long loopCounter_full = 1;
float loop_usec_duration_total = 0.0;
unsigned long countFindPluginId = 0;

unsigned long dailyResetCounter = 0;
volatile unsigned long sw_watchdog_callback_count = 0;

String eventBuffer = "";

uint32_t lowestRAM = 0;
String lowestRAMfunction = "";
uint32_t lowestFreeStack = 0;
String lowestFreeStackfunction = "";


bool shouldReboot=false;
bool firstLoop=true;

boolean activeRuleSets[RULESETS_MAX];

boolean UseRTOSMultitasking = false;

// void (*MainLoopCall_ptr)(void); //FIXME TD-er: No idea what this does.

/*********************************************************************************************\
 * TimingStats
\*********************************************************************************************/
class TimingStats {
    public:
      TimingStats() : _timeTotal(0.0), _count(0), _maxVal(0), _minVal(4294967295) {}

      void add(unsigned long time) {
          _timeTotal += time;
          ++_count;
          if (time > _maxVal) _maxVal = time;
          if (time < _minVal) _minVal = time;
      }

      void reset() {
          _timeTotal = 0.0;
          _count = 0;
          _maxVal = 0;
          _minVal = 4294967295;
      }

      bool isEmpty() const {
          return _count == 0;
      }

      float getAvg() const {
        if (_count == 0) return 0.0;
        return _timeTotal / _count;
      }

      unsigned int getMinMax(unsigned long& minVal, unsigned long& maxVal) const {
          if (_count == 0) {
              minVal = 0;
              maxVal = 0;
              return 0;
          }
          minVal = _minVal;
          maxVal = _maxVal;
          return _count;
      }

      bool thresholdExceeded(unsigned long threshold) const {
        if (_count == 0) {
            return false;
        }
        return _maxVal > threshold;
      }

    private:
      float _timeTotal;
      unsigned int _count;
      unsigned long _maxVal;
      unsigned long _minVal;
};

/*
String getLogLine(const TimingStats& stats) {
    unsigned long minVal, maxVal;
    unsigned int c = stats.getMinMax(minVal, maxVal);
    String log;
    log.reserve(64);
    log += F("Count: ");
    log += c;
    log += F(" Avg/min/max ");
    log += stats.getAvg();
    log += '/';
    log += minVal;
    log += '/';
    log += maxVal;
    log += F(" usec");
    return log;
}
*/

String getPluginFunctionName(int function) {
    switch(function) {
        case PLUGIN_INIT_ALL:              return F("INIT_ALL");
        case PLUGIN_INIT:                  return F("INIT");
        case PLUGIN_READ:                  return F("READ");
        case PLUGIN_ONCE_A_SECOND:         return F("ONCE_A_SECOND");
        case PLUGIN_TEN_PER_SECOND:        return F("TEN_PER_SECOND");
        case PLUGIN_DEVICE_ADD:            return F("DEVICE_ADD");
        case PLUGIN_EVENTLIST_ADD:         return F("EVENTLIST_ADD");
        case PLUGIN_WEBFORM_SAVE:          return F("WEBFORM_SAVE");
        case PLUGIN_WEBFORM_LOAD:          return F("WEBFORM_LOAD");
        case PLUGIN_WEBFORM_SHOW_VALUES:   return F("WEBFORM_SHOW_VALUES");
        case PLUGIN_GET_DEVICENAME:        return F("GET_DEVICENAME");
        case PLUGIN_GET_DEVICEVALUENAMES:  return F("GET_DEVICEVALUENAMES");
        case PLUGIN_WRITE:                 return F("WRITE");
        case PLUGIN_EVENT_OUT:             return F("EVENT_OUT");
        case PLUGIN_WEBFORM_SHOW_CONFIG:   return F("WEBFORM_SHOW_CONFIG");
        case PLUGIN_SERIAL_IN:             return F("SERIAL_IN");
        case PLUGIN_UDP_IN:                return F("UDP_IN");
        case PLUGIN_CLOCK_IN:              return F("CLOCK_IN");
        case PLUGIN_TIMER_IN:              return F("TIMER_IN");
        case PLUGIN_FIFTY_PER_SECOND:      return F("FIFTY_PER_SECOND");
        case PLUGIN_SET_CONFIG:            return F("SET_CONFIG");
        case PLUGIN_GET_DEVICEGPIONAMES:   return F("GET_DEVICEGPIONAMES");
        case PLUGIN_EXIT:                  return F("EXIT");
        case PLUGIN_GET_CONFIG:            return F("GET_CONFIG");
        case PLUGIN_UNCONDITIONAL_POLL:    return F("UNCONDITIONAL_POLL");
        case PLUGIN_REQUEST:               return F("REQUEST");
    }
    return getUnknownString();
}

bool mustLogFunction(int function) {
    switch(function) {
        case PLUGIN_INIT_ALL:              return false;
        case PLUGIN_INIT:                  return false;
        case PLUGIN_READ:                  return true;
        case PLUGIN_ONCE_A_SECOND:         return true;
        case PLUGIN_TEN_PER_SECOND:        return true;
        case PLUGIN_DEVICE_ADD:            return false;
        case PLUGIN_EVENTLIST_ADD:         return false;
        case PLUGIN_WEBFORM_SAVE:          return false;
        case PLUGIN_WEBFORM_LOAD:          return false;
        case PLUGIN_WEBFORM_SHOW_VALUES:   return false;
        case PLUGIN_GET_DEVICENAME:        return false;
        case PLUGIN_GET_DEVICEVALUENAMES:  return false;
        case PLUGIN_WRITE:                 return true;
        case PLUGIN_EVENT_OUT:             return true;
        case PLUGIN_WEBFORM_SHOW_CONFIG:   return false;
        case PLUGIN_SERIAL_IN:             return true;
        case PLUGIN_UDP_IN:                return true;
        case PLUGIN_CLOCK_IN:              return false;
        case PLUGIN_TIMER_IN:              return true;
        case PLUGIN_FIFTY_PER_SECOND:      return true;
        case PLUGIN_SET_CONFIG:            return false;
        case PLUGIN_GET_DEVICEGPIONAMES:   return false;
        case PLUGIN_EXIT:                  return false;
        case PLUGIN_GET_CONFIG:            return false;
        case PLUGIN_UNCONDITIONAL_POLL:    return false;
        case PLUGIN_REQUEST:               return true;
    }
    return false;
}

String getCPluginCFunctionName(int function) {
    switch(function) {
        case CPLUGIN_PROTOCOL_ADD:              return F("CPLUGIN_PROTOCOL_ADD");
        case CPLUGIN_PROTOCOL_TEMPLATE:         return F("CPLUGIN_PROTOCOL_TEMPLATE");
        case CPLUGIN_PROTOCOL_SEND:             return F("CPLUGIN_PROTOCOL_SEND");
        case CPLUGIN_PROTOCOL_RECV:             return F("CPLUGIN_PROTOCOL_RECV");
        case CPLUGIN_GET_DEVICENAME:            return F("CPLUGIN_GET_DEVICENAME");
        case CPLUGIN_WEBFORM_SAVE:              return F("CPLUGIN_WEBFORM_SAVE");
        case CPLUGIN_WEBFORM_LOAD:              return F("CPLUGIN_WEBFORM_LOAD");
        case CPLUGIN_GET_PROTOCOL_DISPLAY_NAME: return F("CPLUGIN_GET_PROTOCOL_DISPLAY_NAME");
        case CPLUGIN_TASK_CHANGE_NOTIFICATION:  return F("CPLUGIN_TASK_CHANGE_NOTIFICATION");
        case CPLUGIN_INIT:                      return F("CPLUGIN_INIT");
        case CPLUGIN_UDP_IN:                    return F("CPLUGIN_UDP_IN");
    }
    return getUnknownString();
}

bool mustLogCFunction(int function) {
    switch(function) {
        case CPLUGIN_PROTOCOL_ADD:              return false;
        case CPLUGIN_PROTOCOL_TEMPLATE:         return false;
        case CPLUGIN_PROTOCOL_SEND:             return true;
        case CPLUGIN_PROTOCOL_RECV:             return true;
        case CPLUGIN_GET_DEVICENAME:            return false;
        case CPLUGIN_WEBFORM_SAVE:              return false;
        case CPLUGIN_WEBFORM_LOAD:              return false;
        case CPLUGIN_GET_PROTOCOL_DISPLAY_NAME: return false;
        case CPLUGIN_TASK_CHANGE_NOTIFICATION:  return false;
        case CPLUGIN_INIT:                      return false;
        case CPLUGIN_UDP_IN:                    return true;
    }
    return false;
}

std::map<int,TimingStats> pluginStats;
std::map<int,TimingStats> controllerStats;
std::map<int,TimingStats> miscStats;
unsigned long timingstats_last_reset = 0;

#define LOADFILE_STATS          0
#define SAVEFILE_STATS          1
#define LOOP_STATS              2
#define PLUGIN_CALL_50PS        3
#define PLUGIN_CALL_10PS        4
#define PLUGIN_CALL_10PSU       5
#define PLUGIN_CALL_1PS         6
#define SENSOR_SEND_TASK        7
#define SEND_DATA_STATS         8
#define COMPUTE_FORMULA_STATS   9
#define PROC_SYS_TIMER          10
#define SET_NEW_TIMER           11
#define TIME_DIFF_COMPUTE       12
#define MQTT_DELAY_QUEUE        13
#define C001_DELAY_QUEUE        14
#define C002_DELAY_QUEUE        15
#define C003_DELAY_QUEUE        16
#define C004_DELAY_QUEUE        17
#define C005_DELAY_QUEUE        18
#define C006_DELAY_QUEUE        19
#define C007_DELAY_QUEUE        20
#define C008_DELAY_QUEUE        21
#define C009_DELAY_QUEUE        22
#define C010_DELAY_QUEUE        23
#define C011_DELAY_QUEUE        24
#define C012_DELAY_QUEUE        25
#define C013_DELAY_QUEUE        26
#define C014_DELAY_QUEUE        27
#define C015_DELAY_QUEUE        28
#define C016_DELAY_QUEUE        29
#define C017_DELAY_QUEUE        30
#define C018_DELAY_QUEUE        31
#define C019_DELAY_QUEUE        32
#define C020_DELAY_QUEUE        33
#define TRY_CONNECT_HOST_TCP    34
#define TRY_CONNECT_HOST_UDP    35
#define HOST_BY_NAME_STATS      36
#define CONNECT_CLIENT_STATS    37
#define LOAD_CUSTOM_TASK_STATS  38
#define WIFI_ISCONNECTED_STATS  39
#define WIFI_NOTCONNECTED_STATS 40
#define LOAD_TASK_SETTINGS      41
#define TRY_OPEN_FILE           42
#define SPIFFS_GC_SUCCESS       43
#define SPIFFS_GC_FAIL          44
#define RULES_PROCESSING        45
#define GRAT_ARP_STATS          46
#define BACKGROUND_TASKS        47
#define HANDLE_SCHEDULER_IDLE   48
#define HANDLE_SCHEDULER_TASK   49




#define START_TIMER const unsigned statisticsTimerStart(micros());
#define STOP_TIMER_TASK(T,F)  if (mustLogFunction(F)) pluginStats[T*256 + F].add(usecPassedSince(statisticsTimerStart));
#define STOP_TIMER_CONTROLLER(T,F)  if (mustLogCFunction(F)) controllerStats[T*256 + F].add(usecPassedSince(statisticsTimerStart));
//#define STOP_TIMER_LOADFILE miscStats[LOADFILE_STATS].add(usecPassedSince(statisticsTimerStart));
#define STOP_TIMER(L)       miscStats[L].add(usecPassedSince(statisticsTimerStart));


String getMiscStatsName(int stat) {
    switch (stat) {
        case LOADFILE_STATS:        return F("Load File");
        case SAVEFILE_STATS:        return F("Save File");
        case LOOP_STATS:            return F("Loop");
        case PLUGIN_CALL_50PS:      return F("Plugin call 50 p/s");
        case PLUGIN_CALL_10PS:      return F("Plugin call 10 p/s");
        case PLUGIN_CALL_10PSU:     return F("Plugin call 10 p/s U");
        case PLUGIN_CALL_1PS:       return F("Plugin call  1 p/s");
        case SENSOR_SEND_TASK:      return F("SensorSendTask()");
        case SEND_DATA_STATS:       return F("sendData()");
        case COMPUTE_FORMULA_STATS: return F("Compute formula");
        case PROC_SYS_TIMER:        return F("proc_system_timer()");
        case SET_NEW_TIMER:         return F("setNewTimerAt()");
        case TIME_DIFF_COMPUTE:     return F("timeDiff()");
        case MQTT_DELAY_QUEUE:      return F("Delay queue MQTT");
        case TRY_CONNECT_HOST_TCP:  return F("try_connect_host() (TCP)");
        case TRY_CONNECT_HOST_UDP:  return F("try_connect_host() (UDP)");
        case HOST_BY_NAME_STATS:    return F("hostByName()");
        case CONNECT_CLIENT_STATS:  return F("connectClient()");
        case LOAD_CUSTOM_TASK_STATS: return F("LoadCustomTaskSettings()");
        case WIFI_ISCONNECTED_STATS: return F("WiFi.isConnected()");
        case WIFI_NOTCONNECTED_STATS: return F("WiFi.isConnected() (fail)");
        case LOAD_TASK_SETTINGS:     return F("LoadTaskSettings()");
        case TRY_OPEN_FILE:          return F("TryOpenFile()");
        case SPIFFS_GC_SUCCESS:      return F("SPIFFS GC success");
        case SPIFFS_GC_FAIL:         return F("SPIFFS GC fail");
        case RULES_PROCESSING:       return F("rulesProcessing()");
        case GRAT_ARP_STATS:         return F("sendGratuitousARP()");
        case BACKGROUND_TASKS:       return F("backgroundtasks()");
        case HANDLE_SCHEDULER_IDLE:  return F("handle_schedule() idle");
        case HANDLE_SCHEDULER_TASK:  return F("handle_schedule() task");
        case C001_DELAY_QUEUE:
        case C002_DELAY_QUEUE:
        case C003_DELAY_QUEUE:
        case C004_DELAY_QUEUE:
        case C005_DELAY_QUEUE:
        case C006_DELAY_QUEUE:
        case C007_DELAY_QUEUE:
        case C008_DELAY_QUEUE:
        case C009_DELAY_QUEUE:
        case C010_DELAY_QUEUE:
        case C011_DELAY_QUEUE:
        case C012_DELAY_QUEUE:
        case C013_DELAY_QUEUE:
        case C014_DELAY_QUEUE:
        case C015_DELAY_QUEUE:
        case C016_DELAY_QUEUE:
        case C017_DELAY_QUEUE:
        case C018_DELAY_QUEUE:
        case C019_DELAY_QUEUE:
        case C020_DELAY_QUEUE:
        {
          String result;
          result.reserve(16);
          result = F("Delay queue ");
          result += get_formatted_Controller_number(static_cast<int>(stat - C001_DELAY_QUEUE + 1));
          return result;
        }
    }
    return getUnknownString();
}


struct portStatusStruct {
  portStatusStruct() : state(-1), output(-1), command(0), init(0), mode(0), task(0), monitor(0), forceMonitor(0), forceEvent(0), previousTask(-1), x(-1) {}

  int8_t state : 2; //-1,0,1
  int8_t output : 2; //-1,0,1
  int8_t command : 2; //0,1
  int8_t init : 2; //0,1

  uint8_t mode : 3; //6 current values (max. 8)
  uint8_t task : 2; //0-3 (max. 4)
  uint8_t monitor : 1; //0,1
  uint8_t forceMonitor : 1; //0,1
  uint8_t forceEvent : 1; //0,1

  int8_t previousTask : 8;

  int8_t x; //used to synchronize the Plugin_prt vector index (x) with the PLUGIN_ID
};

std::map<uint32_t, portStatusStruct> globalMapPortStatus;






void applyFactoryDefaultPref() {
  // TODO TD-er: Store it in more places to make it more persistent
  Settings.ResetFactoryDefaultPreference = ResetFactoryDefaultPreference.getPreference();
}

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

      // case DeviceModel_default: break;
      default: break;
    }
  }

  int8_t button[4];
  int8_t relais[4];
  int8_t status_led = DEFAULT_PIN_STATUS_LED;
  int8_t i2c_sda = DEFAULT_PIN_I2C_SDA;
  int8_t i2c_scl = DEFAULT_PIN_I2C_SCL;
};

void addPredefinedPlugins(const GpioFactorySettingsStruct& gpio_settings);
void addPredefinedRules(const GpioFactorySettingsStruct& gpio_settings);



/* #######################################################################################################
# Supported units of measure as output type for sensor values
####################################################################################################### */
struct UnitOfMeasure {
  enum uom_t {
    latitude,
    longitude,
    altitude,
    speed,
    hdop,
    snr_dBHz,
  };
};

#ifdef USES_PACKED_RAW_DATA

// Data types used in packed encoder.
// p_uint16_1e2 means it is a 16 bit unsigned int, but multiplied by 100 first.
// This allows to store 2 decimals of a floating point value in 8 bits, ranging from 0.00 ... 2.55
// For example p_int24_1e6 is a 24-bit signed value, ideal to store a GPS coordinate
// with 6 decimals using only 3 bytes instead of 4 a normal float would use.
//
// PackedData_uintX_1eY = 0x11XY  (X= #bytes, Y=exponent)
// PackedData_intX_1eY  = 0x12XY  (X= #bytes, Y=exponent)
typedef uint32_t PackedData_enum;
#define PackedData_uint8        0x1110
#define PackedData_uint16       0x1120
#define PackedData_uint24       0x1130
#define PackedData_uint32       0x1140
#define PackedData_uint8_1e3    0x1113
#define PackedData_uint8_1e2    0x1112
#define PackedData_uint8_1e1    0x1111
#define PackedData_uint16_1e5   0x1125
#define PackedData_uint16_1e4   0x1124
#define PackedData_uint16_1e3   0x1123
#define PackedData_uint16_1e2   0x1122
#define PackedData_uint16_1e1   0x1121
#define PackedData_uint24_1e6   0x1136
#define PackedData_uint24_1e5   0x1135
#define PackedData_uint24_1e4   0x1134
#define PackedData_uint24_1e3   0x1133
#define PackedData_uint24_1e2   0x1132
#define PackedData_uint24_1e1   0x1131
#define PackedData_uint32_1e6   0x1146
#define PackedData_uint32_1e5   0x1145
#define PackedData_uint32_1e4   0x1144
#define PackedData_uint32_1e3   0x1143
#define PackedData_uint32_1e2   0x1142
#define PackedData_uint32_1e1   0x1141
#define PackedData_int8         0x1210
#define PackedData_int16        0x1220
#define PackedData_int24        0x1230
#define PackedData_int32        0x1240
#define PackedData_int8_1e3     0x1213
#define PackedData_int8_1e2     0x1212
#define PackedData_int8_1e1     0x1211
#define PackedData_int16_1e5    0x1225
#define PackedData_int16_1e4    0x1224
#define PackedData_int16_1e3    0x1223
#define PackedData_int16_1e2    0x1222
#define PackedData_int16_1e1    0x1221
#define PackedData_int24_1e6    0x1236
#define PackedData_int24_1e5    0x1235
#define PackedData_int24_1e4    0x1234
#define PackedData_int24_1e3    0x1233
#define PackedData_int24_1e2    0x1232
#define PackedData_int24_1e1    0x1231
#define PackedData_int32_1e6    0x1246
#define PackedData_int32_1e5    0x1245
#define PackedData_int32_1e4    0x1244
#define PackedData_int32_1e3    0x1243
#define PackedData_int32_1e2    0x1242
#define PackedData_int32_1e1    0x1241
#define PackedData_pluginid     1
#define PackedData_latLng       2
#define PackedData_hdop         3
#define PackedData_altitude     4
#define PackedData_vcc          5
#define PackedData_pct_8        6

uint8_t getPackedDataTypeSize(PackedData_enum dtype, float& factor, float& offset) {
  offset = 0;
  factor = 1;
  if (dtype > 0x1000 && dtype < 0x12FF) {
    const uint8_t exponent = dtype & 0xF;
    switch(exponent) {
      case 0: factor = 1; break;
      case 1: factor = 1e1; break;
      case 2: factor = 1e2; break;
      case 3: factor = 1e3; break;
      case 4: factor = 1e4; break;
      case 5: factor = 1e5; break;
      case 6: factor = 1e6; break;
    }
    const uint8_t size = (dtype >> 8) & 0xF;
    return size;
  }
  switch (dtype) {
    case PackedData_pluginid:    factor = 1;         return 1;
    case PackedData_latLng:      factor = 46600;     return 3; // 2^23 / 180
    case PackedData_hdop:        factor = 10;        return 1;
    case PackedData_altitude:    factor = 4;     offset = 1000; return 2; // -1000 .. 15383.75 meter
    case PackedData_vcc:         factor = 41.83; offset = 1;    return 1; // -1 .. 5.12V
    case PackedData_pct_8:       factor = 2.56;                 return 1; // 0 .. 100%
    default:
      break;
  }

  // Unknown type
  factor = 1;
  return 0;
}

void LoRa_uintToBytes(uint64_t value, uint8_t byteSize, byte *data, uint8_t& cursor) {
  // Clip values to upper limit
  const uint64_t upperlimit = (1 << (8*byteSize)) - 1;
  if (value > upperlimit) { value = upperlimit; }
  for (uint8_t x = 0; x < byteSize; x++) {
    byte next = 0;
    if (sizeof(value) > x) {
      next = static_cast<byte>((value >> (x * 8)) & 0xFF);
    }
    data[cursor] = next;
    ++cursor;
  }
}

void LoRa_intToBytes(int64_t value, uint8_t byteSize, byte *data, uint8_t& cursor) {
  // Clip values to lower limit
  const int64_t lowerlimit = (1 << ((8*byteSize) - 1)) * -1;
  if (value < lowerlimit) { value = lowerlimit; }
  if (value < 0) {
    value += (1 << (8*byteSize));
  }
  LoRa_uintToBytes(value, byteSize, data, cursor);
}

String LoRa_base16Encode(byte *data, size_t size) {
  String output;
  output.reserve(size * 2);
  char buffer[3];
  for (unsigned i=0; i<size; i++)
  {
    sprintf(buffer, "%02X", data[i]);
    output += buffer[0];
    output += buffer[1];
  }
  return output;
}

String LoRa_addInt(uint64_t value, PackedData_enum datatype) {
  float factor, offset;
  uint8_t byteSize = getPackedDataTypeSize(datatype, factor, offset);
  byte data[4] = {0};
  uint8_t cursor = 0;
  LoRa_uintToBytes((value + offset) * factor, byteSize, &data[0], cursor);
  return LoRa_base16Encode(data, cursor);
}


static String LoRa_addFloat(float value, PackedData_enum datatype) {
  float factor, offset;
  uint8_t byteSize = getPackedDataTypeSize(datatype, factor, offset);
  byte data[4] = {0};
  uint8_t cursor = 0;
  LoRa_intToBytes((value + offset) * factor, byteSize, &data[0], cursor);
  return LoRa_base16Encode(data, cursor);
}

#endif // USES_PACKED_RAW_DATA


// These wifi event functions must be in a .h-file because otherwise the preprocessor
// may not filter the ifdef checks properly.
// Also the functions use a lot of global defined variables, so include at the end of this file.
#include "ESPEasyWiFiEvent.h"
#define SPIFFS_CHECK(result, fname) if (!(result)) { return(FileError(__LINE__, fname)); }
#include "WebServer_Rules.h"

#ifdef USES_BLYNK
// Blynk_get prototype
//boolean Blynk_get(const String& command, byte controllerIndex,float *data = NULL );

int firstEnabledBlynkController() {
  for (byte i = 0; i < CONTROLLER_MAX; ++i) {
    byte ProtocolIndex = getProtocolIndex(Settings.Protocol[i]);
    if (Protocol[ProtocolIndex].Number == 12 && Settings.ControllerEnabled[i]) {
      return i;
    }
  }
  return -1;
}
#endif

// These have to be at the end of this .h file for now, 
// since they use functions or objects otherwise not yet declared


// Must be included after all the defines, since it is using TASKS_MAX
#include "_Plugin_Helper.h"
// Plugin helper needs the defined controller sets, thus include after 'define_plugin_sets.h'
#include "_CPlugin_Helper.h"


#endif /* ESPEASY_GLOBALS_H_ */
