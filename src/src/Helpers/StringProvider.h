#ifndef STRING_PROVIDER_TYPES_H
#define STRING_PROVIDER_TYPES_H

#include "../../ESPEasy_common.h"

#include "../DataStructs/KeyValueStruct.h"

struct KeyValueStruct;

struct LabelType {
  enum Enum : uint8_t {
    UNIT_NR,
    #if FEATURE_ZEROFILLED_UNITNUMBER
    UNIT_NR_0,
    #endif // FEATURE_ZEROFILLED_UNITNUMBER
    UNIT_NAME,
    HOST_NAME,

    LOCAL_TIME,
    TIME_SOURCE,
    TIME_WANDER,
    #if FEATURE_EXT_RTC
    EXT_RTC_UTC_TIME,
    #endif
    UPTIME,
    LOAD_PCT,            // 15.10
    LOOP_COUNT,          // 400
    CPU_ECO_MODE,        // true
#if FEATURE_SET_WIFI_TX_PWR
    WIFI_TX_MAX_PWR,     // Unit: 0.25 dBm, 0 = use default (do not set)
    WIFI_CUR_TX_PWR,     // Unit dBm of current WiFi TX power.
    WIFI_SENS_MARGIN,    // Margin in dB on top of sensitivity
    WIFI_SEND_AT_MAX_TX_PWR,
#endif
    WIFI_AP_CHANNEL,
    WIFI_ENABLE_CAPTIVE_PORTAL,
    WIFI_START_AP_NO_CREDENTIALS,
    WIFI_START_AP_ON_CONNECT_FAIL,
    WIFI_START_AP_ON_NW002_INIT,
    WIFI_NR_RECONNECT_ATTEMPTS,    
    WIFI_MAX_UPTIME_AUTO_START_AP,
    WIFI_AP_MINIMAL_ON_TIME,
#ifdef ESP32
    WIFI_AP_ENABLE_NAPT,
#endif
    WIFI_USE_LAST_CONN_FROM_RTC,

    FREE_MEM,            // 9876
    FREE_STACK,          // 3456
#ifdef USE_SECOND_HEAP
    FREE_HEAP_IRAM,
#endif
#if defined(CORE_POST_2_5_0) || defined(ESP32)
  #ifndef LIMIT_BUILD_SIZE
    HEAP_MAX_FREE_BLOCK, // 7654
  #endif
#endif // if defined(CORE_POST_2_5_0) || defined(ESP32)
#if defined(CORE_POST_2_5_0)
  #ifndef LIMIT_BUILD_SIZE
    HEAP_FRAGMENTATION,  // 12
  #endif
#endif // if defined(CORE_POST_2_5_0)

#ifdef ESP32
    HEAP_SIZE,
    HEAP_MIN_FREE,
    #ifdef BOARD_HAS_PSRAM
    PSRAM_SIZE,
    PSRAM_FREE,
    PSRAM_MIN_FREE,
    PSRAM_MAX_FREE_BLOCK,
    #endif // BOARD_HAS_PSRAM
#endif // ifdef ESP32

