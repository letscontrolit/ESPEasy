#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#ifndef ESPEASY_GLOBALS_H_
#define ESPEASY_GLOBALS_H_

// ********************************************************************************
//   User specific configuration
// ********************************************************************************

// Set default configuration settings if you want (not mandatory)
// You can always change these during runtime and save to eeprom
// After loading firmware, issue a 'reset' command to load the defaults.
// --- Basic Config Settings ------------------------------------------------------------------------
#define DEFAULT_NAME        "ESP_Easy"          // Enter your device friendly name
#define UNIT                            0                                       // Unit Number
#define DEFAULT_DELAY       60                  // Sleep Delay in seconds

// --- Wifi AP Mode (when your Wifi Network is not reachable) ----------------------------------------
#define DEFAULT_AP_IP       192,168,4,1         // Enter IP address (comma separated) for AP (config) mode
#define DEFAULT_AP_SUBNET   255,255,255,0       // Enter IP address (comma separated) for AP (config) mode
#define DEFAULT_AP_KEY      "configesp"         // Enter network WPA key for AP (config) mode

// --- Wifi Client Mode -----------------------------------------------------------------------------
#define DEFAULT_SSID        "ssid"              // Enter your Wifi network SSID
#define DEFAULT_KEY         "wpakey"            // Enter your Wifi network WPA key

#define DEFAULT_USE_STATIC_IP   false           // (true|false) enabled or disabled static IP
#define DEFAULT_IP          "192.168.0.50"      // Enter your IP address
#define DEFAULT_DNS         "192.168.0.1"       // Enter your DNS
#define DEFAULT_GW          "192.168.0.1"       // Enter your Gateway
#define DEFAULT_SUBNET      "255.255.255.0"     // Enter your Subnet
#define DEFAULT_IPRANGE_LOW  "0.0.0.0"          // Allowed IP range to access webserver
#define DEFAULT_IPRANGE_HIGH "255.255.255.255"  // Allowed IP range to access webserver
#define DEFAULT_IP_BLOCK_LEVEL 1                // 0: ALL_ALLOWED  1: LOCAL_SUBNET_ALLOWED  2: ONLY_IP_RANGE_ALLOWED

#define DEFAULT_WIFI_CONNECTION_TIMEOUT  10000  // minimum timeout in ms for WiFi to be connected.

// --- Default Controller ------------------------------------------------------------------------------
#define DEFAULT_CONTROLLER   false              // true or false enabled or disabled, set 1st controller defaults
// using a default template, you also need to set a DEFAULT PROTOCOL to a suitable MQTT protocol !
#define DEFAULT_PUB         "sensors/espeasy/%sysname%/%tskname%/%valname%" // Enter your pub
#define DEFAULT_SUB         "sensors/espeasy/%sysname%/#" // Enter your sub
#define DEFAULT_SERVER      "192.168.0.8"       // Enter your Server IP address
#define DEFAULT_PORT        8080                // Enter your Server port value

#define DEFAULT_PROTOCOL    0                   // Protocol used for controller communications
//   0 = Stand-alone (no controller set)
//   1 = Domoticz HTTP
//   2 = Domoticz MQTT
//   3 = Nodo Telnet
//   4 = ThingSpeak
//   5 = OpenHAB MQTT
//   6 = PiDome MQTT
//   7 = EmonCMS
//   8 = Generic HTTP
//   9 = FHEM HTTP

#define DEFAULT_PIN_I2C_SDA              4
#define DEFAULT_PIN_I2C_SCL              5

#define DEFAULT_PIN_STATUS_LED           -1
#define DEFAULT_PIN_STATUS_LED_INVERSED  true



// --- Advanced Settings ---------------------------------------------------------------------------------
#if defined(ESP32)
  #define USE_RTOS_MULTITASKING
#endif
#ifdef M5STACK_ESP
//  #include <M5Stack.h>
#endif

#define DEFAULT_USE_RULES                       false   // (true|false) Enable Rules?

#define DEFAULT_MQTT_RETAIN                     false   // (true|false) Retain MQTT messages?
#define DEFAULT_MQTT_DELAY                      100    // Time in milliseconds to retain MQTT messages
#define DEFAULT_MQTT_LWT_TOPIC                  ""      // Default lwt topic
#define DEFAULT_MQTT_LWT_CONNECT_MESSAGE        "Connected" // Default lwt message
#define DEFAULT_MQTT_LWT_DISCONNECT_MESSAGE     "Connection Lost" // Default lwt message
#define DEFAULT_MQTT_USE_UNITNAME_AS_CLIENTID   0

#define DEFAULT_USE_NTP                         false   // (true|false) Use NTP Server
#define DEFAULT_NTP_HOST                        ""              // NTP Server Hostname
#define DEFAULT_TIME_ZONE                       0               // Time Offset (in minutes)
#define DEFAULT_USE_DST                         false   // (true|false) Use Daily Time Saving

#define LOG_TO_SERIAL         1
#define LOG_TO_SYSLOG         2
#define LOG_TO_WEBLOG         3
#define LOG_TO_SDCARD         4
#define DEFAULT_SYSLOG_IP                       ""                      // Syslog IP Address
#define DEFAULT_SYSLOG_LEVEL            0                               // Syslog Log Level
#define DEFAULT_SERIAL_LOG_LEVEL        0                               // Serial Log Level
#define DEFAULT_WEB_LOG_LEVEL           LOG_LEVEL_INFO                  // Web Log Level
#define DEFAULT_SD_LOG_LEVEL            0                               // SD Card Log Level
#define DEFAULT_USE_SD_LOG                      false                   // (true|false) Enable Logging to the SD card

