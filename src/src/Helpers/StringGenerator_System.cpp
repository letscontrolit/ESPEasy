#include "../Helpers/StringGenerator_System.h"

#include "../Helpers/ESPEasy_time_calc.h"
#include "../Helpers/StringConverter.h"

/*********************************************************************************************\
   ESPEasy specific strings
\*********************************************************************************************/

#include "../CustomBuild/CompiletimeDefines.h"

#if FEATURE_MQTT

//#include <PubSubClient.h>
#include "../Globals/MQTT.h"

const __FlashStringHelper * getMQTT_state() {
  switch (MQTTclient.state()) {
    case MQTT_CONNECTION_TIMEOUT: return F("Connection timeout");
    case MQTT_CONNECTION_LOST: return F("Connection lost");
    case MQTT_CONNECT_FAILED: return F("Connect failed");
    case MQTT_DISCONNECTED: return F("Disconnected");
    case MQTT_CONNECTED: return F("Connected");
    case MQTT_CONNECT_BAD_PROTOCOL: return F("Connect bad protocol");
    case MQTT_CONNECT_BAD_CLIENT_ID: return F("Connect bad client_id");
    case MQTT_CONNECT_UNAVAILABLE: return F("Connect unavailable");
    case MQTT_CONNECT_BAD_CREDENTIALS: return F("Connect bad credentials");
    case MQTT_CONNECT_UNAUTHORIZED: return F("Connect unauthorized");
    default: break;
  }
  return F("");
}

#endif // if FEATURE_MQTT

/********************************************************************************************\
   Get system information
 \*********************************************************************************************/
const __FlashStringHelper * getLastBootCauseString() {
  switch (lastBootCause)
  {
    case BOOT_CAUSE_MANUAL_REBOOT: return F("Manual Reboot");
    case BOOT_CAUSE_DEEP_SLEEP:    return F("Deep Sleep");
    case BOOT_CAUSE_COLD_BOOT:     return F("Cold Boot");
    case BOOT_CAUSE_EXT_WD:        return F("External Watchdog");
    case BOOT_CAUSE_SOFT_RESTART:  return F("Soft Reboot");
    case BOOT_CAUSE_SW_WATCHDOG:   return F("SW Watchdog");
    case BOOT_CAUSE_EXCEPTION:     return F("Exception");
    case BOOT_CAUSE_POWER_UNSTABLE: return F("PWR Unstable"); // ESP32 only
  }
  return F("Unknown");
}

#ifdef ESP32
 #ifdef ESP32S2
  #include <esp32s2/rom/rtc.h>
 #else
  #include <rom/rtc.h>
 #endif


// See https://github.com/espressif/esp-idf/blob/master/components/esp32/include/rom/rtc.h
const __FlashStringHelper * getResetReasonString_f(uint8_t icore, bool& isDEEPSLEEP_RESET) {
  isDEEPSLEEP_RESET = false;

  #ifdef ESP32S2

	// See tools\sdk\esp32\include\esp_rom\include\esp32s2\rom\rtc.h
  switch (rtc_get_reset_reason(icore)) {
    case POWERON_RESET:          return F("Vbat power on reset");                              // 1
    case RTC_SW_SYS_RESET:       return F("Software reset digital core");                      // 3
    case DEEPSLEEP_RESET:        isDEEPSLEEP_RESET = true; break;                              // 5
    case TG0WDT_SYS_RESET:       return F("Timer Group0 Watch dog reset digital core");        // 7
    case TG1WDT_SYS_RESET:       return F("Timer Group1 Watch dog reset digital core");        // 8
    case RTCWDT_SYS_RESET:       return F("RTC Watch dog Reset digital core");                 // 9
    case INTRUSION_RESET:        return F("Instrusion tested to reset CPU");                   // 10
    case TG0WDT_CPU_RESET:       return F("Time Group0 reset CPU");                            // 11
    case RTC_SW_CPU_RESET:       return F("Software reset CPU");                               // 12
    case RTCWDT_CPU_RESET:       return F("RTC Watch dog Reset CPU");                          // 13
    case RTCWDT_BROWN_OUT_RESET: return F("Reset when the vdd voltage is not stable");         // 15
    case RTCWDT_RTC_RESET:       return F("RTC Watch dog reset digital core and rtc module");  // 16
    case TG1WDT_CPU_RESET:       return F("Time Group1 reset CPU");                            // 17
    case SUPER_WDT_RESET:        return F("Super watchdog reset digital core and rtc module"); // 18
    case GLITCH_RTC_RESET:       return F("Glitch reset digital core and rtc module");         // 19
    case EFUSE_RESET:            return F("EFUSE_RESET"); // FIXME TD-er: No idea what may cause this
    case NO_MEAN:                break; // Undefined, "No Meaning"
  }

  #else

  // See https://github.com/espressif/esp-idf/blob/master/components/esp32/include/rom/rtc.h
  switch (rtc_get_reset_reason((RESET_REASON)icore)) {
    case NO_MEAN:                return F("NO_MEAN");
    case POWERON_RESET:          return F("Vbat power on reset");
    case SW_RESET:               return F("Software reset digital core");
    case OWDT_RESET:             return F("Legacy watch dog reset digital core");
    case DEEPSLEEP_RESET:        isDEEPSLEEP_RESET = true; break;
    case SDIO_RESET:             return F("Reset by SLC module, reset digital core");
    case TG0WDT_SYS_RESET:       return F("Timer Group0 Watch dog reset digital core");
    case TG1WDT_SYS_RESET:       return F("Timer Group1 Watch dog reset digital core");
    case RTCWDT_SYS_RESET:       return F("RTC Watch dog Reset digital core");
    case INTRUSION_RESET:        return F("Instrusion tested to reset CPU");
    case TGWDT_CPU_RESET:        return F("Time Group reset CPU");
    case SW_CPU_RESET:           return F("Software reset CPU");
    case RTCWDT_CPU_RESET:       return F("RTC Watch dog Reset CPU");
    case EXT_CPU_RESET:          return F("for APP CPU, reseted by PRO CPU");
    case RTCWDT_BROWN_OUT_RESET: return F("Reset when the vdd voltage is not stable");
    case RTCWDT_RTC_RESET:       return F("RTC Watch dog reset digital core and rtc module");
    default: break;
  }
  #endif
  return F("");
}