    JSON_BOOL_QUOTES,
#if FEATURE_TIMING_STATS
    ENABLE_TIMING_STATISTICS,
#endif
    ENABLE_RULES_CACHING,
    ENABLE_SERIAL_PORT_CONSOLE,
    CONSOLE_SERIAL_PORT,
#if USES_ESPEASY_CONSOLE_FALLBACK_PORT
    CONSOLE_FALLBACK_TO_SERIAL0,
    CONSOLE_FALLBACK_PORT,
#endif
//    ENABLE_RULES_EVENT_REORDER, // TD-er: Disabled for now
    TASKVALUESET_ALL_PLUGINS,
    ALLOW_OTA_UNLIMITED,
#if FEATURE_CLEAR_I2C_STUCK
    ENABLE_CLEAR_HUNG_I2C_BUS,
#endif
    #if FEATURE_I2C_DEVICE_CHECK
    ENABLE_I2C_DEVICE_CHECK,
    #endif // if FEATURE_I2C_DEVICE_CHECK
#ifndef BUILD_NO_RAM_TRACKER
    ENABLE_RAM_TRACKING,
#endif
#if FEATURE_AUTO_DARK_MODE
    ENABLE_AUTO_DARK_MODE,
#endif
#if FEATURE_RULES_EASY_COLOR_CODE
    DISABLE_RULES_AUTOCOMPLETE,
#endif // if FEATURE_RULES_EASY_COLOR_CODE
#if FEATURE_TARSTREAM_SUPPORT
    DISABLE_SAVE_CONFIG_AS_TAR,
#endif // if FEATURE_TARSTREAM_SUPPORT
    #if FEATURE_TASKVALUE_UNIT_OF_MEASURE
    SHOW_UOM_ON_DEVICES_PAGE,
    #endif // if FEATURE_TASKVALUE_UNIT_OF_MEASURE
#if CONFIG_SOC_WIFI_SUPPORT_5G
    WIFI_BAND_MODE,
#endif
    #if FEATURE_MQTT_CONNECT_BACKGROUND
    MQTT_CONNECT_IN_BACKGROUND,
    #endif // if FEATURE_MQTT_CONNECT_BACKGROUND

    BOOT_TYPE,               // Cold boot
    BOOT_COUNT,              // 0
    RESET_REASON,            // Software/System restart
    DEEP_SLEEP_ALTERNATIVE_CALL,
    LAST_TASK_BEFORE_REBOOT, // Last scheduled task.
    SW_WD_COUNT,

    WIFI_CONNECTION,         // 802.11G
    WIFI_RSSI,               // -67
    IP_CONFIG,               // DHCP
    IP_ADDRESS,              // 192.168.1.123
    IP_SUBNET,               // 255.255.255.0
    IP_ADDRESS_SUBNET,       // 192.168.1.123 / 255.255.255.0
    GATEWAY,                 // 192.168.1.1
#if FEATURE_USE_IPV6
    IP6_LOCAL,
    IP6_GLOBAL,
//    IP6_ALL_ADDRESSES,
//    IP6_ADDRESS_CDIR,
//    IP6_GATEWAY,
#endif
    CLIENT_IP,               // 192.168.1.67
    #if FEATURE_MDNS
    M_DNS,                   // breadboard.local
    #endif // if FEATURE_MDNS
    DNS,                     // 192.168.1.1 / (IP unset)
    DNS_1,
    DNS_2,
    ALLOWED_IP_RANGE,        // 192.168.1.0 - 192.168.1.255
    STA_MAC,                 // EC:FA:BC:0E:AE:5B
    AP_MAC,                  // EE:FA:BC:0E:AE:5B
    SSID,                    // mynetwork
    BSSID,
    CHANNEL,                 // 1
    ENCRYPTION_TYPE_STA,     // WPA2
    CONNECTED,               // 1h16m
    CONNECTED_MSEC,          // 1h16m
    LAST_DISCONNECT_REASON,  // 200
    LAST_DISC_REASON_STR,    // Beacon timeout
    NUMBER_RECONNECTS,       // 5
    WIFI_STORED_SSID1,
    WIFI_STORED_SSID2,

    FORCE_WIFI_BG,
    RESTART_WIFI_LOST_CONN,
    FORCE_WIFI_NOSLEEP,
    PERIODICAL_GRAT_ARP,
    CONNECTION_FAIL_THRESH,
#ifndef ESP32
    WAIT_WIFI_CONNECT,
#endif
    HIDDEN_SSID_SLOW_CONNECT,
    CONNECT_HIDDEN_SSID,
#ifdef ESP32
    WIFI_PASSIVE_SCAN,
#endif
    SDK_WIFI_AUTORECONNECT,
#if FEATURE_USE_IPV6
    ENABLE_IPV6,
#endif

