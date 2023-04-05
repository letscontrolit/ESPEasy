#include "../WebServer/SysInfoPage.h"

#if defined(WEBSERVER_SYSINFO) || SHOW_SYSINFO_JSON

#include "../WebServer/AccessControl.h"
#include "../WebServer/ESPEasy_WebServer.h"
#include "../WebServer/HTML_wrappers.h"
#include "../WebServer/Markup.h"
#include "../WebServer/Markup_Buttons.h"


#include "../../ESPEasy-Globals.h"

#include "../Commands/Diagnostic.h"

#include "../CustomBuild/CompiletimeDefines.h"

#include "../DataStructs/RTCStruct.h"

#include "../ESPEasyCore/ESPEasyEth.h"
#include "../ESPEasyCore/ESPEasyNetwork.h"
#include "../ESPEasyCore/ESPEasyWifi.h"

#include "../Globals/CRCValues.h"
#include "../Globals/ESPEasy_time.h"
#include "../Globals/ESPEasyWiFiEvent.h"
#include "../Globals/NetworkState.h"
#include "../Globals/RTC.h"
#include "../Globals/Settings.h"

#include "../Helpers/Convert.h"
#include "../Helpers/ESPEasyStatistics.h"
#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/Hardware.h"
#include "../Helpers/Memory.h"
#include "../Helpers/Misc.h"
#include "../Helpers/Networking.h"
#include "../Helpers/OTA.h"
#include "../Helpers/StringConverter.h"
#include "../Helpers/StringGenerator_GPIO.h"
#include "../Helpers/StringGenerator_System.h"
#include "../Helpers/StringProvider.h"

#include "../Static/WebStaticData.h"

#if FEATURE_MQTT
# include "../Globals/MQTT.h"
# include "../ESPEasyCore/Controller.h" // For finding enabled MQTT controller
#endif

#ifdef ESP32
# include <esp_partition.h>
#endif // ifdef ESP32





