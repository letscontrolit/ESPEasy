#ifndef STRING_PROVIDER_TYPES_H
#define STRING_PROVIDER_TYPES_H

#include "../../ESPEasy_common.h"

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
#ifdef ESP8266 // TD-er: Disable setting TX power on ESP32 as it seems to cause issues on IDF4.4
    WIFI_TX_MAX_PWR,     // Unit: 0.25 dBm, 0 = use default (do not set)
    WIFI_CUR_TX_PWR,     // Unit dBm of current WiFi TX power.
    WIFI_SENS_MARGIN,    // Margin in dB on top of sensitivity
    WIFI_SEND_AT_MAX_TX_PWR,
#endif
    WIFI_NR_EXTRA_SCANS,    
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
    ENABLE_TIMING_STATISTICS,
    ENABLE_RULES_CACHING,
//    ENABLE_RULES_EVENT_REORDER, // TD-er: Disabled for now
    TASKVALUESET_ALL_PLUGINS,
    ALLOW_OTA_UNLIMITED,
    ENABLE_CLEAR_HUNG_I2C_BUS,
    #if FEATURE_I2C_DEVICE_CHECK
    ENABLE_I2C_DEVICE_CHECK,
    #endif // if FEATURE_I2C_DEVICE_CHECK
#ifndef BUILD_NO_RAM_TRACKER
    ENABLE_RAM_TRACKING,
#endif
#if FEATURE_AUTO_DARK_MODE
    ENABLE_AUTO_DARK_MODE,
#endif

    BOOT_TYPE,               // Cold boot
    BOOT_COUNT,              // 0
    RESET_REASON,            // Software/System restart
    DEEP_SLEEP_ALTERNATIVE_CALL,
    LAST_TASK_BEFORE_REBOOT, // Last scheduled task.
    SW_WD_COUNT,

    WIFI_CONNECTION,         // 802.11G
    WIFI_RSSI,               // -67
    IP_CONFIG,               // DHCP
    IP_CONFIG_STATIC,
    IP_CONFIG_DYNAMIC,
    IP_ADDRESS,              // 192.168.1.123
    IP_SUBNET,               // 255.255.255.0
    IP_ADDRESS_SUBNET,       // 192.168.1.123 / 255.255.255.0
    GATEWAY,                 // 192.168.1.1
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
    WAIT_WIFI_CONNECT,
    SDK_WIFI_AUTORECONNECT,

    BUILD_DESC,
    GIT_BUILD,
    SYSTEM_LIBRARIES,
    PLUGIN_COUNT,
    PLUGIN_DESCRIPTION,
    BUILD_TIME,
    BINARY_FILENAME,
    BUILD_PLATFORM,
    GIT_HEAD,


    I2C_BUS_STATE,
    I2C_BUS_CLEARED_COUNT,

    SYSLOG_LOG_LEVEL,
    SERIAL_LOG_LEVEL,
    WEB_LOG_LEVEL,
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
    ESP_BOARD_NAME,

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
    OTA_2STEP,
    OTA_POSSIBLE,
#if FEATURE_ETHERNET
    ETH_IP_ADDRESS,
    ETH_IP_SUBNET,
    ETH_IP_ADDRESS_SUBNET,
    ETH_IP_GATEWAY,
    ETH_IP_DNS,
    ETH_MAC,
    ETH_DUPLEX,
    ETH_SPEED,
    ETH_STATE,
    ETH_SPEED_STATE,
    ETH_CONNECTED,
#endif // if FEATURE_ETHERNET
    ETH_WIFI_MODE,
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

String getInternalLabel(LabelType::Enum label,
                        char            replaceSpace = '_');
const __FlashStringHelper * getLabel(LabelType::Enum label);
String getValue(LabelType::Enum label);
String getExtendedValue(LabelType::Enum label);


#endif // STRING_PROVIDER_TYPES_H
