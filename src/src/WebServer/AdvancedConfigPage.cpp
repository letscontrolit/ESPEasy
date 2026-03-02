#include "../WebServer/AdvancedConfigPage.h"

#ifdef WEBSERVER_ADVANCED

#include "../WebServer/HTML_wrappers.h"
#include "../WebServer/Markup.h"
#include "../WebServer/Markup_Buttons.h"
#include "../WebServer/Markup_Forms.h"
#include "../WebServer/ESPEasy_WebServer.h"

#include "../../ESPEasy/net/wifi/ESPEasyWifi.h"


#include "../Globals/ESPEasy_time.h"
#include "../Globals/Settings.h"
#include "../Globals/TimeZone.h"

#include "../Helpers/_Plugin_Helper_serial.h"
#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/ESPEasy_time.h"
#include "../Helpers/Hardware_defines.h"
#include "../Helpers/StringConverter.h"

#if FEATURE_I2C_MULTIPLE
#include "../Helpers/I2C_access.h"
#endif

void setLogLevelFor(LogDestination destination, LabelType::Enum label) {
  setLogLevelFor(destination, getFormItemInt(getInternalLabel(label)));
}

// ********************************************************************************
// Web Interface config page
// ********************************************************************************
void handle_advanced() {
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_advanced"));
  #endif

  if (!isLoggedIn()) { return; }
  navMenuIndex = MENU_INDEX_TOOLS;
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate(_HEAD);

  if (!webArg(F("edit")).isEmpty())
  {
//    Settings.MessageDelay_unused = getFormItemInt(F("messagedelay"));
    Settings.IP_Octet     = webArg(F("ip")).toInt();
    strncpy_webserver_arg(Settings.NTPHost, F("ntphost"));
    Settings.TimeZone = getFormItemInt(F("timezone"));
    TimeChangeRule dst_start(getFormItemInt(F("dststartweek")), getFormItemInt(F("dststartdow")), getFormItemInt(F("dststartmonth")), getFormItemInt(F("dststarthour")), Settings.TimeZone);

    if (dst_start.isValid()) { Settings.DST_Start = dst_start.toFlashStoredValue(); }
    TimeChangeRule dst_end(getFormItemInt(F("dstendweek")), getFormItemInt(F("dstenddow")), getFormItemInt(F("dstendmonth")), getFormItemInt(F("dstendhour")), Settings.TimeZone);

    if (dst_end.isValid()) { Settings.DST_End = dst_end.toFlashStoredValue(); }
    webArg2ip(F("syslogip"), Settings.Syslog_IP);
    Settings.WebserverPort = getFormItemInt(F("webport"));
    Settings.UDPPort = getFormItemInt(F("udpport"));

    Settings.SyslogFacility = getFormItemInt(F("syslogfacility"));
    Settings.SyslogPort     = getFormItemInt(F("syslogport"));
    Settings.UseSerial      = isFormItemChecked(LabelType::ENABLE_SERIAL_PORT_CONSOLE);

#if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
    Settings.console_serial_rxpin = getFormItemInt(F("taskdevicepin1"), Settings.console_serial_rxpin);
    Settings.console_serial_txpin = getFormItemInt(F("taskdevicepin2"), Settings.console_serial_txpin);

    serialHelper_webformSave(
      Settings.console_serial_port, 
      Settings.console_serial_rxpin,
      Settings.console_serial_txpin);
#if USES_ESPEASY_CONSOLE_FALLBACK_PORT
    Settings.console_serial0_fallback = isFormItemChecked(LabelType::CONSOLE_FALLBACK_TO_SERIAL0);
#endif

#endif
#if FEATURE_SYSLOG
    setLogLevelFor(LOG_TO_SYSLOG, LabelType::SYSLOG_LOG_LEVEL);
#endif
    setLogLevelFor(LOG_TO_SERIAL, LabelType::SERIAL_LOG_LEVEL);
# ifdef WEBSERVER_LOG
    setLogLevelFor(LOG_TO_WEBLOG, LabelType::WEB_LOG_LEVEL);
#endif
#if FEATURE_SD
    setLogLevelFor(LOG_TO_SDCARD, LabelType::SD_LOG_LEVEL);
#endif // if FEATURE_SD
    Settings.UseValueLogger              = isFormItemChecked(F("valuelogger"));
    Settings.BaudRate                    = getFormItemInt(F("baudrate"));
    Settings.UseNTP(isFormItemChecked(F("usentp")));
    Settings.ExtTimeSource(
      static_cast<ExtTimeSource_e>(getFormItemInt(F("exttimesource")))
    );
    #if FEATURE_I2C_MULTIPLE
    if (getI2CBusCount() > 1) {
      set3BitToUL(Settings.I2C_peripheral_bus, I2C_PERIPHERAL_BUS_CLOCK, getFormItemInt(F("pi2cbusrtc")));
      set3BitToUL(Settings.I2C_peripheral_bus, I2C_PERIPHERAL_BUS_WDT,   getFormItemInt(F("pi2cbuswdt")));
    }
    #endif // if FEATURE_I2C_MULTIPLE
    Settings.DST                         = isFormItemChecked(F("dst"));
    Settings.WDI2CAddress                = getFormItemInt(F("wdi2caddress"));
    #if FEATURE_SSDP
    Settings.UseSSDP                     = isFormItemChecked(F("usessdp"));
    #endif // if FEATURE_SSDP
    Settings.UseRules                    = isFormItemChecked(F("userules"));
    Settings.ConnectionFailuresThreshold = getFormItemInt(LabelType::CONNECTION_FAIL_THRESH);
    Settings.ArduinoOTAEnable            = isFormItemChecked(F("arduinootaenable"));
    Settings.UseRTOSMultitasking         = isFormItemChecked(F("usertosmultitasking"));

    // MQTT settings now moved to the controller settings.
//    Settings.MQTTRetainFlag_unused              = isFormItemChecked(F("mqttretainflag"));
//    Settings.MQTTUseUnitNameAsClientId   = isFormItemChecked(F("mqttuseunitnameasclientid"));
//    Settings.uniqueMQTTclientIdReconnect(isFormItemChecked(F("uniquemqttclientidreconnect")));
    Settings.Latitude  = getFormItemFloat(F("latitude"));
    Settings.Longitude = getFormItemFloat(F("longitude"));
    #ifdef WEBSERVER_NEW_RULES
    Settings.OldRulesEngine(isFormItemChecked(F("oldrulesengine")));
    #endif // WEBSERVER_NEW_RULES
    Settings.TolerantLastArgParse(isFormItemChecked(F("tolerantargparse")));
    Settings.SendToHttp_ack(isFormItemChecked(F("sendtohttp_ack")));
    Settings.SendToHTTP_follow_redirects(isFormItemChecked(F("sendtohttp_redir")));
    Settings.EcoPowerMode(isFormItemChecked(LabelType::CPU_ECO_MODE));
    Settings.JSONBoolWithoutQuotes(isFormItemChecked(LabelType::JSON_BOOL_QUOTES));
#if FEATURE_TIMING_STATS
    Settings.EnableTimingStats(isFormItemChecked(LabelType::ENABLE_TIMING_STATISTICS));
#endif
    Settings.AllowTaskValueSetAllPlugins(isFormItemChecked(LabelType::TASKVALUESET_ALL_PLUGINS));
#if FEATURE_CLEAR_I2C_STUCK
    Settings.EnableClearHangingI2Cbus(isFormItemChecked(LabelType::ENABLE_CLEAR_HUNG_I2C_BUS));
#endif
    #if FEATURE_I2C_DEVICE_CHECK
    Settings.CheckI2Cdevice(isFormItemChecked(LabelType::ENABLE_I2C_DEVICE_CHECK));
    #endif // if FEATURE_I2C_DEVICE_CHECK
#if FEATURE_USE_IPV6
    Settings.EnableIPv6(isFormItemChecked(LabelType::ENABLE_IPV6));
#endif




#ifndef BUILD_NO_RAM_TRACKER
    Settings.EnableRAMTracking(isFormItemChecked(LabelType::ENABLE_RAM_TRACKING));
#endif

    #ifdef ESP8266
    Settings.UseAlternativeDeepSleep(isFormItemChecked(LabelType::DEEP_SLEEP_ALTERNATIVE_CALL));
    #endif

    Settings.EnableRulesCaching(isFormItemChecked(LabelType::ENABLE_RULES_CACHING));
//    Settings.EnableRulesEventReorder(isFormItemChecked(LabelType::ENABLE_RULES_EVENT_REORDER)); // TD-er: Disabled for now

#ifndef NO_HTTP_UPDATER
    Settings.AllowOTAUnlimited(isFormItemChecked(LabelType::ALLOW_OTA_UNLIMITED));
#endif // NO_HTTP_UPDATER
#if FEATURE_AUTO_DARK_MODE
    Settings.setCssMode(getFormItemInt(getInternalLabel(LabelType::ENABLE_AUTO_DARK_MODE)));
#endif // FEATURE_AUTO_DARK_MODE
#if FEATURE_RULES_EASY_COLOR_CODE
    Settings.DisableRulesCodeCompletion(isFormItemChecked(LabelType::DISABLE_RULES_AUTOCOMPLETE));
#endif // if FEATURE_RULES_EASY_COLOR_CODE
#if FEATURE_TARSTREAM_SUPPORT
    Settings.DisableSaveConfigAsTar(isFormItemChecked(LabelType::DISABLE_SAVE_CONFIG_AS_TAR));
#endif // if FEATURE_TARSTREAM_SUPPORT
    #if FEATURE_TASKVALUE_UNIT_OF_MEASURE
    Settings.ShowUnitOfMeasureOnDevicesPage(isFormItemChecked(LabelType::SHOW_UOM_ON_DEVICES_PAGE));
    #endif // if FEATURE_TASKVALUE_UNIT_OF_MEASURE
    #if FEATURE_MQTT_CONNECT_BACKGROUND
    Settings.MQTTConnectInBackground(isFormItemChecked(LabelType::MQTT_CONNECT_IN_BACKGROUND));
    #endif // if FEATURE_MQTT_CONNECT_BACKGROUND

    addHtmlError(SaveSettings());

    if (node_time.systemTimePresent()) {
      node_time.initTime();
    }
  }

  addHtml(F("<form  method='post'>"));
  html_table_class_normal();

  addFormHeader(F("Advanced Settings"), F("RTDTools/Tools.html#advanced"));

  addFormSubHeader(F("Rules Settings"));

  addFormCheckBox(F("Rules"),      F("userules"),       Settings.UseRules);
  #ifdef WEBSERVER_NEW_RULES
  addFormCheckBox(F("Old Engine"), F("oldrulesengine"), Settings.OldRulesEngine());
  #endif // WEBSERVER_NEW_RULES
  addFormCheckBox(LabelType::ENABLE_RULES_CACHING);
//  addFormCheckBox(LabelType::ENABLE_RULES_EVENT_REORDER); // TD-er: Disabled for now

  addFormCheckBox(F("Tolerant last parameter"), F("tolerantargparse"), Settings.TolerantLastArgParse());
  addFormNote(F("Perform less strict parsing on last argument of some commands (e.g. publish and sendToHttp)"));
  addFormCheckBox(F("SendToHTTP wait for ack"), F("sendtohttp_ack"), Settings.SendToHttp_ack());
  addFormCheckBox(F("SendToHTTP Follow Redirects"), F("sendtohttp_redir"), Settings.SendToHTTP_follow_redirects());

  /*
  // MQTT settings now moved to the controller settings.
  addFormSubHeader(F("Controller Settings"));

  addFormNumericBox(F("Message Interval"), F("messagedelay"), Settings.MessageDelay_unused, 0, INT_MAX);
  addUnit(F("ms"));

  addFormCheckBox(F("MQTT Retain Msg"), F("mqttretainflag"), Settings.MQTTRetainFlag_unused);
  addFormCheckBox(F("MQTT use unit name as ClientId"),    F("mqttuseunitnameasclientid"),   Settings.MQTTUseUnitNameAsClientId);
  addFormCheckBox(F("MQTT change ClientId at reconnect"), F("uniquemqttclientidreconnect"), Settings.uniqueMQTTclientIdReconnect_unused());
*/

  addFormSubHeader(F("Time Source"));

  addFormCheckBox(F("Use NTP"), F("usentp"), Settings.UseNTP());
  addFormTextBox(F("NTP Hostname"), F("ntphost"), Settings.NTPHost, 63);
  #if FEATURE_EXT_RTC
  addFormExtTimeSourceSelect(F("External Time Source"), F("exttimesource"), Settings.ExtTimeSource());
  if (Settings.ExtTimeSource() != ExtTimeSource_e::None) {
    addFormNote(concat(getLabel(LabelType::EXT_RTC_UTC_TIME), F(": ")) + getValue(LabelType::EXT_RTC_UTC_TIME));
  }
  #if FEATURE_I2C_MULTIPLE
  {
    const uint8_t i2cBus = Settings.getI2CInterfaceRTC();
    I2CInterfaceSelector(F("Ext. Time Source I2C Bus"),
                        F("pi2cbusrtc"),
                        i2cBus,
                        false);
  }
  #endif // if FEATURE_I2C_MULTIPLE
  #endif

  addFormSubHeader(F("DST Settings"));
  addFormDstSelect(true,  Settings.DST_Start);
  addFormDstSelect(false, Settings.DST_End);
  addFormCheckBox(F("DST"), F("dst"), Settings.DST);

  addFormSubHeader(F("Location Settings"));
  addFormNumericBox(F("Timezone Offset (UTC +)"), F("timezone"), Settings.TimeZone, -720, 840); // UTC-12H ... UTC+14h
  addUnit(F("minutes"));
  addFormFloatNumberBox(F("Latitude"), F("latitude"), Settings.Latitude, -90.0f, 90.0f);
  addUnit(F("&deg;"));
  addFormFloatNumberBox(F("Longitude"), F("longitude"), Settings.Longitude, -180.0f, 180.0f);
  addUnit(F("&deg;"));
  addFormNote(F("Longitude and Latitude are used to compute sunrise and sunset"));

  addFormSubHeader(F("Log Settings"));

  addFormIPBox(F("Syslog IP"), F("syslogip"), Settings.Syslog_IP);
  addFormNumericBox(F("Syslog UDP port"), F("syslogport"), Settings.SyslogPort, 0, 65535);
#if FEATURE_SYSLOG
  addFormLogLevelSelect(LabelType::SYSLOG_LOG_LEVEL, Settings.SyslogLevel);
  addFormLogFacilitySelect(F("Syslog Facility"), F("syslogfacility"), Settings.SyslogFacility);
#endif
  addFormLogLevelSelect(LabelType::SERIAL_LOG_LEVEL, Settings.SerialLogLevel);
# ifdef WEBSERVER_LOG
  addFormLogLevelSelect(LabelType::WEB_LOG_LEVEL,    Settings.WebLogLevel);
#endif

#if FEATURE_SD
  addFormLogLevelSelect(LabelType::SD_LOG_LEVEL,     Settings.SDLogLevel);

  addFormCheckBox(F("SD Card Value Logger"), F("valuelogger"), Settings.UseValueLogger);
#endif // if FEATURE_SD


  addFormSubHeader(F("Serial Console Settings"));
  addFormCheckBox(LabelType::ENABLE_SERIAL_PORT_CONSOLE);
  addFormNumericBox(F("Baud Rate"), F("baudrate"), Settings.BaudRate, 0, 1000000);

#if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
  serialHelper_webformLoad(
    static_cast<ESPEasySerialPort>(Settings.console_serial_port), 
    Settings.console_serial_rxpin, 
    Settings.console_serial_txpin, 
    true);

  // Show serial port selection
  addFormPinSelect(
    PinSelectPurpose::Serial_input, 
    formatGpioName_serialRX(false),
    F("taskdevicepin1"), 
    Settings.console_serial_rxpin);
  addFormPinSelect(
    PinSelectPurpose::Serial_output, 
    formatGpioName_serialTX(false),
    F("taskdevicepin2"), 
    Settings.console_serial_txpin);

  html_add_script(F("document.getElementById('serPort').onchange();"), false);
#if USES_ESPEASY_CONSOLE_FALLBACK_PORT
  addFormCheckBox(LabelType::CONSOLE_FALLBACK_TO_SERIAL0);
#endif

#endif


  addFormSubHeader(F("Inter-ESPEasy Network"));
  if (Settings.UDPPort != 8266 ) addFormNote(F("Preferred P2P port is 8266"));
  addFormNumericBox(F("ESPEasy p2p UDP port"), F("udpport"), Settings.UDPPort, 0, 65535);

  // TODO sort settings in groups or move to other pages/groups
  addFormSubHeader(F("Special and Experimental Settings"));

  addFormNumericBox(F("Webserver port"), F("webport"), Settings.WebserverPort, 0, 65535);
  addFormNote(F("Requires reboot to activate"));

  addFormNumericBox(F("Fixed IP Octet"), F("ip"),           Settings.IP_Octet,     0, 255);

  addFormNumericBox(F("WD I2C Address"), F("wdi2caddress"), Settings.WDI2CAddress, 0, 127);
  addHtml(F(" (decimal)"));
  #if FEATURE_I2C_MULTIPLE
  {
    const uint8_t i2cBus = Settings.getI2CInterfaceWDT();
    I2CInterfaceSelector(F("WD I2C Bus"),
                        F("pi2cbuswdt"),
                        i2cBus,
                        false);
  }
  #endif // if FEATURE_I2C_MULTIPLE

  // TODO: Remove this code
  addRowLabel(F("I2C ClockStretchLimit"));
  addUnit(F("Moved to Hardware page"));

  #if FEATURE_ARDUINO_OTA
  addFormCheckBox(F("Enable Arduino OTA"), F("arduinootaenable"), Settings.ArduinoOTAEnable);
  #endif // if FEATURE_ARDUINO_OTA
  #if defined(ESP32)
  addFormCheckBox_disabled(F("Enable RTOS Multitasking"), F("usertosmultitasking"), Settings.UseRTOSMultitasking);
  #endif // if defined(ESP32)
  {
    LabelType::Enum labels[]{

      LabelType::JSON_BOOL_QUOTES
    #if FEATURE_TIMING_STATS
      ,LabelType::ENABLE_TIMING_STATISTICS
    #endif // if FEATURE_TIMING_STATS
    #ifndef BUILD_NO_RAM_TRACKER
      ,LabelType::ENABLE_RAM_TRACKING
    #endif

      ,LabelType::TASKVALUESET_ALL_PLUGINS
    #if FEATURE_CLEAR_I2C_STUCK
      ,LabelType::ENABLE_CLEAR_HUNG_I2C_BUS
    #endif
      #if FEATURE_I2C_DEVICE_CHECK
      ,LabelType::ENABLE_I2C_DEVICE_CHECK
      #endif // if FEATURE_I2C_DEVICE_CHECK

      #if FEATURE_TASKVALUE_UNIT_OF_MEASURE
      ,LabelType::SHOW_UOM_ON_DEVICES_PAGE
      #endif // if FEATURE_TASKVALUE_UNIT_OF_MEASURE

      #if FEATURE_MQTT_CONNECT_BACKGROUND
      ,LabelType::MQTT_CONNECT_IN_BACKGROUND
      #endif // if FEATURE_MQTT_CONNECT_BACKGROUND

      # ifndef NO_HTTP_UPDATER
      ,LabelType::ALLOW_OTA_UNLIMITED
      # endif // ifndef NO_HTTP_UPDATER
    };

    addFormCheckBoxes(labels, NR_ELEMENTS(labels));
  }
  #if FEATURE_AUTO_DARK_MODE
  {
    const __FlashStringHelper * cssModeNames[] = {
      F("Auto"),
      F("Light"),
      F("Dark"),
    };
    //const int cssModeOptions[] = { 0, 1, 2};
    constexpr int nrCssModeOptions = NR_ELEMENTS(cssModeNames);
    const FormSelectorOptions selector(
      nrCssModeOptions,
      cssModeNames/*,
      cssModeOptions*/);
    selector.addFormSelector(
      getLabel(LabelType::ENABLE_AUTO_DARK_MODE),
      getInternalLabel(LabelType::ENABLE_AUTO_DARK_MODE),
      Settings.getCssMode());
  }
  #endif // FEATURE_AUTO_DARK_MODE
{
  LabelType::Enum labels[]{

  LabelType::CPU_ECO_MODE
  #ifdef ESP8266
  ,LabelType::DEEP_SLEEP_ALTERNATIVE_CALL
  #endif
  #if FEATURE_RULES_EASY_COLOR_CODE
  ,LabelType::DISABLE_RULES_AUTOCOMPLETE
  #endif // if FEATURE_RULES_EASY_COLOR_CODE
  #if FEATURE_TARSTREAM_SUPPORT
  ,LabelType::DISABLE_SAVE_CONFIG_AS_TAR
  #endif // if FEATURE_TARSTREAM_SUPPORT

#if FEATURE_USE_IPV6
  ,LabelType::ENABLE_IPV6
#endif
        };
  addFormCheckBoxes(labels, NR_ELEMENTS(labels));
}
  #if FEATURE_SSDP
  addFormCheckBox_disabled(F("Use SSDP"), F("usessdp"), Settings.UseSSDP);
  #endif // if FEATURE_SSDP

  addFormNumericBox(LabelType::CONNECTION_FAIL_THRESH, 0, 100);



  addFormSeparator(2);

  html_TR_TD();
  html_TD();
  addSubmitButton();
  addHtml(F("<input type='hidden' name='edit' value='1'>"));
  html_end_table();
  html_end_form();
  sendHeadandTail_stdtemplate(_TAIL);
  TXBuffer.endStream();
}

