#ifdef WEBSERVER_TOOLS

// ********************************************************************************
// Web Interface Tools page
// ********************************************************************************
void handle_tools(void) {
  if (!isLoggedIn(void)) { return; }
  navMenuIndex = MENU_INDEX_TOOLS;
  TXBuffer.startStream(void);
  sendHeadandTail_stdtemplate(_HEAD);

  String webrequest = WebServer.arg(F("cmd"));

  TXBuffer += F("<form>");
  html_table_class_normal(void);

  addFormHeader(F("Tools"));

  addFormSubHeader(F("Command"));
  html_TR_TD(void);
  TXBuffer += F("<TR><TD style='width: 180px'>");
  TXBuffer += F("<input class='wide' type='text' name='cmd' value='");
  TXBuffer += webrequest;
  TXBuffer += "'>";
  html_TD(void);
  addSubmitButton(void);
  addHelpButton(F("ESPEasy_Command_Reference"));
  html_TR_TD(void);

  printToWeb     = true;
  printWebString = "";

  if (webrequest.length(void) > 0)
  {
    ExecuteCommand_all(VALUE_SOURCE_WEB_FRONTEND, webrequest.c_str(void));
  }

  if (printWebString.length(void) > 0)
  {
    TXBuffer += F("<TR><TD colspan='2'>Command Output<BR><textarea readonly rows='10' wrap='on'>");
    TXBuffer += printWebString;
    TXBuffer += F("</textarea>");
  }

  addFormSubHeader(F("System"));

  addWideButtonPlusDescription(F("/?cmd=reboot"), F("Reboot"),           F("Reboots ESP"));

  # ifdef WEBSERVER_LOG
  addWideButtonPlusDescription(F("log"),          F("Log"),              F("Open log output"));
  # endif // ifdef WEBSERVER_LOG

  #ifdef WEBSERVER_SYSINFO
  addWideButtonPlusDescription(F("sysinfo"),      F("Info"),             F("Open system info page"));
  #endif

  #ifdef WEBSERVER_ADVANCED
  addWideButtonPlusDescription(F("advanced"),     F("Advanced"),         F("Open advanced settings"));
  #endif

  addWideButtonPlusDescription(F("json"),         F("Show JSON"),        F("Open JSON output"));

  # ifdef WEBSERVER_TIMINGSTATS
  addWideButtonPlusDescription(F("timingstats"),  F("Timing stats"),     F("Open timing statistics of system"));
  # endif // WEBSERVER_TIMINGSTATS

  #ifdef WEBSERVER_PINSTATES
  addWideButtonPlusDescription(F("pinstates"),    F("Pin state buffer"), F("Show Pin state buffer"));
  #endif

  # ifdef WEBSERVER_SYSVARS
  addWideButtonPlusDescription(F("sysvars"),      F("System Variables"), F("Show all system variables and conversions"));
  # endif // ifdef WEBSERVER_SYSVARS

  addFormSubHeader(F("Wifi"));

  addWideButtonPlusDescription(F("/?cmd=wificonnect"),    F("Connect"),    F("Connects to known Wifi network"));
  addWideButtonPlusDescription(F("/?cmd=wifidisconnect"), F("Disconnect"), F("Disconnect from wifi network"));

  #ifdef WEBSERVER_WIFI_SCANNER
  addWideButtonPlusDescription(F("wifiscanner"),          F("Scan"),       F("Scan for wifi networks"));
  #endif

  # ifdef WEBSERVER_I2C_SCANNER
  addFormSubHeader(F("Interfaces"));

  addWideButtonPlusDescription(F("i2cscanner"), F("I2C Scan"), F("Scan for I2C devices"));
  # endif // ifdef WEBSERVER_I2C_SCANNER

  addFormSubHeader(F("Settings"));

  addWideButtonPlusDescription(F("upload"), F("Load"), F("Loads a settings file"));
  addFormNote(F("(File MUST be renamed to \"config.dat\" before upload!)"));
  addWideButtonPlusDescription(F("download"), F("Save"), F("Saves a settings file"));

# ifdef WEBSERVER_NEW_UI
  #  if defined(ESP8266)

  if ((SpiffsFreeSpace(void) / 1024) > 50) {
    TXBuffer += F("<TR><TD>");
    TXBuffer += F(
      "<script>function downloadUI(void) { fetch('https://raw.githubusercontent.com/letscontrolit/espeasy_ui/master/build/index.htm.gz').then(r=>r.arrayBuffer(void)).then(r => {var f=new FormData(void);f.append('file', new File([new Blob([new Uint8Array(r)])], 'index.htm.gz'));f.append('edit', 1);fetch('/upload',{method:'POST',body:f}).then((void) => {window.location.href='/';});}); }</script>");
    TXBuffer += F("<a class=\"button link wide\" onclick=\"downloadUI(void)\">Download new UI</a>");
    TXBuffer += F("</TD><TD>Download new UI(alpha)</TD></TR>");
  }
  #  endif // if defined(ESP8266)
# endif    // WEBSERVER_NEW_UI

# if defined(ESP8266)
  {
    #  ifndef NO_HTTP_UPDATER
    {
      uint32_t maxSketchSize;
      bool     use2step;
      bool     otaEnabled = OTA_possible(maxSketchSize, use2step);
      addFormSubHeader(F("Firmware"));
      html_TR_TD_height(30);
      addWideButton(F("update"), F("Update Firmware"), "", otaEnabled);
      addHelpButton(F("EasyOTA"));
      html_TD(void);
      TXBuffer += F("Load a new firmware");

      if (otaEnabled) {
        if (use2step) {
          TXBuffer += F(" <b>WARNING</b> only use 2-step OTA update.");
        }
      } else {
        TXBuffer += F(" <b>WARNING</b> OTA not possible.");
      }
      TXBuffer += F(" Max sketch size: ");
      TXBuffer += maxSketchSize / 1024;
      TXBuffer += F(" kB (");
      TXBuffer += maxSketchSize;
      TXBuffer += F(" bytes)");
    }
    #  endif // ifndef NO_HTTP_UPDATER
  }
# endif // if defined(ESP8266)

  addFormSubHeader(F("Filesystem"));

  addWideButtonPlusDescription(F("filelist"),         F("File browser"),     F("Show files on internal flash file system"));
  addWideButtonPlusDescription(F("/factoryreset"),    F("Factory Reset"),    F("Select pre-defined configuration or full erase of settings"));
  # ifdef USE_SETTINGS_ARCHIVE
  addWideButtonPlusDescription(F("/settingsarchive"), F("Settings Archive"), F("Download settings from some archive"));
  # endif // ifdef USE_SETTINGS_ARCHIVE
# ifdef FEATURE_SD
  addWideButtonPlusDescription(F("SDfilelist"),       F("SD Card"),          F("Show files on SD-Card"));
# endif // ifdef FEATURE_SD

  html_end_table(void);
  html_end_form(void);
  sendHeadandTail_stdtemplate(_TAIL);
  TXBuffer.endStream(void);
  printWebString = "";
  printToWeb     = false;
}


// ********************************************************************************
// Web Interface debug page
// ********************************************************************************
void addWideButtonPlusDescription(const String& url, const String& buttonText, const String& description)
{
  html_TR_TD_height(30);
  addWideButton(url, buttonText);
  html_TD(void);
  TXBuffer += description;
}

#endif // ifdef WEBSERVER_TOOLS
