#include "src/Globals/RTC.h"
#include "src/DataStructs/RTCStruct.h"
#include "src/Globals/CRCValues.h"
#include "src/Static/WebStaticData.h"

#ifdef WEBSERVER_NEW_UI

// ********************************************************************************
// Web Interface sysinfo page
// ********************************************************************************
void handle_sysinfo_json() {
  checkRAM(F("handle_sysinfo"));

  if (!isLoggedIn()) { return; }
  TXBuffer.startJsonStream();
  json_init();
  json_open();
  json_open(false, F("general"));
  json_number(F("unit"), String(Settings.Unit));
  json_prop(F("time"),   getDateTimeString('-', ':', ' '));
  json_prop(F("uptime"), getExtendedValue(LabelType::UPTIME));
  json_number(F("cpu_load"),   String(getCPUload()));
  json_number(F("loop_count"), String(getLoopCountPerSec()));
  json_close();

  int freeMem = ESP.getFreeHeap();
  json_open(false, F("mem"));
  json_number(F("free"),    String(freeMem));
  json_number(F("low_ram"), String(lowestRAM));
  json_prop(F("low_ram_fn"), String(lowestRAMfunction));
  json_number(F("stack"),     String(getCurrentFreeStack()));
  json_number(F("low_stack"), String(lowestFreeStack));
  json_prop(F("low_stack_fn"), lowestFreeStackfunction);
  json_close();
  json_open(false, F("boot"));
  json_prop(F("last_cause"), getLastBootCauseString());
  json_number(F("counter"), String(RTC.bootCounter));
  json_prop(F("reset_reason"), getResetReasonString());
  json_close();
  json_open(false, F("wifi"));

  # if defined(ESP8266)
  byte PHYmode = wifi_get_phy_mode();
  # endif // if defined(ESP8266)
  # if defined(ESP32)
  byte PHYmode = 3; // wifi_get_phy_mode();
  # endif // if defined(ESP32)

  switch (PHYmode)
  {
    case 1:
      json_prop(F("type"), F("802.11B"));
      break;
    case 2:
      json_prop(F("type"), F("802.11G"));
      break;
    case 3:
      json_prop(F("type"), F("802.11N"));
      break;
  }
  json_number(F("rssi"), String(WiFi.RSSI()));
  json_prop(F("dhcp"),          useStaticIP() ? getLabel(LabelType::IP_CONFIG_STATIC) : getLabel(LabelType::IP_CONFIG_DYNAMIC));
  json_prop(F("ip"),            formatIP(WiFi.localIP()));
  json_prop(F("subnet"),        formatIP(WiFi.subnetMask()));
  json_prop(F("gw"),            formatIP(WiFi.gatewayIP()));
  json_prop(F("dns1"),          formatIP(WiFi.dnsIP(0)));
  json_prop(F("dns2"),          formatIP(WiFi.dnsIP(1)));
  json_prop(F("allowed_range"), describeAllowedIPrange());


  uint8_t  mac[]   = { 0, 0, 0, 0, 0, 0 };
  uint8_t *macread = WiFi.macAddress(mac);
  char     macaddress[20];
  formatMAC(macread, macaddress);

  json_prop(F("sta_mac"), macaddress);

  macread = WiFi.softAPmacAddress(mac);
  formatMAC(macread, macaddress);

  json_prop(F("ap_mac"), macaddress);
  json_prop(F("ssid"),   WiFi.SSID());
  json_prop(F("bssid"),  WiFi.BSSIDstr());
  json_number(F("channel"), String(WiFi.channel()));
  json_prop(F("connected"), format_msec_duration(timeDiff(lastConnectMoment, millis())));
  json_prop(F("ldr"),       getLastDisconnectReason());
  json_number(F("reconnects"), String(wifi_reconnects));
  json_close();

  json_open(false, F("firmware"));
  json_prop(F("build"),       String(BUILD));
  json_prop(F("notes"),       F(BUILD_NOTES));
  json_prop(F("libraries"),   getSystemLibraryString());
  json_prop(F("git_version"), F(BUILD_GIT));
  json_prop(F("plugins"),     getPluginDescriptionString());
  json_prop(F("md5"),         String(CRCValues.compileTimeMD5[0], HEX));
  json_number(F("md5_check"), String(CRCValues.checkPassed()));
  json_prop(F("build_time"), String(CRCValues.compileTime));
  json_prop(F("filename"),   String(CRCValues.binaryFilename));
  json_close();

  json_open(false, F("esp"));

  # if defined(ESP8266)
  json_prop(F("chip_id"), String(ESP.getChipId(), HEX));
  json_number(F("cpu"), String(ESP.getCpuFreqMHz()));
  # endif // if defined(ESP8266)
  # if defined(ESP32)


  uint64_t chipid  = ESP.getEfuseMac(); // The chip ID is essentially its MAC address(length: 6 bytes).
  uint32_t ChipId1 = (uint16_t)(chipid >> 32);
  String   espChipIdS(ChipId1, HEX);
  espChipIdS.toUpperCase();

  json_prop(F("chip_id"), espChipIdS);
  json_prop(F("cpu"),     String(ESP.getCpuFreqMHz()));

  String espChipIdS1(ChipId1, HEX);
  espChipIdS1.toUpperCase();
  json_prop(F("chip_id1"), espChipIdS1);

  # endif // if defined(ESP32)
  # ifdef ARDUINO_BOARD
  json_prop(F("board"), ARDUINO_BOARD);
  # endif // ifdef ARDUINO_BOARD
  json_close();
  json_open(false, F("storage"));

  # if defined(ESP8266)
  uint32_t flashChipId = ESP.getFlashChipId();

  // Set to HEX may be something like 0x1640E0.
  // Where manufacturer is 0xE0 and device is 0x4016.
  json_number(F("chip_id"), String(flashChipId));

  if (flashChipVendorPuya())
  {
    if (puyaSupport()) {
      json_prop(F("vendor"), F("puya, supported"));
    } else {
      json_prop(F("vendor"), F("puya, error"));
    }
  }
  uint32_t flashDevice = (flashChipId & 0xFF00) | ((flashChipId >> 16) & 0xFF);
  json_number(F("device"),    String(flashDevice));
  # endif // if defined(ESP8266)
  json_number(F("real_size"), String(getFlashRealSizeInBytes() / 1024));
  json_number(F("ide_size"),  String(ESP.getFlashChipSize() / 1024));

  // Please check what is supported for the ESP32
  # if defined(ESP8266)
  json_number(F("flash_speed"), String(ESP.getFlashChipSpeed() / 1000000));

  FlashMode_t ideMode = ESP.getFlashChipMode();

  switch (ideMode) {
    case FM_QIO:   json_prop(F("mode"), F("QIO"));  break;
    case FM_QOUT:  json_prop(F("mode"), F("QOUT")); break;
    case FM_DIO:   json_prop(F("mode"), F("DIO"));  break;
    case FM_DOUT:  json_prop(F("mode"), F("DOUT")); break;
    default:
      json_prop(F("mode"), getUnknownString()); break;
  }
  # endif // if defined(ESP8266)

  json_number(F("writes"),        String(RTC.flashDayCounter));
  json_number(F("flash_counter"), String(RTC.flashCounter));
  json_number(F("sketch_size"),   String(ESP.getSketchSize() / 1024));
  json_number(F("sketch_free"),   String(ESP.getFreeSketchSpace() / 1024));

  json_number(F("spiffs_size"),   String(SpiffsTotalBytes() / 1024));
  json_number(F("spiffs_free"),   String(SpiffsFreeSpace() / 1024));
  json_close();
  json_close();

  TXBuffer.endStream();
}

