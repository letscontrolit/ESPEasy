#include "../WebServer/ESPEasy_WebServer.h"

#include "../WebServer/common.h"

#include "../WebServer/404.h"
#include "../WebServer/AccessControl.h"
#include "../WebServer/AdvancedConfigPage.h"
#include "../WebServer/CacheControllerPages.h"
#include "../WebServer/ConfigPage.h"
#include "../WebServer/ControlPage.h"
#include "../WebServer/ControllerPage.h"
#include "../WebServer/CustomPage.h"
#include "../WebServer/DevicesPage.h"
#include "../WebServer/DownloadPage.h"
#include "../WebServer/FactoryResetPage.h"
#include "../WebServer/FileList.h"
#include "../WebServer/HTML_wrappers.h"
#include "../WebServer/HardwarePage.h"
#include "../WebServer/I2C_Scanner.h"
#include "../WebServer/JSON.h"
#include "../WebServer/LoadFromFS.h"
#include "../WebServer/Log.h"
#include "../WebServer/Markup.h"
#include "../WebServer/Markup_Buttons.h"
#include "../WebServer/Markup_Forms.h"
#include "../WebServer/NotificationPage.h"
#include "../WebServer/PinStates.h"
#include "../WebServer/RootPage.h"
#include "../WebServer/Rules.h"
#include "../WebServer/SettingsArchive.h"
#include "../WebServer/SetupPage.h"
#include "../WebServer/SysInfoPage.h"
#include "../WebServer/Metrics.h"
#include "../WebServer/SysVarPage.h"
#include "../WebServer/TimingStats.h"
#include "../WebServer/ToolsPage.h"
#include "../WebServer/UploadPage.h"
#include "../WebServer/WiFiScanner.h"

#include "../WebServer/WebTemplateParser.h"


#include "../../ESPEasy-Globals.h"
#include "../../_Plugin_Helper.h"
#include "../../ESPEasy_common.h"

#include "../CustomBuild/CompiletimeDefines.h"

#include "../DataStructs/TimingStats.h"

#include "../DataTypes/SettingsType.h"

#include "../ESPEasyCore/ESPEasyNetwork.h"
#include "../ESPEasyCore/ESPEasyRules.h"
#include "../ESPEasyCore/ESPEasyWifi.h"

#include "../Globals/CPlugins.h"
#include "../Globals/Device.h"
#include "../Globals/NetworkState.h"
#include "../Globals/Protocol.h"
#include "../Globals/SecuritySettings.h"
#include "../Globals/Settings.h"

#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/Hardware.h"
#include "../Helpers/Networking.h"
#include "../Helpers/OTA.h"
#include "../Helpers/StringConverter.h"

#include "../Static/WebStaticData.h"



void safe_strncpy_webserver_arg(char *dest, const String& arg, size_t max_size) {
  if (hasArg(arg)) { 
    safe_strncpy(dest, webArg(arg).c_str(), max_size); 
  }
}

void safe_strncpy_webserver_arg(char *dest, const __FlashStringHelper * arg, size_t max_size) {
  safe_strncpy_webserver_arg(dest, String(arg), max_size);
}

void sendHeadandTail(const __FlashStringHelper * tmplName, bool Tail, bool rebooting) {
  // This function is called twice per serving a web page.
  // So it must keep track of the timer longer than the scope of this function.
  // Therefore use a local static variable.
  #if FEATURE_TIMING_STATS
  static uint64_t statisticsTimerStart = 0;

  if (!Tail) {
    statisticsTimerStart = getMicros64();
  }
  #endif // if FEATURE_TIMING_STATS
  {
    const String fileName = concat(tmplName, F(".htm"));
    fs::File f = tryOpenFile(fileName, "r");

    WebTemplateParser templateParser(Tail, rebooting);
    if (f) {
      bool success = true;
      while (f.available() && success) { 
        success = templateParser.process((char)f.read());
      }
      f.close();
    } else {
      getWebPageTemplateDefault(tmplName, templateParser);
    }
    #ifndef BUILD_NO_RAM_TRACKER
    checkRAM(F("sendWebPage"));
    #endif // ifndef BUILD_NO_RAM_TRACKER

    // web activity timer
    lastWeb = millis();
  }

  if (shouldReboot) {
    // we only add this here as a seperate chunk to prevent using too much memory at once
    serve_JS(JSfiles_e::Reboot);
  }
  STOP_TIMER(HANDLE_SERVING_WEBPAGE);
}

void sendHeadandTail_stdtemplate(bool Tail, bool rebooting) {
  sendHeadandTail(F("TmplStd"), Tail, rebooting);

  if (!Tail) {
    if (!clientIPinSubnet() && WifiIsAP(WiFi.getMode()) && (WiFi.softAPgetStationNum() > 0)) {
      addHtmlError(F("Warning: Connected via AP"));
    }

    #ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      const int nrArgs = web_server.args();

      if (nrArgs > 0) {
        String log = F(" Webserver ");
        log += nrArgs;
        log += F(" Arguments");

        if (nrArgs > 20) {
          log += F(" (First 20)");
        }
        log += ':';

        for (int i = 0; i < nrArgs && i < 20; ++i) {
          log += ' ';
          log += i;
          log += F(": '");
          log += web_server.argName(i);
          log += F("' length: ");
          log += webArg(i).length();
        }
        addLogMove(LOG_LEVEL_INFO, log);
      }
    }
    #endif // ifndef BUILD_NO_DEBUG
  }
}

