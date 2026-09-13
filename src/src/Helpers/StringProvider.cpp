#include "../Helpers/StringProvider.h"

#if FEATURE_ETHERNET
# include <ETH.h>
#endif // if FEATURE_ETHERNET

#include "../../ESPEasy-Globals.h"

#include "../CustomBuild/CompiletimeDefines.h"

#include "../../ESPEasy/net/ESPEasyNetwork.h"
#include "../../ESPEasy/net/Helpers/NWAccessControl.h"
#include "../../ESPEasy/net/wifi/ESPEasyWifi.h"

#if FEATURE_ETHERNET
# include "../../ESPEasy/net/eth/ESPEasyEth.h"
#endif

#include "../Globals/Device.h"
#include "../Globals/ESPEasy_Console.h"
#include "../Globals/ESPEasy_time.h"

#include "../../ESPEasy/net/Globals/NetworkState.h"
#include "../Globals/RTC.h"
#include "../Globals/SecuritySettings.h"
#include "../Globals/Settings.h"

#include "../Helpers/Convert.h"
#include "../Helpers/ESPEasy_Storage.h"
# if FEATURE_TASKVALUE_UNIT_OF_MEASURE
#include "../Helpers/ESPEasy_UnitOfMeasure.h"
#endif
#include "../Helpers/Hardware_device_info.h"
#if FEATURE_INTERNAL_TEMPERATURE
#include "../Helpers/Hardware_temperature_sensor.h"
#endif
#include "../Helpers/Memory.h"
#include "../Helpers/Misc.h"
#include "../Helpers/Networking.h"
#include "../Helpers/OTA.h"
#include "../Helpers/Scheduler.h"
#include "../Helpers/StringConverter.h"
#include "../Helpers/StringGenerator_System.h"
#include "../Helpers/StringGenerator_WiFi.h"

#include "../WebServer/common.h"

#ifdef ESP32
# include <soc/rtc.h>
#endif

#ifndef LIMIT_BUILD_SIZE
# define KV_SETUNIT(S) kv.setUnit(S)
# define KV_SETID(S) kv.setID(S)
#else // ifndef LIMIT_BUILD_SIZE
# define KV_SETUNIT(S)
# define KV_SETID(S)
#endif // ifndef LIMIT_BUILD_SIZE

KeyValueStruct make_kv(const __FlashStringHelper *key, int val)
{
  return KeyValueStruct(key, val, KeyValueStruct::Format::Default);
}

KeyValueStruct make_kv(const __FlashStringHelper *key, uint32_t val)
{
  return KeyValueStruct(key, val, KeyValueStruct::Format::Default);
}

KeyValueStruct make_kv(const __FlashStringHelper *key, uint64_t val)
{
  return KeyValueStruct(key, val, KeyValueStruct::Format::Default);
}

KeyValueStruct make_kv(const __FlashStringHelper *key, const float & val,
                 uint8_t       nrDecimals)
{
  KeyValueStruct kv(key, KeyValueStruct::Format::Default);
  kv.setValue(ValueStruct(val, nrDecimals));
  return kv;
}

KeyValueStruct make_kv(const __FlashStringHelper *key, const __FlashStringHelper *val)
{
  return KeyValueStruct(key, val, KeyValueStruct::Format::Default);
}

KeyValueStruct make_kv(const __FlashStringHelper *key, String &&val)
{
  return KeyValueStruct(key, val, KeyValueStruct::Format::Default);
}

KeyValueStruct make_kv(const __FlashStringHelper *key, String &&val, KeyValueStruct::Format format)
{
  return KeyValueStruct(key, val, format);
}

KeyValueStruct make_kv(const __FlashStringHelper *key, const IPAddress& val, bool includeZone = false)
{
  ValueStruct v;
  v.setIPAddress(val, includeZone);
  KeyValueStruct kv(key, KeyValueStruct::Format::PreFormatted);
  kv.setValue(std::move(v));
  return kv;
}