#endif // WEBSERVER_NEW_UI

void handle_sysinfo() {
  checkRAM(F("handle_sysinfo"));

  if (!isLoggedIn()) { return; }
  navMenuIndex = MENU_INDEX_TOOLS;
  html_reset_copyTextCounter();
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate();

  TXBuffer += printWebString;
  TXBuffer += F("<form>");

  // the table header
  html_table_class_normal();

  // Not using addFormHeader() to get the copy button on the same header line as 2nd column
  html_TR();
  html_table_header(F("System Info"), 225);
  TXBuffer += "<TH>"; // Needed to get the copy button on the same header line.
  addCopyButton(F("copyText"), F("\\n"), F("Copy info to clipboard"));

  TXBuffer += githublogo;
  html_add_script(false);
  TXBuffer += DATA_GITHUB_CLIPBOARD_JS;
  html_add_script_end();

  handle_sysinfo_basicInfo();

  handle_sysinfo_Network();

  handle_sysinfo_WiFiSettings();

  handle_sysinfo_Firmware();

  handle_sysinfo_SystemStatus();

  handle_sysinfo_ESP_Board();

  handle_sysinfo_Storage();


  html_end_table();
  html_end_form();
  sendHeadandTail_stdtemplate(true);
  TXBuffer.endStream();
}

