
// ********************************************************************************
// Web Interface Tools page
// ********************************************************************************
void handle_tools() {
  if (!isLoggedIn()) { return; }
  navMenuIndex = MENU_INDEX_TOOLS;
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate(_HEAD);

  String webrequest = WebServer.arg(F("cmd"));

  TXBuffer += F("<form>");
  html_table_class_normal();

  addFormHeader(F("Tools"));

  addFormSubHeader(F("Command"));
  html_TR_TD();
  TXBuffer += F("<TR><TD style='width: 180px'>");
  TXBuffer += F("<input class='wide' type='text' name='cmd' value='");
  TXBuffer += webrequest;
  TXBuffer += "'>";
  html_TD();
  addSubmitButton();
  addHelpButton(F("ESPEasy_Command_Reference"));
  html_TR_TD();

  printToWeb     = true;
  printWebString = "";

  if (webrequest.length() > 0)
  {
    struct EventStruct TempEvent;
    webrequest = parseTemplate(webrequest, webrequest.length()); // @giig1967g: parseTemplate before executing the command
    parseCommandString(&TempEvent, webrequest);
    TempEvent.Source = VALUE_SOURCE_WEB_FRONTEND;

    if (!PluginCall(PLUGIN_WRITE, &TempEvent, webrequest)) {
      ExecuteCommand(VALUE_SOURCE_WEB_FRONTEND, webrequest.c_str());
    }
  }

  if (printWebString.length() > 0)
  {
    TXBuffer += F("<TR><TD colspan='2'>Command Output<BR><textarea readonly rows='10' wrap='on'>");
    TXBuffer += printWebString;
    TXBuffer += F("</textarea>");
  }

  addFormSubHeader(F("System"));

  addWideButtonPlusDescription(F("/?cmd=reboot"), F("Reboot"),           F("Reboots ESP"));
  addWideButtonPlusDescription(F("log"),          F("Log"),              F("Open log output"));
  addWideButtonPlusDescription(F("sysinfo"),      F("Info"),             F("Open system info page"));
  addWideButtonPlusDescription(F("advanced"),     F("Advanced"),         F("Open advanced settings"));
  addWideButtonPlusDescription(F("json"),         F("Show JSON"),        F("Open JSON output"));
  #ifdef WEBSERVER_TIMINGSTATS
  addWideButtonPlusDescription(F("timingstats"),  F("Timing stats"),     F("Open timing statistics of system"));
  #endif // WEBSERVER_TIMINGSTATS
  addWideButtonPlusDescription(F("pinstates"),    F("Pin state buffer"), F("Show Pin state buffer"));
  addWideButtonPlusDescription(F("sysvars"),      F("System Variables"), F("Show all system variables and conversions"));

  addFormSubHeader(F("Wifi"));

  addWideButtonPlusDescription(F("/?cmd=wificonnect"),    F("Connect"),    F("Connects to known Wifi network"));
  addWideButtonPlusDescription(F("/?cmd=wifidisconnect"), F("Disconnect"), F("Disconnect from wifi network"));
  addWideButtonPlusDescription(F("wifiscanner"),          F("Scan"),       F("Scan for wifi networks"));

  addFormSubHeader(F("Interfaces"));

  addWideButtonPlusDescription(F("i2cscanner"), F("I2C Scan"), F("Scan for I2C devices"));

  addFormSubHeader(F("Settings"));

  addWideButtonPlusDescription(F("upload"), F("Load"), F("Loads a settings file"));
  addFormNote(F("(File MUST be renamed to \"config.dat\" before upload!)"));
  addWideButtonPlusDescription(F("download"), F("Save"), F("Saves a settings file"));

#ifdef WEBSERVER_NEW_UI
  # if defined(ESP8266)

  if ((SpiffsFreeSpace() / 1024) > 50) {
    TXBuffer += F("<TR><TD>");
    TXBuffer += F(
      "<script>function downloadUI() { fetch('https://raw.githubusercontent.com/letscontrolit/espeasy_ui/master/build/index.htm.gz').then(r=>r.arrayBuffer()).then(r => {var f=new FormData();f.append('file', new File([new Blob([new Uint8Array(r)])], 'index.htm.gz'));f.append('edit', 1);fetch('/upload',{method:'POST',body:f}).then(() => {window.location.href='/';});}); }</script>");
    TXBuffer += F("<a class=\"button link wide\" onclick=\"downloadUI()\">Download new UI</a>");
    TXBuffer += F("</TD><TD>Download new UI(alpha)</TD></TR>");
  }
  # endif // if defined(ESP8266)
#endif // WEBSERVER_NEW_UI

#if defined(ESP8266)
  {
    #ifndef NO_HTTP_UPDATER
    {
      uint32_t maxSketchSize;
      bool     use2step;
      bool     otaEnabled = OTA_possible(maxSketchSize, use2step);
      addFormSubHeader(F("Firmware"));
      html_TR_TD_height(30);
      addWideButton(F("update"), F("Update Firmware"), "", otaEnabled);
      addHelpButton(F("EasyOTA"));
      html_TD();
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
      TXBuffer += F(" kB");
    }
    #endif
  }
#endif // if defined(ESP8266)

  addFormSubHeader(F("Filesystem"));

  addWideButtonPlusDescription(F("filelist"),      F("File browser"),  F("Show files on internal flash file system"));
  addWideButtonPlusDescription(F("/factoryreset"), F("Factory Reset"), F("Select pre-defined configuration or full erase of settings"));
  addWideButtonPlusDescription(F("/settingsarchive"), F("Settings Archive"), F("Download settings from some archive"));
#ifdef FEATURE_SD
  addWideButtonPlusDescription(F("SDfilelist"),    F("SD Card"),       F("Show files on SD-Card"));
#endif // ifdef FEATURE_SD

  html_end_table();
  html_end_form();
  sendHeadandTail_stdtemplate(_TAIL);
  TXBuffer.endStream();
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
  html_TD();
  TXBuffer += description;
}
