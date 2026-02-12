#include "../WebServer/SysInfoPage.h"

#if defined(WEBSERVER_SYSINFO) || SHOW_SYSINFO_JSON

# include "../../ESPEasy-Globals.h"
# include "../../ESPEasy/net/ESPEasyNetwork.h"
# include "../../ESPEasy/net/Globals/ESPEasyWiFi.h"
# include "../../ESPEasy/net/Globals/ESPEasyWiFiEvent.h"
# include "../../ESPEasy/net/Globals/NetworkState.h"
# include "../../ESPEasy/net/Helpers/NWAccessControl.h"
# include "../../ESPEasy/net/Helpers/NW_info_writer.h"
#if FEATURE_ETHERNET
# include "../../ESPEasy/net/eth/ESPEasyEth.h"
#endif
# include "../../ESPEasy/net/wifi/ESPEasyWifi.h"
# include "../Commands/Diagnostic.h"
# include "../CustomBuild/CompiletimeDefines.h"
# include "../DataStructs/RTCStruct.h"
# include "../Globals/CRCValues.h"
# include "../Globals/ESPEasy_time.h"
# include "../Globals/RTC.h"
# include "../Globals/Settings.h"
# include "../Helpers/Convert.h"
# include "../Helpers/ESPEasyStatistics.h"
# include "../Helpers/ESPEasy_Storage.h"
# include "../Helpers/Hardware_device_info.h"
# include "../Helpers/KeyValueWriter_JSON.h"
# include "../Helpers/Memory.h"
# include "../Helpers/Misc.h"
# include "../Helpers/Networking.h"
# include "../Helpers/OTA.h"
# include "../Helpers/StringConverter.h"
# include "../Helpers/StringGenerator_GPIO.h"
# include "../Helpers/StringGenerator_System.h"
# include "../Helpers/StringProvider.h"
# include "../Static/WebStaticData.h"
# include "../WebServer/AccessControl.h"
# include "../WebServer/ESPEasy_WebServer.h"
# include "../WebServer/HTML_wrappers.h"
# include "../WebServer/KeyValueWriter_WebForm.h"
# include "../WebServer/Markup.h"
# include "../WebServer/Markup_Buttons.h"
# include "../WebServer/NetworkPage.h"

# if FEATURE_MQTT
#  include "../Globals/MQTT.h"
#  include "../ESPEasyCore/Controller.h" // For finding enabled MQTT controller
# endif // if FEATURE_MQTT


# if FEATURE_INTERNAL_TEMPERATURE
#  include "../Helpers/Hardware_temperature_sensor.h"
# endif

# ifdef ESP32
#  include <esp_partition.h>
# endif // ifdef ESP32


# if SHOW_SYSINFO_JSON