void handle_sysinfo_basicInfo() {
  addRowLabelValue(LabelType::UNIT_NR);

  if (systemTimePresent())
  {
    addRowLabelValue(LabelType::LOCAL_TIME);
  }

  addRowLabel(getLabel(LabelType::UPTIME));
  {
    TXBuffer += getExtendedValue(LabelType::UPTIME);
  }

  addRowLabel(getLabel(LabelType::LOAD_PCT));

  if (wdcounter > 0)
  {
    TXBuffer += getCPUload();
    TXBuffer += F("% (LC=");
    TXBuffer += getLoopCountPerSec();
    TXBuffer += ')';
  }
  addRowLabelValue(LabelType::CPU_ECO_MODE);

  int freeMem = ESP.getFreeHeap();
  addRowLabel(F("Free Mem"));
  TXBuffer += freeMem;
  TXBuffer += " (";
  TXBuffer += lowestRAM;
  TXBuffer += F(" - ");
  TXBuffer += lowestRAMfunction;
  TXBuffer += ')';
  addRowLabel(F("Free Stack"));
  TXBuffer += getCurrentFreeStack();
  TXBuffer += " (";
  TXBuffer += lowestFreeStack;
  TXBuffer += F(" - ");
  TXBuffer += lowestFreeStackfunction;
  TXBuffer += ')';
#ifdef CORE_POST_2_5_0
  addRowLabelValue(LabelType::HEAP_MAX_FREE_BLOCK);
  addRowLabelValue(LabelType::HEAP_FRAGMENTATION);
  TXBuffer += '%';
#endif // ifdef CORE_POST_2_5_0


  addRowLabel(F("Boot"));
  TXBuffer += getLastBootCauseString();
  TXBuffer += " (";
  TXBuffer += RTC.bootCounter;
  TXBuffer += ')';
  addRowLabelValue(LabelType::RESET_REASON);
  addRowLabelValue(LabelType::LAST_TASK_BEFORE_REBOOT);
  addRowLabelValue(LabelType::SW_WD_COUNT);
}

void handle_sysinfo_Network() {
  addTableSeparator(F("Network"), 2, 3, F("Wifi"));

  if (WiFiConnected())
  {
    addRowLabel(F("Wifi"));
    #if defined(ESP8266)
    byte PHYmode = wifi_get_phy_mode();
    #endif // if defined(ESP8266)
    #if defined(ESP32)
    byte PHYmode = 3; // wifi_get_phy_mode();
    #endif // if defined(ESP32)

    switch (PHYmode)
    {
      case 1:
        TXBuffer += F("802.11B");
        break;
      case 2:
        TXBuffer += F("802.11G");
        break;
      case 3:
        TXBuffer += F("802.11N");
        break;
    }
    TXBuffer += F(" (RSSI ");
    TXBuffer += WiFi.RSSI();
    TXBuffer += F(" dB)");
  }
  addRowLabelValue(LabelType::IP_CONFIG);
  addRowLabelValue(LabelType::IP_ADDRESS_SUBNET);
  addRowLabelValue(LabelType::GATEWAY);
  addRowLabelValue(LabelType::CLIENT_IP);
  addRowLabelValue(LabelType::DNS);
  addRowLabelValue(LabelType::ALLOWED_IP_RANGE);
  addRowLabel(getLabel(LabelType::STA_MAC));

  {
    uint8_t  mac[]   = { 0, 0, 0, 0, 0, 0 };
    uint8_t *macread = WiFi.macAddress(mac);
    char     macaddress[20];
    formatMAC(macread, macaddress);
    TXBuffer += macaddress;

    addRowLabel(getLabel(LabelType::AP_MAC));
    macread = WiFi.softAPmacAddress(mac);
    formatMAC(macread, macaddress);
    TXBuffer += macaddress;
  }

  addRowLabel(getLabel(LabelType::SSID));
  TXBuffer += WiFi.SSID();
  TXBuffer += " (";
  TXBuffer += WiFi.BSSIDstr();
  TXBuffer += ')';

  addRowLabelValue(LabelType::CHANNEL);
  addRowLabelValue(LabelType::CONNECTED);
  addRowLabel(getLabel(LabelType::LAST_DISCONNECT_REASON));
  TXBuffer += getValue(LabelType::LAST_DISC_REASON_STR);
  addRowLabelValue(LabelType::NUMBER_RECONNECTS);
}