bool captivePortal() {
  const bool fromAP = web_server.client().localIP() == apIP;
  const bool hasWiFiCredentials = SecuritySettings.hasWiFiCredentials();
  if (hasWiFiCredentials || !fromAP) {
    return false;
  }
  if (!isIP(web_server.hostHeader()) && web_server.hostHeader() != (NetworkGetHostname() + F(".local"))) {
    String redirectURL = F("http://");
    redirectURL += web_server.client().localIP().toString();
    #ifdef WEBSERVER_SETUP
    if (fromAP && !hasWiFiCredentials) {
      redirectURL += F("/setup");
    }
    #endif
    sendHeader(F("Location"), redirectURL, true);
    web_server.send(302, F("text/plain"), EMPTY_STRING);   // Empty content inhibits Content-length header so we have to close the socket ourselves.
    web_server.client().stop(); // Stop is needed because we sent no content length
    return true;
  }
  return false;
}


// ********************************************************************************
// Web Interface init
// ********************************************************************************
// #include "core_version.h"


void WebServerInit()
{
  if (webserver_init) { return; }
  webserver_init = true;

  // Prepare webserver pages
  #ifdef WEBSERVER_ROOT
  web_server.on(F("/"),             handle_root);
  // Entries for several captive portal URLs.
  // Maybe not needed. Might be handled by notFound handler.
  web_server.on(F("/generate_204"), handle_root);  //Android captive portal.
  web_server.on(F("/fwlink"),       handle_root);  //Microsoft captive portal.
  #endif // ifdef WEBSERVER_ROOT
  #ifdef WEBSERVER_ADVANCED
  web_server.on(F("/advanced"),    handle_advanced);
  #endif // ifdef WEBSERVER_ADVANCED
  #ifdef WEBSERVER_CONFIG
  web_server.on(F("/config"),      handle_config);
  #endif // ifdef WEBSERVER_CONFIG
  #ifdef WEBSERVER_CONTROL
  web_server.on(F("/control"),     handle_control);
  #endif // ifdef WEBSERVER_CONTROL
  #ifdef WEBSERVER_CONTROLLERS
  web_server.on(F("/controllers"), handle_controllers);
  #endif // ifdef WEBSERVER_CONTROLLERS
  #ifdef WEBSERVER_DEVICES
  web_server.on(F("/devices"),     handle_devices);
  #endif // ifdef WEBSERVER_DEVICES
  #ifdef WEBSERVER_DOWNLOAD
  web_server.on(F("/download"),    handle_download);
  #endif // ifdef WEBSERVER_DOWNLOAD

#ifdef USES_C016

  web_server.on(F("/dumpcache"),     handle_dumpcache);  // C016 specific entrie
  web_server.on(F("/cache_json"), handle_cache_json); // C016 specific entrie
  web_server.on(F("/cache_csv"),  handle_cache_csv);  // C016 specific entrie
#endif // USES_C016

  #ifdef WEBSERVER_FACTORY_RESET
  web_server.on(F("/factoryreset"),    handle_factoryreset);
  #endif // ifdef WEBSERVER_FACTORY_RESET
  #if FEATURE_SETTINGS_ARCHIVE
  web_server.on(F("/settingsarchive"), handle_settingsarchive);
  #endif // if FEATURE_SETTINGS_ARCHIVE
  #ifdef WEBSERVER_FILELIST
  web_server.on(F("/filelist"),        handle_filelist);
  #endif // ifdef WEBSERVER_FILELIST
  #ifdef WEBSERVER_HARDWARE
  web_server.on(F("/hardware"),        handle_hardware);
  #endif // ifdef WEBSERVER_HARDWARE
  #ifdef WEBSERVER_I2C_SCANNER
  web_server.on(F("/i2cscanner"),      handle_i2cscanner);
  #endif // ifdef WEBSERVER_I2C_SCANNER
  web_server.on(F("/json"),            handle_json); // Also part of WEBSERVER_NEW_UI
  web_server.on(F("/csv"),             handle_csvval);
  web_server.on(F("/log"),             handle_log);
  web_server.on(F("/logjson"),         handle_log_JSON); // Also part of WEBSERVER_NEW_UI
#if FEATURE_NOTIFIER
  web_server.on(F("/notifications"),   handle_notifications);
#endif // if FEATURE_NOTIFIER
  #ifdef WEBSERVER_PINSTATES
  web_server.on(F("/pinstates"),       handle_pinstates);
  #endif // ifdef WEBSERVER_PINSTATES
  #ifdef WEBSERVER_RULES
  web_server.on(F("/rules"),           handle_rules_new);
  web_server.on(F("/rules/"),          Goto_Rules_Root);
  # ifdef WEBSERVER_NEW_RULES
  web_server.on(F("/rules/add"),       []()
  {
    handle_rules_edit(web_server.uri(), true);
  });
  web_server.on(F("/rules/backup"), handle_rules_backup);
  web_server.on(F("/rules/delete"), handle_rules_delete);
  # endif // WEBSERVER_NEW_RULES
  #endif  // WEBSERVER_RULES
#if FEATURE_SD
  web_server.on(F("/SDfilelist"),  handle_SDfilelist);
#endif   // if FEATURE_SD
#ifdef WEBSERVER_SETUP
  web_server.on(F("/setup"),       handle_setup);
#endif // ifdef WEBSERVER_SETUP
#ifdef WEBSERVER_SYSINFO
  web_server.on(F("/sysinfo"),     handle_sysinfo);
#endif // ifdef WEBSERVER_SYSINFO
#ifdef WEBSERVER_METRICS
  web_server.on(F("/metrics"),     handle_metrics);
#endif // ifdef WEBSERVER_METRICS
#ifdef WEBSERVER_SYSVARS
  web_server.on(F("/sysvars"),     handle_sysvars);
#endif // WEBSERVER_SYSVARS
#ifdef WEBSERVER_TIMINGSTATS
  web_server.on(F("/timingstats"), handle_timingstats);
#endif // WEBSERVER_TIMINGSTATS
#ifdef WEBSERVER_TOOLS
  web_server.on(F("/tools"),       handle_tools);
#endif // ifdef WEBSERVER_TOOLS
#ifdef WEBSERVER_UPLOAD
  web_server.on(F("/upload"),      HTTP_GET,  handle_upload);
  web_server.on(F("/upload"),      HTTP_POST, handle_upload_post, handleFileUpload);
#endif // ifdef WEBSERVER_UPLOAD
#ifdef WEBSERVER_WIFI_SCANNER
  web_server.on(F("/wifiscanner"), handle_wifiscanner);
#endif // ifdef WEBSERVER_WIFI_SCANNER

#ifdef WEBSERVER_NEW_UI
  web_server.on(F("/buildinfo"),         handle_buildinfo); // Also part of WEBSERVER_NEW_UI
  web_server.on(F("/factoryreset_json"), handle_factoryreset_json);
  web_server.on(F("/filelist_json"),     handle_filelist_json);
  web_server.on(F("/i2cscanner_json"),   handle_i2cscanner_json);
  #if FEATURE_ESPEASY_P2P
  web_server.on(F("/node_list_json"),    handle_nodes_list_json);
  #endif
  web_server.on(F("/pinstates_json"),    handle_pinstates_json);
  web_server.on(F("/timingstats_json"),  handle_timingstats_json);
  web_server.on(F("/upload_json"),       HTTP_POST, handle_upload_json, handleFileUpload);
  web_server.on(F("/wifiscanner_json"),  handle_wifiscanner_json);
#endif // WEBSERVER_NEW_UI
#if SHOW_SYSINFO_JSON
    web_server.on(F("/sysinfo_json"),      handle_sysinfo_json);
#endif//SHOW_SYSINFO_JSON

  web_server.onNotFound(handleNotFound);

  // List of headers to be recorded
  // "If-None-Match" is used to see whether we need to serve a static file, or simply can reply with a 304 (not modified)
  const char * headerkeys[] = {"If-None-Match"};
  const size_t headerkeyssize = sizeof(headerkeys)/sizeof(char*);
  web_server.collectHeaders(headerkeys, headerkeyssize );
  #if defined(ESP8266) || defined(ESP32)
  {
    # ifndef NO_HTTP_UPDATER
    uint32_t maxSketchSize;
    bool     use2step;
    // allow OTA to smaller version of ESPEasy/other firmware
    if (Settings.AllowOTAUnlimited() || OTA_possible(maxSketchSize, use2step)) {
      httpUpdater.setup(&web_server);
    }
    # endif // ifndef NO_HTTP_UPDATER
  }
  #endif    // if defined(ESP8266)

  #if defined(ESP8266)

  # if FEATURE_SSDP

  if (Settings.UseSSDP)
  {
    web_server.on(F("/ssdp.xml"), HTTP_GET, []() {
      WiFiClient client(web_server.client());

      #ifdef MUSTFIX_CLIENT_TIMEOUT_IN_SECONDS

      // See: https://github.com/espressif/arduino-esp32/pull/6676
      client.setTimeout((CONTROLLER_CLIENTTIMEOUT_DFLT + 500) / 1000); // in seconds!!!!
      Client *pClient = &client;
      pClient->setTimeout(CONTROLLER_CLIENTTIMEOUT_DFLT);
      #else // ifdef MUSTFIX_CLIENT_TIMEOUT_IN_SECONDS
      client.setTimeout(CONTROLLER_CLIENTTIMEOUT_DFLT);                // in msec as it should be!
      #endif // ifdef MUSTFIX_CLIENT_TIMEOUT_IN_SECONDS

      SSDP_schema(client);
    });
    SSDP_begin();
  }
  # endif // if FEATURE_SSDP
  #endif  // if defined(ESP8266)
}