// ********************************************************************************
// Web Interface sysinfo page
// ********************************************************************************
void handle_sysinfo_json() {
#  ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_sysinfo"));
#  endif // ifndef BUILD_NO_RAM_TRACKER

  if (!isLoggedIn()) { return; }
  TXBuffer.startJsonStream();
  {
    KeyValueWriter_JSON mainLevelWriter(true);
    {

      auto writer = mainLevelWriter.createChild(F("general"));

      if (writer) {

        writer->write({ F("unit"),       Settings.Unit });
        writer->write({ F("time"),       node_time.getDateTimeString('-', ':', ' ') });
        writer->write({ F("uptime"),     getExtendedValue(LabelType::UPTIME) });
        writer->write({ F("cpu_load"),   getCPUload(), 2 });
#  if FEATURE_INTERNAL_TEMPERATURE
        writer->write({ F("cpu_temp"),   getInternalTemperature(), 2 });
#  endif
        writer->write({ F("loop_count"), getLoopCountPerSec() });
      }
    }
    int freeMem = ESP.getFreeHeap();
    {
      auto writer = mainLevelWriter.createChild(F("mem"));

      if (writer) {
        writer->write({ F("free"),   freeMem });
#  ifndef BUILD_NO_RAM_TRACKER
        writer->write({ F("low_ram"), lowestRAM });
        writer->write({ F("low_ram_fn"), lowestRAMfunction });
#  endif // ifndef BUILD_NO_RAM_TRACKER
        writer->write({ F("stack"),    getCurrentFreeStack() });
#  ifndef BUILD_NO_RAM_TRACKER
        writer->write({ F("low_stack"), lowestFreeStack });
        writer->write({ F("low_stack_fn"), lowestFreeStackfunction });
#  endif // ifndef BUILD_NO_RAM_TRACKER
      }
    }
    {
      auto writer = mainLevelWriter.createChild(F("boot"));

      if (writer) {
        writer->write({ F("last_cause"), getLastBootCauseString() });
        writer->write({ F("counter"), RTC.bootCounter });
        writer->write({ F("reset_reason"), getResetReasonString() });
      }
    }
    {
      auto writer = mainLevelWriter.createChild(F("wifi"));

      if (writer) {
        writer->write({ F("type"), toString(ESPEasy::net::wifi::getConnectionProtocol()) });
        writer->write({ F("rssi"), WiFi.RSSI() });
        writer->write({ F("dhcp"), useStaticIP() ? F("static") : F("DHCP") });
        writer->write({ F("ip"),   LabelType::IP_ADDRESS });
#  if FEATURE_USE_IPV6

        if (Settings.EnableIPv6()) {
          writer->write({ F("ip6_local"),  LabelType::IP6_LOCAL });
          writer->write({ F("ip6_global"), LabelType::IP6_GLOBAL });
        }
#  endif // if FEATURE_USE_IPV6

        writer->write({ F("subnet"),        LabelType::IP_SUBNET });
        writer->write({ F("gw"),            LabelType::GATEWAY });
        writer->write({ F("dns1"),          LabelType::DNS_1 });
        writer->write({ F("dns2"),          LabelType::DNS_2 });
        writer->write({ F("allowed_range"), ESPEasy::net::describeAllowedIPrange() });
        writer->write({ F("sta_mac"),       LabelType::STA_MAC });
        writer->write({ F("ap_mac"),        LabelType::AP_MAC });
        writer->write({ F("ssid"),          LabelType::SSID });
        writer->write({ F("bssid"),         LabelType::BSSID });
        writer->write({ F("channel"),       LabelType::CHANNEL });
        writer->write({ F("encryption"),    LabelType::ENCRYPTION_TYPE_STA });
        writer->write({ F("connected"),     LabelType::CONNECTED });
        writer->write({ F("ldr"),           LabelType::LAST_DISC_REASON_STR });
        writer->write({ F("reconnects"),    LabelType::NUMBER_RECONNECTS });
        writer->write({ F("ssid1"),         LabelType::WIFI_STORED_SSID1 });
        writer->write({ F("ssid2"),         LabelType::WIFI_STORED_SSID2 });
      }
    }
    {

#  if FEATURE_ETHERNET
      auto writer = mainLevelWriter.createChild(F("ethernet"));

      if (writer) {
        writer->write({ F("ethwifimode"),   LabelType::ETH_WIFI_MODE });
        writer->write({ F("ethconnected"),  LabelType::ETH_CONNECTED });
        writer->write({ F("ethchip"),       LabelType::ETH_CHIP });
        writer->write({ F("ethduplex"),     LabelType::ETH_DUPLEX });
        writer->write({ F("ethspeed"),      LabelType::ETH_SPEED });
        writer->write({ F("ethstate"),      LabelType::ETH_STATE });
        writer->write({ F("ethspeedstate"), LabelType::ETH_SPEED_STATE });
      }
#  endif // if FEATURE_ETHERNET
    }
    {
      auto writer = mainLevelWriter.createChild(F("firmware"));

      if (writer) {
        writer->write({ F("build"),         getSystemBuildString() });
        writer->write({ F("notes"),         F(BUILD_NOTES) });
        writer->write({ F("libraries"),     getSystemLibraryString() });
        writer->write({ F("git_version"),   LabelType::GIT_BUILD });
        writer->write({ F("plugins"),       getPluginDescriptionString() });
        writer->write({ F("md5"),           formatToHex_array((const uint8_t *)CRCValues.compileTimeMD5, sizeof(CRCValues.compileTimeMD5)) });
        writer->write({ F("md5_check"),     CRCValues.checkPassed() });
        writer->write({ F("build_time"),    get_build_time() });
        writer->write({ F("filename"),      LabelType::BINARY_FILENAME });
        writer->write({ F("build_platform"), LabelType::BUILD_PLATFORM });
        writer->write({ F("git_head"),      LabelType::GIT_HEAD });
#  ifdef CONFIGURATION_CODE
        writer->write({ F("configuration_code"), LabelType::CONFIGURATION_CODE_LBL });
#  endif // ifdef CONFIGURATION_CODE
      }

    }
    {
      auto writer = mainLevelWriter.createChild(F("esp"));

      if (writer) {
        writer->write({ F("chip_id"),   LabelType::ESP_CHIP_ID });
        writer->write({ F("cpu"),       LabelType::ESP_CHIP_FREQ });
#  ifdef ESP32
        writer->write({ F("xtal_freq"), LabelType::ESP_CHIP_XTAL_FREQ });
        writer->write({ F("abp_freq"),  LabelType::ESP_CHIP_APB_FREQ });
#  endif // ifdef ESP32
        writer->write({ F("board"),     LabelType::BOARD_NAME });
      }
    }
    {
      auto writer = mainLevelWriter.createChild(F("storage"));

      if (writer) {
        // Set to HEX may be something like 0x1640E0.
        // Where manufacturer is 0xE0 and device is 0x4016.
        writer->write({ F("chip_id"), LabelType::FLASH_CHIP_ID });

        if (flashChipVendorPuya()) {
          if (puyaSupport()) {
            writer->write({ F("vendor"), F("puya, supported") });
          } else {
            writer->write({ F("vendor"), F("puya, error") });
          }
        } else {
          writer->write({ F("vendor"), LabelType::FLASH_CHIP_VENDOR });
        }
        writer->write({ F("device"),      LabelType::FLASH_CHIP_MODEL });
        writer->write({ F("real_size"),  getFlashRealSizeInBytes() / 1024 });
        writer->write({ F("ide_size"),   ESP.getFlashChipSize() / 1024 });

        // Please check what is supported for the ESP32
        writer->write({ F("flash_speed"), LabelType::FLASH_CHIP_SPEED });

        writer->write({ F("mode"), getFlashChipMode() });

        writer->write({ F("writes"),        RTC.flashDayCounter });
        writer->write({ F("flash_counter"), RTC.flashCounter });
        writer->write({ F("sketch_size"),   getSketchSize() / 1024 });
        writer->write({ F("sketch_free"),   getFreeSketchSpace() / 1024 });

        writer->write({ F("spiffs_size"),   SpiffsTotalBytes() / 1024 });
        writer->write({ F("spiffs_free"),   SpiffsFreeSpace() / 1024 });
      }
    }

  }
  TXBuffer.endStream();
}

