#include "../Helpers/SystemVariables.h"


#include "../../ESPEasy_common.h"

#include "../../ESPEasy-Globals.h"

#include "../CustomBuild/CompiletimeDefines.h"

#include "../DataStructs/TimingStats.h"

#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../ESPEasyCore/ESPEasyNetwork.h"

#include "../Globals/CRCValues.h"
#include "../Globals/ESPEasy_time.h"
#include "../Globals/ESPEasyWiFiEvent.h"
#if FEATURE_MQTT
# include "../Globals/MQTT.h"
#endif // if FEATURE_MQTT
#include "../Globals/NetworkState.h"
#include "../Globals/RulesCalculate.h"
#include "../Globals/RuntimeData.h"
#include "../Globals/Settings.h"
#include "../Globals/Statistics.h"

#include "../Helpers/Convert.h"
#include "../Helpers/Hardware_device_info.h"
#include "../Helpers/Misc.h"
#include "../Helpers/Numerical.h"
#include "../Helpers/StringConverter.h"
#include "../Helpers/StringProvider.h"


#if defined(ESP8266)
  # include <ESP8266WiFi.h>
#endif // if defined(ESP8266)
#if defined(ESP32)
  # include <WiFi.h>
#endif // if defined(ESP32)


String getReplacementString(const String& format, const String& s) {
  int startpos = s.indexOf(format);
  int endpos   = s.indexOf('%', startpos + 1);
  if (endpos == -1) {
    addLog(LOG_LEVEL_ERROR, concat(F("SunTime syntax error: "), format));
    return format;
  }
  String R     = s.substring(startpos, endpos + 1);


#ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log = F("ReplacementString SunTime: ");
    log += R;
    log += F(" offset: ");
    log += ESPEasy_time::getSecOffset(R);
    addLogMove(LOG_LEVEL_DEBUG, log);
  }
#endif // ifndef BUILD_NO_DEBUG
  return R;
}

void replSunRiseTimeString(const String& format, String& s, boolean useURLencode) {
  const String R(getReplacementString(format, s));

  repl(R, node_time.getSunriseTimeString(':', ESPEasy_time::getSecOffset(R)), s, useURLencode);
}

void replSunSetTimeString(const String& format, String& s, boolean useURLencode) {
  const String R(getReplacementString(format, s));

  repl(R, node_time.getSunsetTimeString(':', ESPEasy_time::getSecOffset(R)), s, useURLencode);
}

String timeReplacement_leadZero(int value)
{
  char valueString[5] = { 0 };

  sprintf_P(valueString, PSTR("%02d"), value);
  return valueString;
}