void setWebserverRunning(bool state) {
  if (webserverRunning == state) {
    return;
  }

  if (state) {
    WebServerInit();
    web_server.begin(Settings.WebserverPort);
    addLog(LOG_LEVEL_INFO, F("Webserver: start"));
  } else {
    web_server.stop();
    addLog(LOG_LEVEL_INFO, F("Webserver: stop"));
  }
  webserverRunning = state;
  CheckRunningServices(); // Uses webserverRunning state.
}

void getWebPageTemplateDefault(const String& tmplName, WebTemplateParser& parser)
{
  const bool addJS   = true;
  const bool addMeta = true;

/*
  if (equals(tmplName, F("TmplAP")))
  {

    getWebPageTemplateDefaultHead(parser, addMeta, !addJS);

    if (!parser.isTail()) {
      #ifndef WEBPAGE_TEMPLATE_AP_HEADER
      parser.process(F("<body"
                       #if FEATURE_AUTO_DARK_MODE
                       " data-theme='auto'"
                       #endif // FEATURE_AUTO_DARK_MODE
                       "><header class='apheader'>"
                       "<h1>Welcome to ESP Easy Mega AP</h1>"));
      #else
      parser.process(F(WEBPAGE_TEMPLATE_AP_HEADER));
      #endif

      parser.process(F("</header>"));
    }
    getWebPageTemplateDefaultContentSection(parser);
    getWebPageTemplateDefaultFooter(parser);
  }
  else 
  */
  if (equals(tmplName, F("TmplMsg")))
  {
    getWebPageTemplateDefaultHead(parser, !addMeta, !addJS);
    if (!parser.isTail()) {
      parser.process(F("<body"
                       #if FEATURE_AUTO_DARK_MODE
                       " data-theme='auto'"
                       #endif // FEATURE_AUTO_DARK_MODE
                       ">"));
    }
    getWebPageTemplateDefaultHeader(parser, F("{{name}}"), false);
    getWebPageTemplateDefaultContentSection(parser);
    getWebPageTemplateDefaultFooter(parser);
  }
  else if (equals(tmplName, F("TmplDsh")))
  {
    getWebPageTemplateDefaultHead(parser, !addMeta, addJS);
    parser.process(F("<body"));
    #if FEATURE_AUTO_DARK_MODE
    if (0 == Settings.getCssMode()) {
      parser.process(F(" data-theme='auto'"));
    } else if (2 == Settings.getCssMode()) {
      parser.process(F(" data-theme='dark'"));
    }
    #endif // FEATURE_AUTO_DARK_MODE
    parser.process(F(">"
                     "{{content}}"
                     "</body></html>"));
  }
  else // all other template names e.g. TmplStd
  {
    getWebPageTemplateDefaultHead(parser, addMeta, addJS);
    if (!parser.isTail()) {
      parser.process(F("<body class='bodymenu'"));
      #if FEATURE_AUTO_DARK_MODE
      if (0 == Settings.getCssMode()) {
        parser.process(F(" data-theme='auto'"));
      } else if (2 == Settings.getCssMode()) {
        parser.process(F(" data-theme='dark'"));
      }
      #endif // FEATURE_AUTO_DARK_MODE
      parser.process(F("><span class='message' id='rbtmsg'></span>"));
    }
    getWebPageTemplateDefaultHeader(parser, F("{{name}} {{logo}}"), true);
    getWebPageTemplateDefaultContentSection(parser);
    getWebPageTemplateDefaultFooter(parser);
  }
//  addLog(LOG_LEVEL_INFO, String(F("tmpl.length(): ")) + String(tmpl.length()));
}