#if SHOW_SYSINFO_JSON
// ********************************************************************************
// Web Interface sysinfo page
// ********************************************************************************
void handle_sysinfo_json() {
  # ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_sysinfo"));
  # endif // ifndef BUILD_NO_RAM_TRACKER

  if (!isLoggedIn()) { return; }
  TXBuffer.startJsonStream();
  json_init();
  json_open();
  json_open(false, F("general"));
  json_number(F("unit"), String(Settings.Unit));
  json_prop(F("time"),   node_time.getDateTimeString('-', ':', ' '));
  json_prop(F("uptime"), getExtendedValue(LabelType::UPTIME));
  json_number(F("cpu_load"),   toString(getCPUload()));
  json_number(F("loop_count"), String(getLoopCountPerSec()));
  json_close();

  int freeMem = ESP.getFreeHeap();
  json_open(false, F("mem"));
  json_number(F("free"),    String(freeMem));
  json_number(F("low_ram"), String(
  # ifndef BUILD_NO_RAM_TRACKER
                lowestRAM
  # else // ifndef BUILD_NO_RAM_TRACKER
                0
  # endif // ifndef BUILD_NO_RAM_TRACKER
                ));
  json_prop(F("low_ram_fn"), String(
  # ifndef BUILD_NO_RAM_TRACKER
            lowestRAMfunction
  # else // ifndef BUILD_NO_RAM_TRACKER
            0
  # endif // ifndef BUILD_NO_RAM_TRACKER
            ));
  json_number(F("stack"),     String(getCurrentFreeStack()));
  json_number(F("low_stack"), String(
  # ifndef BUILD_NO_RAM_TRACKER
                lowestFreeStack
  # else // ifndef BUILD_NO_RAM_TRACKER
                0
  # endif // ifndef BUILD_NO_RAM_TRACKER
                ));
  json_prop(F("low_stack_fn"), String(
  # ifndef BUILD_NO_RAM_TRACKER
            lowestFreeStackfunction
  # else // ifndef BUILD_NO_RAM_TRACKER
            0
  # endif // ifndef BUILD_NO_RAM_TRACKER
            ));
  json_close();

  json_open(false, F("boot"));
  json_prop(F("last_cause"),    getLastBootCauseString());
  json_number(F("counter"),     String(RTC.bootCounter));
  json_prop(F("reset_reason"),  getResetReasonString());
  json_close();

  json_open(false, F("wifi"));
  json_prop(F("type"),          toString(getConnectionProtocol()));
  json_number(F("rssi"),        String(WiFi.RSSI()));
  json_prop(F("dhcp"),          useStaticIP() ? getLabel(LabelType::IP_CONFIG_STATIC) : getLabel(LabelType::IP_CONFIG_DYNAMIC));
  json_prop(F("ip"),            getValue(LabelType::IP_ADDRESS));
  json_prop(F("subnet"),        getValue(LabelType::IP_SUBNET));
  json_prop(F("gw"),            getValue(LabelType::GATEWAY));
  json_prop(F("dns1"),          getValue(LabelType::DNS_1));
  json_prop(F("dns2"),          getValue(LabelType::DNS_2));
  json_prop(F("allowed_range"), describeAllowedIPrange());
  json_prop(F("sta_mac"),       getValue(LabelType::STA_MAC));
  json_prop(F("ap_mac"),        getValue(LabelType::AP_MAC));
  json_prop(F("ssid"),          getValue(LabelType::SSID));
  json_prop(F("bssid"),         getValue(LabelType::BSSID));
  json_number(F("channel"),     getValue(LabelType::CHANNEL));
  json_prop(F("encryption"),    getValue(LabelType::ENCRYPTION_TYPE_STA));
  json_prop(F("connected"),     getValue(LabelType::CONNECTED));
  json_prop(F("ldr"),           getValue(LabelType::LAST_DISC_REASON_STR));
  json_number(F("reconnects"),  getValue(LabelType::NUMBER_RECONNECTS));
  json_prop(F("ssid1"),         getValue(LabelType::WIFI_STORED_SSID1));
  json_prop(F("ssid2"),         getValue(LabelType::WIFI_STORED_SSID2));
  json_close();

# if FEATURE_ETHERNET
  json_open(false, F("ethernet"));
  json_prop(F("ethwifimode"),   getValue(LabelType::ETH_WIFI_MODE));
  json_prop(F("ethconnected"),  getValue(LabelType::ETH_CONNECTED));
  json_prop(F("ethduplex"),     getValue(LabelType::ETH_DUPLEX));
  json_prop(F("ethspeed"),      getValue(LabelType::ETH_SPEED));
  json_prop(F("ethstate"),      getValue(LabelType::ETH_STATE));
  json_prop(F("ethspeedstate"), getValue(LabelType::ETH_SPEED_STATE));
  json_close();
# endif // if FEATURE_ETHERNET

  json_open(false, F("firmware"));
  json_prop(F("build"),          getSystemBuildString());
  json_prop(F("notes"),          F(BUILD_NOTES));
  json_prop(F("libraries"),      getSystemLibraryString());
  json_prop(F("git_version"),    getValue(LabelType::GIT_BUILD));
  json_prop(F("plugins"),        getPluginDescriptionString());
  json_prop(F("md5"),            String(CRCValues.compileTimeMD5[0], HEX));
  json_number(F("md5_check"),    String(CRCValues.checkPassed()));
  json_prop(F("build_time"),     get_build_time());
  json_prop(F("filename"),       getValue(LabelType::BINARY_FILENAME));
  json_prop(F("build_platform"), getValue(LabelType::BUILD_PLATFORM));
  json_prop(F("git_head"),       getValue(LabelType::GIT_HEAD));
  json_close();

  json_open(false, F("esp"));
  json_prop(F("chip_id"),        getValue(LabelType::ESP_CHIP_ID));
  json_number(F("cpu"),          getValue(LabelType::ESP_CHIP_FREQ));
#ifdef ESP32
  json_number(F("xtal_freq"),    getValue(LabelType::ESP_CHIP_XTAL_FREQ));
  json_number(F("abp_freq"),     getValue(LabelType::ESP_CHIP_APB_FREQ));
#endif
  json_prop(F("board"),          getValue(LabelType::ESP_BOARD_NAME));
  json_close();

  json_open(false, F("storage"));

  // Set to HEX may be something like 0x1640E0.
  // Where manufacturer is 0xE0 and device is 0x4016.
  json_number(F("chip_id"), getValue(LabelType::FLASH_CHIP_ID));
  if (flashChipVendorPuya()) {
    if (puyaSupport()) {
      json_prop(F("vendor"), F("puya, supported"));
    } else {
      json_prop(F("vendor"), F("puya, error"));
    }
  } else {
    json_prop(F("vendor"),        getValue(LabelType::FLASH_CHIP_VENDOR));
  }
  json_number(F("device"),        getValue(LabelType::FLASH_CHIP_MODEL));
  json_number(F("real_size"),     String(getFlashRealSizeInBytes() / 1024));
  json_number(F("ide_size"),      String(ESP.getFlashChipSize() / 1024));

  // Please check what is supported for the ESP32
  json_number(F("flash_speed"),   getValue(LabelType::FLASH_CHIP_SPEED));

  json_prop(F("mode"),            getFlashChipMode());

  json_number(F("writes"),        String(RTC.flashDayCounter));
  json_number(F("flash_counter"), String(RTC.flashCounter));
  json_number(F("sketch_size"),   String(getSketchSize() / 1024));
  json_number(F("sketch_free"),   String(getFreeSketchSpace() / 1024));

  json_number(F("spiffs_size"),   String(SpiffsTotalBytes() / 1024));
  json_number(F("spiffs_free"),   String(SpiffsFreeSpace() / 1024));
  json_close();
  json_close();

  TXBuffer.endStream();
}