// FIXME TD-er: Try to match these with  StringProvider::getValue
LabelType::Enum SystemVariables2LabelType(SystemVariables::Enum enumval) {
  LabelType::Enum label = LabelType::MAX_LABEL;

  switch (enumval)
  {
    case SystemVariables::IP:                label = LabelType::IP_ADDRESS; break;
#if FEATURE_USE_IPV6
    case SystemVariables::IP6_LOCAL:         label = LabelType::IP6_LOCAL; break;
#endif
    case SystemVariables::SUBNET:            label = LabelType::IP_SUBNET; break;
    case SystemVariables::DNS:               label = LabelType::DNS; break;
    case SystemVariables::DNS_1:             label = LabelType::DNS_1; break;
    case SystemVariables::DNS_2:             label = LabelType::DNS_2; break;
    case SystemVariables::GATEWAY:           label = LabelType::GATEWAY; break;
    case SystemVariables::CLIENTIP:          label = LabelType::CLIENT_IP; break;
    #if FEATURE_INTERNAL_TEMPERATURE
    case SystemVariables::INTERNAL_TEMPERATURE: label = LabelType::INTERNAL_TEMPERATURE; break;
    #endif // if FEATURE_INTERNAL_TEMPERATURE

    #if FEATURE_ETHERNET

    case SystemVariables::ETHWIFIMODE:       label = LabelType::ETH_WIFI_MODE; break; // 0=WIFI, 1=ETH
    case SystemVariables::ETHCONNECTED:      label = LabelType::ETH_CONNECTED; break; // 0=disconnected, 1=connected
    case SystemVariables::ETHDUPLEX:         label = LabelType::ETH_DUPLEX; break;
    case SystemVariables::ETHSPEED:          label = LabelType::ETH_SPEED; break;
    case SystemVariables::ETHSTATE:          label = LabelType::ETH_STATE; break;
    case SystemVariables::ETHSPEEDSTATE:     label = LabelType::ETH_SPEED_STATE; break;
    #endif // if FEATURE_ETHERNET
    case SystemVariables::LCLTIME:           label = LabelType::LOCAL_TIME; break;
    case SystemVariables::MAC:               label = LabelType::STA_MAC; break;
    case SystemVariables::RSSI:              label = LabelType::WIFI_RSSI; break;
    case SystemVariables::SUNRISE_S:         label = LabelType::SUNRISE_S; break;
    case SystemVariables::SUNSET_S:          label = LabelType::SUNSET_S; break;
    case SystemVariables::SUNRISE_M:         label = LabelType::SUNRISE_M; break;
    case SystemVariables::SUNSET_M:          label = LabelType::SUNSET_M; break;
    case SystemVariables::SYSBUILD_DESCR:    label = LabelType::BUILD_DESC; break;
    case SystemVariables::SYSBUILD_FILENAME: label = LabelType::BINARY_FILENAME; break;
    case SystemVariables::SYSBUILD_GIT:      label = LabelType::GIT_BUILD; break;
    case SystemVariables::SYSSTACK:          label = LabelType::FREE_STACK; break;
    case SystemVariables::UNIT_sysvar:       label = LabelType::UNIT_NR; break;
    #if FEATURE_ZEROFILLED_UNITNUMBER
    case SystemVariables::UNIT_0_sysvar:     label = LabelType::UNIT_NR_0; break;
    #endif // FEATURE_ZEROFILLED_UNITNUMBER
    case SystemVariables::FLASH_FREQ:        label = LabelType::FLASH_CHIP_SPEED; break;
    case SystemVariables::FLASH_SIZE:        label = LabelType::FLASH_CHIP_REAL_SIZE; break;
    case SystemVariables::FLASH_CHIP_VENDOR: label = LabelType::FLASH_CHIP_VENDOR; break;
    case SystemVariables::FLASH_CHIP_MODEL:  label = LabelType::FLASH_CHIP_MODEL; break;
    case SystemVariables::FS_SIZE:           label = LabelType::FS_SIZE; break;
    case SystemVariables::FS_FREE:           label = LabelType::FS_FREE; break;

    case SystemVariables::ESP_CHIP_ID:       label = LabelType::ESP_CHIP_ID; break;
    case SystemVariables::ESP_CHIP_FREQ:     label = LabelType::ESP_CHIP_FREQ; break;
    case SystemVariables::ESP_CHIP_MODEL:    label = LabelType::ESP_CHIP_MODEL; break;
    case SystemVariables::ESP_CHIP_REVISION: label = LabelType::ESP_CHIP_REVISION; break;
    case SystemVariables::ESP_CHIP_CORES:    label = LabelType::ESP_CHIP_CORES; break;
    case SystemVariables::BOARD_NAME:        label = LabelType::BOARD_NAME; break;

    default:
      // No matching LabelType yet.
      break;
  }
  return label;
}