void getWebPageTemplateDefaultHead(WebTemplateParser& parser, bool addMeta, bool addJS) {
  if (parser.isTail()) return;
  parser.process(F("<!DOCTYPE html><html lang='en'>"
            "<head>"
            "<meta charset='utf-8'/>"
            "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
            "<title>{{name}}</title>"));

  if (addMeta) { parser.process(F("{{meta}}")); }

  if (addJS) { parser.process(F("{{js}}")); }

  parser.process(F("{{css}}"
                   "</head>"));
}

void getWebPageTemplateDefaultHeader(WebTemplateParser& parser, const __FlashStringHelper * title, bool addMenu) {
  {
    if (parser.isTail()) return;
  #ifndef WEBPAGE_TEMPLATE_DEFAULT_HEADER
    parser.process(F("<header class='headermenu'><h1>ESP Easy Mega: "));
    parser.process(title);
    #if BUILD_IN_WEBHEADER
    parser.process(F("<div style='float:right;font-size:10pt'>Build: " GITHUB_RELEASES_LINK_PREFIX "{{date}}" GITHUB_RELEASES_LINK_SUFFIX "</div>"));
    #endif // #if BUILD_IN_WEBHEADER
    parser.process(F("</h1><BR>"));
  #else // ifndef WEBPAGE_TEMPLATE_DEFAULT_HEADER
    String tmp = F(WEBPAGE_TEMPLATE_DEFAULT_HEADER);
    tmp.replace(F("{{title}}"), title);
    parser.process(tmp);
  #endif // ifndef WEBPAGE_TEMPLATE_DEFAULT_HEADER
  }

  if (addMenu) { parser.process(F("{{menu}}")); }
  parser.process(F("</header>"));
}

void getWebPageTemplateDefaultContentSection(WebTemplateParser& parser) {
  parser.process(F("<section>"
            "<span class='message error'>"
            "{{error}}"
            "</span>"
            "{{content}}"
            "</section>"
            ));
}

void getWebPageTemplateDefaultFooter(WebTemplateParser& parser) {
  if (!parser.isTail()) return;
  #ifndef WEBPAGE_TEMPLATE_DEFAULT_FOOTER
  parser.process(F("<footer>"
            "<br>"
            "<h6>Powered by <a href='http://www.letscontrolit.com' style='font-size: 15px; text-decoration: none'>Let's Control It</a> community"
            #if BUILD_IN_WEBFOOTER
            "<div style='float: right'>Build: " GITHUB_RELEASES_LINK_PREFIX "{{build}} {{date}}" GITHUB_RELEASES_LINK_SUFFIX "</div>"
            #endif // #if BUILD_IN_WEBFOOTER
            "</h6>"
            "</footer>"
            "</body></html>"
            ));
#else // ifndef WEBPAGE_TEMPLATE_DEFAULT_FOOTER
  parser.process(F(WEBPAGE_TEMPLATE_DEFAULT_FOOTER));
#endif // ifndef WEBPAGE_TEMPLATE_DEFAULT_FOOTER
}



