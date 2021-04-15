#ifndef HELPERS_SYSTEMVARIABLES_H
#define HELPERS_SYSTEMVARIABLES_H

#include <Arduino.h>

#include "../../ESPEasy_common.h"

class SystemVariables {

public:

  enum Enum {
    // For optmization, keep enums sorted alfabetically
    BSSID,
    CR,
    IP,
    IP4,  // 4th IP octet
    SUBNET,
    GATEWAY,
    DNS,
    CLIENTIP,
    ISMQTT,
    ISMQTTIMP,
    ISNTP,
    ISWIFI,
    #ifdef HAS_ETHERNET
    ETHWIFIMODE,
    ETHCONNECTED,
    ETHDUPLEX,
    ETHSPEED,
    ETHSTATE,
    ETHSPEEDSTATE,
    #endif
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
    SYSTM_HM,
    SYSTM_HM_AM,
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
    VCC,
    WI_CH,

    // Keep UNKNOWN as last
    UNKNOWN
  };

  // Find the next thing to replace.
  // Return UNKNOWN when nothing needs to be replaced.
  static Enum nextReplacementEnum(const String& str, Enum last_tested);

  static String toString(Enum enumval);

  static void parseSystemVariables(String& s, boolean useURLencode);



};




#endif // HELPERS_SYSTEMVARIABLES_H