String SystemVariables::getSystemVariable(SystemVariables::Enum enumval) {
  const LabelType::Enum label = SystemVariables2LabelType(enumval);

  if (LabelType::MAX_LABEL != label) {
    return getValue(label);
  }
  constexpr int INT_NOT_SET = std::numeric_limits<int>::min();

  int intvalue = INT_NOT_SET;

  switch (enumval)
  {
    case BOOT_CAUSE:        intvalue = lastBootCause; break;                         // Integer value to be used in rules
    case BSSID:             return (WiFiEventData.WiFiDisconnected()) ? MAC_address().toString() : WiFi.BSSIDstr();
    case CR:                return String('\r');
    case IP4:               intvalue = static_cast<int>(NetworkLocalIP()[3]); break; // 4th IP octet
    case ISVAR_DOUBLE:      intvalue =
                            #if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
                            1;
                            #else
                            0;
                            #endif
                            break;
    case ISLIMITED_BUILD:   intvalue =
                            #ifdef LIMIT_BUILD_SIZE
                            1;
                            #else
                            0;
                            #endif
                            break;
    case ISMQTT:            intvalue = 
    #if FEATURE_MQTT
        MQTTclient_connected ? 1 :
    #endif // if FEATURE_MQTT
        0; break;

    case ISMQTTIMP:         intvalue = 
    #ifdef USES_P037
        P037_MQTTImport_connected ? 1 :
    #endif // ifdef USES_P037
        0; break;

    case ISNTP:             intvalue = statusNTPInitialized ? 1 : 0; break;
    case ISWIFI:            intvalue = WiFiEventData.wifiStatus; break; // 0=disconnected, 1=connected, 2=got ip, 4=services
    // initialized
    case LCLTIME_AM:        return node_time.getDateTimeString_ampm('-', ':', ' ');
    case LF:                return String('\n');
    case MAC_INT:           intvalue = getChipId(); break; // Last 24 bit of MAC address as integer, to be used in rules.
    case SPACE:             return String(' ');
    case SSID:              return (WiFiEventData.WiFiDisconnected()) ? String(F("--")) : WiFi.SSID();
    case SYSBUILD_DATE:     return get_build_date();
    case SYSBUILD_TIME:     return get_build_time();
    case SYSDAY:            intvalue = node_time.day(); break;
    case SYSDAY_0:          return timeReplacement_leadZero(node_time.day());
    case SYSHEAP:           intvalue = ESP.getFreeHeap(); break;
    case SYSHOUR:           intvalue = node_time.hour(); break;
    case SYSHOUR_0:         return timeReplacement_leadZero(node_time.hour());
    case SYSLOAD:           return String(getCPUload(), 2);
    case SYSMIN:            intvalue = node_time.minute(); break;
    case SYSMIN_0:          return timeReplacement_leadZero(node_time.minute());
    case SYSMONTH:          intvalue = node_time.month(); break;
    case SYSMONTH_S:        return node_time.month_str();
    case SYSNAME:           return Settings.getHostname();
    case SYSSEC:            intvalue = node_time.second(); break;
    case SYSSEC_0:          return timeReplacement_leadZero(node_time.second());
    case SYSSEC_D:          intvalue = ((node_time.hour() * 60) + node_time.minute()) * 60 + node_time.second(); break;
    case SYSTIME:           return node_time.getTimeString(':');
    case SYSTIME_AM:        return node_time.getTimeString_ampm(':');
    case SYSTIME_AM_0:      return node_time.getTimeString_ampm(':', true, '0');
    case SYSTIME_AM_SP:     return node_time.getTimeString_ampm(':', true, ' ');
    case SYSTM_HM:          return node_time.getTimeString(':', false);
    case SYSTM_HM_0:        return node_time.getTimeString(':', false, '0');
    case SYSTM_HM_SP:       return node_time.getTimeString(':', false, ' ');
    case SYSTM_HM_AM:       return node_time.getTimeString_ampm(':', false);
    case SYSTM_HM_AM_0:     return node_time.getTimeString_ampm(':', false, '0');
    case SYSTM_HM_AM_SP:    return node_time.getTimeString_ampm(':', false, ' ');
    case SYSTZOFFSET:       return node_time.getTimeZoneOffsetString();
    case SYSWEEKDAY:        intvalue = node_time.weekday(); break;
    case SYSWEEKDAY_S:      return node_time.weekday_str();
    case SYSYEAR_0:
    case SYSYEAR:           intvalue = node_time.year(); break;
    case SYSYEARS:          return timeReplacement_leadZero(node_time.year() % 100);
    case SYS_MONTH_0:       return timeReplacement_leadZero(node_time.month());
    case S_CR:              return F("\\r");
    case S_LF:              return F("\\n");
    case UNIXDAY:           intvalue = node_time.getUnixTime() / 86400; break;
    case UNIXDAY_SEC:       intvalue = node_time.getUnixTime() % 86400; break;
    case UNIXTIME:          return String(node_time.getUnixTime());
    case UPTIME:            intvalue = getUptimeMinutes(); break;
    case UPTIME_MS:         return ull2String(getMicros64() / 1000);
    #if FEATURE_ADC_VCC
    case VCC:               return String(vcc);
    #else // if FEATURE_ADC_VCC
    case VCC:               intvalue = -1; break;
    #endif // if FEATURE_ADC_VCC
    case WI_CH:             intvalue = (WiFiEventData.WiFiDisconnected()) ? 0 : WiFi.channel(); break;

    default:
      // Already handled above.
      return EMPTY_STRING;
  }

  if (intvalue != INT_NOT_SET) {
    return String(intvalue);
  }

  return EMPTY_STRING;
}