void writeDefaultCSS(void)
{
  return; // TODO

/*
#ifndef WEBSERVER_USE_CDN_JS_CSS

  if (!fileExists(F("esp.css")))
  {
    fs::File f = tryOpenFile(F("esp.css"), "w");

    if (f)
    {
      String defaultCSS;
      defaultCSS = PGMT(DATA_ESPEASY_DEFAULT_MIN_CSS);

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        String log = F("CSS  : Writing default CSS file to FS (");
        log += defaultCSS.length();
        log += F(" bytes)");
        addLog(LOG_LEVEL_INFO, log);
      }
      f.write((const unsigned char *)defaultCSS.c_str(), defaultCSS.length()); // note: content must be in RAM - a write of F("XXX") does
                                                                               // not work
      f.close();
    }
  }
#endif
*/
}

// ********************************************************************************
// Functions to stream JSON directly to TXBuffer
// FIXME TD-er: replace stream_xxx_json_object* into this code.
// N.B. handling of numerical values differs (string vs. no string)
// ********************************************************************************

int8_t level     = 0;
int8_t lastLevel = -1;

void json_quote_name(const __FlashStringHelper * val) {
  json_quote_name(String(val));
}

void json_quote_name(const String& val) {
  if (lastLevel == level) {
    addHtml(',');
  }

  if (val.length() > 0) {
    json_quote_val(val);
    addHtml(':');
  }
}

void json_quote_val(const String& val) {
  addHtml('\"');
  addHtml(val);
  addHtml('\"');
}

void json_open(bool arr) {
  json_open(arr, EMPTY_STRING);
}

void json_open(bool arr, const __FlashStringHelper * name) {
  json_quote_name(name);
  addHtml(arr ? '[' : '{');
  lastLevel = level;
  level++;
}

void json_open(bool arr, const String& name) {
  json_quote_name(name);
  addHtml(arr ? '[' : '{');
  lastLevel = level;
  level++;
}

void json_init() {
  level     = 0;
  lastLevel = -1;
}

void json_close() {
  json_close(false);
}

void json_close(bool arr) {
  addHtml(arr ? ']' : '}');
  level--;
  lastLevel = level;
}

void json_number(const __FlashStringHelper * name, const String& value)
{
  json_prop(name, value);
}


void json_number(const String& name, const String& value) {
  json_prop(name, value);
}

void json_prop(const __FlashStringHelper * name, const String& value) 
{
  json_quote_name(name);
  json_quote_val(value);
  lastLevel = level;
}


void json_prop(const String& name, const String& value) {
  json_quote_name(name);
  json_quote_val(value);
  lastLevel = level;
}

void json_prop(LabelType::Enum label) {
  json_prop(getInternalLabel(label, '-'), getValue(label));
}

// ********************************************************************************
// Add a task select dropdown list
// This allows to select a task index based on the existing tasks.
// ********************************************************************************
void addTaskSelect(const String& name,  taskIndex_t choice)
{
  String deviceName;

  addHtml(F("<select "));
  addHtmlAttribute(F("id"),       F("selectwidth"));
  addHtmlAttribute(F("name"),     name);
  addHtmlAttribute(F("onchange"), F("return task_select_onchange(frmselect)"));
  addHtml('>');

  for (taskIndex_t x = 0; x <= TASKS_MAX; x++)
  {
    if (validTaskIndex(x)) {
      const deviceIndex_t DeviceIndex = getDeviceIndex_from_TaskIndex(x);
      deviceName = getPluginNameFromDeviceIndex(DeviceIndex);
    } else {
      deviceName = F("Not Set");
    }
    {
      addHtml(F("<option value='"));
      addHtmlInt(x);
      addHtml('\'');

      if (choice == x) {
        addHtml(F(" selected"));
      }
    }

    if (validTaskIndex(x) && !validPluginID_fullcheck(Settings.TaskDeviceNumber[x])) {
      addDisabled();
    }
    {
      addHtml('>');
      if (validTaskIndex(x)) {
        addHtmlInt(x + 1);
      }
      addHtml(F(" - "));
      addHtml(deviceName);
      addHtml(F(" - "));
      addHtml(getTaskDeviceName(x));
      addHtml(F("</option>"));
    }
  }
}

// ********************************************************************************
// Add a Value select dropdown list, based on TaskIndex
// This allows to select a task value, based on the existing tasks.
// ********************************************************************************
void addTaskValueSelect(const String& name, int choice, taskIndex_t TaskIndex)
{
  if (!validTaskIndex(TaskIndex)) { return; }
  const deviceIndex_t DeviceIndex = getDeviceIndex_from_TaskIndex(TaskIndex);

  if (!validDeviceIndex(DeviceIndex)) { return; }

  addHtml(F("<select "));
  addHtmlAttribute(F("id"),   F("selectwidth"));
  addHtmlAttribute(F("name"), name);
  addHtml('>');

  const uint8_t valueCount = getValueCountForTask(TaskIndex);

  for (uint8_t x = 0; x < valueCount; x++)
  {
    addHtml(F("<option value='"));
    addHtmlInt(x);
    addHtml('\'');

    if (choice == x) {
      addHtml(F(" selected"));
    }
    addHtml('>');
    addHtml(getTaskValueName(TaskIndex, x));
    addHtml(F("</option>"));
  }
}