#define DEFAULT_USE_SERIAL                      true    // (true|false) Enable Logging to the Serial Port
#define DEFAULT_SERIAL_BAUD                     115200  // Serial Port Baud Rate

#define DEFAULT_SYSLOG_FACILITY 	0 	// kern

/*
// --- Experimental Advanced Settings (NOT ACTIVES at this time) ------------------------------------

#define DEFAULT_USE_GLOBAL_SYNC                 false           // (true|false)
#define DEFAULT_SYNC_UDP_PORT                   0                       //

#define DEFAULT_IP_OCTET                                0                       //
#define DEFAULT_WD_IC2_ADDRESS                  0                       //
#define DEFAULT_USE_SSDP                        false           // (true|false)
#define DEFAULT_CON_FAIL_THRES                  0                       //
#define DEFAULT_I2C_CLOCK_LIMIT                 0                       //
*/


// Enable FEATURE_ADC_VCC to measure supply voltage using the analog pin
// Please note that the TOUT pin has to be disconnected in this mode
// Use the "System Info" device to read the VCC value
#ifndef FEATURE_ADC_VCC
  #define FEATURE_ADC_VCC                  false
#endif

#if defined(ESP32)
#define ARDUINO_OTA_PORT  3232
#else
#define ARDUINO_OTA_PORT  8266
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

#define BUILD                           20102 // git version 2.1.02
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
#define TIMER_MQTT_DELAY_QUEUE              7
#define TIMER_C001_DELAY_QUEUE              8
#define TIMER_C003_DELAY_QUEUE              9
#define TIMER_C004_DELAY_QUEUE             10
#define TIMER_C007_DELAY_QUEUE             11
#define TIMER_C008_DELAY_QUEUE             12
#define TIMER_C009_DELAY_QUEUE             13
#define TIMER_C010_DELAY_QUEUE             14
#define TIMER_C011_DELAY_QUEUE             15
#define TIMER_C012_DELAY_QUEUE             16
#define TIMER_C013_DELAY_QUEUE             17

#define TIMING_STATS_THRESHOLD         100000

// Minimum delay between messages for a controller to send in msec.
#define CONTROLLER_DELAY_QUEUE_DELAY_MAX   3600000
#define CONTROLLER_DELAY_QUEUE_DELAY_DFLT  100
// Queue length for controller messages not yet sent.
#define CONTROLLER_DELAY_QUEUE_DEPTH_MAX   25
#define CONTROLLER_DELAY_QUEUE_DEPTH_DFLT  10
// Number of retries to send a message by a controller.
// N.B. Retries without a connection to wifi do not count as retry.
#define CONTROLLER_DELAY_QUEUE_RETRY_MAX   10
#define CONTROLLER_DELAY_QUEUE_RETRY_DFLT  10
// Timeout of the client in msec.
#define CONTROLLER_CLIENTTIMEOUT_MAX     1000
#define CONTROLLER_CLIENTTIMEOUT_DFLT    1000


#define PLUGIN_INIT_ALL                     1
#define PLUGIN_INIT                         2
#define PLUGIN_READ                         3
#define PLUGIN_ONCE_A_SECOND                4
#define PLUGIN_TEN_PER_SECOND               5
#define PLUGIN_DEVICE_ADD                   6
#define PLUGIN_EVENTLIST_ADD                7
#define PLUGIN_WEBFORM_SAVE                 8
#define PLUGIN_WEBFORM_LOAD                 9
#define PLUGIN_WEBFORM_SHOW_VALUES         10
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

#define CPLUGIN_PROTOCOL_ADD                1
#define CPLUGIN_PROTOCOL_TEMPLATE           2
#define CPLUGIN_PROTOCOL_SEND               3
#define CPLUGIN_PROTOCOL_RECV               4
#define CPLUGIN_GET_DEVICENAME              5
#define CPLUGIN_WEBFORM_SAVE                6
#define CPLUGIN_WEBFORM_LOAD                7
#define CPLUGIN_GET_PROTOCOL_DISPLAY_NAME   8
#define CPLUGIN_TASK_CHANGE_NOTIFICATION    9
#define CPLUGIN_INIT                       10
#define CPLUGIN_UDP_IN                     11

#define CONTROLLER_HOSTNAME                 1
#define CONTROLLER_IP                       2
#define CONTROLLER_PORT                     3
#define CONTROLLER_USER                     4
#define CONTROLLER_PASS                     5
#define CONTROLLER_SUBSCRIBE                6
#define CONTROLLER_PUBLISH                  7
#define CONTROLLER_LWT_TOPIC                8
#define CONTROLLER_LWT_CONNECT_MESSAGE      9
#define CONTROLLER_LWT_DISCONNECT_MESSAGE  10

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

#if defined(PLUGIN_BUILD_TESTING) || defined(PLUGIN_BUILD_DEV)
  #define DEVICES_MAX                      85
#else
  #ifdef ESP32
    #define DEVICES_MAX                      75
  #else
    #define DEVICES_MAX                      50
  #endif
#endif

#if defined(ESP8266)
  #define TASKS_MAX                          12 // max 12!
  #define MAX_GPIO                           16
#endif
#if defined(ESP32)
  #define TASKS_MAX                          32
  #define MAX_GPIO                           39
#endif

