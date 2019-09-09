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
#include "ESPEasy_plugindefs.h"

#include "ESPEasy_buildinfo.h"


//#include <FS.h>

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

// User configuration
#include "DataStructs/ESPEasyDefaults.h"

// Make sure to have this as early as possible in the build process.
#include "define_plugin_sets.h"


// ********************************************************************************
//   DO NOT CHANGE ANYTHING BELOW THIS LINE
// ********************************************************************************


#define NODE_TYPE_ID_ESP_EASY_STD           1
#define NODE_TYPE_ID_RPI_EASY_STD           5  // https://github.com/enesbcs/rpieasy
#define NODE_TYPE_ID_ESP_EASYM_STD         17
#define NODE_TYPE_ID_ESP_EASY32_STD        33
#define NODE_TYPE_ID_ARDUINO_EASY_STD      65
#define NODE_TYPE_ID_NANO_EASY_STD         81



// ********************************************************************************
//   Timers used in the scheduler
// ********************************************************************************
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


#include <map>
#include <deque>

#include "DataStructs/ESPEasyLimits.h"

#include "DataStructs/ESPEasy_EventStruct.h"
#include "DataStructs/SettingsType.h"
#include "DataStructs/CRCStruct.h"
#include "DataStructs/SecurityStruct.h"
#include "DataStructs/SettingsStruct.h"
#include "DataStructs/DeviceModel.h"
#include "DataStructs/FactoryDefaultPref.h"
#include "DataStructs/ControllerSettingsStruct.h"
#include "DataStructs/NotificationSettingsStruct.h"
#include "DataStructs/ExtraTaskSettingsStruct.h"
#include "DataStructs/DeviceStruct.h"
#include "DataStructs/TimingStats.h"
#include "DataStructs/LogStruct.h"
#include "DataStructs/ProtocolStruct.h"
#include "DataStructs/NotificationStruct.h"
#include "DataStructs/NodeStruct.h"
#include "DataStructs/SystemTimerStruct.h"
#include "DataStructs/RTCStruct.h"
#include "DataStructs/PortStatusStruct.h"


CRCStruct CRCValues;
SecurityStruct SecuritySettings;
SettingsStruct Settings;
ResetFactoryDefaultPreference_struct ResetFactoryDefaultPreference;
ExtraTaskSettingsStruct ExtraTaskSettings;
LogStruct Logging;
NotificationStruct Notification[NPLUGIN_MAX];
RTCStruct RTC;
DeviceVector Device;

std::map<int, TimingStats> pluginStats;
std::map<int, TimingStats> controllerStats;
std::map<int, TimingStats> miscStats;
unsigned long timingstats_last_reset = 0;


#define START_TIMER const unsigned statisticsTimerStart(micros());
#define STOP_TIMER_TASK(T, F) \
  if (mustLogFunction(F)) pluginStats[T * 256 + F].add(usecPassedSince(statisticsTimerStart));
#define STOP_TIMER_CONTROLLER(T, F) \
  if (mustLogCFunction(F)) controllerStats[T * 256 + F].add(usecPassedSince(statisticsTimerStart));

// #define STOP_TIMER_LOADFILE miscStats[LOADFILE_STATS].add(usecPassedSince(statisticsTimerStart));
#define STOP_TIMER(L) miscStats[L].add(usecPassedSince(statisticsTimerStart));


unsigned long connectionFailures = 0;
byte highest_active_log_level = 0;
bool log_to_serial_disabled = false;

boolean (*Plugin_ptr[PLUGIN_MAX])(byte, struct EventStruct*, String&);

std::vector<byte> Plugin_id;
std::vector<int> Task_id_to_Plugin_id;



#include "ESPEasy_Log.h"
#include "WebStaticData.h"
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


/*********************************************************************************************\
 * Custom Variables for usage in rules and http.
 * Syntax: %vX%
 * usage:
 * let,1,10
 * if %v1%=10 do ...
\*********************************************************************************************/
float customFloatVar[CUSTOM_VARS_MAX];

float UserVar[VARS_PER_TASK * TASKS_MAX];










/*********************************************************************************************\
 * Buffer for outputting logs via serial port.
\*********************************************************************************************/
std::deque<char> serialWriteBuffer;




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


int deviceCount = -1;
int protocolCount = -1;
int notificationCount = -1;

boolean printToWeb = false;
String printWebString = "";
boolean printToWebJSON = false;

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
bool channel_changed = false;
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
bool wifiConnectInProgress = false;

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



#endif /* ESPEASY_GLOBALS_H_ */