// ********************************************************************************
// Login state check
// ********************************************************************************
bool isLoggedIn(bool mustProvideLogin)
{
  if (!clientIPallowed()) { return false; }

  if (SecuritySettings.Password[0] == 0) { return true; }
  
  if (!mustProvideLogin) {
    return false;
  }
  
  {
    String www_username = F(DEFAULT_ADMIN_USERNAME);
    if (!web_server.authenticate(www_username.c_str(), SecuritySettings.Password))

    // Basic Auth Method with Custom realm and Failure Response
    // return server.requestAuthentication(BASIC_AUTH, www_realm, authFailResponse);
    // Digest Auth Method with realm="Login Required" and empty Failure Response
    // return server.requestAuthentication(DIGEST_AUTH);
    // Digest Auth Method with Custom realm and empty Failure Response
    // return server.requestAuthentication(DIGEST_AUTH, www_realm);
    // Digest Auth Method with Custom realm and Failure Response
    {
#ifdef CORE_PRE_2_5_0

      // See https://github.com/esp8266/Arduino/issues/4717
      HTTPAuthMethod mode = BASIC_AUTH;
#else // ifdef CORE_PRE_2_5_0
      HTTPAuthMethod mode = DIGEST_AUTH;
#endif // ifdef CORE_PRE_2_5_0
      String message = F("Login Required (default user: ");
      message += www_username;
      message += ')';
      web_server.requestAuthentication(mode, message.c_str());

      if (Settings.UseRules)
      {
        String event = F("Login#Failed");

        // TD-er: Do not add to the eventQueue, but execute right now.
        rulesProcessing(event);
      }

      return false;
    }
  }
  return true;
}

String getControllerSymbol(uint8_t index)
{
  String ret = F("<span style='font-size:20px; background: #00000000;'>&#");

  ret += 10102 + index;
  ret += F(";</span>");
  return ret;
}

/*
   String getValueSymbol(uint8_t index)
   {
   String ret = F("&#");
   ret += 10112 + index;
   ret += ';';
   return ret;
   }
 */

void addSVG_param(const __FlashStringHelper * key, int value) {
  addHtml(' ');
  addHtml(key);
  addHtml('=');
  addHtml('\"');
  addHtmlInt(value);
  addHtml('\"');
}

void addSVG_param(const __FlashStringHelper * key, float value) {
  addSVG_param(key, toString(value, 2));
}

void addSVG_param(const __FlashStringHelper * key, const String& value) {
  addHtml(' ');
  addHtml(key);
  addHtml('=');
  addHtml('\"');
  addHtml(value);
  addHtml('\"');
}

void createSvgRect_noStroke(const __FlashStringHelper * classname, unsigned int fillColor, float xoffset, float yoffset, float width, float height, float rx, float ry) {
  createSvgRect(classname, fillColor, fillColor, xoffset, yoffset, width, height, 0, rx, ry);
}

void createSvgRect(const String& classname,
                   unsigned int fillColor,
                   unsigned int strokeColor,
                   float        xoffset,
                   float        yoffset,
                   float        width,
                   float        height,
                   float        strokeWidth,
                   float        rx,
                   float        ry) {
  addHtml(F("<rect"));
  if (!classname.isEmpty()) {
    addSVG_param(F("class"), classname);
  }
  addSVG_param(F("fill"), formatToHex(fillColor, F("#"), 3));

  if (!approximatelyEqual(strokeWidth, 0)) {
    addSVG_param(F("stroke"),       formatToHex(strokeColor, F("#"), 3));
    addSVG_param(F("stroke-width"), strokeWidth);
  }
  addSVG_param(F("x"),      xoffset);
  addSVG_param(F("y"),      yoffset);
  addSVG_param(F("width"),  width);
  addSVG_param(F("height"), height);
  addSVG_param(F("rx"),     rx);
  addSVG_param(F("ry"),     ry);
  addHtml(F("/>"));
}

void createSvgHorRectPath(unsigned int color, int xoffset, int yoffset, int size, int height, int range, float SVG_BAR_WIDTH) {
  float width = SVG_BAR_WIDTH * size / range;

  if (width < 2) { width = 2; }
  addHtml(formatToHex(color, F("<path fill=\"#")));
  addHtml(F("\" d=\"M"));
  addHtml(toString(SVG_BAR_WIDTH * xoffset / range, 2));
  addHtml(' ');
  addHtmlInt(yoffset);
  addHtml('h');
  addHtml(toString(width, 2));
  addHtml('v');
  addHtmlInt(height);
  addHtml('H');
  addHtml(toString(SVG_BAR_WIDTH * xoffset / range, 2));
  addHtml(F("z\"/>\n"));
}

void createSvgTextElement(const String& text, float textXoffset, float textYoffset) {
  addHtml(F("<text x=\""));
  addHtml(toString(textXoffset, 2));
  addHtml(F("\" y=\""));
  addHtml(toString(textYoffset, 2));
  addHtml(F("\" >\n"));
  addHtml(F("<tspan x=\""));
  addHtml(toString(textXoffset, 2));
  addHtml(F("\" y=\""));
  addHtml(toString(textYoffset, 2));
  addHtml('"', '>');
  addHtml(text);
  addHtml(F("</tspan>\n</text>"));
}

