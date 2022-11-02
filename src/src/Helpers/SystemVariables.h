#ifndef HELPERS_SYSTEMVARIABLES_H
#define HELPERS_SYSTEMVARIABLES_H

#include <Arduino.h>

#include "../../ESPEasy_common.h"

class SystemVariables {

public:

  enum Enum : uint8_t {
    // For optmization, keep enums sorted alfabetically
    BOOT_CAUSE,
    BSSID,
    CR,
    IP,
    IP4,  // 4th IP octet
    SUBNET,
    GATEWAY,
    DNS,
    DNS_1,
    DNS_2,
    CLIENTIP,
    ISMQTT,
    ISMQTTIMP,
    ISNTP,
    ISWIFI,

    #ifdef USES_ESPEASY_NOW
    ESPEASY_NOW_ENABLED,
    ESPEASY_NOW_FORCED_CHANNEL,
    ESPEASY_NOW_CHANNEL,
    ESPEASY_NOW_MQTT,
    ESPEASY_NOW_DISTANCE,
    #endif

    #if FEATURE_ETHERNET
    ETHWIFIMODE,
    ETHCONNECTED,
    ETHDUPLEX,
    ETHSPEED,
    ETHSTATE,
    ETHSPEEDSTATE,
    #endif // if FEATURE_ETHERNET

    LCLTIME,
    LCLTIME_AM,
    LF,
    MAC,
    MAC_INT,
    RSSI,
    SPACE,
    SSID,
    SUNRISE,
    SUNSET,
    SUNRISE_S,
    SUNSET_S,
    SUNRISE_M,
    SUNSET_M,
    SYSBUILD_DATE,
    SYSBUILD_DESCR,
    SYSBUILD_FILENAME,
    SYSBUILD_GIT,
    SYSBUILD_TIME,
    SYSDAY,
    SYSDAY_0,
    SYSHEAP,
    SYSHOUR,
    SYSHOUR_0,
    SYSLOAD,
    SYSMIN,
    SYSMIN_0,
    SYSMONTH,
    SYSNAME,
    SYSSEC,
    SYSSEC_0,
    SYSSEC_D,
    SYSSTACK,
    SYSTIME,
    SYSTIME_AM,
    SYSTIME_AM_0,
    SYSTIME_AM_SP,
    SYSTM_HM,
    SYSTM_HM_0,
    SYSTM_HM_SP,
    SYSTM_HM_AM,
    SYSTM_HM_AM_0,
    SYSTM_HM_AM_SP,
    SYSWEEKDAY,
    SYSWEEKDAY_S,
    SYSYEAR,
    SYSYEARS,
    SYSYEAR_0,
    SYS_MONTH_0,
    S_CR,
    S_LF,
    UNIT_sysvar,   // We already use UNIT as define.
    UNIXDAY,
    UNIXDAY_SEC,
    UNIXTIME,
    UPTIME,
    UPTIME_MS,
    VCC,
    WI_CH,
    FLASH_FREQ,    // Frequency of the flash chip
    FLASH_SIZE,    // Real size of the flash chip
    FLASH_CHIP_VENDOR,
    FLASH_CHIP_MODEL,
    FS_SIZE,       // Size of the file system
    FS_FREE,       // Free space (in bytes) on the file system

    ESP_CHIP_ID,
    ESP_CHIP_FREQ,
    ESP_CHIP_MODEL,
    ESP_CHIP_REVISION,
    ESP_CHIP_CORES,
    ESP_BOARD_NAME,

    // Keep UNKNOWN as last
    UNKNOWN
  };

  // Find the next thing to replace.
  // Return UNKNOWN when nothing needs to be replaced.
  static SystemVariables::Enum nextReplacementEnum(const String& str, SystemVariables::Enum last_tested);

  static String toString(SystemVariables::Enum enumval);
  static const __FlashStringHelper * toFlashString(SystemVariables::Enum enumval);

  static String getSystemVariable(SystemVariables::Enum enumval);

  static void parseSystemVariables(String& s, boolean useURLencode);

};




#endif // HELPERS_SYSTEMVARIABLES_H