# endif // SHOW_SYSINFO_JSON

# ifdef WEBSERVER_SYSINFO

void handle_sysinfo() {
  #  ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_sysinfo"));
  #  endif // ifndef BUILD_NO_RAM_TRACKER

  if (!isLoggedIn()) { return; }
  navMenuIndex = MENU_INDEX_TOOLS;
#ifdef WEBSERVER_GITHUB_COPY
  html_reset_copyTextCounter();
#endif
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate(_HEAD);

  addHtml(printWebString);
  addHtml(F("<form>"));

  // the table header
  html_table_class_normal();


  #  ifdef WEBSERVER_GITHUB_COPY

  // Not using addFormHeader() to get the copy button on the same header line as 2nd column
  html_TR();
  html_table_header(F("System Info"), 225);
  addHtml(F("<TH>")); // Needed to get the copy button on the same header line.
  addCopyButton(F("copyText"), F("\\n"), F("Copy info to clipboard"));

  TXBuffer.addFlashString((PGM_P)FPSTR(githublogo));
  serve_JS(JSfiles_e::GitHubClipboard);

  #  else // ifdef WEBSERVER_GITHUB_COPY
  addFormHeader(F("System Info"));

  #  endif // ifdef WEBSERVER_GITHUB_COPY

  handle_sysinfo_basicInfo();

#  ifndef WEBSERVER_SYSINFO_MINIMAL
  handle_sysinfo_memory();
#  endif

  handle_sysinfo_Network();

#  ifndef WEBSERVER_SYSINFO_MINIMAL
  handle_sysinfo_WiFiSettings();
#  endif

  handle_sysinfo_NetworkAdapters();

  handle_sysinfo_Firmware();

#  ifndef WEBSERVER_SYSINFO_MINIMAL
  handle_sysinfo_SystemStatus();

  handle_sysinfo_NetworkServices();

  handle_sysinfo_ESP_Board();

  handle_sysinfo_Storage();
#  endif // ifndef WEBSERVER_SYSINFO_MINIMAL


  html_end_table();
  html_end_form();
  sendHeadandTail_stdtemplate(_TAIL);
  TXBuffer.endStream();
}

