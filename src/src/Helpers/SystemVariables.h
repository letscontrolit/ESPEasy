#ifndef HELPERS_SYSTEMVARIABLES_H
#define HELPERS_SYSTEMVARIABLES_H

#include "../../ESPEasy_common.h"

class SystemVariables {
public:

  enum Enum : uint8_t {
    // For optmization, keep enums sorted alfabetically by their flash string
    BOARD_NAME,
    BOOT_CAUSE,
    BSSID,
    CLIENTIP,
    CR,
    DNS,
    DNS_1,
    DNS_2,
    ESP_CHIP_CORES,
    ESP_CHIP_FREQ,
    ESP_CHIP_ID,
    ESP_CHIP_MODEL,
    ESP_CHIP_REVISION,
#if FEATURE_ETHERNET
    ETHCONNECTED,
    ETHDUPLEX,
    ETHSPEED,
    ETHSPEEDSTATE,
    ETHSTATE,
    ETHWIFIMODE,
#endif // if FEATURE_ETHERNET

    FLASH_CHIP_MODEL,
    FLASH_CHIP_VENDOR,
    FLASH_FREQ,
    FLASH_SIZE,
    FS_FREE,
    FS_SIZE,
    GATEWAY,
#if FEATURE_INTERNAL_TEMPERATURE
    INTERNAL_TEMPERATURE,
#endif // if FEATURE_INTERNAL_TEMPERATURE

    IP4,
    IP,
#if FEATURE_USE_IPV6
    IP6_LOCAL,
#endif
    ISVAR_DOUBLE,
    ISLIMITED_BUILD,
    ISMQTT,
    ISMQTTIMP,
    ISNTP,
    ISWIFI,
    LCLTIME,
    LCLTIME_AM,
    LF,
    SUNRISE_M,
    SUNSET_M,
    MAC,
    MAC_INT,
    S_LF,
    S_CR,
    RSSI,
    SPACE,
    SSID,
    SUBNET,
    SUNRISE,
    SUNRISE_S,
    SUNSET,
    SUNSET_S,
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
    SYSMONTH_S,
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
    SYSTM_HM_AM,
    SYSTM_HM_AM_0,
    SYSTM_HM_AM_SP,
    SYSTM_HM_SP,
    SYSTZOFFSET,
    SYSWEEKDAY,
    SYSWEEKDAY_S,
    SYSYEAR,
    SYSYEARS,
    SYSYEAR_0,
    SYS_MONTH_0,
    UNIT_sysvar,
#if FEATURE_ZEROFILLED_UNITNUMBER
    UNIT_0_sysvar,
#endif // FEATURE_ZEROFILLED_UNITNUMBER
    UNIXDAY,
    UNIXDAY_SEC,
    UNIXTIME,
    UPTIME,
    UPTIME_MS,
    VCC,
    VARIABLE, // Can not be the first 'v' variable, as the name is only 1 character long
    WI_CH,


    // Keep UNKNOWN as last
    UNKNOWN
  };

  // Find the next thing to replace.
  // Return UNKNOWN when nothing needs to be replaced.
  static SystemVariables::Enum nextReplacementEnum(const String        & str,
                                                   SystemVariables::Enum last_tested,
                                                   int                 & last_percent_pos);

  static String                     toString(SystemVariables::Enum enumval);

  static SystemVariables::Enum      startIndex_beginWith(char beginchar);
  static const __FlashStringHelper* toFlashString(SystemVariables::Enum enumval);

  static String                     getSystemVariable(SystemVariables::Enum enumval);

  static void                       parseSystemVariables(String& s,
                                                         boolean useURLencode);
};


#endif // HELPERS_SYSTEMVARIABLES_H