KeyValueStruct getKeyValue(LabelType::Enum label, bool extendedValue)
{
  switch (label)
  {
    case LabelType::UNIT_NR:
    {
      return make_kv(F("Unit Number"), Settings.Unit);
    }
    #if FEATURE_ZEROFILLED_UNITNUMBER
    case LabelType::UNIT_NR_0:
    {
      // Fixed 3-digit unitnumber
      return make_kv(F("Unit Number 0-filled"), formatIntLeadingZeroes(Settings.Unit, 3));
    }
    #endif // FEATURE_ZEROFILLED_UNITNUMBER
    case LabelType::UNIT_NAME:
    {
      // Only return the set name, no appended unit.
      return make_kv(F("Unit Name"), Settings.getName());
    }
    case LabelType::HOST_NAME:
    {
      return make_kv(F("Hostname"), ESPEasy::net::NetworkGetHostname());
    }

    case LabelType::LOCAL_TIME:

      if (node_time.systemTimePresent())
      {
        return make_kv(F("Local Time"), node_time.getDateTimeString('-', ':', ' '));
      } else if (extendedValue) {
        return make_kv(F("Local Time"), F("<font color='red'>No system time source</font>"));
      }
      break;
    case LabelType::TIME_SOURCE:

      if (node_time.systemTimePresent())
      {
        String timeSource_str = toString(node_time.getTimeSource());

        if (((node_time.getTimeSource() == timeSource_t::ESPEASY_p2p_UDP) ||
             (node_time.getTimeSource() == timeSource_t::ESP_now_peer)) &&
            (node_time.timeSource_p2p_unit != 0))
        {
          timeSource_str = strformat(F("%s (%u)"), timeSource_str.c_str(), node_time.timeSource_p2p_unit);
        }

        return make_kv(F("Time Source"), std::move(timeSource_str));
      }
      break;
    case LabelType::TIME_WANDER:

      if (node_time.systemTimePresent())
      {
        KeyValueStruct kv = make_kv(F("Time Wander"), node_time.timeWander, 3);
#if FEATURE_TASKVALUE_UNIT_OF_MEASURE
        KV_SETUNIT(UOM_ppm);
#endif
        return kv;
      }
      break;
    #if FEATURE_EXT_RTC
    case LabelType::EXT_RTC_UTC_TIME:
    {
      if (Settings.ExtTimeSource() == ExtTimeSource_e::None) { break; }
      String rtcTime = F("Not Set");

      // Try to read the stored time in the ext. time source to allow to check if it is working properly.
      uint32_t unixtime;

      if (node_time.ExtRTC_get(unixtime)) {
        struct tm RTC_time;
        breakTime(unixtime, RTC_time);
        rtcTime = formatDateTimeString(RTC_time);
      }
      return make_kv(F("UTC time stored in RTC chip"), std::move(rtcTime));
    }
    #endif // if FEATURE_EXT_RTC
    case LabelType::UPTIME:
    {
      if (extendedValue) {
        return make_kv(F("Uptime"), minutesToDayHourMinute(getUptimeMinutes()));

      } else {
        return make_kv(F("Uptime"), getUptimeMinutes());

      }
    }
    case LabelType::LOAD_PCT:

      if (wdcounter > 0)
      {
        if (extendedValue) {
          return make_kv(F("Load"), strformat(
                                  F("%.2f [%%] (LC=%d)"),
                                  getCPUload(),
                                  getLoopCountPerSec()));
        }
        KeyValueStruct kv = make_kv(F("Load"), getCPUload(), 2);
#if FEATURE_TASKVALUE_UNIT_OF_MEASURE
        KV_SETUNIT(UOM_percent);
#endif
        return kv;
      }
      break;
    case LabelType::LOOP_COUNT:
    {
      if (extendedValue) { break; }
      return make_kv(F("Load LC"), getLoopCountPerSec());
    }
    case LabelType::CPU_ECO_MODE:
    {
      return make_kv(F("CPU Eco Mode"), Settings.EcoPowerMode());
    }

#if FEATURE_SET_WIFI_TX_PWR
    case LabelType::WIFI_TX_MAX_PWR:
    {
      KeyValueStruct kv = make_kv(F("Max WiFi TX Power"), Settings.getWiFi_TX_power(), 2);
# if FEATURE_TASKVALUE_UNIT_OF_MEASURE
      KV_SETUNIT(UOM_dBm);
# endif
      return kv;
    }
    case LabelType::WIFI_CUR_TX_PWR:
    if (ESPEasy::net::wifi::WiFiConnected())
    {
      KeyValueStruct kv = make_kv(F("Current WiFi TX Power"), ESPEasy::net::wifi::GetWiFiTXpower(), 2);
# if FEATURE_TASKVALUE_UNIT_OF_MEASURE
      KV_SETUNIT(UOM_dBm);
# endif
      return kv;
    }
    break;
    case LabelType::WIFI_SENS_MARGIN:
    {
      KeyValueStruct kv = make_kv(F("WiFi Sensitivity Margin"), Settings.WiFi_sensitivity_margin);
# if FEATURE_TASKVALUE_UNIT_OF_MEASURE
      KV_SETUNIT(UOM_dB);
# endif
      return kv;
    }
    case LabelType::WIFI_SEND_AT_MAX_TX_PWR:
    {
      return make_kv(F("Send With Max TX Power"), Settings.UseMaxTXpowerForSending());
    }
#endif // if FEATURE_SET_WIFI_TX_PWR
    case LabelType::WIFI_AP_CHANNEL:
    {
      return make_kv(F("Wifi AP channel"), Settings.WiFiAP_channel);
    }
    case LabelType::WIFI_ENABLE_CAPTIVE_PORTAL:
    {
      return make_kv(F("Force /setup in AP-Mode"), Settings.ApCaptivePortal());
    }
    case LabelType::WIFI_START_AP_NO_CREDENTIALS:
    {
      return make_kv(F("Start AP on No Credentials"), Settings.StartAPfallback_NoCredentials());
    }
    case LabelType::WIFI_START_AP_ON_CONNECT_FAIL:
    {
      return make_kv(F("Start AP on Connect Fail"), !Settings.DoNotStartAPfallback_ConnectFail());
    }
    case LabelType::WIFI_NR_RECONNECT_ATTEMPTS:
    {
      return make_kv(F("Connect Retry Attempts"), Settings.ConnectFailRetryCount);
    }
    case LabelType::WIFI_MAX_UPTIME_AUTO_START_AP:
    {
      KeyValueStruct kv = make_kv(F("Max. Uptime to Start AP"), Settings.APfallback_autostart_max_uptime_m());
# if FEATURE_TASKVALUE_UNIT_OF_MEASURE
      KV_SETUNIT(UOM_min);
# endif
      return kv;
    }
    case LabelType::WIFI_AP_MINIMAL_ON_TIME:
    {
      KeyValueStruct kv = make_kv(F("AP Minimal 'on' Time"), Settings.APfallback_minimal_on_time_sec());
# if FEATURE_TASKVALUE_UNIT_OF_MEASURE
      KV_SETUNIT(UOM_sec);
# endif
      return kv;
    }
#ifdef ESP32
    case LabelType::WIFI_AP_ENABLE_NAPT:
    {
      return make_kv(F("Enable NAPT"), Settings.WiFi_AP_enable_NAPT());
    }
#endif

    case LabelType::WIFI_USE_LAST_CONN_FROM_RTC:
    {
      return make_kv(F("Use Last Connected AP from RTC"), Settings.UseLastWiFiFromRTC());
    }

    case LabelType::FREE_MEM:
    {
#ifndef BUILD_NO_RAM_TRACKER

      if (extendedValue) {
        return make_kv(F("Free RAM"),
                              strformat(
                                F("%d [B] (%d - %s)"),
                                FreeMem(),
                                lowestRAM,
                                lowestRAMfunction.c_str()));
      }
#endif // ifndef BUILD_NO_RAM_TRACKER

      KeyValueStruct kv = make_kv(F("Free RAM"), FreeMem());
#if FEATURE_TASKVALUE_UNIT_OF_MEASURE
      KV_SETUNIT(UOM_Byte);
#endif
      return kv;
    }
    case LabelType::FREE_STACK:
    {
#ifndef BUILD_NO_RAM_TRACKER

      if (extendedValue) {
        return make_kv(F("Free Stack"),
                              strformat(
                                F("%d [B] (%d - %s)"),
                                getCurrentFreeStack(),
                                lowestFreeStack,
                                lowestFreeStackfunction.c_str()));
      }
#endif // ifndef BUILD_NO_RAM_TRACKER
      KeyValueStruct kv = make_kv(F("Free Stack"), getCurrentFreeStack());
#if FEATURE_TASKVALUE_UNIT_OF_MEASURE
      KV_SETUNIT(UOM_Byte);
#endif

      return kv;
    }
#ifdef USE_SECOND_HEAP
    case LabelType::FREE_HEAP_IRAM:
    {
      KeyValueStruct kv = make_kv(F("Free 2nd Heap"), FreeMem2ndHeap());
# if FEATURE_TASKVALUE_UNIT_OF_MEASURE
      KV_SETUNIT(UOM_Byte);
# endif
      return kv;
    }
#endif // ifdef USE_SECOND_HEAP

#if defined(CORE_POST_2_5_0) || defined(ESP32)
  # ifndef LIMIT_BUILD_SIZE
    case LabelType::HEAP_MAX_FREE_BLOCK:
    {
      KeyValueStruct kv = make_kv(F("Heap Max Free Block"),
#  ifdef ESP32
                        ESP.getMaxAllocHeap()
#  else
                        ESP.getMaxFreeBlockSize()
#  endif // ifdef ESP32
                        );
#  if FEATURE_TASKVALUE_UNIT_OF_MEASURE
      KV_SETUNIT(UOM_Byte);
#  endif
      return kv;
    }
  # endif // ifndef LIMIT_BUILD_SIZE
#endif // if defined(CORE_POST_2_5_0) || defined(ESP32)
#if defined(CORE_POST_2_5_0)
  # ifndef LIMIT_BUILD_SIZE
    case LabelType::HEAP_FRAGMENTATION:
    {
      KeyValueStruct kv = make_kv(F("Heap Fragmentation"), ESP.getHeapFragmentation());
#  if FEATURE_TASKVALUE_UNIT_OF_MEASURE
      KV_SETUNIT(UOM_percent);
#  endif
      return kv;
    }
  # endif // ifndef LIMIT_BUILD_SIZE
#endif // if defined(CORE_POST_2_5_0)

#ifdef ESP32
    case LabelType::HEAP_SIZE:
    {
      KeyValueStruct kv = make_kv(F("Heap Size"), ESP.getHeapSize());
# if FEATURE_TASKVALUE_UNIT_OF_MEASURE
      KV_SETUNIT(UOM_Byte);
# endif
      return kv;
    }
    case LabelType::HEAP_MIN_FREE:
    {
      KeyValueStruct kv = make_kv(F("Heap Min Free"), ESP.getMinFreeHeap());
# if FEATURE_TASKVALUE_UNIT_OF_MEASURE
      KV_SETUNIT(UOM_Byte);
# endif
      return kv;
    }
    # ifdef BOARD_HAS_PSRAM
    case LabelType::PSRAM_SIZE:
    {
      if (!UsePSRAM()) { break; }
      KeyValueStruct kv = make_kv(F("PSRAM Size"), ESP.getPsramSize());
#  if FEATURE_TASKVALUE_UNIT_OF_MEASURE
      KV_SETUNIT(UOM_Byte);
#  endif
      return kv;
    }
    case LabelType::PSRAM_FREE:
    {
      if (!UsePSRAM()) { break; }
      KeyValueStruct kv = make_kv(F("PSRAM Free"), ESP.getFreePsram());
#  if FEATURE_TASKVALUE_UNIT_OF_MEASURE
      KV_SETUNIT(UOM_Byte);
#  endif
      return kv;
    }
    case LabelType::PSRAM_MIN_FREE:
    {
      if (!UsePSRAM()) { break; }
      KeyValueStruct kv = make_kv(F("PSRAM Min Free"), ESP.getMinFreePsram());
#  if FEATURE_TASKVALUE_UNIT_OF_MEASURE
      KV_SETUNIT(UOM_Byte);
#  endif
      return kv;
    }
    case LabelType::PSRAM_MAX_FREE_BLOCK:
    {
      if (!UsePSRAM()) { break; }
      KeyValueStruct kv = make_kv(F("PSRAM Max Free Block"), ESP.getMaxAllocPsram());
#  if FEATURE_TASKVALUE_UNIT_OF_MEASURE
      KV_SETUNIT(UOM_Byte);
#  endif
      return kv;
    }
    # endif // BOARD_HAS_PSRAM
#endif // ifdef ESP32

    case LabelType::JSON_BOOL_QUOTES:
    {
      return make_kv(F("JSON bool output without quotes"), Settings.JSONBoolWithoutQuotes());
    }
#if FEATURE_TIMING_STATS
    case LabelType::ENABLE_TIMING_STATISTICS:
    {
      return make_kv(F("Collect Timing Statistics"), Settings.EnableTimingStats());
    }
#endif // if FEATURE_TIMING_STATS
    case LabelType::ENABLE_RULES_CACHING:
    {
      return make_kv(F("Enable Rules Cache"), Settings.EnableRulesCaching());
    }
    case LabelType::ENABLE_SERIAL_PORT_CONSOLE:
    {
      return make_kv(F("Enable Serial Port Console"), !!Settings.UseSerial); // Cast to bool to make sure it is shown as a checkmark
    }
    case LabelType::CONSOLE_SERIAL_PORT:
    {
      return make_kv(F("Console Serial Port"), ESPEasy_Console.getPortDescription());
    }
#if USES_ESPEASY_CONSOLE_FALLBACK_PORT
    case LabelType::CONSOLE_FALLBACK_TO_SERIAL0:
    {
      return make_kv(F("Fallback to Serial 0"), !!Settings.console_serial0_fallback); // Cast to bool to make sure it is shown as a checkmark
    }
    case LabelType::CONSOLE_FALLBACK_PORT:
    {
      return make_kv(F("Console Fallback Port"), ESPEasy_Console.getFallbackPortDescription());
    }
#endif // if USES_ESPEASY_CONSOLE_FALLBACK_PORT

    //    case LabelType::ENABLE_RULES_EVENT_REORDER: {
    // return KeyValueStruct( F("Optimize Rules Cache Event Order"), Settings.EnableRulesEventReorder() );
    // } // TD-er: Disabled for now
    case LabelType::TASKVALUESET_ALL_PLUGINS:
    {
      return make_kv(F("Allow TaskValueSet on all plugins"), Settings.AllowTaskValueSetAllPlugins());
    }
    case LabelType::ALLOW_OTA_UNLIMITED:
    {
      return make_kv(F("Allow OTA without size-check"), Settings.AllowOTAUnlimited());
    }
#if FEATURE_CLEAR_I2C_STUCK
    case LabelType::ENABLE_CLEAR_HUNG_I2C_BUS:
    {
      return make_kv(F("Try clear I2C bus when stuck"), Settings.EnableClearHangingI2Cbus());
    }
#endif // if FEATURE_CLEAR_I2C_STUCK
    #if FEATURE_I2C_DEVICE_CHECK
    case LabelType::ENABLE_I2C_DEVICE_CHECK:
    {
      return make_kv(F("Check I2C devices when enabled"), Settings.CheckI2Cdevice());
    }
    #endif // if FEATURE_I2C_DEVICE_CHECK
#ifndef BUILD_NO_RAM_TRACKER
    case LabelType::ENABLE_RAM_TRACKING:
    {
      return make_kv(F("Enable RAM Tracker"), Settings.EnableRAMTracking());
    }
#endif // ifndef BUILD_NO_RAM_TRACKER
#if FEATURE_AUTO_DARK_MODE
    case LabelType::ENABLE_AUTO_DARK_MODE:
    {
      return make_kv(F("Web light/dark mode"), Settings.getCssMode());
    }
#endif // FEATURE_AUTO_DARK_MODE
#if FEATURE_RULES_EASY_COLOR_CODE
    case LabelType::DISABLE_RULES_AUTOCOMPLETE:
    {
      return make_kv(F("Disable Rules auto-completion"), Settings.DisableRulesCodeCompletion());
    }
#endif // if FEATURE_RULES_EASY_COLOR_CODE
#if FEATURE_TARSTREAM_SUPPORT
    case LabelType::DISABLE_SAVE_CONFIG_AS_TAR:
    {
      return make_kv(F("Disable Save Config as .tar"), Settings.DisableSaveConfigAsTar());
    }
#endif // if FEATURE_TARSTREAM_SUPPORT
#if FEATURE_TASKVALUE_UNIT_OF_MEASURE
    case LabelType::SHOW_UOM_ON_DEVICES_PAGE:
    {
      return make_kv(F("Show Unit of Measure"), Settings.ShowUnitOfMeasureOnDevicesPage());
    }
#endif // if FEATURE_TASKVALUE_UNIT_OF_MEASURE
#if FEATURE_MQTT_CONNECT_BACKGROUND
    case LabelType::MQTT_CONNECT_IN_BACKGROUND:
    {
      return make_kv(F("MQTT Connect in background"), Settings.MQTTConnectInBackground());
    }
#endif // if FEATURE_MQTT_CONNECT_BACKGROUND
    #if FEATURE_MQTT_DISCOVER
    case LabelType::MQTT_DISCOVER_GROUP_INCL_TASKNAME:
    {
      return make_kv(F("MQTT Discover, Group incl. Taskname"), Settings.MQTTDiscoverGroupInclTaskname());
    }
    #endif // if FEATURE_MQTT_DISCOVER
#if FEATURE_COLORIZE_CONSOLE_LOGS
    case LabelType::COLORIZE_CONSOLE_LOGS:
    {
      return make_kv(F("Colorize Console Logs"), Settings.ColorizeSerialLog());
    }
#endif


#if CONFIG_SOC_WIFI_SUPPORT_5G
    case LabelType::WIFI_BAND_MODE:
    {
      return make_kv(F("WiFi Band Mode"), ESPEasy::net::wifi::getWifiBandModeString(Settings.WiFi_band_mode()));
    }
#endif // if CONFIG_SOC_WIFI_SUPPORT_5G

    case LabelType::BOOT_TYPE:
    {
      if (extendedValue) {
        return make_kv(
          F("Boot"),
          concat(
            getLastBootCauseString(),
            strformat(F(" (%d)"), RTC.bootCounter)));
      }
      return make_kv(F("Last Boot Cause"), getLastBootCauseString());
    }
    case LabelType::BOOT_COUNT:
    {
      return make_kv(F("Boot Count"), RTC.bootCounter);
    }
    case LabelType::DEEP_SLEEP_ALTERNATIVE_CALL:
    {
      return make_kv(F("Deep Sleep Alternative"), Settings.UseAlternativeDeepSleep());
    }
    case LabelType::RESET_REASON:
    {
      return make_kv(F("Reset Reason"), getResetReasonString());
    }
    case LabelType::LAST_TASK_BEFORE_REBOOT:
    {
      return make_kv(F("Last Action before Reboot"), ESPEasy_Scheduler::decodeSchedulerId(lastMixedSchedulerId_beforereboot));
    }
    case LabelType::SW_WD_COUNT:
    {
      return make_kv(F("SW WD count"), sw_watchdog_callback_count);
    }

    case LabelType::WIFI_CONNECTION:
    {
      return KeyValueStruct(F("WiFi Connection") /*, value*/);
    }
    case LabelType::WIFI_RSSI:

      if (ESPEasy::net::wifi::WiFiConnected())
      {
        if (extendedValue) {
          return make_kv(F("RSSI"), strformat(
                                  F("%d [dBm] (%s)"),
                                  WiFi.RSSI(),
                                  WiFi.SSID().c_str()));
        }
        KeyValueStruct kv = make_kv(F("RSSI"), WiFi.RSSI());
#if FEATURE_TASKVALUE_UNIT_OF_MEASURE
        if (!extendedValue) {
          KV_SETUNIT(UOM_dBm);
        }
#endif
        return kv;
      }
      break;
    case LabelType::IP_CONFIG:
    {
      KeyValueStruct kv = make_kv(F("IP Config"), useStaticIP() ? F("static") : F("DHCP"));
      KV_SETID(F("dhcp"));
      return kv;
    }
#if FEATURE_USE_IPV6
    case LabelType::IP6_LOCAL:

      if (Settings.EnableIPv6()) {
        auto ip = ESPEasy::net::NetworkLocalIP6();

        if (ip != IN6ADDR_ANY) {
          return make_kv(F("IPv6 link local"), ip, true);
        }
      }
      break;
    case LabelType::IP6_GLOBAL:

      if (Settings.EnableIPv6()) {
        auto ip = ESPEasy::net::NetworkGlobalIP6();

        if (ip != IN6ADDR_ANY) {
          return make_kv(F("IPv6 global"), ip);
        }
      }
      break;

      // case LabelType::IP6_ALL_ADDRESSES:      {
      // KeyValueStruct kv = make_kv(F("IPv6 all addresses"));
      // IP6Addresses_t addresses = NetworkAllIPv6();
      // for (auto it = addresses.begin(); it != addresses.end(); ++it)
      // {
      //   kv.appendValue(it->toString());
      // }
      // return kv;
      // }
#endif // if FEATURE_USE_IPV6
    case LabelType::IP_ADDRESS:
    {
      KeyValueStruct kv = make_kv(F("IP Address"), ESPEasy::net::NetworkLocalIP());
      KV_SETID(F("ip"));
      return kv;
    }
    case LabelType::IP_SUBNET:
    {
      KeyValueStruct kv = make_kv(F("IP Subnet"), ESPEasy::net::NetworkSubnetMask());
      KV_SETID(F("subnet"));
      return kv;
    }
    case LabelType::IP_ADDRESS_SUBNET:
    {
      KeyValueStruct kv(F("IP / Subnet"));
      kv.appendValue(getValue(LabelType::IP_ADDRESS));
      kv.appendValue(getValue(LabelType::IP_SUBNET));
      return kv;
    }
    case LabelType::GATEWAY:
    {
      KeyValueStruct kv = make_kv(F("Gateway"), ESPEasy::net::NetworkGatewayIP());
      KV_SETID(F("gw"));
      return kv;
    }
    case LabelType::CLIENT_IP:
    {
      return make_kv(F("Client IP"), web_server.client().remoteIP(), true);
    }
    #if FEATURE_MDNS
    case LabelType::M_DNS:
    {
      if (!Settings.Use_mDNS()) break;
      String hostname;
      #ifdef ESP32
      // Retrieve and print the actual (resolved) hostname - checking if there are any conflicts in the network
      char actualHostname[MDNS_NAME_BUF_LEN];  // MDNS_NAME_BUF_LEN is 64 from mdns.h
      esp_err_t err = mdns_hostname_get(actualHostname);
      if (err == ESP_OK) {
        hostname = String(actualHostname);
      }
      if (hostname.isEmpty()) {
        hostname = ESPEasy::net::NetworkGetHostname();
      }
      #endif
      #ifdef ESP8266
      hostname = ESPEasy::net::NetworkGetHostname();
      #endif
      
      String url = concat(hostname, F(".local"));
      url.toLowerCase();

      if (extendedValue) {
        return make_kv(F("mDNS"),
                              strformat(
                                F("<a href='http://%s'>%s</a>"),
                                url.c_str(),
                                url.c_str()));
      }
      return make_kv(F("mDNS"), std::move(url));
    }
    case LabelType::USE_MDNS:
    {
      return make_kv(F("Enable mDNS"), Settings.Use_mDNS());
    }    
    #endif // if FEATURE_MDNS
    case LabelType::DNS:
    {
      if (!extendedValue) { break; }
      KeyValueStruct kv(F("DNS"));
      kv.appendValue(getValue(LabelType::DNS_1));
      kv.appendValue(getValue(LabelType::DNS_2));
      #ifdef ESP32
      kv.appendValue(formatIP(ESPEasy::net::NetworkDnsIP(2)));
      #endif

      return kv;
    }
    case LabelType::DNS_1:
    case LabelType::DNS_2:
    {
      if (extendedValue) { break; }
      const bool dns1 = label == LabelType::DNS_1;
      KeyValueStruct kv = make_kv(dns1 ? F("DNS 1"):F("DNS 2") , ESPEasy::net::NetworkDnsIP(dns1 ? 0 : 1));
      KV_SETID(dns1 ? F("dns1") : F("dns2"));
      return kv;
    }
    case LabelType::ALLOWED_IP_RANGE:
    {
      KeyValueStruct kv = make_kv(F("Allowed IP Range"), ESPEasy::net::describeAllowedIPrange(), KeyValueStruct::Format::PreFormatted);
      KV_SETID(F("allowed_range"));
      return kv;
    }
    case LabelType::STA_MAC:
    {
      return make_kv(F("STA MAC"), ESPEasy::net::WifiSTAmacAddress().toString(), KeyValueStruct::Format::PreFormatted);
    }
    case LabelType::AP_MAC:
    {
      return make_kv(F("AP MAC"), ESPEasy::net::WifiSoftAPmacAddress().toString(), KeyValueStruct::Format::PreFormatted);
    }
    case LabelType::SSID:
    {
      return make_kv(F("SSID"), WiFi.SSID(), KeyValueStruct::Format::PreFormatted);
    }
    case LabelType::BSSID:
    {
      return make_kv(F("BSSID"), WiFi.BSSIDstr(), KeyValueStruct::Format::PreFormatted);
    }
    case LabelType::CHANNEL:
    {
      return make_kv(F("Channel"), (int)WiFi.channel());
    }
    case LabelType::ENCRYPTION_TYPE_STA:
    {
      KeyValueStruct kv = make_kv(F("Encryption Type"), getWiFi_encryptionType());
      KV_SETID(F("encryption"));
      return kv;
    }
    case LabelType::CONNECTED:
    {
      return make_kv(F("Connected"), format_msec_duration(ESPEasy::net::NetworkConnectDuration_ms()));
    }
    case LabelType::CONNECTED_MSEC:
    {
      return make_kv(F("Connected msec"), ESPEasy::net::NetworkConnectDuration_ms());
    }
    case LabelType::LAST_DISCONNECT_REASON:
    {
      return make_kv(F("Last Disconnect Reason"), getWiFi_disconnectReason());
    }
    case LabelType::LAST_DISC_REASON_STR:
    {
      return make_kv(F("Last Disconnect Reason str"), getWiFi_disconnectReason_str());
    }
    case LabelType::NUMBER_RECONNECTS:
    {
      return make_kv(F("Number Reconnects"), ESPEasy::net::NetworkConnectCount());
    }
    case LabelType::WIFI_STORED_SSID1:
    {
      return make_kv(F("Configured SSID1"), SecuritySettings.WifiSSID, KeyValueStruct::Format::PreFormatted);
    }
    case LabelType::WIFI_STORED_SSID2:
    {
      return make_kv(F("Configured SSID2"), SecuritySettings.WifiSSID2, KeyValueStruct::Format::PreFormatted);
    }


    case LabelType::FORCE_WIFI_BG:
    {
      return make_kv(F("Force WiFi B/G"), Settings.ForceWiFi_bg_mode());
    }
    case LabelType::RESTART_WIFI_LOST_CONN:
    {
      #ifndef BOARD_HAS_SDIO_ESP_HOSTED
      // Disable option for ESP-hosted WiFi as this always needs to restart when forcing disconnect.
      return make_kv(F("Restart WiFi Lost Conn"), Settings.WiFiRestart_connection_lost());
      #endif
    }
    case LabelType::FORCE_WIFI_NOSLEEP:
    {
      return make_kv(F("Force WiFi No Sleep"), Settings.WifiNoneSleep());
    }
    case LabelType::PERIODICAL_GRAT_ARP:
    {
      return make_kv(F("Periodical send Gratuitous ARP"), Settings.gratuitousARP());
    }
    case LabelType::CONNECTION_FAIL_THRESH:
    {
      return make_kv(F("Connection Failure Threshold"), Settings.ConnectionFailuresThreshold);
    }
#ifndef ESP32
    case LabelType::WAIT_WIFI_CONNECT:
    {
      return make_kv(F("Extra Wait WiFi Connect"), Settings.WaitWiFiConnect());
    }
#endif // ifndef ESP32
    case LabelType::CONNECT_HIDDEN_SSID:
    {
      return make_kv(F("Include Hidden SSID"), Settings.IncludeHiddenSSID());
    }
#ifdef ESP32
    case LabelType::WIFI_PASSIVE_SCAN:
    {
      return make_kv(F("Passive WiFi Scan"), Settings.PassiveWiFiScan());
    }
#endif // ifdef ESP32
    case LabelType::HIDDEN_SSID_SLOW_CONNECT:
    {
      return make_kv(F("Hidden SSID Slow Connect"), Settings.HiddenSSID_SlowConnectPerBSSID());
    }
    case LabelType::SDK_WIFI_AUTORECONNECT:
    {
      return make_kv(F("Enable SDK WiFi Auto Reconnect"), Settings.SDK_WiFi_autoreconnect());
    }
#if FEATURE_USE_IPV6
    case LabelType::ENABLE_IPV6:
    {
      return make_kv(F("Enable IPv6"), Settings.EnableIPv6());
    }
#endif // if FEATURE_USE_IPV6


    case LabelType::BUILD_DESC:
    {
      String descr = getSystemBuildString();

      if (extendedValue) {
        descr += ' ';
        descr += F(BUILD_NOTES);
      }
      return make_kv(F("Build"), std::move(descr), KeyValueStruct::Format::PreFormatted);
    }
    case LabelType::BUILD_ORIGIN:
    {
      return make_kv(F("Build Origin"), get_build_origin());
    }
    case LabelType::GIT_BUILD:
    {
      String res(F(BUILD_GIT));

      if (res.isEmpty()) { res = get_git_head(); }
      return make_kv(F("Git Build"), std::move(res), KeyValueStruct::Format::PreFormatted);
    }
    case LabelType::SYSTEM_LIBRARIES:
    {
      return make_kv(F("System Libraries"), getSystemLibraryString(), KeyValueStruct::Format::PreFormatted);
    }
#ifdef ESP32
    case LabelType::ESP_IDF_SDK_VERSION:
    {
      return make_kv(
        F("ESP-IDF Version"), 
        strformat(
          F("%d.%d.%d"),
          ESP_IDF_VERSION_MAJOR,
          ESP_IDF_VERSION_MINOR,
          ESP_IDF_VERSION_PATCH), 
        KeyValueStruct::Format::PreFormatted);
    }
#endif // ifdef ESP32
    case LabelType::PLUGIN_COUNT:
    {
      return make_kv(F("Plugin Count"), getDeviceCount() + 1);
    }
    case LabelType::PLUGIN_DESCRIPTION:
    {
      return make_kv(F("Plugin Description"), getPluginDescriptionString(), KeyValueStruct::Format::PreFormatted);
    }
    case LabelType::BUILD_TIME:
    {
      return make_kv(F("Build Time"), concat(get_build_date(), concat(' ', get_build_time())));
    }
    case LabelType::BINARY_FILENAME:
    {
      return make_kv(F("Binary Filename"), get_binary_filename(), KeyValueStruct::Format::PreFormatted);
    }
    case LabelType::BUILD_PLATFORM:
    {
      return make_kv(F("Build Platform"), get_build_platform(), KeyValueStruct::Format::PreFormatted);
    }
    case LabelType::GIT_HEAD:
    {
      return make_kv(F("Git HEAD"), get_git_head(), KeyValueStruct::Format::PreFormatted);
    }
    #ifdef CONFIGURATION_CODE
    case LabelType::CONFIGURATION_CODE_LBL:
    {
      return make_kv(F("Configuration code"), getConfigurationCode(), KeyValueStruct::Format::PreFormatted);
    }
    #endif // ifdef CONFIGURATION_CODE
#if FEATURE_CLEAR_I2C_STUCK
    case LabelType::I2C_BUS_STATE:
    {
      return make_kv(F("I2C Bus State"), toString(I2C_state));
    }
    case LabelType::I2C_BUS_CLEARED_COUNT:
    {
      return make_kv(F("I2C bus cleared count"), I2C_bus_cleared_count);
    }
#endif // if FEATURE_CLEAR_I2C_STUCK
#if FEATURE_SYSLOG
    case LabelType::SYSLOG_LOG_LEVEL:
    {
      return make_kv(F("Syslog Log Level"), getLogLevelDisplayString(Settings.SyslogLevel));
    }
#endif
    case LabelType::SERIAL_LOG_LEVEL:
    {
      return make_kv(F("Serial Log Level"), getLogLevelDisplayString(getSerialLogLevel()));
    }
# ifdef WEBSERVER_LOG
    case LabelType::WEB_LOG_LEVEL:
    {
      return make_kv(F("Web Log Level"), getLogLevelDisplayString(getWebLogLevel()));
    }
#endif
  #if FEATURE_SD
    case LabelType::SD_LOG_LEVEL:
    {
      return make_kv(F("SD Log Level"), getLogLevelDisplayString(Settings.SDLogLevel));
    }
  #endif // if FEATURE_SD

    case LabelType::ESP_CHIP_ID:
    {
      return KeyValueStruct::makeHexFormatted(F("ESP Chip ID"), getChipId(), 6);
    }
    case LabelType::ESP_CHIP_FREQ:
    {
      KeyValueStruct kv = make_kv(F("ESP Chip Frequency"), ESP.getCpuFreqMHz());
#if FEATURE_TASKVALUE_UNIT_OF_MEASURE
      KV_SETUNIT(UOM_MHz);
#endif
      return kv;
    }
#ifdef ESP32
    case LabelType::ESP_CHIP_XTAL_FREQ:
    {
      KeyValueStruct kv = make_kv(F("ESP Crystal Frequency"), getXtalFrequencyMHz());
# if FEATURE_TASKVALUE_UNIT_OF_MEASURE
      KV_SETUNIT(UOM_MHz);
# endif
      return kv;
    }
    case LabelType::ESP_CHIP_APB_FREQ:
    {
      KeyValueStruct kv = make_kv(F("ESP APB Frequency"), rtc_clk_apb_freq_get() / 1000000);
# if FEATURE_TASKVALUE_UNIT_OF_MEASURE
      KV_SETUNIT(UOM_MHz);
# endif
      return kv;
    }
#endif // ifdef ESP32
    case LabelType::ESP_CHIP_MODEL:
    {
      return make_kv(F("ESP Chip Model"), getChipModel(), KeyValueStruct::Format::PreFormatted);
    }
    case LabelType::ESP_CHIP_REVISION:
    {
      return make_kv(F("ESP Chip Revision"), getChipRevision());
    }
    case LabelType::ESP_CHIP_CORES:
    {
      return make_kv(F("ESP Chip Cores"), getChipCores());
    }

    case LabelType::BOARD_NAME:
    {
      return make_kv(F("ESP Board Name"), get_board_name());
    }

    case LabelType::FLASH_CHIP_ID:
    {
      auto flashChipId = getFlashChipId();

      if (flashChipId == 0) { break; }

      return KeyValueStruct::makeHexFormatted(F("Flash Chip ID"), flashChipId, 6);
    }
    case LabelType::FLASH_CHIP_VENDOR:
    {
      const uint32_t flashChipId = getFlashChipId();

      if (flashChipId == 0) { break; }
      String id = formatToHex(flashChipId & 0xFF, 2);

      if (extendedValue && flashChipVendorPuya()) {
        id += concat(F(" (PUYA"), puyaSupport() ? F(", supported") : F(HTML_SYMBOL_WARNING)) + ')';
      }

      return make_kv(F("Flash Chip Vendor"), std::move(id), KeyValueStruct::Format::PreFormatted);
    }
    case LabelType::FLASH_CHIP_MODEL:
    {
      #ifndef LIMIT_BUILD_SIZE
      const uint32_t flashChipId = getFlashChipId();
      const uint32_t flashDevice = (flashChipId & 0xFF00) | ((flashChipId >> 16) & 0xFF);

      String model(formatToHex(flashDevice, 4));
    # ifdef ESP32

      if (extendedValue && getChipFeatures().embeddedFlash) {
        model += F(" (Embedded)");
      }
    # endif // ifdef ESP32
      return make_kv(F("Flash Chip Model"), std::move(model), KeyValueStruct::Format::PreFormatted);
      #else // ifndef LIMIT_BUILD_SIZE
      return KeyValueStruct(F("Flash Chip Model"), getFlashChipId(), KeyValueStruct::Format::PreFormatted);
      #endif // ifndef LIMIT_BUILD_SIZE
    }
    case LabelType::FLASH_CHIP_REAL_SIZE:
    {
      KeyValueStruct kv = make_kv(F("Flash Chip Real Size"), getFlashRealSizeInBytes() >> 10);
#if FEATURE_TASKVALUE_UNIT_OF_MEASURE
      KV_SETUNIT(UOM_kB);
#endif
      return kv;
    }
    case LabelType::FLASH_CHIP_SPEED:
    {
      KeyValueStruct kv = make_kv(F("Flash Chip Speed"), getFlashChipSpeed() / 1000000);
#if FEATURE_TASKVALUE_UNIT_OF_MEASURE
      KV_SETUNIT(UOM_MHz);
#endif
      return kv;
    }
    case LabelType::FLASH_IDE_SIZE:
    {
      KeyValueStruct kv = make_kv(F("Flash IDE Size"), ESP.getFlashChipSize() >> 10);
#if FEATURE_TASKVALUE_UNIT_OF_MEASURE
      KV_SETUNIT(UOM_kB);
#endif
      return kv;
    }
    case LabelType::FLASH_IDE_SPEED:
    {
      KeyValueStruct kv = make_kv(F("Flash IDE Speed"), getFlashChipSpeed() / 1000000);
#if FEATURE_TASKVALUE_UNIT_OF_MEASURE
      KV_SETUNIT(UOM_MHz);
#endif
      return kv;
    }
    case LabelType::FLASH_IDE_MODE:
    {
      KeyValueStruct kv = make_kv(F("Flash IDE Mode"), getFlashChipMode(), KeyValueStruct::Format::PreFormatted);
      KV_SETID(F("mode"));
      return kv;
    }
    case LabelType::FLASH_WRITE_COUNT:
    {
      if (extendedValue) {
        return make_kv(
          F("Flash Writes"),
          strformat(
            F("%d daily / %d cold boot"),
            RTC.flashDayCounter,
            static_cast<int>(RTC.flashCounter)));
      }
      return make_kv(F("Flash Writes"), RTC.flashCounter);
    }
    case LabelType::SKETCH_SIZE:
    {
      String str;

      if (extendedValue) {
        uint32_t maxSketchSize;
        bool     use2step;
        OTA_possible(maxSketchSize, use2step);
        const uint32_t sketchsize_kB = getSketchSize() >> 10;
        maxSketchSize >>= 10;

        if (maxSketchSize >= sketchsize_kB)
          str += strformat(
            F("%d [kB] (%d kB not used)"),
            sketchsize_kB,
            (maxSketchSize - sketchsize_kB));
        else
          str += strformat(
            F("%d [kB]"),
            sketchsize_kB);

      } else {
        str = (getSketchSize() >> 10);
      }

      KeyValueStruct kv = make_kv(F("Sketch Size"), std::move(str));
      KV_SETID(F("sketch_size"));

#if FEATURE_TASKVALUE_UNIT_OF_MEASURE
      if (!extendedValue) {
        KV_SETUNIT(UOM_kB);
      }
#endif
      return kv;
    }
    case LabelType::SKETCH_FREE:
    {
      KeyValueStruct kv = make_kv(F("Sketch Free"), getFreeSketchSpace() >> 10);
      KV_SETID(F("sketch_free"));
#if FEATURE_TASKVALUE_UNIT_OF_MEASURE
      KV_SETUNIT(UOM_kB);
#endif
      return kv;
    }
    case LabelType::FS_SIZE:
    {
      KeyValueStruct kv = make_kv(
        #ifdef USE_LITTLEFS
        F("Little FS Size"),
        #else
        F("SPIFFS Size"),
        #endif // ifdef USE_LITTLEFS
        static_cast<uint32_t>(SpiffsTotalBytes() >> 10));
      KV_SETID(F("fs_size"));

#if FEATURE_TASKVALUE_UNIT_OF_MEASURE
      KV_SETUNIT(UOM_kB);
#endif
      return kv;
    }
    case LabelType::FS_FREE:
    {
      KeyValueStruct kv = make_kv(
        #ifdef USE_LITTLEFS
        F("Little FS Free"),
        #else
        F("SPIFFS Free"),
        #endif // ifdef USE_LITTLEFS
        static_cast<uint32_t>(SpiffsFreeSpace() >> 10));
      KV_SETID(F("fs_free"));
#if FEATURE_TASKVALUE_UNIT_OF_MEASURE
      KV_SETUNIT(UOM_kB);
#endif
      return kv;
    }
    case LabelType::MAX_OTA_SKETCH_SIZE:
    {
      uint32_t maxSketchSize;
      bool     use2step;
      OTA_possible(maxSketchSize, use2step);

      return make_kv(F("Max. OTA Sketch Size"), strformat(
                              F("%d [kB] (%d bytes)"),
                              maxSketchSize / 1024,
                              maxSketchSize));
    }
#ifdef ESP8266
    case LabelType::OTA_2STEP:
    {
      uint32_t maxSketchSize;
      bool     use2step;
      OTA_possible(maxSketchSize, use2step);

      return make_kv(F("OTA 2-step Needed"), use2step);
    }
    case LabelType::OTA_POSSIBLE:
    {
      uint32_t maxSketchSize;
      bool     use2step;
    # if defined(ESP8266)
      bool otaEnabled =
    # endif // if defined(ESP8266)
      OTA_possible(maxSketchSize, use2step);
      return make_kv(F("OTA possible"), otaEnabled);
    }
#endif // ifdef ESP8266
    #if FEATURE_INTERNAL_TEMPERATURE
    case LabelType::INTERNAL_TEMPERATURE:
    {
      KeyValueStruct kv = make_kv(F("Internal Temperature"), getInternalTemperature(), 1);
# if FEATURE_TASKVALUE_UNIT_OF_MEASURE
      KV_SETUNIT(UOM_degC);
# endif
      return kv;
    }
    #endif // if FEATURE_INTERNAL_TEMPERATURE
#if FEATURE_ETHERNET
    case LabelType::ETH_MAC:
    {
      return make_kv(F("Eth MAC"), ESPEasy::net::NetworkMacAddress().toString());
    }
    case LabelType::ETH_DUPLEX:
    {
      KeyValueStruct kv = make_kv(F("Eth Mode"), ESPEasy::net::EthLinkUp() ?
                        (ESPEasy::net::EthFullDuplex() ? F("Full Duplex") : F("Half Duplex")) : F("Link Down"));
      KV_SETID(F("ethduplex"));
      return kv;
    }
    case LabelType::ETH_SPEED:
    {
      KeyValueStruct kv = make_kv(F("Eth Speed"), getEthSpeed());
      KV_SETID(F("ethspeed"));
# if FEATURE_TASKVALUE_UNIT_OF_MEASURE
      KV_SETUNIT(UOM_Mbps);
# endif
      return kv;
    }
    case LabelType::ETH_STATE:
    {
      KeyValueStruct kv = make_kv(F("Eth State"), ESPEasy::net::EthLinkUp() ? F("Link Up") : F("Link Down"));
      KV_SETID(F("ethstate"));
      return kv;
    }
    case LabelType::ETH_SPEED_STATE:
    {
      if (active_network_medium != ESPEasy::net::NetworkMedium_t::Ethernet) { break; }
      KeyValueStruct kv = make_kv(F("Eth Speed State"), getEthLinkSpeedState());
      KV_SETID(F("ethspeedstate"));
      return kv;
    }
    case LabelType::ETH_CONNECTED:
    {
      KeyValueStruct kv = make_kv(F("Eth connected"), ESPEasy::net::eth::ETHConnected() ? F("CONNECTED") : F("DISCONNECTED"));
      KV_SETID(F("ethconnected"));
      return kv;
    }
    case LabelType::ETH_CHIP:
    {
      // FIXME TD-er: Might no longer be needed? Otherwise need to query ETH Network interface, not settings.
      KeyValueStruct kv = make_kv(F("Eth chip"), toString(Settings.ETH_Phy_Type));
      KV_SETID(F("ethchip"));
      return kv;
    }
#endif // if FEATURE_ETHERNET
#if FEATURE_ETHERNET || defined(USES_ESPEASY_NOW)
    case LabelType::ETH_WIFI_MODE:
    {
      KeyValueStruct kv = make_kv(F("Network Type"), toString(active_network_medium));
      KV_SETID(F("ethwifimode"));
      return kv;
    }
#endif // if FEATURE_ETHERNET || defined(USES_ESPEASY_NOW)
    case LabelType::SUNRISE:
    {
      return make_kv(F("Sunrise"), node_time.getSunriseTimeString(':'));
    }
    case LabelType::SUNSET:
    {
      return make_kv(F("Sunset"), node_time.getSunsetTimeString(':'));
    }
    case LabelType::SUNRISE_S:
    {
      return make_kv(F("Sunrise sec."), node_time.sunRise.tm_hour * 3600 + node_time.sunRise.tm_min * 60 +
                            node_time.sunRise.tm_sec);
    }
    case LabelType::SUNSET_S:
    {
      return make_kv(F("Sunset sec."), node_time.sunSet.tm_hour * 3600 + node_time.sunSet.tm_min * 60 + node_time.sunSet.tm_sec);
    }
    case LabelType::SUNRISE_M:
    {
      return make_kv(F("Sunrise min."), node_time.sunRise.tm_hour * 60 + node_time.sunRise.tm_min);
    }
    case LabelType::SUNSET_M:
    {
      return make_kv(F("Sunset min."), node_time.sunSet.tm_hour * 60 + node_time.sunSet.tm_min);
    }
    case LabelType::ISNTP:
    {
      return make_kv(F("Use NTP"), Settings.UseNTP());
    }
    case LabelType::UPTIME_MS:
    {
      return make_kv(F("Uptime (ms)"), getMicros64() / 1000);
    }
    case LabelType::TIMEZONE_OFFSET:
    {
      return make_kv(F("Timezone Offset"), Settings.TimeZone);
    }
    case LabelType::LATITUDE:
    {
      return make_kv(F("Latitude"), Settings.Latitude, 6);
    }
    case LabelType::LONGITUDE:
    {
      return make_kv(F("Longitude"), Settings.Longitude, 6);
    }

    case LabelType::MAX_LABEL:
      break;
  }
  return KeyValueStruct();
}