#endif // SHOW_SYSINFO_JSON

#ifdef WEBSERVER_SYSINFO

void handle_sysinfo() {
  # ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_sysinfo"));
  # endif // ifndef BUILD_NO_RAM_TRACKER

  if (!isLoggedIn()) { return; }
  navMenuIndex = MENU_INDEX_TOOLS;
  html_reset_copyTextCounter();
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate(_HEAD);

  addHtml(printWebString);
  addHtml(F("<form>"));

  // the table header
  html_table_class_normal();


  # ifdef WEBSERVER_GITHUB_COPY

  // Not using addFormHeader() to get the copy button on the same header line as 2nd column
  html_TR();
  html_table_header(F("System Info"), 225);
  addHtml(F("<TH>")); // Needed to get the copy button on the same header line.
  addCopyButton(F("copyText"), F("\\n"), F("Copy info to clipboard"));

  TXBuffer.addFlashString((PGM_P)FPSTR(githublogo));
  serve_JS(JSfiles_e::GitHubClipboard);

  # else // ifdef WEBSERVER_GITHUB_COPY
  addFormHeader(F("System Info"));

  # endif // ifdef WEBSERVER_GITHUB_COPY

  handle_sysinfo_basicInfo();

#ifndef WEBSERVER_SYSINFO_MINIMAL
  handle_sysinfo_memory();
#endif

  handle_sysinfo_Network();

# if FEATURE_ETHERNET
  handle_sysinfo_Ethernet();
# endif // if FEATURE_ETHERNET

#ifndef WEBSERVER_SYSINFO_MINIMAL
  handle_sysinfo_WiFiSettings();
#endif

  handle_sysinfo_Firmware();

#ifndef WEBSERVER_SYSINFO_MINIMAL
  handle_sysinfo_SystemStatus();

  handle_sysinfo_NetworkServices();

  handle_sysinfo_ESP_Board();

  handle_sysinfo_Storage();
#endif


  html_end_table();
  html_end_form();
  sendHeadandTail_stdtemplate(_TAIL);
  TXBuffer.endStream();
}