void handle_sysinfo_WiFiSettings() {
  addTableSeparator(F("WiFi Settings"), 2, 3);
  addRowLabelValue(LabelType::FORCE_WIFI_BG);
  addRowLabelValue(LabelType::RESTART_WIFI_LOST_CONN);
#ifdef ESP8266
  addRowLabelValue(LabelType::FORCE_WIFI_NOSLEEP);
#endif // ifdef ESP8266
#ifdef SUPPORT_ARP
  addRowLabelValue(LabelType::PERIODICAL_GRAT_ARP);
#endif // ifdef SUPPORT_ARP
  addRowLabelValue(LabelType::CONNECTION_FAIL_THRESH);
}

void handle_sysinfo_Firmware() {
  addTableSeparator(F("Firmware"), 2, 3);

  addRowLabelValue_copy(LabelType::BUILD_DESC);
  TXBuffer += ' ';
  TXBuffer += F(BUILD_NOTES);

  addRowLabelValue_copy(LabelType::SYSTEM_LIBRARIES);
  addRowLabelValue_copy(LabelType::GIT_BUILD);
  addRowLabelValue_copy(LabelType::PLUGINS);
  TXBuffer += getPluginDescriptionString();

  bool filenameDummy = String(CRCValues.binaryFilename).startsWith(F("ThisIsTheDummy"));

  if (!filenameDummy) {
    addRowLabel(F("Build Md5"));

    for (byte i = 0; i < 16; i++) { TXBuffer += String(CRCValues.compileTimeMD5[i], HEX); }

    addRowLabel(F("Md5 check"));

    if (!CRCValues.checkPassed()) {
      TXBuffer += F("<font color = 'red'>fail !</font>");
    }
    else { TXBuffer += F("passed."); }
  }
  addRowLabel_copy(getLabel(LabelType::BUILD_TIME));
  TXBuffer += String(CRCValues.compileDate);
  TXBuffer += ' ';
  TXBuffer += String(CRCValues.compileTime);

  addRowLabel_copy(getLabel(LabelType::BINARY_FILENAME));

  if (filenameDummy) {
    TXBuffer += F("<b>Self built!</b>");
  } else {
    TXBuffer += String(CRCValues.binaryFilename);
  }
}

void handle_sysinfo_SystemStatus() {
  addTableSeparator(F("System Status"), 2, 3);

  // Actual Loglevel
  addRowLabelValue(LabelType::SYSLOG_LOG_LEVEL);
  addRowLabelValue(LabelType::SERIAL_LOG_LEVEL);
  addRowLabelValue(LabelType::WEB_LOG_LEVEL);
    #ifdef FEATURE_SD
  addRowLabelValue(LabelType::SD_LOG_LEVEL);
    #endif // ifdef FEATURE_SD
}

void handle_sysinfo_ESP_Board() {
  addTableSeparator(F("ESP Board"), 2, 3);

  addRowLabel(getLabel(LabelType::ESP_CHIP_ID));
  #if defined(ESP8266)
  TXBuffer += ESP.getChipId();
  TXBuffer += F(" (0x");
  String espChipId(ESP.getChipId(), HEX);
  espChipId.toUpperCase();
  TXBuffer += espChipId;
  TXBuffer += ')';

  addRowLabel(getLabel(LabelType::ESP_CHIP_FREQ));
  TXBuffer += ESP.getCpuFreqMHz();
  TXBuffer += F(" MHz");
  #endif // if defined(ESP8266)
  #if defined(ESP32)
  TXBuffer += F(" (0x");
  uint64_t chipid  = ESP.getEfuseMac(); // The chip ID is essentially its MAC address(length: 6 bytes).
  uint32_t ChipId1 = (uint16_t)(chipid >> 32);
  String   espChipIdS(ChipId1, HEX);
  espChipIdS.toUpperCase();
  TXBuffer += espChipIdS;
  ChipId1   = (uint32_t)chipid;
  String espChipIdS1(ChipId1, HEX);
  espChipIdS1.toUpperCase();
  TXBuffer += espChipIdS1;
  TXBuffer += ')';

  addRowLabel(getLabel(LabelType::ESP_CHIP_FREQ));
  TXBuffer += ESP.getCpuFreqMHz();
  TXBuffer += F(" MHz");
  #endif // if defined(ESP32)
  #ifdef ARDUINO_BOARD
  addRowLabel(getLabel(LabelType::ESP_BOARD_NAME));
  TXBuffer += ARDUINO_BOARD;
  #endif // ifdef ARDUINO_BOARD
}