String getInternalLabel(const KeyValueStruct& kv,
                        char            replaceSpace)
{
  String res = kv.getID();
  if (replaceSpace != '\0') res.replace(' ', replaceSpace);

  return res;
}


String getInternalLabel(LabelType::Enum label, char replaceSpace) {
  return getInternalLabel(getKeyValue(label), replaceSpace);
}

String getLabel(const KeyValueStruct& kv)
{
  if (kv._key.isEmpty()) {
    return F("MissingString");
  }
  return kv._key.toString();
}

String getLabel(LabelType::Enum label) {
  return getLabel(getKeyValue(label));
}


String getValue(const KeyValueStruct& kv)
{
  if (kv._values.size() && kv._values[0].isSet()) { return kv._values[0].toString(); }
  return EMPTY_STRING;
}

int64_t getValue_int(const KeyValueStruct& kv)
{
  if (kv._values.size() && kv._values[0].isSet()) { 
    return kv._values[0].toInt();
  }
  return 0;
}

double  getValue_float(const KeyValueStruct& kv)
{
  if (kv._values.size() && kv._values[0].isSet()) { 
    return kv._values[0].toFloat();
  }
  return 0;
}

String getValue(LabelType::Enum label) {
  return getValue(getKeyValue(label));
}

#if FEATURE_ETHERNET

