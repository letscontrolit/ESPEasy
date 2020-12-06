#include "StringProvider.h"

#ifdef HAS_ETHERNET
# include "ETH.h"
#endif // ifdef HAS_ETHERNET
#include "../../ESPEasy_fdwdecl.h"
#include "../../ESPEasy-Globals.h"

#include "../ESPEasyCore/ESPEasyNetwork.h"
#include "../ESPEasyCore/ESPEasyWifi.h"

#include "../Globals/ESPEasy_Scheduler.h"
#include "../Globals/ESPEasy_time.h"
#include "../Globals/ESPEasyWiFiEvent.h"
#include "../Globals/NetworkState.h"
#include "../Globals/Settings.h"

#include "../Helpers/CompiletimeDefines.h"
#include "../Helpers/Memory.h"
#include "../Helpers/Scheduler.h"
#include "../Helpers/StringConverter.h"
#include "../Helpers/StringGenerator_System.h"

#include "../WebServer/JSON.h"
#include "../WebServer/AccessControl.h"

String getInternalLabel(LabelType::Enum label, char replaceSpace) {
  return to_internal_string(getLabel(label), replaceSpace);
}

String getLabel(LabelType::Enum label) {
  switch (label)
  {
    case LabelType::UNIT_NR:                return F("Unit Number");
    case LabelType::UNIT_NAME:              return F("Unit Name");
    case LabelType::HOST_NAME:              return F("Hostname");

    case LabelType::LOCAL_TIME:             return F("Local Time");
    case LabelType::UPTIME:                 return F("Uptime");
    case LabelType::LOAD_PCT:               return F("Load");
    case LabelType::LOOP_COUNT:             return F("Load LC");
    case LabelType::CPU_ECO_MODE:           return F("CPU Eco Mode");

    case LabelType::FREE_MEM:               return F("Free RAM");
    case LabelType::FREE_STACK:             return F("Free Stack");
#if defined(CORE_POST_2_5_0) || defined(ESP32)
    case LabelType::HEAP_MAX_FREE_BLOCK:    return F("Heap Max Free Block");
#endif // if defined(CORE_POST_2_5_0) || defined(ESP32)
#if defined(CORE_POST_2_5_0)
    case LabelType::HEAP_FRAGMENTATION:     return F("Heap Fragmentation");
#endif // if defined(CORE_POST_2_5_0)

#ifdef ESP32
    case LabelType::HEAP_SIZE:              return F("Heap Size");
    case LabelType::HEAP_MIN_FREE:          return F("Heap Min Free");
    case LabelType::PSRAM_SIZE:             return F("PSRAM Size");
    case LabelType::PSRAM_FREE:             return F("PSRAM Free");
    case LabelType::PSRAM_MIN_FREE:         return F("PSRAM Min Free");
    case LabelType::PSRAM_MAX_FREE_BLOCK:   return F("PSRAM Max Free Block");
#endif // ifdef ESP32

    case LabelType::BOOT_TYPE:              return F("Last Boot Cause");
    case LabelType::BOOT_COUNT:             return F("Boot Count");
    case LabelType::RESET_REASON:           return F("Reset Reason");
    case LabelType::LAST_TASK_BEFORE_REBOOT: return F("Last Action before Reboot");
    case LabelType::SW_WD_COUNT:            return F("SW WD count");


    case LabelType::WIFI_CONNECTION:        return F("WiFi Connection");
    case LabelType::WIFI_RSSI:              return F("RSSI");
    case LabelType::IP_CONFIG:              return F("IP Config");
    case LabelType::IP_CONFIG_STATIC:       return F("Static");
    case LabelType::IP_CONFIG_DYNAMIC:      return F("DHCP");
    case LabelType::IP_ADDRESS:             return F("IP Address");
    case LabelType::IP_SUBNET:              return F("IP Subnet");
    case LabelType::IP_ADDRESS_SUBNET:      return F("IP / Subnet");
    case LabelType::GATEWAY:                return F("Gateway");
    case LabelType::CLIENT_IP:              return F("Client IP");
    #ifdef FEATURE_MDNS
    case LabelType::M_DNS:                  return F("mDNS");
    #endif // ifdef FEATURE_MDNS
    case LabelType::DNS:                    return F("DNS");
    case LabelType::DNS_1:                  return F("DNS 1");
    case LabelType::DNS_2:                  return F("DNS 2");
    case LabelType::ALLOWED_IP_RANGE:       return F("Allowed IP Range");
    case LabelType::STA_MAC:                return F("STA MAC");
    case LabelType::AP_MAC:                 return F("AP MAC");
    case LabelType::SSID:                   return F("SSID");
    case LabelType::BSSID:                  return F("BSSID");
    case LabelType::CHANNEL:                return F("Channel");
    case LabelType::CONNECTED:              return F("Connected");
    case LabelType::CONNECTED_MSEC:         return F("Connected msec");
    case LabelType::LAST_DISCONNECT_REASON: return F("Last Disconnect Reason");
    case LabelType::LAST_DISC_REASON_STR:   return F("Last Disconnect Reason str");
    case LabelType::NUMBER_RECONNECTS:      return F("Number Reconnects");

    case LabelType::FORCE_WIFI_BG:          return F("Force WiFi B/G");
    case LabelType::RESTART_WIFI_LOST_CONN: return F("Restart WiFi Lost Conn");
    case LabelType::FORCE_WIFI_NOSLEEP:     return F("Force WiFi No Sleep");
    case LabelType::PERIODICAL_GRAT_ARP:    return F("Periodical send Gratuitous ARP");
    case LabelType::CONNECTION_FAIL_THRESH: return F("Connection Failure Threshold");

    case LabelType::BUILD_DESC:             return F("Build");
    case LabelType::GIT_BUILD:              return F("Git Build");
    case LabelType::SYSTEM_LIBRARIES:       return F("System Libraries");
    case LabelType::PLUGIN_COUNT:           return F("Plugin Count");
    case LabelType::PLUGIN_DESCRIPTION:     return F("Plugin Description");
    case LabelType::BUILD_TIME:             return F("Build Time");
    case LabelType::BINARY_FILENAME:        return F("Binary Filename");
    case LabelType::BUILD_PLATFORM:         return F("Build Platform");
    case LabelType::GIT_HEAD:               return F("Git HEAD");

    case LabelType::SYSLOG_LOG_LEVEL:       return F("Syslog Log Level");
    case LabelType::SERIAL_LOG_LEVEL:       return F("Serial Log Level");
    case LabelType::WEB_LOG_LEVEL:          return F("Web Log Level");
  #ifdef FEATURE_SD
    case LabelType::SD_LOG_LEVEL:           return F("SD Log Level");
  #endif // ifdef FEATURE_SD

    case LabelType::ESP_CHIP_ID:            return F("ESP Chip ID");
    case LabelType::ESP_CHIP_FREQ:          return F("ESP Chip Frequency");
    case LabelType::ESP_CHIP_MODEL:         return F("ESP Chip Model");
    case LabelType::ESP_CHIP_REVISION:      return F("ESP Chip Revision");
    case LabelType::ESP_CHIP_CORES:         return F("ESP Chip Cores");

    case LabelType::ESP_BOARD_NAME:         return F("ESP Board Name");

    case LabelType::FLASH_CHIP_ID:          return F("Flash Chip ID");
    case LabelType::FLASH_CHIP_REAL_SIZE:   return F("Flash Chip Real Size");
    case LabelType::FLASH_IDE_SIZE:         return F("Flash IDE Size");
    case LabelType::FLASH_IDE_SPEED:        return F("Flash IDE Speed");
    case LabelType::FLASH_IDE_MODE:         return F("Flash IDE Mode");
    case LabelType::FLASH_WRITE_COUNT:      return F("Flash Writes");
    case LabelType::SKETCH_SIZE:            return F("Sketch Size");
    case LabelType::SKETCH_FREE:            return F("Sketch Free");
    #ifdef USE_LITTLEFS
    case LabelType::FS_SIZE:                return F("Little FS Size");
    case LabelType::FS_FREE:                return F("Little FS Free");
    #else // ifdef USE_LITTLEFS
    case LabelType::FS_SIZE:                return F("SPIFFS Size");
    case LabelType::FS_FREE:                return F("SPIFFS Free");
    #endif // ifdef USE_LITTLEFS
    case LabelType::MAX_OTA_SKETCH_SIZE:    return F("Max. OTA Sketch Size");
    case LabelType::OTA_2STEP:              return F("OTA 2-step Needed");
    case LabelType::OTA_POSSIBLE:           return F("OTA possible");
#ifdef HAS_ETHERNET
    case LabelType::ETH_IP_ADDRESS:         return F("Eth IP Address");
    case LabelType::ETH_IP_SUBNET:          return F("Eth IP Subnet");
    case LabelType::ETH_IP_ADDRESS_SUBNET:  return F("Eth IP / Subnet");
    case LabelType::ETH_IP_GATEWAY:         return F("Eth Gateway");
    case LabelType::ETH_IP_DNS:             return F("Eth DNS");
    case LabelType::ETH_MAC:                return F("Eth MAC");
    case LabelType::ETH_DUPLEX:             return F("Eth Mode");
    case LabelType::ETH_SPEED:              return F("Eth Speed");
    case LabelType::ETH_STATE:              return F("Eth State");
    case LabelType::ETH_SPEED_STATE:        return F("Eth Speed State");
    case LabelType::ETH_WIFI_MODE:          return F("Network Type");
    case LabelType::ETH_CONNECTED:          return F("Eth connected");
#endif // ifdef HAS_ETHERNET
  }
  return F("MissingString");
}