void handle_sysinfo_Storage() {
  addTableSeparator(F("Storage"), 2, 3);

  addRowLabel(getLabel(LabelType::FLASH_CHIP_ID));
  #if defined(ESP8266)
  uint32_t flashChipId = ESP.getFlashChipId();

  // Set to HEX may be something like 0x1640E0.
  // Where manufacturer is 0xE0 and device is 0x4016.
  TXBuffer += F("Vendor: ");
  TXBuffer += formatToHex(flashChipId & 0xFF);

  if (flashChipVendorPuya())
  {
    TXBuffer += F(" (PUYA");

    if (puyaSupport()) {
      TXBuffer += F(", supported");
    } else {
      TXBuffer += F(HTML_SYMBOL_WARNING);
    }
    TXBuffer += ')';
  }
  TXBuffer += F(" Device: ");
  uint32_t flashDevice = (flashChipId & 0xFF00) | ((flashChipId >> 16) & 0xFF);
  TXBuffer += formatToHex(flashDevice);
  #endif // if defined(ESP8266)
  uint32_t realSize = getFlashRealSizeInBytes();
  uint32_t ideSize  = ESP.getFlashChipSize();

  addRowLabel(getLabel(LabelType::FLASH_CHIP_REAL_SIZE));
  TXBuffer += realSize / 1024;
  TXBuffer += F(" kB");

  addRowLabel(getLabel(LabelType::FLASH_IDE_SIZE));
  TXBuffer += ideSize / 1024;
  TXBuffer += F(" kB");

  // Please check what is supported for the ESP32
  #if defined(ESP8266)
  addRowLabel(getLabel(LabelType::FLASH_IDE_SPEED));
  TXBuffer += ESP.getFlashChipSpeed() / 1000000;
  TXBuffer += F(" MHz");

  FlashMode_t ideMode = ESP.getFlashChipMode();
  addRowLabel(getLabel(LabelType::FLASH_IDE_MODE));

  switch (ideMode) {
    case FM_QIO:   TXBuffer += F("QIO");  break;
    case FM_QOUT:  TXBuffer += F("QOUT"); break;
    case FM_DIO:   TXBuffer += F("DIO");  break;
    case FM_DOUT:  TXBuffer += F("DOUT"); break;
    default:
      TXBuffer += getUnknownString(); break;
  }
  #endif // if defined(ESP8266)

  addRowLabel(getLabel(LabelType::FLASH_WRITE_COUNT));
  TXBuffer += RTC.flashDayCounter;
  TXBuffer += F(" daily / ");
  TXBuffer += RTC.flashCounter;
  TXBuffer += F(" boot");

  addRowLabel(getLabel(LabelType::SKETCH_SIZE));
  #if defined(ESP8266)
  TXBuffer += ESP.getSketchSize() / 1024;
  TXBuffer += F(" kB (");
  TXBuffer += ESP.getFreeSketchSpace() / 1024;
  TXBuffer += F(" kB free)");
  #endif // if defined(ESP8266)

  addRowLabel(getLabel(LabelType::SPIFFS_SIZE));
  TXBuffer += SpiffsTotalBytes() / 1024;
  TXBuffer += F(" kB (");
  TXBuffer += SpiffsFreeSpace() / 1024;
  TXBuffer += F(" kB free)");

  addRowLabel(F("Page size"));
  TXBuffer += String(SpiffsPagesize());

  addRowLabel(F("Block size"));
  TXBuffer += String(SpiffsBlocksize());

  addRowLabel(F("Number of blocks"));
  TXBuffer += String(SpiffsTotalBytes() / SpiffsBlocksize());

  {
  #if defined(ESP8266)
    fs::FSInfo fs_info;
    SPIFFS.info(fs_info);
    addRowLabel(F("Maximum open files"));
    TXBuffer += String(fs_info.maxOpenFiles);

    addRowLabel(F("Maximum path length"));
    TXBuffer += String(fs_info.maxPathLength);

  #endif // if defined(ESP8266)
  }

#ifndef BUILD_MINIMAL_OTA

  if (showSettingsFileLayout) {
    addTableSeparator(F("Settings Files"), 2, 3);
    html_TR_TD();
    TXBuffer += F("Layout Settings File");
    html_TD();
    getConfig_dat_file_layout();
    html_TR_TD();
    html_TD();
    TXBuffer += F("(offset / size per item / index)");

    for (int st = 0; st < SettingsType_MAX; ++st) {
      SettingsType settingsType = static_cast<SettingsType>(st);
      html_TR_TD();
      TXBuffer += getSettingsTypeString(settingsType);
      html_TD();
      getStorageTableSVG(settingsType);
    }
  }
#endif // ifndef BUILD_MINIMAL_OTA

  #ifdef ESP32
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
  #endif // ifdef ESP32
}