void handle_sysinfo_basicInfo() {
  static const LabelType::Enum labels[] PROGMEM =
  {
    LabelType::UNIT_NR,

    LabelType::LOCAL_TIME,
    #  if FEATURE_EXT_RTC

    LabelType::EXT_RTC_UTC_TIME,
    #  endif // if FEATURE_EXT_RTC
    LabelType::TIME_SOURCE,
    LabelType::TIME_WANDER,
    LabelType::UPTIME,
    LabelType::LOAD_PCT,
#  if FEATURE_INTERNAL_TEMPERATURE
    LabelType::INTERNAL_TEMPERATURE,
#  endif

    LabelType::CPU_ECO_MODE,

    LabelType::RESET_REASON,
    LabelType::LAST_TASK_BEFORE_REBOOT,
    LabelType::SW_WD_COUNT,
    LabelType::BOOT_TYPE,

    LabelType::MAX_LABEL
  };

  addRowLabelValues(labels);
}

#  ifndef WEBSERVER_SYSINFO_MINIMAL

void handle_sysinfo_memory() {
  addTableSeparator(F("Memory"), 2, 3);

  static const LabelType::Enum labels[] PROGMEM =
  {
#   ifdef ESP32
    LabelType::HEAP_SIZE,
    LabelType::HEAP_MIN_FREE,
#   endif // ifdef ESP32

    LabelType::FREE_MEM,
#   if defined(CORE_POST_2_5_0) || defined(ESP32)
#    ifndef LIMIT_BUILD_SIZE
    LabelType::HEAP_MAX_FREE_BLOCK,
#    endif // ifndef LIMIT_BUILD_SIZE
#   endif   // if defined(CORE_POST_2_5_0) || defined(ESP32)
#   if defined(CORE_POST_2_5_0)
#    ifndef LIMIT_BUILD_SIZE
    LabelType::HEAP_FRAGMENTATION,
#    endif // ifndef LIMIT_BUILD_SIZE

#    ifdef USE_SECOND_HEAP
    LabelType::FREE_HEAP_IRAM,
#    endif

#   endif // if defined(CORE_POST_2_5_0)
    LabelType::FREE_STACK,

#   if defined(ESP32) && defined(BOARD_HAS_PSRAM)

    LabelType::PSRAM_SIZE,

    LabelType::PSRAM_FREE,
    LabelType::PSRAM_MIN_FREE,
    LabelType::PSRAM_MAX_FREE_BLOCK,
#   endif // if defined(ESP32) && defined(BOARD_HAS_PSRAM)
    LabelType::MAX_LABEL
  };
  addRowLabelValues(labels);
}

#  endif // ifndef WEBSERVER_SYSINFO_MINIMAL


void handle_sysinfo_Network() {
  addTableSeparator(F("Network"), 2, 3);

  {
    static const LabelType::Enum labels[] PROGMEM =
    {
#  if FEATURE_ETHERNET || defined(USES_ESPEASY_NOW)
      LabelType::ETH_WIFI_MODE,
#  endif
      LabelType::IP_CONFIG,
      LabelType::IP_ADDRESS_SUBNET,
#  if FEATURE_USE_IPV6
      LabelType::IP6_LOCAL,
      LabelType::IP6_GLOBAL,

      //      LabelType::IP6_ALL_ADDRESSES,
#  endif // if FEATURE_USE_IPV6
      LabelType::GATEWAY,
      LabelType::CLIENT_IP,
      LabelType::DNS,
      LabelType::ALLOWED_IP_RANGE,
      LabelType::CONNECTED,
      LabelType::NUMBER_RECONNECTS,

      LabelType::MAX_LABEL
    };

    addRowLabelValues(labels);
  }
}