void addFormDstSelect(bool isStart, uint16_t choice) {
  uint16_t tmpstart(choice);
  uint16_t tmpend(choice);

  if (!TimeChangeRule(choice, 0).isValid()) {
    time_zone.getDefaultDst_flash_values(tmpstart, tmpend);
  }
  TimeChangeRule rule(isStart ? tmpstart : tmpend, 0);
  {
    const __FlashStringHelper *  week[] = { F("Last"), F("1st"), F("2nd"), F("3rd"), F("4th") };
    constexpr int weekValues[] = { 0, 1, 2, 3, 4 };
    addRowLabel(concat(
      isStart ? F("Start")  : F("End"),
      F(" (week, dow, month)")));

    const FormSelectorOptions selector(NR_ELEMENTS(weekValues), week, weekValues);
    selector.addSelector(
      isStart ? F("dststartweek")  : F("dstendweek"), 
      rule.week);
  }
  html_BR();
  {
    const __FlashStringHelper *  dow[] = { F("Sun"), F("Mon"), F("Tue"), F("Wed"), F("Thu"), F("Fri"), F("Sat") };
    constexpr int dowValues[]  = { 1, 2, 3, 4, 5, 6, 7 };

    const FormSelectorOptions selector(NR_ELEMENTS(dowValues), dow, dowValues);
    selector.addSelector(
      isStart ? F("dststartdow")   : F("dstenddow"),
      rule.dow);
  }
  html_BR();
  {
    const __FlashStringHelper * month[] = { F("Jan"), F("Feb"), F("Mar"), F("Apr"), F("May"), F("Jun"), F("Jul"), F("Aug"), F("Sep"), F("Oct"), F("Nov"), F(
                             "Dec") };
    constexpr int monthValues[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };

    const FormSelectorOptions selector(NR_ELEMENTS(monthValues), month, monthValues);
    selector.addSelector(
      isStart ? F("dststartmonth") : F("dstendmonth"),
      rule.month);
  }

  addFormNumericBox(
    isStart ? F("Start (localtime, e.g. 2h&rarr;3h)")  : F("End (localtime, e.g. 3h&rarr;2h)"),
    isStart ? F("dststarthour")  : F("dstendhour"),
    rule.hour, 0, 23);
  addUnit(isStart ? F("hour &#x21b7;") : F("hour &#x21b6;"));
}