#define CONTROLLER_MAX                      3 // max 4!
#define NOTIFICATION_MAX                    3 // max 4!
#define VARS_PER_TASK                       4
#define PLUGIN_MAX                DEVICES_MAX
#define PLUGIN_CONFIGVAR_MAX                8
#define PLUGIN_CONFIGFLOATVAR_MAX           4
#define PLUGIN_CONFIGLONGVAR_MAX            4
#define PLUGIN_EXTRACONFIGVAR_MAX          16
#define CPLUGIN_MAX                        16
#define NPLUGIN_MAX                         4
#define UNIT_MAX                          254 // unit 255 = broadcast
#define RULES_TIMER_MAX                     8
#define PINSTATE_TABLE_MAX                 32
#define RULES_MAX_SIZE                   2048
#define RULES_MAX_NESTING_LEVEL             3
#define RULESETS_MAX                        4
#define RULES_BUFFER_SIZE                  64
#define NAME_FORMULA_LENGTH_MAX            40
#define RULES_IF_MAX_NESTING_LEVEL          4
#define CUSTOM_VARS_MAX                    16

#define UDP_PACKETSIZE_MAX               2048

#define PIN_MODE_UNDEFINED                  0
#define PIN_MODE_INPUT                      1
#define PIN_MODE_OUTPUT                     2
#define PIN_MODE_PWM                        3
#define PIN_MODE_SERVO                      4

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

// Forward declaration
struct ControllerSettingsStruct;
void scheduleNextDelayQueue(unsigned long id, unsigned long nextTime);
String LoadControllerSettings(int ControllerIndex, ControllerSettingsStruct& controller_settings);
String get_formatted_Controller_number(int controller_index);
bool loglevelActiveFor(byte logLevel);
void addToLog(byte loglevel, const String& string);
void addToLog(byte logLevel, const __FlashStringHelper* flashString);
void statusLED(boolean traffic);
void backgroundtasks();
uint32_t getCurrentFreeStack();
uint32_t getFreeStackWatermark();
bool canYield();

bool getBitFromUL(uint32_t number, byte bitnr);
void setBitToUL(uint32_t& number, byte bitnr, bool value);

enum SettingsType {
  BasicSettings_Type = 0,
  TaskSettings_Type,
  CustomTaskSettings_Type,
  ControllerSettings_Type,
  CustomControllerSettings_Type,
  NotificationSettings_Type,

  SettingsType_MAX

};
bool getSettingsParameters(SettingsType settingsType, int index, int& offset, int& max_size);
String getSettingsTypeString(SettingsType settingsType) {
  switch (settingsType) {
    case BasicSettings_Type:            return F("Settings");
    case TaskSettings_Type:             return F("TaskSettings");
    case CustomTaskSettings_Type:       return F("CustomTaskSettings");
    case ControllerSettings_Type:       return F("ControllerSettings");
    case CustomControllerSettings_Type: return F("CustomControllerSettings");
    case NotificationSettings_Type:     return F("NotificationSettings");
    default:
      break;
  }
  return String();
}
bool showSettingsFileLayout = false;

/*
        To modify the stock configuration without changing this repo file :
    - define USE_CUSTOM_H as a build flags. ie : export PLATFORMIO_BUILD_FLAGS="'-DUSE_CUSTOM_H'"
        - add a "Custom.h" file in this folder.

*/
#ifdef USE_CUSTOM_H
#include "Custom.h"
#endif

#include "WebStaticData.h"
#include "ESPEasyTimeTypes.h"
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
  extern "C" uint32_t _SPIFFS_start;
  extern "C" uint32_t _SPIFFS_end;
  extern "C" uint32_t _SPIFFS_page;
  extern "C" uint32_t _SPIFFS_block;
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
  #include  "esp32_ping.h"
  #include <WebServer.h>
  #include "SPIFFS.h"
  #include <rom/rtc.h>
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
#include <PubSubClient.h>
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

#define ESPEASY_WIFI_DISCONNECTED            0
#define ESPEASY_WIFI_CONNECTED               1
#define ESPEASY_WIFI_GOT_IP                  2
#define ESPEASY_WIFI_SERVICES_INITIALIZED    3

#if defined(ESP32)
void WiFiEvent(system_event_id_t event, system_event_info_t info);
#else
WiFiEventHandler stationConnectedHandler;
WiFiEventHandler stationDisconnectedHandler;
WiFiEventHandler stationGotIpHandler;
WiFiEventHandler APModeStationConnectedHandler;
WiFiEventHandler APModeStationDisconnectedHandler;
#endif

// Setup DNS, only used if the ESP has no valid WiFi config
const byte DNS_PORT = 53;
IPAddress apIP(DEFAULT_AP_IP);
DNSServer dnsServer;
bool dnsServerActive = false;
#ifdef FEATURE_MDNS
MDNSResponder mdns;
#endif

// MQTT client
WiFiClient mqtt;
PubSubClient MQTTclient(mqtt);
bool MQTTclient_should_reconnect = true;
bool MQTTclient_connected = false;
int mqtt_reconnect_count = 0;

//NTP status
bool statusNTPInitialized = false;

// mqtt import status
bool P037_MQTTImport_connected = false;

// udp protocol stuff (syslog, global sync, node info list, ntp time)
WiFiUDP portUDP;

class TimingStats;