void handle_sysinfo_NetworkAdapters() {
#ifndef LIMIT_BUILD_SIZE
  for (ESPEasy::net::networkIndex_t x = 0; x < NETWORK_MAX; ++x)
  {
    if (Settings.getNetworkEnabled(x)) {
      auto pluginID = Settings.getNWPluginID_for_network(x);

      if (pluginID != ESPEasy::net::INVALID_NW_PLUGIN_ID) {
        KeyValueWriter_WebForm writer(
          strformat(F("%s (%d)"),
                    getNWPluginNameFromNWPluginID(pluginID).c_str(),
                    x + 1));
#  ifdef WEBSERVER_NETWORK
#   ifdef ESP32
        ESPEasy::net::write_NetworkAdapterFlags(x, &writer);
        ESPEasy::net::write_IP_config(x, &writer);
#   endif // ifdef ESP32
#  endif // ifdef WEBSERVER_NETWORK
        ESPEasy::net::write_NetworkConnectionInfo(x, &writer);
      }
    }
  }
#endif

  html_TR();
}

#  ifndef WEBSERVER_SYSINFO_MINIMAL

void handle_sysinfo_WiFiSettings() {
  if (!ESPEasy::net::wifi::WiFiConnected()) return;
  addTableSeparator(F("WiFi Settings"), 2, 3);

  static const LabelType::Enum labels[] PROGMEM =
  {
    LabelType::FORCE_WIFI_BG,
    LabelType::RESTART_WIFI_LOST_CONN,
    LabelType::FORCE_WIFI_NOSLEEP,
#   ifdef SUPPORT_ARP
    LabelType::PERIODICAL_GRAT_ARP,
#   endif // ifdef SUPPORT_ARP
    LabelType::CONNECTION_FAIL_THRESH,
#   if FEATURE_SET_WIFI_TX_PWR
    LabelType::WIFI_TX_MAX_PWR,
    LabelType::WIFI_CUR_TX_PWR,
    LabelType::WIFI_SENS_MARGIN,
    LabelType::WIFI_SEND_AT_MAX_TX_PWR,
#   endif // if FEATURE_SET_WIFI_TX_PWR
    LabelType::WIFI_NR_RECONNECT_ATTEMPTS,
    LabelType::WIFI_MAX_UPTIME_AUTO_START_AP,
    LabelType::WIFI_AP_MINIMAL_ON_TIME,
#   ifdef USES_ESPEASY_NOW
    LabelType::USE_ESPEASY_NOW,
    LabelType::FORCE_ESPEASY_NOW_CHANNEL,
#   endif // ifdef USES_ESPEASY_NOW
    LabelType::WIFI_USE_LAST_CONN_FROM_RTC,
#   ifndef ESP32
    LabelType::WAIT_WIFI_CONNECT,
#   endif
#   ifdef ESP32
    LabelType::WIFI_PASSIVE_SCAN,
#   endif
    LabelType::HIDDEN_SSID_SLOW_CONNECT,
    LabelType::CONNECT_HIDDEN_SSID,
    LabelType::SDK_WIFI_AUTORECONNECT,
#   if FEATURE_USE_IPV6
    LabelType::ENABLE_IPV6,
#   endif

    LabelType::MAX_LABEL
  };

  addRowLabelValues(labels);
}

#  endif // ifndef WEBSERVER_SYSINFO_MINIMAL