void handle_sysinfo_basicInfo() {
  addRowLabelValue(LabelType::UNIT_NR);

  if (node_time.systemTimePresent())
  {
    addRowLabelValue(LabelType::LOCAL_TIME);
    #if FEATURE_EXT_RTC
    if (Settings.ExtTimeSource() != ExtTimeSource_e::None) {
      addRowLabelValue(LabelType::EXT_RTC_UTC_TIME);
    }
    #endif
    addRowLabelValue(LabelType::TIME_SOURCE);
    addRowLabelValue(LabelType::TIME_WANDER);
    addUnit(F("ppm"));
  }

  addRowLabel(LabelType::UPTIME);
  {
    addHtml(getExtendedValue(LabelType::UPTIME));
  }

  addRowLabel(LabelType::LOAD_PCT);

  if (wdcounter > 0)
  {
    addHtml(String(getCPUload()));
    addHtml(F("% (LC="));
    addHtmlInt(getLoopCountPerSec());
    addHtml(')');
  }
  addRowLabelValue(LabelType::CPU_ECO_MODE);


  addRowLabel(F("Boot"));
  {
    addHtml(getLastBootCauseString());
    addHtml(F(" ("));
    addHtmlInt(static_cast<uint32_t>(RTC.bootCounter));
    addHtml(')');
  }
  addRowLabelValue(LabelType::RESET_REASON);
  addRowLabelValue(LabelType::LAST_TASK_BEFORE_REBOOT);
  addRowLabelValue(LabelType::SW_WD_COUNT);
}

#ifndef WEBSERVER_SYSINFO_MINIMAL
void handle_sysinfo_memory() {
  addTableSeparator(F("Memory"), 2, 3);

# ifdef ESP32
  addRowLabelValue(LabelType::HEAP_SIZE);
  addRowLabelValue(LabelType::HEAP_MIN_FREE);
# endif // ifdef ESP32

  int freeMem = ESP.getFreeHeap();
  addRowLabel(LabelType::FREE_MEM);
  {
    addHtmlInt(freeMem);
# ifndef BUILD_NO_RAM_TRACKER
    addHtml(F(" ("));
    addHtmlInt(lowestRAM);
    addHtml(F(" - "));
    addHtml(lowestRAMfunction);
    addHtml(')');
# endif // ifndef BUILD_NO_RAM_TRACKER
  }
# if defined(CORE_POST_2_5_0) || defined(ESP32)
 #  ifndef LIMIT_BUILD_SIZE
  addRowLabelValue(LabelType::HEAP_MAX_FREE_BLOCK);
 #  endif // ifndef LIMIT_BUILD_SIZE
# endif   // if defined(CORE_POST_2_5_0) || defined(ESP32)
# if defined(CORE_POST_2_5_0)
  #  ifndef LIMIT_BUILD_SIZE
  addRowLabelValue(LabelType::HEAP_FRAGMENTATION);
  addHtml('%');
  #  endif // ifndef LIMIT_BUILD_SIZE
  {
    #ifdef USE_SECOND_HEAP
    addRowLabelValue(LabelType::FREE_HEAP_IRAM);
    #endif
  }
# endif // if defined(CORE_POST_2_5_0)


  addRowLabel(LabelType::FREE_STACK);
  {
    addHtmlInt(getCurrentFreeStack());
# ifndef BUILD_NO_RAM_TRACKER
    addHtml(F(" ("));
    addHtmlInt(lowestFreeStack);
    addHtml(F(" - "));
    addHtml(lowestFreeStackfunction);
    addHtml(')');
# endif // ifndef BUILD_NO_RAM_TRACKER
  }

# if defined(ESP32) && defined(BOARD_HAS_PSRAM)

  addRowLabelValue(LabelType::PSRAM_SIZE);
  if (UsePSRAM()) {
    addRowLabelValue(LabelType::PSRAM_FREE);
    addRowLabelValue(LabelType::PSRAM_MIN_FREE);
    addRowLabelValue(LabelType::PSRAM_MAX_FREE_BLOCK);
  } 
# endif // if defined(ESP32) && defined(BOARD_HAS_PSRAM)
}
#endif

