#include "SystemVariables.h"


#include "../../ESPEasy_common.h"
#include "../../ESPEasy_fdwdecl.h"
#include "../../ESPEasy-Globals.h"

#include "../DataStructs/TimingStats.h"

#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../ESPEasyCore/ESPEasyNetwork.h"

#include "../Globals/CRCValues.h"
#include "../Globals/ESPEasy_time.h"
#include "../Globals/ESPEasyWiFiEvent.h"
#ifdef USES_MQTT
# include "../Globals/MQTT.h"
#endif // ifdef USES_MQTT
#include "../Globals/NetworkState.h"
#include "../Globals/RuntimeData.h"
#include "../Globals/Settings.h"

#include "../Helpers/CompiletimeDefines.h"
#include "../Helpers/Hardware.h"
#include "../Helpers/Numerical.h"
#include "../Helpers/StringConverter.h"
#include "../Helpers/StringProvider.h"



String getReplacementString(const String& format, String& s) {
  int startpos = s.indexOf(format);
  int endpos   = s.indexOf('%', startpos + 1);
  String R     = s.substring(startpos, endpos + 1);

#ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log = F("ReplacementString SunTime: ");
    log += R;
    log += F(" offset: ");
    log += ESPEasy_time::getSecOffset(R);
    addLog(LOG_LEVEL_DEBUG, log);
  }
#endif // ifndef BUILD_NO_DEBUG
  return R;
}

void replSunRiseTimeString(const String& format, String& s, boolean useURLencode) {
  String R = getReplacementString(format, s);

  repl(R, node_time.getSunriseTimeString(':', ESPEasy_time::getSecOffset(R)), s, useURLencode);
}

void replSunSetTimeString(const String& format, String& s, boolean useURLencode) {
  String R = getReplacementString(format, s);

  repl(R, node_time.getSunsetTimeString(':', ESPEasy_time::getSecOffset(R)), s, useURLencode);
}


String timeReplacement_leadZero(int value) 
{
  char valueString[5] = { 0 };
  sprintf_P(valueString, PSTR("%02d"), value);
  return valueString;
}

#define SMART_REPL_T(T, S)   if (s.indexOf(T) != -1) { (S((T), s, useURLencode)); }

// FIXME TD-er: Try to match these with  StringProvider::getValue