void handle_sysinfo_Firmware() {
  addTableSeparator(F("Firmware"), 2, 3);

  static const LabelType::Enum labels[] PROGMEM =
  {
    LabelType::BUILD_DESC,
    LabelType::SYSTEM_LIBRARIES,
#  ifdef ESP32
    LabelType::ESP_IDF_SDK_VERSION,
#  endif
    LabelType::GIT_BUILD,
    LabelType::PLUGIN_COUNT,
    LabelType::PLUGIN_DESCRIPTION,
    LabelType::BUILD_ORIGIN,
    LabelType::BUILD_TIME,
    LabelType::BINARY_FILENAME,
    LabelType::BUILD_PLATFORM,
    LabelType::GIT_HEAD,
  #  ifdef CONFIGURATION_CODE
    LabelType::CONFIGURATION_CODE_LBL,
  #  endif  // ifdef CONFIGURATION_CODE

    LabelType::MAX_LABEL
  };

  addRowLabelValues(labels);
}

#  ifndef WEBSERVER_SYSINFO_MINIMAL

void handle_sysinfo_SystemStatus() {
  addTableSeparator(F("System Status"), 2, 3);

  static const LabelType::Enum labels[] PROGMEM =
  {
    // Actual Loglevel
#if FEATURE_SYSLOG
    LabelType::SYSLOG_LOG_LEVEL,
#endif
    LabelType::SERIAL_LOG_LEVEL,
# ifdef WEBSERVER_LOG
    LabelType::WEB_LOG_LEVEL,
#endif
#   if FEATURE_SD
    LabelType::SD_LOG_LEVEL,
#   endif // if FEATURE_SD

    LabelType::ENABLE_SERIAL_PORT_CONSOLE,
    LabelType::CONSOLE_SERIAL_PORT,
#   if USES_ESPEASY_CONSOLE_FALLBACK_PORT
    LabelType::CONSOLE_FALLBACK_TO_SERIAL0,
    LabelType::CONSOLE_FALLBACK_PORT,
#   endif // if USES_ESPEASY_CONSOLE_FALLBACK_PORT
#   if FEATURE_CLEAR_I2C_STUCK
    LabelType::I2C_BUS_STATE,
    LabelType::I2C_BUS_CLEARED_COUNT,
#   endif // if FEATURE_CLEAR_I2C_STUCK

    LabelType::MAX_LABEL
  };

  addRowLabelValues(labels);
}

#  endif // ifndef WEBSERVER_SYSINFO_MINIMAL

#  ifndef WEBSERVER_SYSINFO_MINIMAL

void handle_sysinfo_NetworkServices() {
  addTableSeparator(F("Network Services"), 2, 3);

  addRowLabel(F("Network Connected"));
  addEnabled(ESPEasy::net::NetworkConnected());

  addRowLabel(F("NTP Initialized"));
  addEnabled(statusNTPInitialized);

  #   if FEATURE_MQTT

  if (validControllerIndex(firstEnabledMQTT_ControllerIndex())) {
    addRowLabel(F("MQTT Client Connected"));
    addEnabled(MQTTclient_connected);
  }
  #   endif // if FEATURE_MQTT
}

#  endif // ifndef WEBSERVER_SYSINFO_MINIMAL

#  ifndef WEBSERVER_SYSINFO_MINIMAL

void handle_sysinfo_ESP_Board() {
  addTableSeparator(F("ESP Board"), 2, 3);

  static const LabelType::Enum labels[] PROGMEM =
  {
    LabelType::ESP_CHIP_ID,
    LabelType::ESP_CHIP_FREQ,
#   ifdef ESP32
    LabelType::ESP_CHIP_XTAL_FREQ,
    LabelType::ESP_CHIP_APB_FREQ,
#   endif // ifdef ESP32

    LabelType::ESP_CHIP_MODEL,
#   if defined(ESP32)
    LabelType::ESP_CHIP_REVISION,
#   endif // if defined(ESP32)
    LabelType::ESP_CHIP_CORES,
    LabelType::BOARD_NAME,

    LabelType::MAX_LABEL
  };

  addRowLabelValues(labels);

#   if defined(ESP32)
  addRowLabel(F("ESP Chip Features"));
  addHtml(getChipFeaturesString());
#   endif // if defined(ESP32)
}

#  endif // ifndef WEBSERVER_SYSINFO_MINIMAL

#  ifndef WEBSERVER_SYSINFO_MINIMAL