String getValue(LabelType::Enum label) {
  switch (label)
  {
    case LabelType::UNIT_NR:                return String(Settings.Unit);
    case LabelType::UNIT_NAME:              return String(Settings.Name); // Only return the set name, no appended unit.
    case LabelType::HOST_NAME:              return NetworkGetHostname();


    case LabelType::LOCAL_TIME:             return node_time.getDateTimeString('-', ':', ' ');
    case LabelType::UPTIME:                 return String(wdcounter / 2);
    case LabelType::LOAD_PCT:               return String(getCPUload());
    case LabelType::LOOP_COUNT:             return String(getLoopCountPerSec());
    case LabelType::CPU_ECO_MODE:           return jsonBool(Settings.EcoPowerMode());

    case LabelType::FREE_MEM:               return String(ESP.getFreeHeap());
    case LabelType::FREE_STACK:             return String(getCurrentFreeStack());
#if defined(CORE_POST_2_5_0)
    case LabelType::HEAP_MAX_FREE_BLOCK:    return String(ESP.getMaxFreeBlockSize());
#endif // if defined(CORE_POST_2_5_0)
#if  defined(ESP32)
    case LabelType::HEAP_MAX_FREE_BLOCK:    return String(ESP.getMaxAllocHeap());
#endif // if  defined(ESP32)
#if defined(CORE_POST_2_5_0)
    case LabelType::HEAP_FRAGMENTATION:     return String(ESP.getHeapFragmentation());
#endif // if defined(CORE_POST_2_5_0)
#ifdef ESP32
    case LabelType::HEAP_SIZE:              return String(ESP.getHeapSize());
    case LabelType::HEAP_MIN_FREE:          return String(ESP.getMinFreeHeap());
    case LabelType::PSRAM_SIZE:             return String(ESP.getPsramSize());
    case LabelType::PSRAM_FREE:             return String(ESP.getFreePsram());
    case LabelType::PSRAM_MIN_FREE:         return String(ESP.getMinFreeHeap());
    case LabelType::PSRAM_MAX_FREE_BLOCK:   return String(ESP.getMaxAllocPsram());
#endif // ifdef ESP32


    case LabelType::BOOT_TYPE:              return getLastBootCauseString();
    case LabelType::BOOT_COUNT:             break;
    case LabelType::RESET_REASON:           return getResetReasonString();
    case LabelType::LAST_TASK_BEFORE_REBOOT: return ESPEasy_Scheduler::decodeSchedulerId(lastMixedSchedulerId_beforereboot);
    case LabelType::SW_WD_COUNT:            return String(sw_watchdog_callback_count);

    case LabelType::WIFI_CONNECTION:        break;
    case LabelType::WIFI_RSSI:              return String(WiFi.RSSI());
    case LabelType::IP_CONFIG:              return useStaticIP() ? getLabel(LabelType::IP_CONFIG_STATIC) : getLabel(
        LabelType::IP_CONFIG_DYNAMIC);
    case LabelType::IP_CONFIG_STATIC:       break;
    case LabelType::IP_CONFIG_DYNAMIC:      break;
    case LabelType::IP_ADDRESS:             return NetworkLocalIP().toString();
    case LabelType::IP_SUBNET:              return NetworkSubnetMask().toString();
    case LabelType::IP_ADDRESS_SUBNET:      return String(getValue(LabelType::IP_ADDRESS) + F(" / ") + getValue(LabelType::IP_SUBNET));
    case LabelType::GATEWAY:                return NetworkGatewayIP().toString();
    case LabelType::CLIENT_IP:              return formatIP(web_server.client().remoteIP());

    #ifdef FEATURE_MDNS
    case LabelType::M_DNS:                  return String(NetworkGetHostname()) + F(".local");
    #endif // ifdef FEATURE_MDNS
    case LabelType::DNS:                    return String(getValue(LabelType::DNS_1) + F(" / ") + getValue(LabelType::DNS_2));
    case LabelType::DNS_1:                  return NetworkDnsIP(0).toString();
    case LabelType::DNS_2:                  return NetworkDnsIP(1).toString();
    case LabelType::ALLOWED_IP_RANGE:       return describeAllowedIPrange();
    case LabelType::STA_MAC:                return NetworkMacAddress();
    case LabelType::AP_MAC:                 return WifiSoftAPmacAddress();
    case LabelType::SSID:                   return WiFi.SSID();
    case LabelType::BSSID:                  return WiFi.BSSIDstr();
    case LabelType::CHANNEL:                return String(WiFi.channel());
    case LabelType::CONNECTED:              return format_msec_duration(WiFiEventData.lastConnectMoment.millisPassedSince());

    // Use only the nr of seconds to fit it in an int32, plus append '000' to have msec format again.
    case LabelType::CONNECTED_MSEC:         return String(static_cast<int32_t>(WiFiEventData.lastConnectMoment.millisPassedSince() / 1000ll)) + F("000"); 
    case LabelType::LAST_DISCONNECT_REASON: return String(WiFiEventData.lastDisconnectReason);
    case LabelType::LAST_DISC_REASON_STR:   return getLastDisconnectReason();
    case LabelType::NUMBER_RECONNECTS:      return String(WiFiEventData.wifi_reconnects);

    case LabelType::FORCE_WIFI_BG:          return jsonBool(Settings.ForceWiFi_bg_mode());
    case LabelType::RESTART_WIFI_LOST_CONN: return jsonBool(Settings.WiFiRestart_connection_lost());
    case LabelType::FORCE_WIFI_NOSLEEP:     return jsonBool(Settings.WifiNoneSleep());
    case LabelType::PERIODICAL_GRAT_ARP:    return jsonBool(Settings.gratuitousARP());
    case LabelType::CONNECTION_FAIL_THRESH: return String(Settings.ConnectionFailuresThreshold);

    case LabelType::BUILD_DESC:             return String(BUILD);
    case LabelType::GIT_BUILD:              return String(F(BUILD_GIT));
    case LabelType::SYSTEM_LIBRARIES:       return getSystemLibraryString();
    case LabelType::PLUGIN_COUNT:           return String(deviceCount + 1);
    case LabelType::PLUGIN_DESCRIPTION:     return getPluginDescriptionString();
    case LabelType::BUILD_TIME:             return get_build_date() + " " + get_build_time();
    case LabelType::BINARY_FILENAME:        return get_binary_filename();
    case LabelType::BUILD_PLATFORM:         return get_build_platform();
    case LabelType::GIT_HEAD:               return get_git_head();
    case LabelType::SYSLOG_LOG_LEVEL:       return getLogLevelDisplayString(Settings.SyslogLevel);
    case LabelType::SERIAL_LOG_LEVEL:       return getLogLevelDisplayString(getSerialLogLevel());
    case LabelType::WEB_LOG_LEVEL:          return getLogLevelDisplayString(getWebLogLevel());
  #ifdef FEATURE_SD
    case LabelType::SD_LOG_LEVEL:           return getLogLevelDisplayString(Settings.SDLogLevel);
  #endif // ifdef FEATURE_SD

    case LabelType::ESP_CHIP_ID:            return String(getChipId(), HEX);
    case LabelType::ESP_CHIP_FREQ:          return String(ESP.getCpuFreqMHz());
    case LabelType::ESP_CHIP_MODEL:         return getChipModel();
    case LabelType::ESP_CHIP_REVISION:      return String(getChipRevision());
    case LabelType::ESP_CHIP_CORES:         return String(getChipCores());
    case LabelType::ESP_BOARD_NAME:         break;

    case LabelType::FLASH_CHIP_ID:          break;
    case LabelType::FLASH_CHIP_REAL_SIZE:   break;
    case LabelType::FLASH_IDE_SIZE:         break;
    case LabelType::FLASH_IDE_SPEED:        break;
    case LabelType::FLASH_IDE_MODE:         break;
    case LabelType::FLASH_WRITE_COUNT:      break;
    case LabelType::SKETCH_SIZE:            break;
    case LabelType::SKETCH_FREE:            break;
    case LabelType::FS_SIZE:                break;
    case LabelType::FS_FREE:                break;
    case LabelType::MAX_OTA_SKETCH_SIZE:    break;
    case LabelType::OTA_2STEP:              break;
    case LabelType::OTA_POSSIBLE:           break;
#ifdef HAS_ETHERNET
    case LabelType::ETH_IP_ADDRESS:         return NetworkLocalIP().toString();
    case LabelType::ETH_IP_SUBNET:          return NetworkSubnetMask().toString();
    case LabelType::ETH_IP_ADDRESS_SUBNET:  return String(getValue(LabelType::ETH_IP_ADDRESS) + F(" / ") +
                                                          getValue(LabelType::ETH_IP_SUBNET));
    case LabelType::ETH_IP_GATEWAY:         return NetworkGatewayIP().toString();
    case LabelType::ETH_IP_DNS:             return NetworkDnsIP(0).toString();
    case LabelType::ETH_MAC:                return NetworkMacAddress();
    case LabelType::ETH_DUPLEX:             return eth_connected ? (ETH.fullDuplex() ? F("Full Duplex") : F("Half Duplex")) : F("No Ethernet");
    case LabelType::ETH_SPEED:              return eth_connected ? getEthSpeed() : F("No Ethernet");
    case LabelType::ETH_STATE:              return eth_connected ? (ETH.linkUp() ? F("Link Up") : F("Link Down")) : F("No Ethernet");
    case LabelType::ETH_SPEED_STATE:        return eth_connected ? getEthLinkSpeedState() : F("No Ethernet");
    case LabelType::ETH_WIFI_MODE:          return active_network_medium == NetworkMedium_t::WIFI ? F("WIFI") : F("ETHERNET");
    case LabelType::ETH_CONNECTED:          return eth_connected ? F("CONNECTED") : F("DISCONNECTED"); // 0=disconnected, 1=connected
#endif // ifdef HAS_ETHERNET
  }
  return F("MissingString");
}