/*
#define SMART_REPL_T(T, S) \
  while (s.indexOf(T) != -1) { (S((T), s, useURLencode)); }
*/

#define SMART_REPL_T(T, S) \
  const String T_str(T); int __pos__ = s.indexOf(T_str); \
  while (__pos__ != -1) { (S((T_str), s, useURLencode)); __pos__ = s.indexOf(T_str, __pos__ + 1);}

// Parse %vN% to replace ESPEasy variables
bool parse_pct_v_num_pct(String& s, boolean useURLencode, int start_pos)
{
  const String key_prefix = F("%v");
  int v_index = s.indexOf(key_prefix, start_pos);

  bool somethingReplaced = false;

  while ((v_index != -1)) {
    // Exclude "%valname% or %value%"
    // FIXME TD-er: Must find a more elegant way to fix this
    if (!isalpha(s.charAt(v_index + 2))) {
      // Check for:
      // - Calculations indicated with leading '='
      // - nested indirections like %v%v1%%
      if ((s.charAt(v_index + 2) == '=') ||
          (s.charAt(v_index + 2) == '%' && s.charAt(v_index + 3) == 'v')) {
        // FIXME TD-er: This may lead to stack overflow if we do an awful lot of nested user variables
        if (parse_pct_v_num_pct(s, useURLencode, v_index + 2)) {
          somethingReplaced = true;
        }
      }

      uint32_t i{};
      // variable index may contain a calculation
      // Calculations are enforced by a leading '='
      // like: %v=1+%v2%%
      const int pos_closing_pct = s.indexOf('%', v_index + 1);
      const String arg = s.substring(v_index + 2, pos_closing_pct);
      i = CalculateParam(arg, -1);
      //addLog(LOG_LEVEL_INFO, strformat(F("calc parse: %s => %u"), arg.c_str(), i));
      if (i >= 0) {
        // Need to replace the entire arg and not just the 'i'
        const String key = strformat(F("%%v%s%%"), arg.c_str());

        if (s.indexOf(key) != -1) {
          const bool trimTrailingZeros = true;
          const ESPEASY_RULES_FLOAT_TYPE floatvalue = getCustomFloatVar(i);
          const unsigned char nr_decimals = maxNrDecimals_fpType(floatvalue);
          #if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
          const String value = doubleToString(floatvalue, nr_decimals, trimTrailingZeros);
          #else // if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
          const String value = floatToString(floatvalue, nr_decimals, trimTrailingZeros);
          #endif // if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
          if (repl(key, value, s, useURLencode)) {
            somethingReplaced = true;
          }
        }
      }
    }
    v_index = s.indexOf(key_prefix, v_index + 1); // Find next occurance
    //addLog(LOG_LEVEL_INFO, strformat(F("parse: %s"), s.c_str()));
  }
  return somethingReplaced;
}