#define SVG_BAR_HEIGHT 16
#define SVG_BAR_WIDTH 400

void write_SVG_image_header(int width, int height, bool useViewbox) {
  addHtml(F("<svg xmlns=\"http://www.w3.org/2000/svg\" width=\""));
  addHtmlInt(width);
  addHtml(F("\" height=\""));
  addHtmlInt(height);
  addHtml(F("\" version=\"1.1\""));

  if (useViewbox) {
    addHtml(F(" viewBox=\"0 0 100 100\""));
  }
  addHtml('>');
  addHtml(F("<style>text{line-height:1.25;stroke-width:.3;font-family:sans-serif;font-size:8;letter-spacing:0;word-spacing:0;"));
  #if FEATURE_AUTO_DARK_MODE
  if (2 == Settings.getCssMode()) { // Dark
    addHtml(F("fill:#c3c3c3;"));    // Copied from espeasy_default.css var(--c4) in dark section
  }
  addHtml('}');
  if (0 == Settings.getCssMode()) { // Auto
    addHtml(F("@media(prefers-color-scheme:dark){text{fill:#c3c3c3;}}")); // ditto
  }
  #else // FEATURE_AUTO_DARK_MODE
  addHtml('}'); // close 'text' style
  #endif // FEATURE_AUTO_DARK_MODE
  addHtml(F("</style>"));
}

/*
   void getESPeasyLogo(int width_pixels) {
   write_SVG_image_header(width_pixels, width_pixels, true);
   addHtml(F("<g transform=\"translate(-33.686 -7.8142)\"><rect x=\"49\" y=\"23.1\" width=\"69.3\" height=\"69.3\" fill=\"#2c72da\"
      stroke=\"#2c72da\"
      stroke-linecap=\"round\"stroke-linejoin=\"round\" stroke-width=\"30.7\"/><g transform=\"matrix(3.3092 0 0 3.3092 -77.788
         -248.96)\"><path d=\"m37.4 89 7.5-7.5M37.4 96.5l15-15M37.4 96.5l15-15M37.4 104l22.5-22.5M44.9 104l15-15\"
      fill=\"none\"stroke=\"#fff\" stroke-linecap=\"round\" stroke-width=\"2.6\"/><circle cx=\"58\" cy=\"102.1\" r=\"3\"
         fill=\"#fff\"/></g></g></svg>");
   }
 */
void getWiFi_RSSI_icon(int rssi, int width_pixels)
{
  const int nbars_filled = (rssi + 100) / 8;
  int nbars              = 5;
  int white_between_bar  = (static_cast<float>(width_pixels) / nbars) * 0.2f;

  if (white_between_bar < 1) { white_between_bar = 1; }
  const int barWidth   = (width_pixels - (nbars - 1) * white_between_bar) / nbars;
  int svg_width_pixels = nbars * barWidth + (nbars - 1) * white_between_bar;

  write_SVG_image_header(svg_width_pixels, svg_width_pixels, true);
  float scale               = 100.0f / svg_width_pixels;
  const int bar_height_step = 100 / nbars;

  for (int i = 0; i < nbars; ++i) {
    const unsigned int color = i < nbars_filled ? 0x07d : 0xBFa1a1a1; // Blue/Grey75%
    const int barHeight      = (i + 1) * bar_height_step;
    createSvgRect_noStroke(i < nbars_filled ? F("bar_highlight") : F("bar_dimmed"), color, i * (barWidth + white_between_bar) * scale, 100 - barHeight, barWidth, barHeight, 0, 0);
  }
  addHtml(F("</svg>\n"));
}

#ifndef BUILD_MINIMAL_OTA
void getConfig_dat_file_layout() {
  const int shiftY  = 2;
  float     yOffset = shiftY;

  write_SVG_image_header(SVG_BAR_WIDTH + 250, SVG_BAR_HEIGHT + shiftY);

  int max_index, offset, max_size;
  int struct_size = 0;

  // background
  const uint32_t realSize = SettingsType::getFileSize(SettingsType::Enum::TaskSettings_Type);

  createSvgHorRectPath(0xcdcdcd, 0, yOffset, realSize, SVG_BAR_HEIGHT - 2, realSize, SVG_BAR_WIDTH);

  for (int st = 0; st < static_cast<int>(SettingsType::Enum::SettingsType_MAX); ++st) {
    SettingsType::Enum settingsType = static_cast<SettingsType::Enum>(st);

    if (SettingsType::getSettingsFile(settingsType) == SettingsType::SettingsFileEnum::FILE_CONFIG_type) {
      unsigned int color = SettingsType::getSVGcolor(settingsType);
      SettingsType::getSettingsParameters(settingsType, 0, max_index, offset, max_size, struct_size);

      for (int i = 0; i < max_index; ++i) {
        SettingsType::getSettingsParameters(settingsType, i, offset, max_size);

        // Struct position
        createSvgHorRectPath(color, offset, yOffset, max_size, SVG_BAR_HEIGHT - 2, realSize, SVG_BAR_WIDTH);
      }
    }
  }

  // Text labels
  float textXoffset = SVG_BAR_WIDTH + 2;
  float textYoffset = yOffset + 0.9f * SVG_BAR_HEIGHT;

  createSvgTextElement(SettingsType::getSettingsFileName(SettingsType::Enum::TaskSettings_Type), textXoffset, textYoffset);
  addHtml(F("</svg>\n"));
}