# if FEATURE_ETHERNET
void handle_sysinfo_Ethernet() {
  if (active_network_medium == NetworkMedium_t::Ethernet) {
    addTableSeparator(F("Ethernet"), 2, 3);
    addRowLabelValue(LabelType::ETH_STATE);
    addRowLabelValue(LabelType::ETH_SPEED);
    addRowLabelValue(LabelType::ETH_DUPLEX);
    addRowLabelValue(LabelType::ETH_MAC);
//    addRowLabelValue(LabelType::ETH_IP_ADDRESS_SUBNET);
//    addRowLabelValue(LabelType::ETH_IP_GATEWAY);
//    addRowLabelValue(LabelType::ETH_IP_DNS);
  }
}

# endif // if FEATURE_ETHERNET

void handle_sysinfo_Network() {
  addTableSeparator(F("Network"), 2, 3);

  # if FEATURE_ETHERNET || defined(USES_ESPEASY_NOW)
  addRowLabelValue(LabelType::ETH_WIFI_MODE);
  # endif 

  addRowLabelValue(LabelType::IP_CONFIG);
  addRowLabelValue(LabelType::IP_ADDRESS_SUBNET);
  addRowLabelValue(LabelType::GATEWAY);
  addRowLabelValue(LabelType::CLIENT_IP);
  addRowLabelValue(LabelType::DNS);
  addRowLabelValue(LabelType::ALLOWED_IP_RANGE);
  addRowLabelValue(LabelType::CONNECTED);
  addRowLabelValue(LabelType::NUMBER_RECONNECTS);

  addTableSeparator(F("WiFi"), 2, 3, F("Wifi"));

  const bool showWiFiConnectionInfo = !WiFiEventData.WiFiDisconnected();


  addRowLabel(LabelType::WIFI_CONNECTION);
  if (showWiFiConnectionInfo)
  {
    addHtml(toString(getConnectionProtocol()));
    addHtml(F(" (RSSI "));
    addHtmlInt(WiFi.RSSI());
    addHtml(F(" dBm)"));
  } else addHtml('-');

  addRowLabel(LabelType::SSID);
  if (showWiFiConnectionInfo)
  {
    addHtml(WiFi.SSID());
    addHtml(F(" ("));
    addHtml(WiFi.BSSIDstr());
    addHtml(')');
  } else addHtml('-');

  addRowLabel(getLabel(LabelType::CHANNEL));
  if (showWiFiConnectionInfo) {
    addHtml(getValue(LabelType::CHANNEL));
  } else addHtml('-');

  addRowLabel(getLabel(LabelType::ENCRYPTION_TYPE_STA));
  if (showWiFiConnectionInfo) {
    addHtml(getValue(LabelType::ENCRYPTION_TYPE_STA));
  } else addHtml('-');

  if (active_network_medium == NetworkMedium_t::WIFI)
  {
    addRowLabel(LabelType::LAST_DISCONNECT_REASON);
    addHtml(getValue(LabelType::LAST_DISC_REASON_STR));
    addRowLabelValue(LabelType::WIFI_STORED_SSID1);
    addRowLabelValue(LabelType::WIFI_STORED_SSID2);
  }

  addRowLabelValue(LabelType::STA_MAC);
  addRowLabelValue(LabelType::AP_MAC);
  html_TR();
}

