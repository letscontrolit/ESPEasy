#ifndef STRING_PROVIDER_TYPES_H
#define STRING_PROVIDER_TYPES_H

struct LabelType;
//enum LabelType::Enum : short;

struct LabelType {
enum Enum : short {
  UNIT_NR,
  UNIT_NAME,
  HOST_NAME,

  LOCAL_TIME,
  UPTIME,
  LOAD_PCT,                    // 15.10
  LOOP_COUNT,                  // 400
  CPU_ECO_MODE,                // true

  FREE_MEM,                    // 9876
  FREE_STACK,                  // 3456
#ifdef CORE_POST_2_5_0
  HEAP_MAX_FREE_BLOCK,         // 7654
  HEAP_FRAGMENTATION,          // 12
#endif

  BOOT_TYPE,                   // Cold boot
  BOOT_COUNT,                  // 0
  RESET_REASON,                // Software/System restart
  LAST_TASK_BEFORE_REBOOT,     // Last scheduled task.
  SW_WD_COUNT,

  WIFI_CONNECTION,             // 802.11G
  WIFI_RSSI,                   // -67
  IP_CONFIG,                   // DHCP
  IP_CONFIG_STATIC,
  IP_CONFIG_DYNAMIC,
  IP_ADDRESS,                  // 192.168.1.123
  IP_SUBNET,                   // 255.255.255.0
  IP_ADDRESS_SUBNET,           // 192.168.1.123 / 255.255.255.0
  GATEWAY,                     // 192.168.1.1
  CLIENT_IP,                   // 192.168.1.67
  DNS,                         // 192.168.1.1 / (IP unset)
  DNS_1,
  DNS_2,
  ALLOWED_IP_RANGE,            // 192.168.1.0 - 192.168.1.255
  STA_MAC,                     // EC:FA:BC:0E:AE:5B
  AP_MAC,                      // EE:FA:BC:0E:AE:5B
  SSID,                        // mynetwork
  BSSID,
  CHANNEL,                     // 1
  CONNECTED,                   // 1h16m
  CONNECTED_MSEC,                   // 1h16m
  LAST_DISCONNECT_REASON,      // 200
  LAST_DISC_REASON_STR,        // Beacon timeout
  NUMBER_RECONNECTS,           // 5

  FORCE_WIFI_BG,
  RESTART_WIFI_LOST_CONN,
  FORCE_WIFI_NOSLEEP,
  PERIODICAL_GRAT_ARP,
  CONNECTION_FAIL_THRESH,

  BUILD_DESC,
  GIT_BUILD,
  SYSTEM_LIBRARIES,
  PLUGINS,
  PLUGIN_DESCRIPTION,
  BUILD_TIME,
  BINARY_FILENAME,

  SYSLOG_LOG_LEVEL,
  SERIAL_LOG_LEVEL,
  WEB_LOG_LEVEL,
#ifdef FEATURE_SD
  SD_LOG_LEVEL,
#endif

  ESP_CHIP_ID,
  ESP_CHIP_FREQ,
  ESP_BOARD_NAME,

  FLASH_CHIP_ID,
  FLASH_CHIP_REAL_SIZE,
  FLASH_IDE_SIZE,
  FLASH_IDE_SPEED,
  FLASH_IDE_MODE,
  FLASH_WRITE_COUNT,
  SKETCH_SIZE,
  SKETCH_FREE,
  SPIFFS_SIZE,
  SPIFFS_FREE,


};
};


String getInternalLabel(LabelType::Enum label);
String getLabel(LabelType::Enum label);
String getValue(LabelType::Enum label);
void stream_next_json_object_value(LabelType::Enum label);
void stream_last_json_object_value(LabelType::Enum label);
void addRowLabelValue(LabelType::Enum label);
void addRowLabelValue_copy(LabelType::Enum label);
void addFormCheckBox(LabelType::Enum label, boolean checked, bool disabled = false);
void addFormCheckBox_disabled(LabelType::Enum label, boolean checked);



#endif // STRING_PROVIDER_TYPES_H