void SystemVariables::parseSystemVariables(String& s, boolean useURLencode)
{
  START_TIMER

  if (s.indexOf('%') == -1) {
    STOP_TIMER(PARSE_SYSVAR_NOCHANGE);
    return;
  }

  SystemVariables::Enum enumval = static_cast<SystemVariables::Enum>(0);

  do {
    enumval = SystemVariables::nextReplacementEnum(s, enumval);
    String value;

    switch (enumval)
    {
      case BSSID:             value = String((WiFiEventData.WiFiDisconnected()) ? F("00:00:00:00:00:00") : WiFi.BSSIDstr()); break;
      case CR:                value = "\r"; break;
      case IP:                value = getValue(LabelType::IP_ADDRESS); break;
      case IP4:               value = String( (int) NetworkLocalIP()[3] ); break; // 4th IP octet
      case SUBNET:            value = getValue(LabelType::IP_SUBNET); break;
      case DNS:               value = getValue(LabelType::DNS); break;
      case GATEWAY:           value = getValue(LabelType::GATEWAY); break;
      case CLIENTIP:          value = getValue(LabelType::CLIENT_IP); break;
      #ifdef USES_MQTT
      case ISMQTT:            value = String(MQTTclient_connected); break;
      #else // ifdef USES_MQTT
      case ISMQTT:            value = "0"; break;
      #endif // ifdef USES_MQTT

      #ifdef USES_P037
      case ISMQTTIMP:         value = String(P037_MQTTImport_connected); break;
      #else // ifdef USES_P037
      case ISMQTTIMP:         value = "0"; break;
      #endif // USES_P037


      case ISNTP:             value = String(statusNTPInitialized ? 1 : 0); break;
      case ISWIFI:            value = String(WiFiEventData.wifiStatus); break; // 0=disconnected, 1=connected, 2=got ip, 4=services initialized
      // TODO: PKR: Add ETH Objects
      #ifdef HAS_ETHERNET
      
      case ETHWIFIMODE:       value = getValue(LabelType::ETH_WIFI_MODE); break; // 0=WIFI, 1=ETH
      case ETHCONNECTED:      value = getValue(LabelType::ETH_CONNECTED); break; // 0=disconnected, 1=connected
      case ETHDUPLEX:         value = getValue(LabelType::ETH_DUPLEX); break;
      case ETHSPEED:          value = getValue(LabelType::ETH_SPEED); break;
      case ETHSTATE:          value = getValue(LabelType::ETH_STATE); break;
      case ETHSPEEDSTATE:     value = getValue(LabelType::ETH_SPEED_STATE); break;
      #endif
      case LCLTIME:           value = getValue(LabelType::LOCAL_TIME); break;
      case LCLTIME_AM:        value = node_time.getDateTimeString_ampm('-', ':', ' '); break;
      case LF:                value = "\n"; break;
      case MAC:               value = getValue(LabelType::STA_MAC); break;
      case MAC_INT:           value = String(getChipId()); break; // Last 24 bit of MAC address as integer, to be used in rules.
      case RSSI:              value = getValue(LabelType::WIFI_RSSI); break;
      case SPACE:             value = " "; break;
      case SSID:              value = (WiFiEventData.WiFiDisconnected()) ? F("--") : WiFi.SSID(); break;
      case SUNRISE:           SMART_REPL_T(SystemVariables::toString(enumval), replSunRiseTimeString); break;
      case SUNSET:            SMART_REPL_T(SystemVariables::toString(enumval), replSunSetTimeString); break;
      case SYSBUILD_DATE:     value = get_build_date(); break;
      case SYSBUILD_DESCR:    value = getValue(LabelType::BUILD_DESC); break;
      case SYSBUILD_FILENAME: value = getValue(LabelType::BINARY_FILENAME); break;
      case SYSBUILD_GIT:      value = getValue(LabelType::GIT_BUILD); break;
      case SYSBUILD_TIME:     value = get_build_time(); break;
      case SYSDAY:            value = String(node_time.day()); break;
      case SYSDAY_0:          value = timeReplacement_leadZero(node_time.day()); break;
      case SYSHEAP:           value = String(ESP.getFreeHeap()); break;
      case SYSHOUR:           value = String(node_time.hour()); break;
      case SYSHOUR_0:         value = timeReplacement_leadZero(node_time.hour()); break;
      case SYSLOAD:           value = String(getCPUload()); break;
      case SYSMIN:            value = String(node_time.minute()); break;
      case SYSMIN_0:          value = timeReplacement_leadZero(node_time.minute()); break;
      case SYSMONTH:          value = String(node_time.month()); break;
      case SYSNAME:           value = Settings.getHostname(); break;
      case SYSSEC:            value = String(node_time.second()); break;
      case SYSSEC_0:          value = timeReplacement_leadZero(node_time.second()); break;
      case SYSSEC_D:          value = String(((node_time.hour() * 60) + node_time.minute()) * 60 + node_time.second()); break;
      case SYSSTACK:          value = getValue(LabelType::FREE_STACK); break;
      case SYSTIME:           value = node_time.getTimeString(':'); break;
      case SYSTIME_AM:        value = node_time.getTimeString_ampm(':'); break;
      case SYSTM_HM:          value = node_time.getTimeString(':', false); break;
      case SYSTM_HM_AM:       value = node_time.getTimeString_ampm(':', false); break;
      case SYSWEEKDAY:        value = String(node_time.weekday()); break;
      case SYSWEEKDAY_S:      value = node_time.weekday_str(); break;
      case SYSYEAR_0:
      case SYSYEAR:           value = String(node_time.year()); break;
      case SYSYEARS:          value = timeReplacement_leadZero(node_time.year() % 100); break;
      case SYS_MONTH_0:       value = timeReplacement_leadZero(node_time.month()); break;
      case S_CR:              value = F("\\r"); break;
      case S_LF:              value = F("\\n"); break;
      case UNIT_sysvar:       value = getValue(LabelType::UNIT_NR); break;
      case UNIXDAY:           value = String(node_time.getUnixTime() / 86400); break;
      case UNIXDAY_SEC:       value = String(node_time.getUnixTime() % 86400); break;
      case UNIXTIME:          value = String(node_time.getUnixTime()); break;
      case UPTIME:            value = String(wdcounter / 2); break;
      #if FEATURE_ADC_VCC
      case VCC:               value = String(vcc); break;
      #else // if FEATURE_ADC_VCC
      case VCC:               value = String(-1); break;
      #endif // if FEATURE_ADC_VCC
      case WI_CH:             value = String((WiFiEventData.WiFiDisconnected()) ? 0 : WiFi.channel()); break;

      case UNKNOWN:
        break;
    }

    switch (enumval)
    {
      case SUNRISE:
      case SUNSET:
      case UNKNOWN:

        // Do not replace
        break;
      default:

        repl(SystemVariables::toString(enumval), value, s, useURLencode);
        break;
    }
  }
  while (enumval != SystemVariables::Enum::UNKNOWN);

  int v_index = s.indexOf(F("%v"));

  while ((v_index != -1)) {
    unsigned int i;
    if (validUIntFromString(s.substring(v_index + 2), i)) {
      const String key = String(F("%v")) + String(i) + '%';
      if (s.indexOf(key) != -1) {
        const bool trimTrailingZeros = true;
        const String value = doubleToString(getCustomFloatVar(i), 6, trimTrailingZeros);
        repl(key, value, s, useURLencode);
      }
    }
    v_index = s.indexOf(F("%v"), v_index + 1); // Find next occurance
  }

  STOP_TIMER(PARSE_SYSVAR);
}