    BUILD_DESC,
    BUILD_ORIGIN,
    GIT_BUILD,
    SYSTEM_LIBRARIES,
#ifdef ESP32
    ESP_IDF_SDK_VERSION,
#endif
    PLUGIN_COUNT,
    PLUGIN_DESCRIPTION,
    BUILD_TIME,
    BINARY_FILENAME,
    BUILD_PLATFORM,
    GIT_HEAD,
    #ifdef CONFIGURATION_CODE
    CONFIGURATION_CODE_LBL,
    #endif // ifdef CONFIGURATION_CODE

#if FEATURE_CLEAR_I2C_STUCK
    I2C_BUS_STATE,
    I2C_BUS_CLEARED_COUNT,
#endif
#if FEATURE_SYSLOG
    SYSLOG_LOG_LEVEL,
#endif
    SERIAL_LOG_LEVEL,
# ifdef WEBSERVER_LOG
    WEB_LOG_LEVEL,
#endif
#if FEATURE_SD
    SD_LOG_LEVEL,
#endif // if FEATURE_SD

    ESP_CHIP_ID,
    ESP_CHIP_FREQ,
#ifdef ESP32
    ESP_CHIP_XTAL_FREQ,
    ESP_CHIP_APB_FREQ,
#endif
    ESP_CHIP_MODEL,
    ESP_CHIP_REVISION,
    ESP_CHIP_CORES,
    BOARD_NAME,

    FLASH_CHIP_ID,
    FLASH_CHIP_VENDOR,
    FLASH_CHIP_MODEL,
    FLASH_CHIP_REAL_SIZE,
    FLASH_CHIP_SPEED,
    FLASH_IDE_SIZE,
    FLASH_IDE_SPEED,
    FLASH_IDE_MODE,
    FLASH_WRITE_COUNT,
    SKETCH_SIZE,
    SKETCH_FREE,
    FS_SIZE,
    FS_FREE,
    MAX_OTA_SKETCH_SIZE,
#ifdef ESP8266
    OTA_2STEP,
    OTA_POSSIBLE,
#endif
    #if FEATURE_INTERNAL_TEMPERATURE
    INTERNAL_TEMPERATURE,
    #endif // if FEATURE_INTERNAL_TEMPERATURE
#if FEATURE_ETHERNET
    ETH_MAC,
    ETH_DUPLEX,
    ETH_SPEED,
    ETH_STATE,
    ETH_SPEED_STATE,
    ETH_CONNECTED,
    ETH_CHIP,
#endif // if FEATURE_ETHERNET
# if FEATURE_ETHERNET || defined(USES_ESPEASY_NOW)
    ETH_WIFI_MODE,
#endif
    SUNRISE,
    SUNSET,
    ISNTP,
    UPTIME_MS,
    TIMEZONE_OFFSET,
    LATITUDE,
    LONGITUDE,
    SUNRISE_S,
    SUNSET_S,
    SUNRISE_M,
    SUNSET_M,


    MAX_LABEL  // Keep as last
  };
};


#if FEATURE_ETHERNET
String getEthSpeed();

String getEthLinkSpeedState();
#endif // if FEATURE_ETHERNET

String getInternalLabel(const KeyValueStruct& kv,
                        char            replaceSpace = '_');
String getInternalLabel(LabelType::Enum label,
                        char            replaceSpace = '_');

String getLabel(const KeyValueStruct& kv);
String getLabel(LabelType::Enum label);

String getValue(const KeyValueStruct& kv);
int64_t getValue_int(const KeyValueStruct& kv);
double  getValue_float(const KeyValueStruct& kv);
String getValue(LabelType::Enum label);
String getExtendedValue(LabelType::Enum label);

String getFormNote(LabelType::Enum label);
#if FEATURE_TASKVALUE_UNIT_OF_MEASURE
String getFormUnit(LabelType::Enum label);
#endif
KeyValueStruct getKeyValue(LabelType::Enum label, bool extendedValue = false);


#endif // STRING_PROVIDER_TYPES_H