void SystemVariables::parseSystemVariables(String& s, boolean useURLencode)
{
  START_TIMER

  if (s.indexOf('%') == -1) {
    STOP_TIMER(PARSE_SYSVAR_NOCHANGE);
    return;
  }

  bool somethingReplaced = false;

  // Parse ESPEasy user variables first as they might be combined 
  // as arument or index for other variables
  parse_pct_v_num_pct(s, useURLencode, 0);

  do {
    int last_percent_pos = -1;
    somethingReplaced = false;
    SystemVariables::Enum enumval = static_cast<SystemVariables::Enum>(0);
    do {
      enumval = SystemVariables::nextReplacementEnum(s, enumval, last_percent_pos);

      switch (enumval)
      {
        case SUNRISE: {
          SMART_REPL_T(SystemVariables::toString(enumval), replSunRiseTimeString);
          somethingReplaced = true;
          break;
        }
        case SUNSET: {
          SMART_REPL_T(SystemVariables::toString(enumval), replSunSetTimeString);
          somethingReplaced = true;
          break;
        }
        case VARIABLE:
        {
          // Should not be present anymore, but just in case...
          if (parse_pct_v_num_pct(s, useURLencode, 0))
            somethingReplaced = true;
    
          break;
        }
        case UNKNOWN:

          // Do not replace
          break;
        default:
        {
          const String sysvar_str(SystemVariables::toString(enumval));
          if (s.indexOf(sysvar_str) != -1) {
            if (repl(
              sysvar_str, 
              getSystemVariable(enumval), 
              s, 
              useURLencode))
              somethingReplaced = true;
          }
          break;
        }
      }
    }
    while (enumval != SystemVariables::Enum::UNKNOWN);
  }
  while (somethingReplaced);

  STOP_TIMER(PARSE_SYSVAR);
}

#undef SMART_REPL_T