String getEthSpeed() {
  if (ESPEasy::net::EthLinkUp()) {
    return String(ESPEasy::net::EthLinkSpeed());
  }
  return getValue(LabelType::ETH_STATE);
}

String getEthLinkSpeedState() {
  if (ESPEasy::net::EthLinkUp()) {
    return strformat(F("%s %s %s"),
                     getValue(LabelType::ETH_STATE).c_str(),
                     getValue(LabelType::ETH_DUPLEX).c_str(),
                     getEthSpeed().c_str());
  }
  return getValue(LabelType::ETH_STATE);
}

#endif // if FEATURE_ETHERNET

String getExtendedValue(LabelType::Enum label) {
  switch (label)
  {
    case LabelType::UPTIME:
    {
      return minutesToDayHourMinute(getUptimeMinutes());
    }

    default:
      break;
  }
  return EMPTY_STRING;
}

String getFormNote(LabelType::Enum label)
{
  // Keep flash string till the end of the function, to reduce build size
  // Otherwise lots of calls to String() constructor are included.
  const __FlashStringHelper *flash_str = F("");

  switch (label)
  {
#ifndef MINIMAL_OTA
    case LabelType::CONNECT_HIDDEN_SSID:
      flash_str = F("Must be checked to connect to a hidden SSID");
      break;
# ifdef ESP32
    case LabelType::WIFI_PASSIVE_SCAN:
      flash_str = F("Passive scan listens for WiFi beacons, Active scan probes for AP.");
      break;
# endif // ifdef ESP32
    case LabelType::HIDDEN_SSID_SLOW_CONNECT:
      flash_str = F("Required for some AP brands like Mikrotik to connect to hidden SSID");
      break;
# if FEATURE_USE_IPV6
    case LabelType::ENABLE_IPV6:
      flash_str = F("Toggling IPv6 requires reboot");
      break;
# endif // if FEATURE_USE_IPV6
# ifndef NO_HTTP_UPDATER
    case LabelType::ALLOW_OTA_UNLIMITED:
      flash_str = F("When enabled, OTA updating can overwrite the filesystem and settings!<br>Requires reboot to activate");
      break;
# endif // ifndef NO_HTTP_UPDATER
# if FEATURE_RULES_EASY_COLOR_CODE
    case LabelType::DISABLE_RULES_AUTOCOMPLETE:
      flash_str = F("Also disables Rules syntax highlighting!");
      break;
# endif // if FEATURE_RULES_EASY_COLOR_CODE

    case LabelType::FORCE_WIFI_NOSLEEP:
      flash_str = F("Change WiFi sleep settings requires reboot to activate");
      break;

    case LabelType::CPU_ECO_MODE:
      flash_str = F("Node may miss receiving packets with Eco mode enabled");
      break;

#if FEATURE_MDNS
    case LabelType::USE_MDNS:
      flash_str = F("Allow node to be known on the local network as <hostname>.local");
      break;
#endif

    case LabelType::WIFI_AP_CHANNEL:
      flash_str = F("WiFi channel to be used when only WiFi AP is active");
      break;
    case LabelType::WIFI_ENABLE_CAPTIVE_PORTAL:
      flash_str = F("When set, user will be redirected to /setup or root page when connecting to this AP. /setup can still be called when disabled.");
      break;

    case LabelType::WIFI_NR_RECONNECT_ATTEMPTS:
      flash_str = F("Number of retry attempts before counting as 'failed'");
      break;
    case LabelType::WIFI_MAX_UPTIME_AUTO_START_AP:
      flash_str = F("Only start AP automatically when system uptime is less than set minutes (0 = allow always)");
      break;
    case LabelType::WIFI_AP_MINIMAL_ON_TIME:
      flash_str = F("Minimal time to have the AP actively waiting for a user to connect");
      break;
# ifndef ESP32
    case LabelType::WAIT_WIFI_CONNECT:
      flash_str = F("Wait for 1000 msec right after connecting to WiFi.<BR>May improve success on some APs like Fritz!Box");
      break;
# endif // ifndef ESP32
#ifdef ESP32
    case LabelType::WIFI_AP_ENABLE_NAPT:
      flash_str = F("NAPT will have no effect when 'force /setup' is enabled");
      break;

#endif
#endif // ifndef MINIMAL_OTA

#if FEATURE_SET_WIFI_TX_PWR
    case LabelType::WIFI_TX_MAX_PWR:
    case LabelType::WIFI_SENS_MARGIN:
    {
      float maxTXpwr;
      float sensitivity = ESPEasy::net::wifi::GetRSSIthreshold(maxTXpwr);

      if (LabelType::WIFI_TX_MAX_PWR == label) {
        return strformat(
          F("Current max: %.2f dBm"), maxTXpwr);
      }
      return strformat(
        F("Adjust TX power to target the AP with (sensitivity + margin) dBm signal strength. Current sensitivity: %.2f dBm"),
        sensitivity);
    }
#endif // if FEATURE_SET_WIFI_TX_PWR

    default:
      return EMPTY_STRING;
  }

  return flash_str;
}

#if FEATURE_TASKVALUE_UNIT_OF_MEASURE
String getFormUnit(LabelType::Enum label)
{
  auto kv = getKeyValue(label);

  return kv.getUnit();
}

#endif // if FEATURE_TASKVALUE_UNIT_OF_MEASURE