#undef SMART_REPL_T


SystemVariables::Enum SystemVariables::nextReplacementEnum(const String& str, SystemVariables::Enum last_tested)
{
  if (str.indexOf('%') == -1) {
    return Enum::UNKNOWN;
  }

  SystemVariables::Enum nextTested = static_cast<SystemVariables::Enum>(0);

  if (last_tested > nextTested) {
    nextTested = static_cast<SystemVariables::Enum>(last_tested + 1);
  }

  if (nextTested >= Enum::UNKNOWN) {
    return Enum::UNKNOWN;
  }

  String str_prefix        = SystemVariables::toString(nextTested).substring(0, 2);
  bool   str_prefix_exists = str.indexOf(str_prefix) != -1;

  for (int i = nextTested; i < Enum::UNKNOWN; ++i) {
    SystemVariables::Enum enumval = static_cast<SystemVariables::Enum>(i);
    String new_str_prefix         = SystemVariables::toString(enumval).substring(0, 2);

    if ((str_prefix == new_str_prefix) && !str_prefix_exists) {
      // Just continue
    } else {
      str_prefix        = new_str_prefix;
      str_prefix_exists = str.indexOf(str_prefix) != -1;

      if (str_prefix_exists) {
        if (str.indexOf(SystemVariables::toString(enumval)) != -1) {
          return enumval;
        }
      }
    }
  }

  return Enum::UNKNOWN;
}