/*********************************************************************************************\
 * CRCStruct
\*********************************************************************************************/
struct CRCStruct{
  char compileTimeMD5[16+32+1]= "MD5_MD5_MD5_MD5_BoundariesOfTheSegmentsGoHere...";
  char binaryFilename[32+32+1]= "ThisIsTheDummyPlaceHolderForTheBinaryFilename64ByteLongFilenames";
  char compileTime[16]= __TIME__;
  char compileDate[16]= __DATE__;
  uint8_t runTimeMD5[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  bool checkPassed (void){ return memcmp(compileTimeMD5,runTimeMD5,16)==0 ; }
  uint32_t numberOfCRCBytes=0;
}CRCValues;


bool WiFiConnected(uint32_t timeout_ms);
bool WiFiConnected();
bool hostReachable(const IPAddress& ip);
bool hostReachable(const String& hostname);
void formatMAC(const uint8_t* mac, char (&strMAC)[20]);
void formatIP(const IPAddress& ip, char (&strIP)[20]);
String to_json_object_value(const String& object, const String& value);


bool I2C_read_bytes(uint8_t i2caddr, I2Cdata_bytes& data);
bool I2C_write8_reg(uint8_t i2caddr, byte reg, byte value);
uint8_t I2C_read8_reg(uint8_t i2caddr, byte reg, bool * is_ok = NULL);
uint16_t I2C_read16_reg(uint8_t i2caddr, byte reg);
int32_t I2C_read24_reg(uint8_t i2caddr, byte reg);
uint16_t I2C_read16_LE_reg(uint8_t i2caddr, byte reg);
int16_t I2C_readS16_reg(uint8_t i2caddr, byte reg);
int16_t I2C_readS16_LE_reg(uint8_t i2caddr, byte reg);
I2Cdev i2cdev;

/*********************************************************************************************\
 * SecurityStruct
\*********************************************************************************************/
struct SecurityStruct
{
  SecurityStruct() {
    memset(WifiSSID, 0, sizeof(WifiSSID));
    memset(WifiKey, 0, sizeof(WifiKey));
    memset(WifiSSID2, 0, sizeof(WifiSSID2));
    memset(WifiKey2, 0, sizeof(WifiKey2));
    memset(WifiAPKey, 0, sizeof(WifiAPKey));
    for (byte i = 0; i < CONTROLLER_MAX; ++i) {
      memset(ControllerUser[i], 0, sizeof(ControllerUser[i]));
      memset(ControllerPassword[i], 0, sizeof(ControllerPassword[i]));
    }
    memset(Password, 0, sizeof(Password));
  }
  char          WifiSSID[32];
  char          WifiKey[64];
  char          WifiSSID2[32];
  char          WifiKey2[64];
  char          WifiAPKey[64];
  char          ControllerUser[CONTROLLER_MAX][26];
  char          ControllerPassword[CONTROLLER_MAX][64];
  char          Password[26];
  byte          AllowedIPrangeLow[4]; // TD-er: Use these
  byte          AllowedIPrangeHigh[4];
  byte          IPblockLevel;

  //its safe to extend this struct, up to 4096 bytes, default values in config are 0. Make sure crc is last
  uint8_t       ProgmemMd5[16]; // crc of the binary that last saved the struct to file.
  uint8_t       md5[16];
} SecuritySettings;


/*********************************************************************************************\
 * Custom Variables for usage in rules and http.
 * Syntax: %vX%
 * usage:
 * let,1,10
 * if %v1%=10 do ...
\*********************************************************************************************/
float         customFloatVar[CUSTOM_VARS_MAX];

/*********************************************************************************************\
 * SettingsStruct
\*********************************************************************************************/
struct SettingsStruct
{
  SettingsStruct() {
    clearAll();
  }

  // VariousBits1 defaults to 0, keep in mind when adding bit lookups.
  bool appendUnitToHostname() {  return !getBitFromUL(VariousBits1, 1); }
  void appendUnitToHostname(bool value) { setBitToUL(VariousBits1, 1, !value); }

  void validate() {
    if (UDPPort > 65535) UDPPort = 0;

    if (Latitude  < -90.0  || Latitude > 90.0) Latitude = 0.0;
    if (Longitude < -180.0 || Longitude > 180.0) Longitude = 0.0;
    if (VariousBits1 > (1 << 30)) VariousBits1 = 0;
  }

  bool networkSettingsEmpty() {
    return (IP[0] == 0 && Gateway[0] == 0 && Subnet[0] == 0 && DNS[0] == 0);
  }

  void clearNetworkSettings() {
    for (byte i = 0; i < 4; ++i) {
      IP[i] = 0;
      Gateway[i] = 0;
      Subnet[i] = 0;
      DNS[i] = 0;
    }
  }

  void clearAll() {
    PID = 0;
    Version = 0;
    Build = 0;
    IP_Octet = 0;
    Unit = 0;
    Name[0] = 0;
    NTPHost[0] = 0;
    Delay = 0;
    Pin_i2c_sda = -1;
    Pin_i2c_scl = -1;
    Pin_status_led = -1;
    Pin_sd_cs = -1;
    for (byte i = 0; i < 17; ++i) { PinBootStates[i] = 0; }
    for (byte i = 0; i < 4; ++i) {  Syslog_IP[i] = 0; }
    UDPPort = 0;
    SyslogLevel = 0;
    SerialLogLevel = 0;
    WebLogLevel = 0;
    SDLogLevel = 0;
    BaudRate = 0;
    MessageDelay = 0;
    deepSleep = 0;
    CustomCSS = false;
    DST = false;
    WDI2CAddress = 0;
    UseRules = false;
    UseSerial = false;
    UseSSDP = false;
    UseNTP = false;
    WireClockStretchLimit = 0;
    GlobalSync = false;
    ConnectionFailuresThreshold = 0;
    TimeZone = 0;
    MQTTRetainFlag = false;
    InitSPI = false;
    Pin_status_led_Inversed = false;
    deepSleepOnFail = false;
    UseValueLogger = false;
    ArduinoOTAEnable = false;
    DST_Start = 0;
    DST_End = 0;
    UseRTOSMultitasking = false;
    Pin_Reset = -1;
    SyslogFacility = DEFAULT_SYSLOG_FACILITY;
    StructSize = sizeof(SettingsStruct);
    MQTTUseUnitNameAsClientId = 0;
    Latitude = 0.0;
    Longitude = 0.0;
    VariousBits1 = 0;

    for (byte i = 0; i < CONTROLLER_MAX; ++i) {
      Protocol[i] = 0;
      ControllerEnabled[i] = false;
    }
    for (byte i = 0; i < NOTIFICATION_MAX; ++i) {
      Notification[i] = 0;
      NotificationEnabled[i] = false;
    }
    for (byte task = 0; task < TASKS_MAX; ++task) {
      clearTask(task);
    }
    clearNetworkSettings();
  }