void addFormExtTimeSourceSelect(const __FlashStringHelper * label, const __FlashStringHelper * id, ExtTimeSource_e choice)
{
  addRowLabel(label);
  const __FlashStringHelper * options[] =
    { F("None"), F("DS1307"), F("DS3231"), F("PCF8523"), F("PCF8563")};
  constexpr int optionValues[] = { 
    static_cast<int>(ExtTimeSource_e::None),
    static_cast<int>(ExtTimeSource_e::DS1307),
    static_cast<int>(ExtTimeSource_e::DS3231),
    static_cast<int>(ExtTimeSource_e::PCF8523),
    static_cast<int>(ExtTimeSource_e::PCF8563)
    };

  const FormSelectorOptions selector(NR_ELEMENTS(optionValues), options, optionValues);
  selector.addSelector(id, static_cast<int>(choice));
}


void addFormLogLevelSelect(LabelType::Enum label, int choice)
{
  #ifdef BUILD_NO_DEBUG
  if (choice > LOG_LEVEL_INFO) choice = LOG_LEVEL_INFO;
  #endif

  addRowLabel(getLabel(label));
  addLogLevelFormSelectorOptions(getInternalLabel(label), choice);
}

#if FEATURE_SYSLOG
void addFormLogFacilitySelect(const __FlashStringHelper * label, const __FlashStringHelper * id, int choice)
{
  addRowLabel(label);
  const __FlashStringHelper * options[] =
  { F("Kernel"), F("User"),   F("Daemon"),   F("Message"), F("Local0"),  F("Local1"),
    F("Local2"), F("Local3"), F("Local4"),   F("Local5"),  F("Local6"),  F("Local7") };
  const int optionValues[] = { 0, 1, 3, 5, 16, 17, 18, 19, 20, 21, 22, 23 };

  const FormSelectorOptions selector(NR_ELEMENTS(options), options, optionValues);
  selector.addSelector(id, choice);
}
#endif

#endif // ifdef WEBSERVER_ADVANCED