String SystemVariables::toString(SystemVariables::Enum enumval)
{
  switch (enumval) {
    case Enum::BSSID:           return F("%bssid%");
    case Enum::CR:              return F("%CR%");
    case Enum::IP4:             return F("%ip4%");
    case Enum::IP:              return F("%ip%");
    case Enum::SUBNET:          return F("%subnet%");
    case Enum::DNS:             return F("%dns%");
    case Enum::GATEWAY:         return F("%gateway%");
    case Enum::CLIENTIP:        return F("%clientip%");
    case Enum::ISMQTT:          return F("%ismqtt%");
    case Enum::ISMQTTIMP:       return F("%ismqttimp%");
    case Enum::ISNTP:           return F("%isntp%");
    case Enum::ISWIFI:          return F("%iswifi%");
    #ifdef HAS_ETHERNET
    case Enum::ETHWIFIMODE:   return F("%ethwifimode%");
    case Enum::ETHCONNECTED:   return F("%ethconnected%");
    case Enum::ETHDUPLEX:      return F("%ethduplex%");
    case Enum::ETHSPEED:       return F("%ethspeed%");
    case Enum::ETHSTATE:       return F("%ethstate%");
    case Enum::ETHSPEEDSTATE:       return F("%ethspeedstate%");
    #endif
    case Enum::LCLTIME:         return F("%lcltime%");
    case Enum::LCLTIME_AM:      return F("%lcltime_am%");
    case Enum::LF:              return F("%LF%");
    case Enum::MAC:             return F("%mac%");
    case Enum::MAC_INT:         return F("%mac_int%");
    case Enum::RSSI:            return F("%rssi%");
    case Enum::SPACE:           return F("%SP%");
    case Enum::SSID:            return F("%ssid%");
    case Enum::SUNRISE:         return F("%sunrise");
    case Enum::SUNSET:          return F("%sunset");
    case Enum::SYSBUILD_DATE:   return F("%sysbuild_date%");
    case Enum::SYSBUILD_DESCR:  return F("%sysbuild_desc%");
    case Enum::SYSBUILD_FILENAME:  return F("%sysbuild_filename%");
    case Enum::SYSBUILD_GIT:    return F("%sysbuild_git%");
    case Enum::SYSBUILD_TIME:   return F("%sysbuild_time%");
    case Enum::SYSDAY:          return F("%sysday%");
    case Enum::SYSDAY_0:        return F("%sysday_0%");
    case Enum::SYSHEAP:         return F("%sysheap%");
    case Enum::SYSHOUR:         return F("%syshour%");
    case Enum::SYSHOUR_0:       return F("%syshour_0%");
    case Enum::SYSLOAD:         return F("%sysload%");
    case Enum::SYSMIN:          return F("%sysmin%");
    case Enum::SYSMIN_0:        return F("%sysmin_0%");
    case Enum::SYSMONTH:        return F("%sysmonth%");
    case Enum::SYSNAME:         return F("%sysname%");
    case Enum::SYSSEC:          return F("%syssec%");
    case Enum::SYSSEC_0:        return F("%syssec_0%");
    case Enum::SYSSEC_D:        return F("%syssec_d%");
    case Enum::SYSSTACK:        return F("%sysstack%");
    case Enum::SYSTIME:         return F("%systime%");
    case Enum::SYSTIME_AM:      return F("%systime_am%");
    case Enum::SYSTM_HM:        return F("%systm_hm%");
    case Enum::SYSTM_HM_AM:     return F("%systm_hm_am%");
    case Enum::SYSWEEKDAY:      return F("%sysweekday%");
    case Enum::SYSWEEKDAY_S:    return F("%sysweekday_s%");
    case Enum::SYSYEAR:         return F("%sysyear%");
    case Enum::SYSYEARS:        return F("%sysyears%");
    case Enum::SYSYEAR_0:       return F("%sysyear_0%");
    case Enum::SYS_MONTH_0:     return F("%sysmonth_0%");
    case Enum::S_CR:            return F("%R%");
    case Enum::S_LF:            return F("%N%");
    case Enum::UNIT_sysvar:     return F("%unit%");
    case Enum::UNIXDAY:         return F("%unixday%");
    case Enum::UNIXDAY_SEC:     return F("%unixday_sec%");
    case Enum::UNIXTIME:        return F("%unixtime%");
    case Enum::UPTIME:          return F("%uptime%");
    case Enum::VCC:             return F("%vcc%");
    case Enum::WI_CH:           return F("%wi_ch%");
    case Enum::UNKNOWN: break;
  }
  return F("Unknown");
}