  void clearTask(byte task) {
    for (byte i = 0; i < CONTROLLER_MAX; ++i) {
      TaskDeviceID[i][task] = 0;
      TaskDeviceSendData[i][task] = false;
    }
    TaskDeviceNumber[task] = 0;
    OLD_TaskDeviceID[task] = 0;
    TaskDevicePin1[task] = -1;
    TaskDevicePin2[task] = -1;
    TaskDevicePin3[task] = -1;
    TaskDevicePort[task] = 0;
    TaskDevicePin1PullUp[task] = false;
    for (byte cv = 0; cv < PLUGIN_CONFIGVAR_MAX; ++cv) {
      TaskDevicePluginConfig[task][cv] = 0;
    }
    TaskDevicePin1Inversed[task] = false;
    for (byte cv = 0; cv < PLUGIN_CONFIGFLOATVAR_MAX; ++cv) {
      TaskDevicePluginConfigFloat[task][cv] = 0.0;
    }
    for (byte cv = 0; cv < PLUGIN_CONFIGLONGVAR_MAX; ++cv) {
      TaskDevicePluginConfigLong[task][cv] = 0;
    }
    OLD_TaskDeviceSendData[task] = false;
    TaskDeviceGlobalSync[task] = false;
    TaskDeviceDataFeed[task] = 0;
    TaskDeviceTimer[task] = 0;
    TaskDeviceEnabled[task] = false;
  }

  unsigned long PID;
  int           Version;
  int16_t       Build;
  byte          IP[4];
  byte          Gateway[4];
  byte          Subnet[4];
  byte          DNS[4];
  byte          IP_Octet;
  byte          Unit;
  char          Name[26];
  char          NTPHost[64];
  unsigned long Delay;
  int8_t        Pin_i2c_sda;
  int8_t        Pin_i2c_scl;
  int8_t        Pin_status_led;
  int8_t        Pin_sd_cs;
  int8_t        PinBootStates[17];  // FIXME TD-er: this is ESP8266 number of pins. ESP32 has double.
  byte          Syslog_IP[4];
  unsigned int  UDPPort;
  byte          SyslogLevel;
  byte          SerialLogLevel;
  byte          WebLogLevel;
  byte          SDLogLevel;
  unsigned long BaudRate;
  unsigned long MessageDelay;
  byte          deepSleep;
  boolean       CustomCSS;
  boolean       DST;
  byte          WDI2CAddress;
  boolean       UseRules;
  boolean       UseSerial;
  boolean       UseSSDP;
  boolean       UseNTP;
  unsigned long WireClockStretchLimit;
  boolean       GlobalSync;
  unsigned long ConnectionFailuresThreshold;
  int16_t       TimeZone;
  boolean       MQTTRetainFlag;
  boolean       InitSPI;
  byte          Protocol[CONTROLLER_MAX];
  byte          Notification[NOTIFICATION_MAX]; //notifications, point to a NPLUGIN id
  byte          TaskDeviceNumber[TASKS_MAX];
  unsigned int  OLD_TaskDeviceID[TASKS_MAX];
  union {
    struct {
      int8_t        TaskDevicePin1[TASKS_MAX];
      int8_t        TaskDevicePin2[TASKS_MAX];
      int8_t        TaskDevicePin3[TASKS_MAX];
      byte          TaskDevicePort[TASKS_MAX];
    };
    int8_t        TaskDevicePin[4][TASKS_MAX];
  };
  boolean       TaskDevicePin1PullUp[TASKS_MAX];
  int16_t       TaskDevicePluginConfig[TASKS_MAX][PLUGIN_CONFIGVAR_MAX];
  boolean       TaskDevicePin1Inversed[TASKS_MAX];
  float         TaskDevicePluginConfigFloat[TASKS_MAX][PLUGIN_CONFIGFLOATVAR_MAX];
  long          TaskDevicePluginConfigLong[TASKS_MAX][PLUGIN_CONFIGLONGVAR_MAX];
  boolean       OLD_TaskDeviceSendData[TASKS_MAX];
  boolean       TaskDeviceGlobalSync[TASKS_MAX];
  byte          TaskDeviceDataFeed[TASKS_MAX];    // When set to 0, only read local connected sensorsfeeds
  unsigned long TaskDeviceTimer[TASKS_MAX];
  boolean       TaskDeviceEnabled[TASKS_MAX];
  boolean       ControllerEnabled[CONTROLLER_MAX];
  boolean       NotificationEnabled[NOTIFICATION_MAX];
  unsigned int  TaskDeviceID[CONTROLLER_MAX][TASKS_MAX];        // IDX number (mainly used by Domoticz)
  boolean       TaskDeviceSendData[CONTROLLER_MAX][TASKS_MAX];
  boolean       Pin_status_led_Inversed;
  boolean       deepSleepOnFail;
  boolean       UseValueLogger;
  boolean       ArduinoOTAEnable;
  uint16_t      DST_Start;
  uint16_t      DST_End;
  boolean       UseRTOSMultitasking;
  int8_t        Pin_Reset;
  byte          SyslogFacility;
  uint32_t      StructSize;  // Forced to be 32 bit, to make sure alignment is clear.
  boolean       MQTTUseUnitNameAsClientId;