String getResetReasonString(uint8_t icore) {
  bool isDEEPSLEEP_RESET(false);
  const String res = getResetReasonString_f(icore, isDEEPSLEEP_RESET);
  if (!res.isEmpty()) return res;

  if (isDEEPSLEEP_RESET) {
    String reason = F("Deep Sleep, Wakeup reason (");
    reason += rtc_get_wakeup_cause();
    reason += ')';

/*
  switch (reason) {
  #if CONFIG_IDF_TARGET_ESP32S2
    case POWERON_RESET:
    case RTC_SW_CPU_RESET:
    case DEEPSLEEP_RESET:
    case RTC_SW_SYS_RESET:
  #elif CONFIG_IDF_TARGET_ESP32
    case POWERON_RESET:
    case SW_CPU_RESET:
    case DEEPSLEEP_RESET:
    case SW_RESET:
  #endif
  }
*/
    return reason;
  }

  return F("Unknown");
}

#endif // ifdef ESP32

String getResetReasonString() {
  #ifdef ESP32
  String reason = F("CPU0: ");
  reason += getResetReasonString(0);
  if (getChipCores() > 1) { // Only report if we really have more than 1 core
    reason += F(" CPU1: ");
    reason += getResetReasonString(1);
  }
  return reason;
  #else // ifdef ESP32
  return ESP.getResetReason();
  #endif // ifdef ESP32
}

String getSystemBuildString() {
  return formatSystemBuildNr(get_build_nr());
}

String formatSystemBuildNr(uint16_t buildNr) {
  if (buildNr < 20200) return String(buildNr);

  // Build NR is used as a "revision" nr for settings
  // As of 2022-08-18, it is the nr of days since 2022-08-18 + 20200
  const uint32_t seconds_since_start = (buildNr - 20200) * 86400;
  const uint32_t unix_time_start = 1660780800; // Thu Aug 18 2022 00:00:00 GMT+0000
  struct tm build_time;
  breakTime(unix_time_start + seconds_since_start, build_time);

  String res = formatDateString(build_time, '\0');
  return res;
}

String getPluginDescriptionString() {
  return F(
    ""
  #ifdef PLUGIN_BUILD_NORMAL
  "[Normal]"
  #endif // ifdef PLUGIN_BUILD_NORMAL
  #ifdef PLUGIN_BUILD_COLLECTION
  "[Collection]"
  #endif // ifdef PLUGIN_BUILD_COLLECTION
  #ifdef PLUGIN_BUILD_DEV
  "[Development]"
  #endif // ifdef PLUGIN_BUILD_DEV
  #ifdef PLUGIN_DESCR
  "[" PLUGIN_DESCR "]"
  #endif // ifdef PLUGIN_DESCR
  #ifdef BUILD_NO_DEBUG
  "[No Debug Log]"
  #endif
  #if FEATURE_NON_STANDARD_24_TASKS && defined(ESP8266)
  "[24tasks]"
  #endif // if FEATURE_NON_STANDARD_24_TASKS && defined(ESP8266)
  );
}

String getSystemLibraryString() {
  String result;

  #if defined(ESP32)
  result += F("ESP32 SDK ");
  result += ESP.getSdkVersion();
  #else // if defined(ESP32)
  result += F("ESP82xx Core ");
  result += ESP.getCoreVersion();
  result += F(", NONOS SDK ");
  result += system_get_sdk_version();
  result += F(", LWIP: ");
  result += getLWIPversion();
  #endif // if defined(ESP32)

  if (puyaSupport()) {
    result += F(" PUYA support");
  }
  return result;
}

#ifdef ESP8266
String getLWIPversion() {
  String result;

  result += LWIP_VERSION_MAJOR;
  result += '.';
  result += LWIP_VERSION_MINOR;
  result += '.';
  result += LWIP_VERSION_REVISION;

  if (LWIP_VERSION_IS_RC) {
    result += F("-RC");
    result += LWIP_VERSION_RC;
  } else if (LWIP_VERSION_IS_DEVELOPMENT) {
    result += F("-dev");
  }
  return result;
}

#endif // ifdef ESP8266