#ifndef WEBSERVER_SYSINFO_MINIMAL
void handle_sysinfo_WiFiSettings() {
  addTableSeparator(F("WiFi Settings"), 2, 3);
  addRowLabelValue(LabelType::FORCE_WIFI_BG);
  addRowLabelValue(LabelType::RESTART_WIFI_LOST_CONN);
  addRowLabelValue(LabelType::FORCE_WIFI_NOSLEEP);
# ifdef SUPPORT_ARP
  addRowLabelValue(LabelType::PERIODICAL_GRAT_ARP);
# endif // ifdef SUPPORT_ARP
  addRowLabelValue(LabelType::CONNECTION_FAIL_THRESH);
#ifdef ESP8266 // TD-er: Disable setting TX power on ESP32 as it seems to cause issues on IDF4.4
  addRowLabelValue(LabelType::WIFI_TX_MAX_PWR);
  addRowLabelValue(LabelType::WIFI_CUR_TX_PWR);
  addRowLabelValue(LabelType::WIFI_SENS_MARGIN);
  addRowLabelValue(LabelType::WIFI_SEND_AT_MAX_TX_PWR);
#endif
  addRowLabelValue(LabelType::WIFI_NR_EXTRA_SCANS);
#ifdef USES_ESPEASY_NOW
  addRowLabelValue(LabelType::USE_ESPEASY_NOW);
  addRowLabelValue(LabelType::FORCE_ESPEASY_NOW_CHANNEL);
#endif
  addRowLabelValue(LabelType::WIFI_USE_LAST_CONN_FROM_RTC);
  addRowLabelValue(LabelType::WAIT_WIFI_CONNECT);
  addRowLabelValue(LabelType::SDK_WIFI_AUTORECONNECT);
}
#endif

void handle_sysinfo_Firmware() {
  addTableSeparator(F("Firmware"), 2, 3);

  addRowLabelValue_copy(LabelType::BUILD_DESC);
  addHtml(' ');
  addHtml(F(BUILD_NOTES));

  addRowLabelValue_copy(LabelType::SYSTEM_LIBRARIES);
  addRowLabelValue_copy(LabelType::GIT_BUILD);
  addRowLabelValue_copy(LabelType::PLUGIN_COUNT);
  addHtml(' ');
  addHtml(getPluginDescriptionString());

  addRowLabel(F("Build Origin"));
  addHtml(get_build_origin());
  addRowLabelValue_copy(LabelType::BUILD_TIME);
  addRowLabelValue_copy(LabelType::BINARY_FILENAME);
  addRowLabelValue_copy(LabelType::BUILD_PLATFORM);
  addRowLabelValue_copy(LabelType::GIT_HEAD);
}

#ifndef WEBSERVER_SYSINFO_MINIMAL
void handle_sysinfo_SystemStatus() {
  addTableSeparator(F("System Status"), 2, 3);

  // Actual Loglevel
  addRowLabelValue(LabelType::SYSLOG_LOG_LEVEL);
  addRowLabelValue(LabelType::SERIAL_LOG_LEVEL);
  addRowLabelValue(LabelType::WEB_LOG_LEVEL);
  # if FEATURE_SD
  addRowLabelValue(LabelType::SD_LOG_LEVEL);
  # endif // if FEATURE_SD

  if (Settings.EnableClearHangingI2Cbus()) {
    addRowLabelValue(LabelType::I2C_BUS_STATE);
    addRowLabelValue(LabelType::I2C_BUS_CLEARED_COUNT);
  }
}
#endif

#ifndef WEBSERVER_SYSINFO_MINIMAL
void handle_sysinfo_NetworkServices() {
  addTableSeparator(F("Network Services"), 2, 3);

  addRowLabel(F("Network Connected"));
  addEnabled(NetworkConnected());

  addRowLabel(F("NTP Initialized"));
  addEnabled(statusNTPInitialized);

  #if FEATURE_MQTT
  if (validControllerIndex(firstEnabledMQTT_ControllerIndex())) {
    addRowLabel(F("MQTT Client Connected"));
    addEnabled(MQTTclient_connected);
  }
  #endif
}
#endif