  //its safe to extend this struct, up to several bytes, default values in config are 0
  //look in misc.ino how config.dat is used because also other stuff is stored in it at different offsets.
  //TODO: document config.dat somewhere here
  float         Latitude;
  float         Longitude;
  uint32_t      VariousBits1;

  // FIXME @TD-er: As discussed in #1292, the CRC for the settings is now disabled.
  // make sure crc is the last value in the struct
  // Try to extend settings to make the checksum 4-byte aligned.
//  uint8_t       ProgmemMd5[16]; // crc of the binary that last saved the struct to file.
//  uint8_t       md5[16];
} Settings;
/*
SettingsStruct* SettingsStruct_ptr = new SettingsStruct;
SettingsStruct& Settings = *SettingsStruct_ptr;
*/


/*********************************************************************************************\
 *  Analyze SettingsStruct and report inconsistencies
 *  Not a member function to be able to use the F-macro
\*********************************************************************************************/
bool SettingsCheck(String& error) {
  error = "";
  if (!Settings.networkSettingsEmpty()) {
    if (Settings.IP[0] == 0 || Settings.Gateway[0] == 0 || Settings.Subnet[0] == 0 || Settings.DNS[0] == 0) {
      error += F("Error: Either fill all IP settings fields or leave all empty");
    }
  }

  return error.length() == 0;
}

/*********************************************************************************************\
 * ControllerSettingsStruct definition
\*********************************************************************************************/
struct ControllerSettingsStruct
{
  ControllerSettingsStruct() : UseDNS(false), Port(0),
      MinimalTimeBetweenMessages(100), MaxQueueDepth(10), MaxRetry(10),
      DeleteOldest(false), ClientTimeout(100), MustCheckReply(false) {
    for (byte i = 0; i < 4; ++i) {
      IP[i] = 0;
    }
    memset(HostName, 0, sizeof(HostName));
    memset(Publish, 0, sizeof(Publish));
    memset(Subscribe, 0, sizeof(Subscribe));
    memset(MQTTLwtTopic, 0, sizeof(MQTTLwtTopic));
    memset(LWTMessageConnect, 0, sizeof(LWTMessageConnect));
    memset(LWTMessageDisconnect, 0, sizeof(LWTMessageDisconnect));
  }
  boolean       UseDNS;
  byte          IP[4];
  unsigned int  Port;
  char          HostName[65];
  char          Publish[129];
  char          Subscribe[129];
  char          MQTTLwtTopic[129];
  char          LWTMessageConnect[129];
  char          LWTMessageDisconnect[129];
  unsigned int  MinimalTimeBetweenMessages;
  unsigned int  MaxQueueDepth;
  unsigned int  MaxRetry;
  boolean       DeleteOldest; // Action to perform when buffer full, delete oldest, or ignore newest.
  unsigned int  ClientTimeout;
  boolean       MustCheckReply; // When set to false, a sent message is considered always successful.

  void validate() {
    if (Port > 65535) Port = 0;
    if (MinimalTimeBetweenMessages < 1  ||  MinimalTimeBetweenMessages > CONTROLLER_DELAY_QUEUE_DELAY_MAX)
      MinimalTimeBetweenMessages = CONTROLLER_DELAY_QUEUE_DELAY_DFLT;
    if (MaxQueueDepth > CONTROLLER_DELAY_QUEUE_DEPTH_MAX) MaxQueueDepth = CONTROLLER_DELAY_QUEUE_DEPTH_DFLT;
    if (MaxRetry > CONTROLLER_DELAY_QUEUE_RETRY_MAX) MaxRetry = CONTROLLER_DELAY_QUEUE_RETRY_MAX;
    if (ClientTimeout < 10 || ClientTimeout > CONTROLLER_CLIENTTIMEOUT_MAX) {
      ClientTimeout = CONTROLLER_CLIENTTIMEOUT_DFLT;
    }
  }

  IPAddress getIP() const {
    IPAddress host(IP[0], IP[1], IP[2], IP[3]);
    return host;
  }

  String getHost() const {
    if (UseDNS) {
      return HostName;
    }
    return getIP().toString();
  }

  void setHostname(const String& controllerhostname) {
    strncpy(HostName, controllerhostname.c_str(), sizeof(HostName));
    updateIPcache();
  }

  boolean checkHostReachable(bool quick) {
    if (!WiFiConnected(10)) {
      return false; // Not connected, so no use in wasting time to connect to a host.
    }
    delay(1); // Make sure the Watchdog will not trigger a reset.
    if (quick && ipSet()) return true;
    if (UseDNS) {
      if (!updateIPcache()) {
        return false;
      }
    }
    return hostReachable(getIP());
  }

  boolean connectToHost(WiFiClient &client) {
    if (!checkHostReachable(true)) {
      return false; // Host not reachable
    }
    byte retry = 2;
    bool connected = false;
    while (retry > 0 && !connected) {
      --retry;
      // In case of domain name resolution error result can be negative.
      // https://github.com/esp8266/Arduino/blob/18f643c7e2d6a0da9d26ff2b14c94e6536ab78c1/libraries/Ethernet/src/Dns.cpp#L44
      // Thus must match the result with 1.
      connected = (client.connect(getIP(), Port) == 1);
      if (connected) return true;
      if (!checkHostReachable(false))
        return false;
    }
    return false;
  }