void getStorageTableSVG(SettingsType::Enum settingsType) {
  uint32_t realSize   = SettingsType::getFileSize(settingsType);
  unsigned int color  = SettingsType::getSVGcolor(settingsType);
  const int    shiftY = 2;

  int max_index, offset, max_size;
  int struct_size = 0;

  SettingsType::getSettingsParameters(settingsType, 0, max_index, offset, max_size, struct_size);

  if (max_index == 0) { return; }

  // One more to add bar indicating struct size vs. reserved space.
  write_SVG_image_header(SVG_BAR_WIDTH + 250, (max_index + 1) * SVG_BAR_HEIGHT + shiftY);
  float yOffset = shiftY;

  for (int i = 0; i < max_index; ++i) {
    SettingsType::getSettingsParameters(settingsType, i, offset, max_size);

    // background
    createSvgHorRectPath(0xcdcdcd, 0,      yOffset, realSize, SVG_BAR_HEIGHT - 2, realSize, SVG_BAR_WIDTH);

    // Struct position
    createSvgHorRectPath(color,    offset, yOffset, max_size, SVG_BAR_HEIGHT - 2, realSize, SVG_BAR_WIDTH);

    // Text labels
    float textXoffset = SVG_BAR_WIDTH + 2;
    float textYoffset = yOffset + 0.9f * SVG_BAR_HEIGHT;
    createSvgTextElement(formatHumanReadable(offset, 1024),   textXoffset, textYoffset);
    textXoffset = SVG_BAR_WIDTH + 60;
    createSvgTextElement(formatHumanReadable(max_size, 1024), textXoffset, textYoffset);
    textXoffset = SVG_BAR_WIDTH + 130;
    createSvgTextElement(String(i),                           textXoffset, textYoffset);
    yOffset += SVG_BAR_HEIGHT;
  }

  // usage
  createSvgHorRectPath(0xcdcdcd, 0, yOffset, max_size, SVG_BAR_HEIGHT - 2, max_size, SVG_BAR_WIDTH);

  // Struct size (used part of the reserved space)
  if (struct_size != 0) {
    createSvgHorRectPath(color, 0, yOffset, struct_size, SVG_BAR_HEIGHT - 2, max_size, SVG_BAR_WIDTH);
  }

  // Text labels
  float textXoffset = SVG_BAR_WIDTH + 2;
  float textYoffset = yOffset + 0.9f * SVG_BAR_HEIGHT;

  if (struct_size != 0) {
    String text;
    text.reserve(32);
    text += formatHumanReadable(struct_size, 1024);
    text += '/';
    text += formatHumanReadable(max_size, 1024);
    text += F(" per item");
    createSvgTextElement(text, textXoffset, textYoffset);
  } else {
    createSvgTextElement(F("Variable size"), textXoffset, textYoffset);
  }
  addHtml(F("</svg>\n"));
}

#endif // ifndef BUILD_MINIMAL_OTA

#ifdef ESP32

# include <esp_partition.h>


void getPartitionTableSVG(uint8_t pType, unsigned int partitionColor) {
  int nrPartitions = getPartionCount(pType);

  if (nrPartitions == 0) { return; }
  const int shiftY = 2;

  uint32_t realSize                      = getFlashRealSizeInBytes();
  esp_partition_type_t     partitionType = static_cast<esp_partition_type_t>(pType);
  const esp_partition_t   *_mypart;
  esp_partition_iterator_t _mypartiterator = esp_partition_find(partitionType, ESP_PARTITION_SUBTYPE_ANY, nullptr);

  write_SVG_image_header(SVG_BAR_WIDTH + 250, nrPartitions * SVG_BAR_HEIGHT + shiftY);
  float yOffset = shiftY;

  if (_mypartiterator) {
    do {
      _mypart = esp_partition_get(_mypartiterator);
      createSvgHorRectPath(0xcdcdcd,       0,                yOffset, realSize,      SVG_BAR_HEIGHT - 2, realSize, SVG_BAR_WIDTH);
      createSvgHorRectPath(partitionColor, _mypart->address, yOffset, _mypart->size, SVG_BAR_HEIGHT - 2, realSize, SVG_BAR_WIDTH);
      float textXoffset = SVG_BAR_WIDTH + 2;
      float textYoffset = yOffset + 0.9f * SVG_BAR_HEIGHT;
      createSvgTextElement(formatHumanReadable(_mypart->size, 1024),          textXoffset, textYoffset);
      textXoffset = SVG_BAR_WIDTH + 60;
      createSvgTextElement(_mypart->label,                                    textXoffset, textYoffset);
      textXoffset = SVG_BAR_WIDTH + 130;
      createSvgTextElement(getPartitionType(_mypart->type, _mypart->subtype), textXoffset, textYoffset);
      yOffset += SVG_BAR_HEIGHT;
    } while ((_mypartiterator = esp_partition_next(_mypartiterator)) != nullptr);
  }
  addHtml(F("</svg>\n"));
  esp_partition_iterator_release(_mypartiterator);
}

#endif // ifdef ESP32

bool webArg2ip(const __FlashStringHelper * arg, uint8_t *IP) {
  return str2ip(webArg(arg), IP);
}