void handle_sysinfo_Storage() {
  addTableSeparator(F("Storage"), 2, 3);

  static const LabelType::Enum labels[] PROGMEM =
  {
    LabelType::FLASH_CHIP_ID,
    LabelType::FLASH_CHIP_VENDOR,
    LabelType::FLASH_CHIP_MODEL,
    LabelType::FLASH_CHIP_REAL_SIZE,
    LabelType::FLASH_IDE_SIZE,
    LabelType::FLASH_CHIP_SPEED,
    LabelType::FLASH_IDE_SPEED,
    LabelType::FLASH_IDE_MODE,
    LabelType::FLASH_WRITE_COUNT,
    LabelType::SKETCH_SIZE,
    LabelType::MAX_OTA_SKETCH_SIZE,
#   ifdef ESP8266
    LabelType::OTA_POSSIBLE,
    LabelType::OTA_2STEP,
#   endif // ifdef ESP8266
    LabelType::FS_SIZE,
    LabelType::FS_FREE,

    LabelType::MAX_LABEL
  };

  addRowLabelValues(labels);

  #   ifndef LIMIT_BUILD_SIZE
  addRowLabel(F("Page size"));
  addHtmlInt(SpiffsPagesize());
#if FEATURE_TASKVALUE_UNIT_OF_MEASURE
  addUnit(getFormUnit(LabelType::FREE_MEM)); // FIXME TD-er: Add labeltype
#endif
  addRowLabel(F("Block size"));
  addHtmlInt(SpiffsBlocksize());
#if FEATURE_TASKVALUE_UNIT_OF_MEASURE
  addUnit(getFormUnit(LabelType::FREE_MEM)); // FIXME TD-er: Add labeltype
#endif

  addRowLabel(F("Number of blocks"));
  addHtmlInt(SpiffsTotalBytes() / SpiffsBlocksize());

  {
  #    if defined(ESP8266)
    fs::FSInfo fs_info;
    ESPEASY_FS.info(fs_info);
    addRowLabel(F("Maximum open files"));
    addHtmlInt(fs_info.maxOpenFiles);

    addRowLabel(F("Maximum path length"));
    addHtmlInt(fs_info.maxPathLength);

  #    endif // if defined(ESP8266)
  }
  #   endif // ifndef LIMIT_BUILD_SIZE

#   if FEATURE_CHART_STORAGE_LAYOUT

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
      const SettingsType::Enum settingsType = static_cast<SettingsType::Enum>(st);
      #    if !FEATURE_NOTIFIER

      if (settingsType == SettingsType::Enum::NotificationSettings_Type) {
        continue;
      }
      #    endif // if !FEATURE_NOTIFIER
      html_TR_TD();
      addHtml(SettingsType::getSettingsTypeString(settingsType));
      html_BR();
      addHtml(SettingsType::getSettingsFileName(settingsType));
      html_TD();
      getStorageTableSVG(settingsType);
      delay(1);
    }
  }

  #    ifdef ESP32
  addTableSeparator(F("Partitions"), 2, 3,
                    F("https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/partition-tables.html"));

  addRowLabel(F("Data Partition Table"));

  //   TXBuffer += getPartitionTableHeader(F(" - "), F("<BR>"));
  //   TXBuffer += getPartitionTable(ESP_PARTITION_TYPE_DATA, F(" - "), F("<BR>"));
  getPartitionTableSVG(ESP_PARTITION_TYPE_DATA, 0x5856e6);

  addRowLabel(F("App Partition Table"));

  //   TXBuffer += getPartitionTableHeader(F(" - "), F("<BR>"));
  //   TXBuffer += getPartitionTable(ESP_PARTITION_TYPE_APP , F(" - "), F("<BR>"));
  getPartitionTableSVG(ESP_PARTITION_TYPE_APP, 0xab56e6);
  #    endif // ifdef ESP32

  #    ifdef ESP8266
  addTableSeparator(F("Partitions"), 2, 3);

  addRowLabel(F("Partition Table"));

  getPartitionTableSVG();
  #    endif // ifdef ESP8266
#   endif // if FEATURE_CHART_STORAGE_LAYOUT
}

#  endif // ifndef WEBSERVER_SYSINFO_MINIMAL

# endif    // ifdef WEBSERVER_SYSINFO


#endif // if defined(WEBSERVER_SYSINFO) || SHOW_SYSINFO_JSON