  // Returns 1 if successful, 0 if there was a problem resolving the hostname or port
  int beginPacket(WiFiUDP &client) {
    if (!checkHostReachable(true)) {
      return 0; // Host not reachable
    }
    byte retry = 2;
    int connected = 0;
    while (retry > 0 && connected == 0) {
      --retry;
      connected = client.beginPacket(getIP(), Port);
      if (connected != 0) return connected;
      if (!checkHostReachable(false))
        return 0;
      delay(10);
    }
    return 0;
  }

  String getHostPortString() const {
    String result = getHost();
    result += ":";
    result += Port;
    return result;
  }

private:
  bool ipSet() {
    for (byte i = 0; i < 4; ++i) {
      if (IP[i] != 0) return true;
    }
    return false;
  }

  bool updateIPcache() {
    if (!UseDNS) {
      return true;
    }
    if (!WiFiConnected()) return false;
    IPAddress tmpIP;
    if (WiFi.hostByName(HostName, tmpIP)) {
      for (byte x = 0; x < 4; x++) {
        IP[x] = tmpIP[x];
      }
      return true;
    }
    return false;
  }

};

typedef std::shared_ptr<ControllerSettingsStruct> ControllerSettingsStruct_ptr_type;
#define MakeControllerSettings(T) ControllerSettingsStruct_ptr_type ControllerSettingsStruct_ptr(new ControllerSettingsStruct());\
                                    ControllerSettingsStruct& T = *ControllerSettingsStruct_ptr;



/*********************************************************************************************\
 * NotificationSettingsStruct
\*********************************************************************************************/
struct NotificationSettingsStruct
{
  NotificationSettingsStruct() : Port(0), Pin1(0), Pin2(0) {
    memset(Server,   0, sizeof(Server));
    memset(Domain,   0, sizeof(Domain));
    memset(Sender,   0, sizeof(Sender));
    memset(Receiver, 0, sizeof(Receiver));
    memset(Subject,  0, sizeof(Subject));
    memset(Body,     0, sizeof(Body));
    memset(User,     0, sizeof(User));
    memset(Pass,     0, sizeof(Pass));
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
    for (byte j = 0; j < (NAME_FORMULA_LENGTH_MAX + 1); ++j) {
      TaskDeviceName[j] = 0;
    }
    for (byte i = 0; i < VARS_PER_TASK; ++i) {
      for (byte j = 0; j < (NAME_FORMULA_LENGTH_MAX + 1); ++j) {
        TaskDeviceFormula[i][j] = 0;
        TaskDeviceValueNames[i][j] = 0;
        TaskDeviceValueDecimals[i] = 2;
      }
    }
    for (byte i = 0; i < PLUGIN_EXTRACONFIGVAR_MAX; ++i) {
      TaskDevicePluginConfigLong[i] = 0;
      TaskDevicePluginConfig[i] = 0;
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
 * EventStruct
\*********************************************************************************************/
struct EventStruct
{
  EventStruct() :
    Data(NULL), idx(0), Par1(0), Par2(0), Par3(0), Par4(0), Par5(0),
    Source(0), TaskIndex(TASKS_MAX), ControllerIndex(0), ProtocolIndex(0), NotificationIndex(0),
    BaseVarIndex(0), sensorType(0), OriginTaskIndex(0) {}
  EventStruct(const struct EventStruct& event):
          String1(event.String1)
        , String2(event.String2)
        , String3(event.String3)
        , String4(event.String4)
        , String5(event.String5)
        , Data(event.Data)
        , idx(event.idx)
        , Par1(event.Par1), Par2(event.Par2), Par3(event.Par3), Par4(event.Par4), Par5(event.Par5)
        , Source(event.Source), TaskIndex(event.TaskIndex), ControllerIndex(event.ControllerIndex)
        , ProtocolIndex(event.ProtocolIndex), NotificationIndex(event.NotificationIndex)
        , BaseVarIndex(event.BaseVarIndex), sensorType(event.sensorType)
        , OriginTaskIndex(event.OriginTaskIndex)
         {}

  String String1;
  String String2;
  String String3;
  String String4;
  String String5;
  byte *Data;
  int idx;
  int Par1;
  int Par2;
  int Par3;
  int Par4;
  int Par5;
  byte Source;
  byte TaskIndex; // index position in TaskSettings array, 0-11
  byte ControllerIndex; // index position in Settings.Controller, 0-3
  byte ProtocolIndex; // index position in protocol array, depending on which controller plugins are loaded.
  byte NotificationIndex; // index position in Settings.Notification, 0-3
  //Edwin: Not needed, and wasnt used. We can determine the protocol index with getNotificationProtocolIndex(NotificationIndex)
  // byte NotificationProtocolIndex; // index position in notification array, depending on which controller plugins are loaded.
  byte BaseVarIndex;
  byte sensorType;
  byte OriginTaskIndex;
};


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

    int write_idx;
    int read_idx;
    unsigned long timeStamp[LOG_STRUCT_MESSAGE_LINES];
    unsigned long lastReadTimeStamp;
    byte log_level[LOG_STRUCT_MESSAGE_LINES];
    String Message[LOG_STRUCT_MESSAGE_LINES];

} Logging;

std::deque<char> serialLogBuffer;
unsigned long last_serial_log_read = 0;

byte highest_active_log_level = 0;
bool log_to_serial_disabled = false;
bool log_to_serial_disabled_temporary = false;
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
    usesTemplate(false), usesID(false), Custom(false) {}
  uint16_t defaultPort;
  byte Number;
  boolean usesMQTT;
  boolean usesAccount;
  boolean usesPassword;
  boolean usesTemplate;
  boolean usesID;
  boolean Custom;
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
  byte ip[4];
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

/*********************************************************************************************\
 * pinStatesStruct
\*********************************************************************************************/
struct pinStatesStruct
{
  pinStatesStruct() : value(0), plugin(0), index(0), mode(0) {}
  uint16_t value;
  byte plugin;
  byte index;
  byte mode;
} pinStates[PINSTATE_TABLE_MAX];


// this offsets are in blocks, bytes = blocks * 4
#define RTC_BASE_STRUCT 64
#define RTC_BASE_USERVAR 74

/*********************************************************************************************\
 * RTCStruct
\*********************************************************************************************/
//max 40 bytes: ( 74 - 64 ) * 4
struct RTCStruct
{
  RTCStruct() : ID1(0), ID2(0), unused1(false), factoryResetCounter(0),
                deepSleepState(0), bootFailedCount(0), flashDayCounter(0),
                flashCounter(0), bootCounter(0) {}
  byte ID1;
  byte ID2;
  boolean unused1;
  byte factoryResetCounter;
  byte deepSleepState;
  byte bootFailedCount;
  byte flashDayCounter;
  unsigned long flashCounter;
  unsigned long bootCounter;
} RTC;

int deviceCount = -1;
int protocolCount = -1;
int notificationCount = -1;

boolean printToWeb = false;
String printWebString = "";
boolean printToWebJSON = false;

float UserVar[VARS_PER_TASK * TASKS_MAX];

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

unsigned long timermqtt_interval = 250;
unsigned long lastSend = 0;
unsigned long lastWeb = 0;
byte cmd_within_mainloop = 0;
unsigned long connectionFailures = 0;
unsigned long wdcounter = 0;
unsigned long timerAPoff = 0;
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

boolean (*CPlugin_ptr[CPLUGIN_MAX])(byte, struct EventStruct*, String&);
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


#ifndef ESP32
// To do some reconnection check.
#include <Ticker.h>
Ticker connectionCheck;
#endif

bool reconnectChecker = false;
void connectionCheckHandler()
{
  if (reconnectChecker == false && !WiFiConnected()){
    reconnectChecker = true;
    WiFi.reconnect();
  }
  else if (WiFiConnected() && reconnectChecker == true){
    reconnectChecker = false;
  }
}

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
bool processedConnect = true;
bool processedDisconnect = true;
bool processedGetIP = true;
bool processedConnectAPmode = true;
bool processedDisconnectAPmode = true;
bool processedScanDone = true;

bool webserver_state = false;
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
    return F("Unknown");
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

std::map<int,TimingStats> pluginStats;
std::map<int,TimingStats> miscStats;
unsigned long timediff_calls = 0;
unsigned long timediff_cpu_cycles_total = 0;
unsigned long timingstats_last_reset = 0;

#define LOADFILE_STATS        0
#define SAVEFILE_STATS        1
#define LOOP_STATS            2
#define PLUGIN_CALL_50PS      3
#define PLUGIN_CALL_10PS      4
#define PLUGIN_CALL_10PSU     5
#define PLUGIN_CALL_1PS       6
#define SENSOR_SEND_TASK      7
#define SEND_DATA_STATS       8
#define COMPUTE_FORMULA_STATS 9
#define PROC_SYS_TIMER       10
#define SET_NEW_TIMER        11
#define TIME_DIFF_COMPUTE    12
#define MQTT_DELAY_QUEUE     13
#define C001_DELAY_QUEUE     14
#define C002_DELAY_QUEUE     15
#define C003_DELAY_QUEUE     16
#define C004_DELAY_QUEUE     17
#define C005_DELAY_QUEUE     18
#define C006_DELAY_QUEUE     19
#define C007_DELAY_QUEUE     20
#define C008_DELAY_QUEUE     21
#define C009_DELAY_QUEUE     22
#define C010_DELAY_QUEUE     23
#define C011_DELAY_QUEUE     24
#define C012_DELAY_QUEUE     25
#define C013_DELAY_QUEUE     26






#define START_TIMER const unsigned statisticsTimerStart(micros());
#define STOP_TIMER_TASK(T,F)  if (mustLogFunction(F)) pluginStats[T*32 + F].add(usecPassedSince(statisticsTimerStart));
//#define STOP_TIMER_LOADFILE miscStats[LOADFILE_STATS].add(usecPassedSince(statisticsTimerStart));
#define STOP_TIMER(L)       miscStats[L].add(usecPassedSince(statisticsTimerStart));


String getMiscStatsName(int stat) {
    switch (stat) {
        case LOADFILE_STATS: return F("Load File");
        case SAVEFILE_STATS: return F("Save File");
        case LOOP_STATS:     return F("Loop");
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
        {
          String result = F("Delay queue ");
          result += get_formatted_Controller_number(static_cast<int>(stat - C001_DELAY_QUEUE + 1));
          return result;
        }
    }
    return F("Unknown");
}

// These wifi event functions must be in a .h-file because otherwise the preprocessor
// may not filter the ifdef checks properly.
// Also the functions use a lot of global defined variables, so include at the end of this file.
#include "ESPEasyWiFiEvent.h"


#endif /* ESPEASY_GLOBALS_H_ */