#ifndef WEBSERVER_SYSINFO_MINIMAL
void handle_sysinfo_ESP_Board() {
  addTableSeparator(F("ESP Board"), 2, 3);


  addRowLabel(LabelType::ESP_CHIP_ID);
  {
    addHtmlInt(getChipId());
    addHtml(' ', '(');
    addHtml(formatToHex(getChipId(), 6));
    addHtml(')');
  }

  addRowLabelValue(LabelType::ESP_CHIP_FREQ);
  addHtml(F(" MHz"));
#ifdef ESP32
  addRowLabelValue(LabelType::ESP_CHIP_XTAL_FREQ);
  addHtml(F(" MHz"));
  addRowLabelValue(LabelType::ESP_CHIP_APB_FREQ);
  addHtml(F(" MHz"));
#endif

  addRowLabelValue(LabelType::ESP_CHIP_MODEL);

  # if defined(ESP32)
  addRowLabelValue(LabelType::ESP_CHIP_REVISION);
  # endif // if defined(ESP32)
  addRowLabelValue(LabelType::ESP_CHIP_CORES);
  addRowLabelValue(LabelType::ESP_BOARD_NAME);
}
#endif

#ifndef WEBSERVER_SYSINFO_MINIMAL
void handle_sysinfo_Storage() {
  addTableSeparator(F("Storage"), 2, 3);

  uint32_t flashChipId = getFlashChipId();

  if (flashChipId != 0) {
    addRowLabel(LabelType::FLASH_CHIP_ID);


    // Set to HEX may be something like 0x1640E0.
    // Where manufacturer is 0xE0 and device is 0x4016.
    addHtml(F("Vendor: "));
    addHtml(getValue(LabelType::FLASH_CHIP_VENDOR));

    if (flashChipVendorPuya())
    {
      addHtml(F(" (PUYA"));

      if (puyaSupport()) {
        addHtml(F(", supported"));
      } else {
        addHtml(F(HTML_SYMBOL_WARNING));
      }
      addHtml(')');
    }
    addHtml(F(" Device: "));
    addHtml(getValue(LabelType::FLASH_CHIP_MODEL));
  }
  const uint32_t realSize = getFlashRealSizeInBytes();
  const uint32_t ideSize  = ESP.getFlashChipSize();

  addRowLabel(LabelType::FLASH_CHIP_REAL_SIZE);
  addHtmlInt(realSize / 1024);
  addHtml(F(" kB"));

  addRowLabel(LabelType::FLASH_IDE_SIZE);
  addHtmlInt(ideSize / 1024);
  addHtml(F(" kB"));

  addRowLabel(LabelType::FLASH_CHIP_SPEED);
  addHtmlInt(getFlashChipSpeed() / 1000000);
  addHtml(F(" MHz"));

  // Please check what is supported for the ESP32
  addRowLabel(LabelType::FLASH_IDE_SPEED);
  addHtmlInt(ESP.getFlashChipSpeed() / 1000000);
  addHtml(F(" MHz"));

  addRowLabelValue(LabelType::FLASH_IDE_MODE);

  addRowLabel(LabelType::FLASH_WRITE_COUNT);
  {
    addHtmlInt(RTC.flashDayCounter);
    addHtml(F(" daily / "));
    addHtmlInt(static_cast<int>(RTC.flashCounter));
    addHtml(F(" boot"));
  }

  {
    // FIXME TD-er: Must also add this for ESP32.
    addRowLabel(LabelType::SKETCH_SIZE);
    {
      addHtmlInt(getSketchSize() / 1024);
      addHtml(F(" kB ("));
      addHtmlInt(getFreeSketchSpace() / 1024);
      addHtml(F(" kB free)"));
    }

    uint32_t maxSketchSize;
    bool     use2step;
    # if defined(ESP8266)
    bool otaEnabled =
    # endif // if defined(ESP8266)
    OTA_possible(maxSketchSize, use2step);
    addRowLabel(LabelType::MAX_OTA_SKETCH_SIZE);
    {
      addHtmlInt(maxSketchSize / 1024);
      addHtml(F(" kB ("));
      addHtmlInt(maxSketchSize);
      addHtml(F(" bytes)"));
    }

    # if defined(ESP8266)
    addRowLabel(LabelType::OTA_POSSIBLE);
    addHtml(boolToString(otaEnabled));

    addRowLabel(LabelType::OTA_2STEP);
    addHtml(boolToString(use2step));
    # endif // if defined(ESP8266)
  }

  addRowLabel(LabelType::FS_SIZE);
  {
    addHtmlInt(SpiffsTotalBytes() / 1024);
    addHtml(F(" kB ("));
    addHtmlInt(SpiffsFreeSpace() / 1024);
    addHtml(F(" kB free)"));
  }
  # ifndef LIMIT_BUILD_SIZE
  addRowLabel(F("Page size"));
  addHtmlInt(SpiffsPagesize());

  addRowLabel(F("Block size"));
  addHtmlInt(SpiffsBlocksize());

  addRowLabel(F("Number of blocks"));
  addHtmlInt(SpiffsTotalBytes() / SpiffsBlocksize());

  {
  #  if defined(ESP8266)
    fs::FSInfo fs_info;
    ESPEASY_FS.info(fs_info);
    addRowLabel(F("Maximum open files"));
    addHtmlInt(fs_info.maxOpenFiles);

    addRowLabel(F("Maximum path length"));
    addHtmlInt(fs_info.maxPathLength);

  #  endif // if defined(ESP8266)
  }
  # endif // ifndef LIMIT_BUILD_SIZE

# ifndef BUILD_MINIMAL_OTA

  if (showSettingsFileLayout) {
    addTableSeparator(F("Settings Files"), 2, 3);
    html_TR_TD();
    addHtml(F("Layout Settings File"));
    html_TD();
    getConfig_dat_file_layout();
    html_TR_TD();
    html_TD();
    addHtml(F("(offset / size per item / index)"));

    for (int st = 0; st < static_cast<int>(SettingsType::Enum::SettingsType_MAX); ++st) {
      SettingsType::Enum settingsType = static_cast<SettingsType::Enum>(st);
      html_TR_TD();
      addHtml(SettingsType::getSettingsTypeString(settingsType));
      html_BR();
      addHtml(SettingsType::getSettingsFileName(settingsType));
      html_TD();
      getStorageTableSVG(settingsType);
    }
  }
# endif // ifndef BUILD_MINIMAL_OTA

  # ifdef ESP32
  addTableSeparator(F("Partitions"), 2, 3,
                    F("https://dl.espressif.com/doc/esp-idf/latest/api-guides/partition-tables.html"));

  addRowLabel(F("Data Partition Table"));

  //   TXBuffer += getPartitionTableHeader(F(" - "), F("<BR>"));
  //   TXBuffer += getPartitionTable(ESP_PARTITION_TYPE_DATA, F(" - "), F("<BR>"));
  getPartitionTableSVG(ESP_PARTITION_TYPE_DATA, 0x5856e6);

  addRowLabel(F("App Partition Table"));

  //   TXBuffer += getPartitionTableHeader(F(" - "), F("<BR>"));
  //   TXBuffer += getPartitionTable(ESP_PARTITION_TYPE_APP , F(" - "), F("<BR>"));
  getPartitionTableSVG(ESP_PARTITION_TYPE_APP, 0xab56e6);
  # endif // ifdef ESP32
}
#endif

#endif    // ifdef WEBSERVER_SYSINFO


#endif