SystemVariables::Enum SystemVariables::nextReplacementEnum(const String& str, SystemVariables::Enum last_tested, int& last_percent_pos)
{
  SystemVariables::Enum nextTested;
  int percent_pos = last_percent_pos;

  do {
    // Find first position in string which might be a good candidate to look for a system variable.
    // Look for "%N" where 'N' is the first letter of a variable name we support.
    percent_pos = str.indexOf('%', percent_pos + 1);

    if (percent_pos == -1) {
      return Enum::UNKNOWN;
    }

    nextTested = SystemVariables::startIndex_beginWith(str[percent_pos + 1]);
  } while (Enum::UNKNOWN == nextTested);

  if (last_percent_pos < percent_pos) {
    last_percent_pos = percent_pos;
    last_tested      = nextTested;
  }

  if (last_tested > nextTested) {
    // Iterate over the possible system variables
    nextTested = static_cast<SystemVariables::Enum>(last_tested + 1);
    const char firstChar_nextTested = static_cast<char>(pgm_read_byte(SystemVariables::toFlashString(nextTested)));
    const char firstChar_expected = str[percent_pos + 1];

    if (firstChar_nextTested != firstChar_expected) {
      nextTested = Enum::UNKNOWN;
    }
  }

  if (nextTested >= Enum::UNKNOWN) {
    // We have tested all possible system variables
    // Skip unsupported ones or maybe it is just a single percentage symbol in a string.
    percent_pos = str.indexOf('%', percent_pos + 1);

    if (percent_pos == -1) {
      return Enum::UNKNOWN;
    }
    last_percent_pos = percent_pos;
    return SystemVariables::startIndex_beginWith(str[percent_pos + 1]);
  }

  const __FlashStringHelper *fstr_sysvar = SystemVariables::toFlashString(nextTested);
  String str_prefix        = strformat(F("%%%c"), static_cast<char>(pgm_read_byte(fstr_sysvar)));
  bool   str_prefix_exists = str.indexOf(str_prefix) != -1;

  for (int i = nextTested; i < Enum::UNKNOWN; ++i) {
    SystemVariables::Enum enumval = static_cast<SystemVariables::Enum>(i);
    fstr_sysvar = SystemVariables::toFlashString(enumval);
    const String new_str_prefix = strformat(F("%%%c"), static_cast<char>(pgm_read_byte(fstr_sysvar)));

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

String SystemVariables::toString(Enum enumval)
{
  if ((enumval == Enum::SUNRISE) || (enumval == Enum::SUNSET) || enumval == Enum::VARIABLE) {
    // These need variables, so only prepend a %, not wrap.
    return String('%') + SystemVariables::toFlashString(enumval);
  }

  return wrap_String(SystemVariables::toFlashString(enumval), '%');
}

SystemVariables::Enum SystemVariables::startIndex_beginWith(char beginchar)
{
  switch (tolower(beginchar))
  {
    case 'b': return Enum::BOARD_NAME;
    case 'c': return Enum::CLIENTIP;
    case 'd': return Enum::DNS;
#if FEATURE_ETHERNET
    case 'e': return Enum::ETHCONNECTED;
#endif // if FEATURE_ETHERNET
    case 'f': return Enum::FLASH_CHIP_MODEL;
    case 'g': return Enum::GATEWAY;
#if FEATURE_INTERNAL_TEMPERATURE
    case 'i': return Enum::INTERNAL_TEMPERATURE;
#else // if FEATURE_INTERNAL_TEMPERATURE
    case 'i': return Enum::IP4;
#endif // if FEATURE_INTERNAL_TEMPERATURE
    case 'l': return Enum::LCLTIME;
    case 'm': return Enum::SUNRISE_M;
    case 'n': return Enum::S_LF;
    case 'r': return Enum::S_CR;
    case 's': return Enum::SPACE;
    case 'u': return Enum::UNIT_sysvar;
    // case 'v': return Enum::VARIABLE; // Can not be the first 'v' variable, as the name is only 1 character long
    case 'v': return Enum::VCC;
    case 'w': return Enum::WI_CH;
  }

  return Enum::UNKNOWN;
}

const __FlashStringHelper * SystemVariables::toFlashString(SystemVariables::Enum enumval)
{
  switch (enumval) {
    case Enum::BOARD_NAME:         return F("board_name");
    case Enum::BOOT_CAUSE:         return F("bootcause");
    case Enum::BSSID:              return F("bssid");
    case Enum::CLIENTIP:           return F("clientip");
    case Enum::CR:                 return F("CR");
    case Enum::ESP_CHIP_CORES:     return F("cpu_cores");
    case Enum::ESP_CHIP_FREQ:      return F("cpu_freq");
    case Enum::ESP_CHIP_ID:        return F("cpu_id");
    case Enum::ESP_CHIP_MODEL:     return F("cpu_model");
    case Enum::ESP_CHIP_REVISION:  return F("cpu_rev");
    case Enum::DNS:                return F("dns");
    case Enum::DNS_1:              return F("dns1");
    case Enum::DNS_2:              return F("dns2");
#if FEATURE_ETHERNET
    case Enum::ETHCONNECTED:       return F("ethconnected");
    case Enum::ETHDUPLEX:          return F("ethduplex");
    case Enum::ETHSPEED:           return F("ethspeed");
    case Enum::ETHSPEEDSTATE:      return F("ethspeedstate");
    case Enum::ETHSTATE:           return F("ethstate");
    case Enum::ETHWIFIMODE:        return F("ethwifimode");
#endif // if FEATURE_ETHERNET

    case Enum::FLASH_CHIP_MODEL:   return F("flash_chip_model");
    case Enum::FLASH_CHIP_VENDOR:  return F("flash_chip_vendor");
    case Enum::FLASH_FREQ:         return F("flash_freq");
    case Enum::FLASH_SIZE:         return F("flash_size");
    case Enum::FS_FREE:            return F("fs_free");
    case Enum::FS_SIZE:            return F("fs_size");
    case Enum::GATEWAY:            return F("gateway");
#if FEATURE_INTERNAL_TEMPERATURE
    case Enum::INTERNAL_TEMPERATURE: return F("inttemp");
#endif // if FEATURE_INTERNAL_TEMPERATURE

    case Enum::IP4:                return F("ip4");
    case Enum::IP:                 return F("ip");
#if FEATURE_USE_IPV6
    case Enum::IP6_LOCAL:          return F("ipv6local");
#endif
    case Enum::ISVAR_DOUBLE:       return F("isvar_double");
    case Enum::ISLIMITED_BUILD:    return F("islimited_build");
    case Enum::ISMQTT:             return F("ismqtt");
    case Enum::ISMQTTIMP:          return F("ismqttimp");
    case Enum::ISNTP:              return F("isntp");
    case Enum::ISWIFI:             return F("iswifi");
    case Enum::LCLTIME:            return F("lcltime");
    case Enum::LCLTIME_AM:         return F("lcltime_am");
    case Enum::LF:                 return F("LF");
    case Enum::SUNRISE_M:          return F("m_sunrise");
    case Enum::SUNSET_M:           return F("m_sunset");
    case Enum::MAC:                return F("mac");
    case Enum::MAC_INT:            return F("mac_int");
    case Enum::S_LF:               return F("N");
    case Enum::S_CR:               return F("R");
    case Enum::RSSI:               return F("rssi");
    case Enum::SPACE:              return F("SP");
    case Enum::SSID:               return F("ssid");
    case Enum::SUBNET:             return F("subnet");
    case Enum::SUNRISE:            return F("sunrise");
    case Enum::SUNRISE_S:          return F("s_sunrise");
    case Enum::SUNSET:             return F("sunset");
    case Enum::SUNSET_S:           return F("s_sunset");
    case Enum::SYSBUILD_DATE:      return F("sysbuild_date");
    case Enum::SYSBUILD_DESCR:     return F("sysbuild_desc");
    case Enum::SYSBUILD_FILENAME:  return F("sysbuild_filename");
    case Enum::SYSBUILD_GIT:       return F("sysbuild_git");
    case Enum::SYSBUILD_TIME:      return F("sysbuild_time");
    case Enum::SYSDAY:             return F("sysday");
    case Enum::SYSDAY_0:           return F("sysday_0");
    case Enum::SYSHEAP:            return F("sysheap");
    case Enum::SYSHOUR:            return F("syshour");
    case Enum::SYSHOUR_0:          return F("syshour_0");
    case Enum::SYSLOAD:            return F("sysload");
    case Enum::SYSMIN:             return F("sysmin");
    case Enum::SYSMIN_0:           return F("sysmin_0");
    case Enum::SYSMONTH:           return F("sysmonth");
    case Enum::SYSMONTH_S:         return F("sysmonth_s");
    case Enum::SYSNAME:            return F("sysname");
    case Enum::SYSSEC:             return F("syssec");
    case Enum::SYSSEC_0:           return F("syssec_0");
    case Enum::SYSSEC_D:           return F("syssec_d");
    case Enum::SYSSTACK:           return F("sysstack");
    case Enum::SYSTIME:            return F("systime");
    case Enum::SYSTIME_AM:         return F("systime_am");
    case Enum::SYSTIME_AM_0:       return F("systime_am_0");
    case Enum::SYSTIME_AM_SP:      return F("systime_am_sp");
    case Enum::SYSTM_HM:           return F("systm_hm");
    case Enum::SYSTM_HM_0:         return F("systm_hm_0");
    case Enum::SYSTM_HM_AM:        return F("systm_hm_am");
    case Enum::SYSTM_HM_AM_0:      return F("systm_hm_am_0");
    case Enum::SYSTM_HM_AM_SP:     return F("systm_hm_am_sp");
    case Enum::SYSTM_HM_SP:        return F("systm_hm_sp");
    case Enum::SYSTZOFFSET:        return F("systzoffset");
    case Enum::SYSWEEKDAY:         return F("sysweekday");
    case Enum::SYSWEEKDAY_S:       return F("sysweekday_s");
    case Enum::SYSYEAR:            return F("sysyear");
    case Enum::SYSYEARS:           return F("sysyears");
    case Enum::SYSYEAR_0:          return F("sysyear_0");
    case Enum::SYS_MONTH_0:        return F("sysmonth_0");
    case Enum::UNIT_sysvar:        return F("unit");
#if FEATURE_ZEROFILLED_UNITNUMBER
    case Enum::UNIT_0_sysvar:      return F("unit_0");
#endif // FEATURE_ZEROFILLED_UNITNUMBER
    case Enum::UNIXDAY:            return F("unixday");
    case Enum::UNIXDAY_SEC:        return F("unixday_sec");
    case Enum::UNIXTIME:           return F("unixtime");
    case Enum::UPTIME:             return F("uptime");
    case Enum::UPTIME_MS:          return F("uptime_ms");
    case Enum::VCC:                return F("vcc");
    case Enum::VARIABLE:           return F("v"); // Can not be the first 'v' variable, as the name is only 1 character long
    case Enum::WI_CH:              return F("wi_ch");

    case Enum::UNKNOWN: break;
  }
  return F("Unknown");
}
