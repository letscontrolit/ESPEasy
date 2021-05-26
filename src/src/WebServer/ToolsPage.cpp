#include "../WebServer/ToolsPage.h"

#include "../WebServer/WebServer.h"
#include "../WebServer/HTML_wrappers.h"
#include "../WebServer/Markup.h"
#include "../WebServer/Markup_Buttons.h"
#include "../WebServer/Markup_Forms.h"

#include "../Helpers/OTA.h"

#include "../../ESPEasy-Globals.h"


#ifdef WEBSERVER_TOOLS

# include "../Commands/InternalCommands.h"
# include "../Helpers/WebServer_commandHelper.h"

// ********************************************************************************
// Web Interface Tools page
// ********************************************************************************
void handle_tools() {
  if (!isLoggedIn()) { return; }
  navMenuIndex = MENU_INDEX_TOOLS;
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate(_HEAD);

  String webrequest = webArg(F("cmd"));

  handle_command_from_web(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, webrequest);
  printToWeb     = false;
  printToWebJSON = false;

  addHtml(F("<form>"));
  html_table_class_normal();

  addFormHeader(F("Tools"));

  addFormSubHeader(F("Command"));
  html_TR_TD();
  addHtml(F("<TR><TD colspan='2'>"));
  addHtml(F("<input "));
  addHtmlAttribute(F("style"), F("width: 98%"));
  addHtmlAttribute(F("type"),  F("text"));
  addHtmlAttribute(F("name"),  F("cmd"));
  addHtmlAttribute(F("value"), webrequest);
  addHtml('>');

  html_TR_TD();
  addSubmitButton();
  addHelpButton(F("ESPEasy_Command_Reference"));
  addRTDHelpButton(F("Reference/Command.html"));
  html_TR_TD();

  if (printWebString.length() > 0)
  {
    addHtml(F("<TR><TD colspan='2'>Command Output<BR><textarea readonly rows='10' wrap='on'>"));
    addHtml(printWebString);
    addHtml(F("</textarea>"));
    printWebString = "";
  }


  addFormSubHeader(F("System"));

  addWideButtonPlusDescription(F("/?cmd=reboot"), F("Reboot"), F("Reboots ESP"));

  # ifdef WEBSERVER_LOG
  addWideButtonPlusDescription(F("log"),          F("Log"),    F("Open log output"));
  # endif // ifdef WEBSERVER_LOG

  # ifdef WEBSERVER_SYSINFO
  addWideButtonPlusDescription(F("sysinfo"), F("Info"), F("Open system info page"));
  # endif // ifdef WEBSERVER_SYSINFO

  # ifdef WEBSERVER_ADVANCED
  addWideButtonPlusDescription(F("advanced"),    F("Advanced"),     F("Open advanced settings"));
  # endif // ifdef WEBSERVER_ADVANCED

  addWideButtonPlusDescription(F("json"),        F("Show JSON"),    F("Open JSON output"));

  # ifdef WEBSERVER_TIMINGSTATS
  addWideButtonPlusDescription(F("timingstats"), F("Timing stats"), F("Open timing statistics of system"));
  # endif // WEBSERVER_TIMINGSTATS

  # ifdef WEBSERVER_PINSTATES
  addWideButtonPlusDescription(F("pinstates"), F("Pin state buffer"), F("Show Pin state buffer"));
  # endif // ifdef WEBSERVER_PINSTATES

  # ifdef WEBSERVER_SYSVARS
  addWideButtonPlusDescription(F("sysvars"), F("System Variables"), F("Show all system variables and conversions"));
  # endif // ifdef WEBSERVER_SYSVARS

  addFormSubHeader(F("Wifi"));

  addWideButtonPlusDescription(F("/?cmd=wificonnect"),    F("Connect"),    F("Connects to known Wifi network"));
  addWideButtonPlusDescription(F("/?cmd=wifidisconnect"), F("Disconnect"), F("Disconnect from wifi network"));

  # ifdef WEBSERVER_WIFI_SCANNER
  addWideButtonPlusDescription(F("wifiscanner"),          F("Scan"),       F("Scan for wifi networks"));
  # endif // ifdef WEBSERVER_WIFI_SCANNER

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

  if ((SpiffsFreeSpace() / 1024) > 50) {
    html_TR_TD();
    addHtml(F(
              "<script>function downloadUI() { fetch('https://raw.githubusercontent.com/letscontrolit/espeasy_ui/master/build/index.htm.gz').then(r=>r.arrayBuffer()).then(r => {var f=new FormData();f.append('file', new File([new Blob([new Uint8Array(r)])], 'index.htm.gz'));f.append('edit', 1);fetch('/upload',{method:'POST',body:f}).then(() => {window.location.href='/';});}); }</script>"));
    addHtml(F("<a class=\"button link wide\" onclick=\"downloadUI()\">Download new UI</a>"));
    addHtml(F("</TD><TD>Download new UI(alpha)</TD></TR>"));
  }
  #  endif // if defined(ESP8266)
# endif    // WEBSERVER_NEW_UI

# if defined(ESP8266) || defined(ESP32)
  {
    #  ifndef NO_HTTP_UPDATER
    {
      uint32_t maxSketchSize;
      bool     use2step;
      bool     otaEnabled = OTA_possible(maxSketchSize, use2step);
      addFormSubHeader(F("Firmware"));
      html_TR_TD_height(30);
      addWideButton(F("update"), F("Update Firmware"), EMPTY_STRING, otaEnabled);
      addHelpButton(F("EasyOTA"));
      html_TD();
      addHtml(F("Load a new firmware "));

      if (otaEnabled) {
        if (use2step) {
          html_B(F("WARNING"));
          addHtml(F(" only use 2-step OTA update."));
        }
      } else {
        html_B(F("WARNING"));
        addHtml(F(" OTA not possible."));
      }
      addHtml(F(" Max sketch size: "));
      addHtmlInt(maxSketchSize / 1024);
      addHtml(F(" kB ("));
      addHtmlInt(maxSketchSize);
      addHtml(F(" bytes)"));
    }
    #  endif // ifndef NO_HTTP_UPDATER
  }
# endif     // if defined(ESP8266)

  addFormSubHeader(F("Filesystem"));

  addWideButtonPlusDescription(F("filelist"),         F("File browser"),     F("Show files on internal flash file system"));
  addWideButtonPlusDescription(F("/factoryreset"),    F("Factory Reset"),    F("Select pre-defined configuration or full erase of settings"));
  # ifdef USE_SETTINGS_ARCHIVE
  addWideButtonPlusDescription(F("/settingsarchive"), F("Settings Archive"), F("Download settings from some archive"));
  # endif // ifdef USE_SETTINGS_ARCHIVE
# ifdef FEATURE_SD
  addWideButtonPlusDescription(F("SDfilelist"),       F("SD Card"),          F("Show files on SD-Card"));
# endif   // ifdef FEATURE_SD

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
void addWideButtonPlusDescription(const __FlashStringHelper * url,
                                  const __FlashStringHelper * buttonText,
                                  const __FlashStringHelper * description)
{
  html_TR_TD_height(30);
  addWideButton(url, buttonText);
  html_TD();
  addHtml(description);
}

void addWideButtonPlusDescription(const String& url, const String& buttonText, const String& description)
{
  html_TR_TD_height(30);
  addWideButton(url, buttonText);
  html_TD();
  addHtml(description);
}

#endif // ifdef WEBSERVER_TOOLS