#ifdef HAS_ETHERNET
String getEthSpeed() {
  String result;

  result.reserve(7);
  result += ETH.linkSpeed();
  result += F("Mbps");
  return result;
}

String getEthLinkSpeedState() {
  String result;

  result.reserve(29);

  if (ETH.linkUp()) {
    result += getValue(LabelType::ETH_STATE);
    result += ' ';
    result += getValue(LabelType::ETH_DUPLEX);
    result += ' ';
    result += getEthSpeed();
  } else {
    result = getValue(LabelType::ETH_STATE);
  }
  return result;
}

#endif // ifdef HAS_ETHERNET

String getExtendedValue(LabelType::Enum label) {
  switch (label)
  {
    case LabelType::UPTIME:
    {
      String result;
      result.reserve(40);
      int minutes = wdcounter / 2;
      int days    = minutes / 1440;
      minutes = minutes % 1440;
      int hrs = minutes / 60;
      minutes = minutes % 60;

      result += days;
      result += F(" days ");
      result += hrs;
      result += F(" hours ");
      result += minutes;
      result += F(" minutes");
      return result;
    }

    default:
      break;
  }
  return "";
}

String getFileName(FileType::Enum filetype) {
  String result;

  switch (filetype)
  {
    case FileType::CONFIG_DAT:
      result += F("config.dat");
      break;
    case FileType::NOTIFICATION_DAT:
      result += F("notification.dat");
      break;
    case FileType::SECURITY_DAT:
      result += F("security.dat");
      break;
    case FileType::RULES_TXT:
      // Use getRulesFileName
      break;
  }
  return result;
}

String getFileName(FileType::Enum filetype, unsigned int filenr) {
  if (filetype == FileType::RULES_TXT) {
    return getRulesFileName(filenr);
  }
  return getFileName(filetype);
}

// filenr = 0...3 for files rules1.txt ... rules4.txt
String getRulesFileName(unsigned int filenr) {
  String result;

  if (filenr < 4) {
    result += F("rules");
    result += filenr + 1;
    result += F(".txt");
  }
  return result;
}
