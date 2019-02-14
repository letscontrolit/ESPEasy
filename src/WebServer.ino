//********************************************************************************
// Allowed IP range check
//********************************************************************************
#define ALL_ALLOWED            0
#define LOCAL_SUBNET_ALLOWED   1
#define ONLY_IP_RANGE_ALLOWED  2
#define _HEAD false
#define _TAIL true
#define CHUNKED_BUFFER_SIZE          400

void sendContentBlocking(String& data);
void sendHeaderBlocking(bool json, const String& origin = "");

class StreamingBuffer {
private:
  bool lowMemorySkip;

public:
  uint32_t initialRam;
  uint32_t beforeTXRam;
  uint32_t duringTXRam;
  uint32_t finalRam;
  uint32_t maxCoreUsage;
  uint32_t maxServerUsage;
  unsigned int sentBytes;
  uint32_t flashStringCalls;
  uint32_t flashStringData;
  String buf;

  StreamingBuffer(void) : lowMemorySkip(false),
    initialRam(0), beforeTXRam(0), duringTXRam(0), finalRam(0), maxCoreUsage(0),
    maxServerUsage(0), sentBytes(0), flashStringCalls(0), flashStringData(0)
  {
    buf.reserve(CHUNKED_BUFFER_SIZE + 50);
    buf = "";
  }
  StreamingBuffer operator= (String& a)                 { flush(); return addString(a); }
  StreamingBuffer operator= (const String& a)           { flush(); return addString(a); }
  StreamingBuffer operator+= (char a)                   { return addString(String(a)); }
  StreamingBuffer operator+= (long unsigned int  a)     { return addString(String(a)); }
  StreamingBuffer operator+= (float a)                  { return addString(String(a)); }
  StreamingBuffer operator+= (int a)                    { return addString(String(a)); }
  StreamingBuffer operator+= (uint32_t a)               { return addString(String(a)); }
  StreamingBuffer operator+= (const String& a)          { return addString(a); }

  StreamingBuffer operator+= (PGM_P str) {
    ++flashStringCalls;
    if (!str) return *this; // return if the pointer is void
    if (lowMemorySkip) return *this;
    int flush_step = CHUNKED_BUFFER_SIZE - this->buf.length();
    if (flush_step < 1) flush_step = 0;
    unsigned int pos = 0;
    const unsigned int length = strlen_P((PGM_P)str);
    if (length == 0) return *this;
    flashStringData += length;
    while (pos < length) {
      if (flush_step == 0) {
        sendContentBlocking(this->buf);
        flush_step = CHUNKED_BUFFER_SIZE;
      }
      this->buf += (char)pgm_read_byte(&str[pos]);
      ++pos;
      --flush_step;
    }
    checkFull();
    return *this;
  }


  StreamingBuffer addString(const String& a) {
    if (lowMemorySkip) return *this;
    int flush_step = CHUNKED_BUFFER_SIZE - this->buf.length();
    if (flush_step < 1) flush_step = 0;
    int pos = 0;
    const int length = a.length();
    while (pos < length) {
      if (flush_step == 0) {
        sendContentBlocking(this->buf);
        flush_step = CHUNKED_BUFFER_SIZE;
      }
      this->buf += a[pos];
      ++pos;
      --flush_step;
    }
    checkFull();
    return *this;
  }

  void flush() {
    if (lowMemorySkip) {
      this->buf = "";
    } else {
      sendContentBlocking(this->buf);
    }
  }

  void checkFull(void) {
    if (lowMemorySkip) this->buf = "";
    if (this->buf.length() > CHUNKED_BUFFER_SIZE) {
      trackTotalMem();
      sendContentBlocking(this->buf);
    }
  }

  void startStream() {
    startStream(false, "");
  }

  void startStream(const String& origin) {
    startStream(false, origin);
  }

  void startJsonStream() {
    startStream(true, "*");
  }

private:
  void startStream(bool json, const String& origin) {
    maxCoreUsage = maxServerUsage = 0;
    initialRam = ESP.getFreeHeap();
    beforeTXRam = initialRam;
    sentBytes = 0;
    buf = "";
    if (beforeTXRam < 3000) {
      lowMemorySkip = true;
      WebServer.send(200, "text/plain", "Low memory. Cannot display webpage :-(");
       #if defined(ESP8266)
         tcpCleanup();
       #endif
      return;
    } else
      sendHeaderBlocking(json, origin);
  }

  void trackTotalMem() {
    beforeTXRam = ESP.getFreeHeap();
    if ((initialRam - beforeTXRam) > maxServerUsage)
      maxServerUsage = initialRam - beforeTXRam;
  }

public:

  void trackCoreMem() {
    duringTXRam = ESP.getFreeHeap();
    if ((initialRam - duringTXRam) > maxCoreUsage)
      maxCoreUsage = (initialRam - duringTXRam);
  }

  void endStream(void) {
    if (!lowMemorySkip) {
      if (buf.length() > 0) sendContentBlocking(buf);
      buf = "";
      sendContentBlocking(buf);
      finalRam = ESP.getFreeHeap();
      /*
      if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
        String log = String("Ram usage: Webserver only: ") + maxServerUsage +
                     " including Core: " + maxCoreUsage +
                     " flashStringCalls: " + flashStringCalls +
                     " flashStringData: " + flashStringData;
        addLog(LOG_LEVEL_DEBUG, log);
      }
      */
    } else {
      addLog(LOG_LEVEL_ERROR, String("Webpage skipped: low memory: ") + finalRam);
      lowMemorySkip = false;
    }
  }
} TXBuffer;

void sendContentBlocking(String& data) {
  checkRAM(F("sendContentBlocking"));
  uint32_t freeBeforeSend = ESP.getFreeHeap();
  const uint32_t length = data.length();
#ifndef BUILD_NO_DEBUG
  addLog(LOG_LEVEL_DEBUG_DEV, String("sendcontent free: ") + freeBeforeSend + " chunk size:" + length);
#endif
  freeBeforeSend = ESP.getFreeHeap();
  if (TXBuffer.beforeTXRam > freeBeforeSend)
    TXBuffer.beforeTXRam = freeBeforeSend;
  TXBuffer.duringTXRam = freeBeforeSend;
#if defined(ESP8266) && defined(ARDUINO_ESP8266_RELEASE_2_3_0)
  String size = formatToHex(length) + "\r\n";
  // do chunked transfer encoding ourselves (WebServer doesn't support it)
  WebServer.sendContent(size);
  if (length > 0) WebServer.sendContent(data);
  WebServer.sendContent("\r\n");
#else  // ESP8266 2.4.0rc2 and higher and the ESP32 webserver supports chunked http transfer
  unsigned int timeout = 0;
  if (freeBeforeSend < 5000) timeout = 100;
  if (freeBeforeSend < 4000) timeout = 1000;
  const uint32_t beginWait = millis();
  WebServer.sendContent(data);
  while ((ESP.getFreeHeap() < freeBeforeSend) &&
         !timeOutReached(beginWait + timeout)) {
    if (ESP.getFreeHeap() < TXBuffer.duringTXRam)
      TXBuffer.duringTXRam = ESP.getFreeHeap();
    ;
    TXBuffer.trackCoreMem();
    checkRAM(F("duringDataTX"));
    delay(1);
  }
#endif

  TXBuffer.sentBytes += length;
  data = "";
  delay(0);
}

void sendHeaderBlocking(bool json, const String& origin) {
  checkRAM(F("sendHeaderBlocking"));
  WebServer.client().flush();
  String contenttype;
  if (json)
    contenttype = F("application/json");
  else
    contenttype = F("text/html");

#if defined(ESP8266) && defined(ARDUINO_ESP8266_RELEASE_2_3_0)
  WebServer.setContentLength(CONTENT_LENGTH_UNKNOWN);
  WebServer.sendHeader(F("Accept-Ranges"), F("none"));
  WebServer.sendHeader(F("Cache-Control"), F("no-cache"));
  WebServer.sendHeader(F("Transfer-Encoding"), F("chunked"));
  if (json)
    WebServer.sendHeader(F("Access-Control-Allow-Origin"),"*");
  WebServer.send(200, contenttype, "");
#else
  unsigned int timeout = 0;
  uint32_t freeBeforeSend = ESP.getFreeHeap();
  if (freeBeforeSend < 5000) timeout = 100;
  if (freeBeforeSend < 4000) timeout = 1000;
  const uint32_t beginWait = millis();
  WebServer.setContentLength(CONTENT_LENGTH_UNKNOWN);
  WebServer.sendHeader(F("Cache-Control"), F("no-cache"));
  if (origin.length() > 0) {
    WebServer.sendHeader(F("Access-Control-Allow-Origin"), origin);
  }
  WebServer.send(200, contenttype, "");
  // dont wait on 2.3.0. Memory returns just too slow.
  while ((ESP.getFreeHeap() < freeBeforeSend) &&
         !timeOutReached(beginWait + timeout)) {
    checkRAM(F("duringHeaderTX"));
    delay(1);
  }
#endif
  delay(0);
}

void sendHeadandTail(const String& tmplName, boolean Tail = false, boolean rebooting = false) {
  String pageTemplate = "";
  String fileName = tmplName;
  fileName += F(".htm");
  fs::File f = SPIFFS.open(fileName, "r+");

  if (f) {
    pageTemplate.reserve(f.size());
    while (f.available()) pageTemplate += (char)f.read();
    f.close();
  } else {
    // TODO TD-er: Should send data directly to TXBuffer instead of using large strings.
    getWebPageTemplateDefault(tmplName, pageTemplate);
  }
  checkRAM(F("sendWebPage"));
  // web activity timer
  lastWeb = millis();

  if (Tail) {
    TXBuffer += pageTemplate.substring(
        11 + // Size of "{{content}}"
        pageTemplate.indexOf(F("{{content}}")));  // advance beyond content key
  } else {
    int indexStart = 0;
    int indexEnd = 0;
    int readPos = 0; // Position of data sent to TXBuffer
    String varName;  //, varValue;
    String meta;
    if(rebooting){
      meta = F("<meta http-equiv='refresh' content='10 url=/'>");
    }
    while ((indexStart = pageTemplate.indexOf(F("{{"), indexStart)) >= 0) {
      TXBuffer += pageTemplate.substring(readPos, indexStart);
      readPos = indexStart;
      if ((indexEnd = pageTemplate.indexOf(F("}}"), indexStart)) > 0) {
        varName = pageTemplate.substring(indexStart + 2, indexEnd);
        indexStart = indexEnd + 2;
        readPos = indexEnd + 2;
        varName.toLowerCase();

        if (varName == F("content")) {  // is var == page content?
          break;  // send first part of result only
        } else if (varName == F("error")) {
          getErrorNotifications();
        }
        else if(varName == F("meta")) {
          TXBuffer += meta;
        }
        else {
          getWebPageTemplateVar(varName);
        }
      } else {  // no closing "}}"
        // eat "{{"
        readPos += 2;
        indexStart += 2;
      }
    }
  }
  if (shouldReboot) {
    //we only add this here as a seperate chunk to prevent using too much memory at once
    html_add_script(false);
    TXBuffer += DATA_REBOOT_JS;
    html_add_script_end();
  }
}

void sendHeadandTail_stdtemplate(boolean Tail = false, boolean rebooting = false) {
  sendHeadandTail(F("TmplStd"), Tail, rebooting);
}

boolean ipLessEqual(const IPAddress& ip, const IPAddress& high)
{
  for (byte i = 0; i < 4; ++i) {
    if (ip[i] > high[i]) return false;
  }
  return true;
}

boolean ipInRange(const IPAddress& ip, const IPAddress& low, const IPAddress& high)
{
  return (ipLessEqual(low, ip) && ipLessEqual(ip, high));
}

String describeAllowedIPrange() {
  String  reply;
  switch (SecuritySettings.IPblockLevel) {
    case ALL_ALLOWED:
     reply +=  F("All Allowed");
      break;
    default:
    {
      IPAddress low, high;
      getIPallowedRange(low, high);
      reply +=  formatIP(low);
      reply +=  F(" - ");
      reply +=  formatIP(high);
    }
  }
  return reply;
}

bool getIPallowedRange(IPAddress& low, IPAddress& high)
{
  switch (SecuritySettings.IPblockLevel) {
    case LOCAL_SUBNET_ALLOWED:
      return getSubnetRange(low, high);
    case ONLY_IP_RANGE_ALLOWED:
      low = SecuritySettings.AllowedIPrangeLow;
      high = SecuritySettings.AllowedIPrangeHigh;
      break;
    default:
      low = IPAddress(0,0,0,0);
      high = IPAddress(255,255,255,255);
      return false;
  }
  return true;
}

boolean clientIPallowed()
{
  // TD-er Must implement "safe time after boot"
  IPAddress low, high;
  if (!getIPallowedRange(low, high))
  {
    // No subnet range determined, cannot filter on IP
    return true;
  }
  WiFiClient client(WebServer.client());
  if (ipInRange(client.remoteIP(), low, high))
    return true;

  if (WifiIsAP(WiFi.getMode())) {
    // @TD-er Fixme: Should match subnet of SoftAP.
    return true;
  }
  String response = F("IP blocked: ");
  response += formatIP(client.remoteIP());
  WebServer.send(403, "text/html", response);
  if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
    response += F(" Allowed: ");
    response += formatIP(low);
    response += F(" - ");
    response += formatIP(high);
    addLog(LOG_LEVEL_ERROR, response);
  }
  return false;
}

void clearAccessBlock()
{
  SecuritySettings.IPblockLevel = ALL_ALLOWED;
}

//********************************************************************************
// Web Interface init
//********************************************************************************
//#include "core_version.h"
#define HTML_SYMBOL_WARNING "&#9888;"
#define HTML_SYMBOL_INPUT   "&#8656;"
#define HTML_SYMBOL_OUTPUT  "&#8658;"
#define HTML_SYMBOL_I_O     "&#8660;"


#if defined(ESP8266)
  #define TASKS_PER_PAGE 12
#endif
#if defined(ESP32)
  #define TASKS_PER_PAGE 32
#endif

int getFormItemInt(const String &key, int defaultValue) {
  int value = defaultValue;
  getCheckWebserverArg_int(key, value);
  return value;
}

bool getCheckWebserverArg_int(const String &key, int& value) {
  String valueStr = WebServer.arg(key);
  if (!isInt(valueStr)) return false;
  value = valueStr.toInt();
  return true;
}

#define strncpy_webserver_arg(D,N) safe_strncpy(D, WebServer.arg(N).c_str(), sizeof(D));
#define update_whenset_FormItemInt(K,V) { int tmpVal; if (getCheckWebserverArg_int(K, tmpVal)) V=tmpVal;}


bool isFormItemChecked(const String& id)
{
  return WebServer.arg(id) == F("on");
}

int getFormItemInt(const String& id)
{
  return getFormItemInt(id, 0);
}

float getFormItemFloat(const String& id)
{
  String val = WebServer.arg(id);
  if (!isFloat(val)) return 0.0;
  return val.toFloat();
}

bool isFormItem(const String& id)
{
  return (WebServer.arg(id).length() != 0);
}



//if there is an error-string, add it to the html code with correct formatting
void addHtmlError(const String& error){
  if (error.length()>0)
  {
    TXBuffer += F("<div class=\"");
    if (error.startsWith(F("Warn"))) {
      TXBuffer += F("warning");
    } else {
      TXBuffer += F("alert");
    }
    TXBuffer += F("\"><span class=\"closebtn\" onclick=\"this.parentElement.style.display='none';\">&times;</span>");
    TXBuffer += error;
    TXBuffer += F("</div>");
  }
  else
  {
    TXBuffer += jsToastMessageBegin;
    // we can push custom messages here in future releases...
    TXBuffer += F("Submitted");
    TXBuffer += jsToastMessageEnd;
  }
}

void addHtml(const String& html) {
  TXBuffer += html;
}

void addDisabled() {
  TXBuffer += F(" disabled");
}

#ifdef ARDUINO_ESP8266_RELEASE_2_3_0
void WebServerInit()
{
  // Prepare webserver pages
  WebServer.on("/", handle_root);
  WebServer.on("/config", handle_config);
  WebServer.on("/controllers", handle_controllers);
  WebServer.on("/hardware", handle_hardware);
  WebServer.on("/devices", handle_devices);
#ifndef NOTIFIER_SET_NONE
  WebServer.on("/notifications", handle_notifications);
#endif
  WebServer.on("/log", handle_log);
  WebServer.on("/logjson", handle_log_JSON);
  WebServer.on("/tools", handle_tools);
  WebServer.on("/i2cscanner", handle_i2cscanner);
  WebServer.on("/i2cscanner_json", handle_i2cscanner_json);
  WebServer.on("/wifiscanner", handle_wifiscanner);
  WebServer.on("/wifiscanner_json", handle_wifiscanner_json);
  WebServer.on("/login", handle_login);
  WebServer.on("/control", handle_control);
  WebServer.on("/download", handle_download);
  WebServer.on("/upload", HTTP_GET, handle_upload);
  WebServer.on("/upload", HTTP_POST, handle_upload_post, handleFileUpload);
  WebServer.onNotFound(handleNotFound);
  WebServer.on("/filelist", handle_filelist);
  WebServer.on("/filelist_json", handle_filelist_json);
#ifdef FEATURE_SD
  WebServer.on("/SDfilelist", handle_SDfilelist);
#endif
  WebServer.on("/advanced", handle_advanced);
  WebServer.on("/setup", handle_setup);
  WebServer.on("/json", handle_json);
  WebServer.on("/timingstats_json", handle_timingstats_json);
  WebServer.on("/timingstats", handle_timingstats);
  WebServer.on("/rules", handle_rules_new);
  WebServer.on("/rules/", Goto_Rules_Root);
  WebServer.on("/rules/add", []()
  {
    handle_rules_edit(WebServer.uri(),true);
  });
  WebServer.on("/rules/backup", handle_rules_backup);
  WebServer.on("/rules/delete", handle_rules_delete);
  WebServer.on("/sysinfo", handle_sysinfo);
  WebServer.on("/pinstates", handle_pinstates);
  WebServer.on("/pinstates_json", handle_pinstates_json);
  WebServer.on("/sysvars", handle_sysvars);
  WebServer.on("/factoryreset", handle_factoryreset);
  WebServer.on("/favicon.ico", handle_favicon);

  #if defined(ESP8266)
  {
    uint32_t maxSketchSize;
    bool use2step;
    if (OTA_possible(maxSketchSize, use2step)) {
      httpUpdater.setup(&WebServer);
    }
  }
  #endif

  #if defined(ESP8266)
  if (Settings.UseSSDP)
  {
    WebServer.on("/ssdp.xml", HTTP_GET, []() {
      WiFiClient client(WebServer.client());
      SSDP_schema(client);
    });
    SSDP_begin();
  }
  #endif
}

#else
void WebServerInit()
{
  // Prepare webserver pages
  WebServer.on("/", handle_root);
  WebServer.on(F("/config"), handle_config);
  WebServer.on(F("/controllers"), handle_controllers);
  WebServer.on(F("/hardware"), handle_hardware);
  WebServer.on(F("/devices"), handle_devices);
#ifndef NOTIFIER_SET_NONE
  WebServer.on(F("/notifications"), handle_notifications);
#endif
  WebServer.on(F("/log"), handle_log);
  WebServer.on(F("/logjson"), handle_log_JSON);
  WebServer.on(F("/tools"), handle_tools);
  WebServer.on(F("/i2cscanner"), handle_i2cscanner);
  WebServer.on(F("/wifiscanner"), handle_wifiscanner);
  WebServer.on(F("/login"), handle_login);
  WebServer.on(F("/control"), handle_control);
  WebServer.on(F("/download"), handle_download);
  WebServer.on(F("/upload"), HTTP_GET, handle_upload);
  WebServer.on(F("/upload"), HTTP_POST, handle_upload_post, handleFileUpload);
  WebServer.onNotFound(handleNotFound);
  WebServer.on(F("/filelist"), handle_filelist);
#ifdef FEATURE_SD
  WebServer.on(F("/SDfilelist"), handle_SDfilelist);
#endif
  WebServer.on(F("/advanced"), handle_advanced);
  WebServer.on(F("/setup"), handle_setup);
  WebServer.on(F("/json"), handle_json);
  WebServer.on(F("/timingstats_json"), handle_timingstats_json);
  WebServer.on(F("/timingstats"), handle_timingstats);
  WebServer.on(F("/rules"), handle_rules_new);
  WebServer.on(F("/rules/"), Goto_Rules_Root);
  WebServer.on(F("/rules/add"), []()
  {
    handle_rules_edit(WebServer.uri(),true);
  });
  WebServer.on(F("/rules/backup"), handle_rules_backup);
  WebServer.on(F("/rules/delete"), handle_rules_delete);
  WebServer.on(F("/sysinfo"), handle_sysinfo);
  WebServer.on(F("/pinstates"), handle_pinstates);
  WebServer.on(F("/sysvars"), handle_sysvars);
  WebServer.on(F("/factoryreset"), handle_factoryreset);
  WebServer.on(F("/favicon.ico"), handle_favicon);

  #if defined(ESP8266)
  {
    uint32_t maxSketchSize;
    bool use2step;
    if (OTA_possible(maxSketchSize, use2step)) {
      httpUpdater.setup(&WebServer);
    }
  }
  #endif

  #if defined(ESP8266)
  if (Settings.UseSSDP)
  {
    WebServer.on(F("/ssdp.xml"), HTTP_GET, []() {
      WiFiClient client(WebServer.client());
      SSDP_schema(client);
    });
    SSDP_begin();
  }
  #endif
}
#endif // ARDUINO_ESP8266_RELEASE_2_3_0

void setWebserverRunning(bool state) {
  if (webserver_state == state)
    return;
  if (state) {
    if (!webserver_init) {
      WebServerInit();
      webserver_init = true;
    }
    WebServer.begin();
    addLog(LOG_LEVEL_INFO, F("Webserver: start"));
  } else {
    WebServer.stop();
    addLog(LOG_LEVEL_INFO, F("Webserver: stop"));
  }
  webserver_state = state;
}


void getWebPageTemplateDefault(const String& tmplName, String& tmpl)
{
  tmpl.reserve(576);
  const bool addJS = true;
  const bool addMeta = true;
  if (tmplName == F("TmplAP"))
  {
    getWebPageTemplateDefaultHead(tmpl, !addMeta, !addJS);
    tmpl += F("<body>"
              "<header class='apheader'>"
              "<h1>Welcome to ESP Easy Mega AP</h1>"
              "</header>");
    getWebPageTemplateDefaultContentSection(tmpl);
    getWebPageTemplateDefaultFooter(tmpl);
  }
  else if (tmplName == F("TmplMsg"))
  {
    getWebPageTemplateDefaultHead(tmpl, !addMeta, !addJS);
    tmpl += F("<body>");
    getWebPageTemplateDefaultHeader(tmpl, F("{{name}}"), false);
    getWebPageTemplateDefaultContentSection(tmpl);
    getWebPageTemplateDefaultFooter(tmpl);
  }
  else if (tmplName == F("TmplDsh"))
  {
    getWebPageTemplateDefaultHead(tmpl, !addMeta, addJS);
    tmpl += F(
        "<body>"
        "{{content}}"
        "</body></html>"
            );
  }
  else   //all other template names e.g. TmplStd
  {
    getWebPageTemplateDefaultHead(tmpl, addMeta, addJS);
    tmpl += F("<body class='bodymenu'>"
              "<span class='message' id='rbtmsg'></span>");
    getWebPageTemplateDefaultHeader(tmpl, F("{{name}} {{logo}}"), true);
    getWebPageTemplateDefaultContentSection(tmpl);
    getWebPageTemplateDefaultFooter(tmpl);
  }
}

void getWebPageTemplateDefaultHead(String& tmpl, bool addMeta, bool addJS) {
  tmpl += F("<!DOCTYPE html><html lang='en'>"
              "<head>"
                "<meta charset='utf-8'/>"
                "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
                "<title>{{name}}</title>");
  if (addMeta) tmpl += F("{{meta}}");
  if (addJS) tmpl += F("{{js}}");

  tmpl += F("{{css}}"
            "</head>");
}

void getWebPageTemplateDefaultHeader(String& tmpl, const String& title, bool addMenu) {
  tmpl += F("<header class='headermenu'>"
            "<h1>ESP Easy Mega: ");
  tmpl += title;
  tmpl += F("</h1><BR>");
  if (addMenu) tmpl += F("{{menu}}");
  tmpl += F("</header>");
}

void getWebPageTemplateDefaultContentSection(String& tmpl) {
  tmpl += F("<section>"
              "<span class='message error'>"
                "{{error}}"
              "</span>"
              "{{content}}"
            "</section>"
          );
}

void getWebPageTemplateDefaultFooter(String& tmpl) {
  tmpl += F("<footer>"
              "<br>"
              "<h6>Powered by <a href='http://www.letscontrolit.com' style='font-size: 15px; text-decoration: none'>Let's Control It</a> community</h6>"
            "</footer>"
            "</body></html>"
          );
}

void getErrorNotifications() {
  // Check number of MQTT controllers active.
  int nrMQTTenabled = 0;
  for (byte x = 0; x < CONTROLLER_MAX; x++) {
    if (Settings.Protocol[x] != 0) {
      byte ProtocolIndex = getProtocolIndex(Settings.Protocol[x]);
      if (Settings.ControllerEnabled[x] && Protocol[ProtocolIndex].usesMQTT) {
        ++nrMQTTenabled;
      }
    }
  }
  if (nrMQTTenabled > 1) {
    // Add warning, only one MQTT protocol should be used.
    addHtmlError(F("Only one MQTT controller should be active."));
  }
  // Check checksum of stored settings.
}


#define MENU_INDEX_MAIN          0
#define MENU_INDEX_CONFIG        1
#define MENU_INDEX_CONTROLLERS   2
#define MENU_INDEX_HARDWARE      3
#define MENU_INDEX_DEVICES       4
#define MENU_INDEX_RULES         5
#define MENU_INDEX_NOTIFICATIONS 6
#define MENU_INDEX_TOOLS         7
static byte navMenuIndex = MENU_INDEX_MAIN;


void getWebPageTemplateVar(const String& varName )
{
 // serialPrint(varName); serialPrint(" : free: "); serialPrint(ESP.getFreeHeap());   serialPrint("var len before:  "); serialPrint (varValue.length()) ;serialPrint("after:  ");
 //varValue = "";

  if (varName == F("name"))
  {
    TXBuffer += Settings.Name;
  }

  else if (varName == F("unit"))
  {
    TXBuffer += String(Settings.Unit);
  }

  else if (varName == F("menu"))
  {
    static const __FlashStringHelper* gpMenu[8][3] = {
      // See https://github.com/letscontrolit/ESPEasy/issues/1650
      // Icon,        Full width label,   URL
      F("&#8962;"),   F("Main"),          F("/"),              //0
      F("&#9881;"),   F("Config"),        F("/config"),        //1
      F("&#128172;"), F("Controllers"),   F("/controllers"),   //2
      F("&#128204;"), F("Hardware"),      F("/hardware"),      //3
      F("&#128268;"), F("Devices"),       F("/devices"),       //4
      F("&#10740;"),  F("Rules"),         F("/rules"),         //5
      F("&#9993;"),   F("Notifications"), F("/notifications"), //6
      F("&#128295;"), F("Tools"),         F("/tools"),         //7
    };

    TXBuffer += F("<div class='menubar'>");

    for (byte i = 0; i < 8; i++)
    {
      if (i == MENU_INDEX_RULES && !Settings.UseRules)   //hide rules menu item
        continue;
#ifdef NOTIFIER_SET_NONE
      if (i == MENU_INDEX_NOTIFICATIONS)   //hide notifications menu item
        continue;
#endif

      TXBuffer += F("<a class='menu");
      if (i == navMenuIndex)
        TXBuffer += F(" active");
      TXBuffer += F("' href='");
      TXBuffer += gpMenu[i][2];
      TXBuffer += "'>";
      TXBuffer += gpMenu[i][0];
      TXBuffer += F("<span class='showmenulabel'>");
      TXBuffer += gpMenu[i][1];
      TXBuffer += F("</span>");
      TXBuffer += F("</a>");
    }

    TXBuffer += F("</div>");
  }

  else if (varName == F("logo"))
  {
    if (SPIFFS.exists(F("esp.png")))
    {
      TXBuffer = F("<img src=\"esp.png\" width=48 height=48 align=right>");
    }
  }

  else if (varName == F("css"))
  {
    if (SPIFFS.exists(F("esp.css")))   //now css is written in writeDefaultCSS() to SPIFFS and always present
    //if (0) //TODO
    {
      TXBuffer = F("<link rel=\"stylesheet\" type=\"text/css\" href=\"esp.css\">");
    }
   else
    {
      TXBuffer += F("<style>");
      // Send CSS per chunk to avoid sending either too short or too large strings.
      TXBuffer += DATA_ESPEASY_DEFAULT_MIN_CSS;
      TXBuffer += F("</style>");
    }
  }


  else if (varName == F("js"))
  {
    html_add_autosubmit_form();
  }

  else if (varName == F("error"))
  {
    //print last error - not implemented yet
  }

  else if (varName == F("debug"))
  {
    //print debug messages - not implemented yet
  }

  else
  {
    if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
      String log = F("Templ: Unknown Var : ");
      log += varName;
      addLog(LOG_LEVEL_ERROR, log);
    }
    //no return string - eat var name
  }

 }


void writeDefaultCSS(void)
{
  return; //TODO

  if (!SPIFFS.exists(F("esp.css")))
  {
    String defaultCSS;

    fs::File f = SPIFFS.open(F("esp.css"), "w");
    if (f)
    {
      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        String log = F("CSS  : Writing default CSS file to SPIFFS (");
        log += defaultCSS.length();
        log += F(" bytes)");
        addLog(LOG_LEVEL_INFO, log);
      }
      defaultCSS= PGMT(DATA_ESPEASY_DEFAULT_MIN_CSS);
      f.write((const unsigned char*)defaultCSS.c_str(), defaultCSS.length());   //note: content must be in RAM - a write of F("XXX") does not work
      f.close();
    }

  }
}


//********************************************************************************
// Add top menu
//********************************************************************************
void addHeader(boolean showMenu, String& str)
{
  //not longer used - now part of template
}


//********************************************************************************
// Add footer to web page
//********************************************************************************
void addFooter(const String& str)
{
  //not longer used - now part of template
}


//********************************************************************************
// Web Interface root page
//********************************************************************************
void handle_root() {
  checkRAM(F("handle_root"));
  // if Wifi setup, launch setup wizard
  if (wifiSetup)
  {
    WebServer.send(200, "text/html", F("<meta HTTP-EQUIV='REFRESH' content='0; url=/setup'>"));
    return;
  }
  if (!isLoggedIn()) return;
  navMenuIndex = 0;
  TXBuffer.startStream();
  String sCommand = WebServer.arg(F("cmd"));
  boolean rebootCmd = strcasecmp_P(sCommand.c_str(), PSTR("reboot")) == 0;
  sendHeadandTail_stdtemplate(_HEAD, rebootCmd);

  int freeMem = ESP.getFreeHeap();


  if ((strcasecmp_P(sCommand.c_str(), PSTR("wifidisconnect")) != 0) && (rebootCmd == false)&& (strcasecmp_P(sCommand.c_str(), PSTR("reset")) != 0))
  {
    if (timerAPoff)
      timerAPoff = millis() + 2000L;  //user has reached the main page - AP can be switched off in 2..3 sec



    printToWeb = true;
    printWebString = "";
    if (sCommand.length() > 0) {
      ExecuteCommand(VALUE_SOURCE_HTTP, sCommand.c_str());
    }

    IPAddress ip = WiFi.localIP();
    // IPAddress gw = WiFi.gatewayIP();

    TXBuffer += printWebString;
    TXBuffer += F("<form>");
    html_table_class_normal();
    addFormHeader(F("System Info"));

    addRowLabel(F("Unit"));
    TXBuffer += String(Settings.Unit);

    addRowLabel(F("GIT version"));
    TXBuffer += BUILD_GIT;

    addRowLabel(F("Local Time"));
    if (systemTimePresent())
    {
      TXBuffer += getDateTimeString('-', ':', ' ');
    }
    else
      TXBuffer += F("<font color='red'>No system time source</font>");

    addRowLabel(F("Uptime"));
    {
        int minutes = wdcounter / 2;
        int days = minutes / 1440;
        minutes = minutes % 1440;
        int hrs = minutes / 60;
        minutes = minutes % 60;
        char strUpTime[40];
        sprintf_P(strUpTime, PSTR("%d days %d hours %d minutes"), days, hrs, minutes);
        TXBuffer += strUpTime;
    }
    addRowLabel(F("Load"));
    if (wdcounter > 0)
    {
      TXBuffer += String(getCPUload());
      TXBuffer += F("% (LC=");
      TXBuffer += String(getLoopCountPerSec());
      TXBuffer += ')';
    }

    addRowLabel(F("Free Mem"));
    TXBuffer += String(freeMem);
    TXBuffer += " (";
    TXBuffer += String(lowestRAM);
    TXBuffer += F(" - ");
    TXBuffer += String(lowestRAMfunction);
    TXBuffer += ')';
    addRowLabel(F("Free Stack"));
    TXBuffer += String(getCurrentFreeStack());
    TXBuffer += " (";
    TXBuffer += String(lowestFreeStack);
    TXBuffer += F(" - ");
    TXBuffer += String(lowestFreeStackfunction);
    TXBuffer += ')';

    addRowLabel(F("IP"));
    TXBuffer += formatIP(ip);

    addRowLabel(F("Wifi RSSI"));
    if (WiFiConnected())
    {
      TXBuffer += String(WiFi.RSSI());
      TXBuffer += F(" dB (");
      TXBuffer += WiFi.SSID();
      TXBuffer += ')';
    }

    #ifdef FEATURE_MDNS
      html_TR_TD();
      TXBuffer += F("mDNS:<TD><a href='http://");
      TXBuffer += WifiGetHostname();
      TXBuffer += F(".local'>");
      TXBuffer += WifiGetHostname();
      TXBuffer += F(".local</a>");
      html_TD(3);
    #endif
    html_TR_TD();
    html_TD();
    addButton(F("sysinfo"), F("More info"));

    html_end_table();
    html_BR();
    html_BR();
    html_table_class_multirow_noborder();
    html_TR();
    html_table_header(F("Node List"));
    html_table_header("Name");
    html_table_header(F("Build"));
    html_table_header("Type");
    html_table_header("IP", 160); // Should fit "255.255.255.255"
    html_table_header("Age");
    for (NodesMap::iterator it = Nodes.begin(); it != Nodes.end(); ++it)
    {
      if (it->second.ip[0] != 0)
      {
        bool isThisUnit = it->first == Settings.Unit;
        if (isThisUnit)
          html_TR_TD_highlight();
        else
          html_TR_TD();

        TXBuffer += F("Unit ");
        TXBuffer += String(it->first);
        html_TD();
        if (isThisUnit)
          TXBuffer += Settings.Name;
        else
          TXBuffer += it->second.nodeName;
        html_TD();
        if (it->second.build)
          TXBuffer += String(it->second.build);
        html_TD();
        TXBuffer += getNodeTypeDisplayString(it->second.nodeType);
        html_TD();
        html_add_wide_button_prefix();
        TXBuffer += F("http://");
        TXBuffer += it->second.ip.toString();
        TXBuffer += "'>";
        TXBuffer += it->second.ip.toString();
        TXBuffer += "</a>";
        html_TD();
        TXBuffer += String( it->second.age);
      }
    }

    html_end_table();
    html_end_form();

    printWebString = "";
    printToWeb = false;
    sendHeadandTail_stdtemplate(_TAIL);
    TXBuffer.endStream();

  }
  else
  {
    //TODO: move this to handle_tools, from where it is actually called?

    // have to disconnect or reboot from within the main loop
    // because the webconnection is still active at this point
    // disconnect here could result into a crash/reboot...
    if (strcasecmp_P(sCommand.c_str(), PSTR("wifidisconnect")) == 0)
    {
      addLog(LOG_LEVEL_INFO, F("WIFI : Disconnecting..."));
      cmd_within_mainloop = CMD_WIFI_DISCONNECT;
    }

    if (strcasecmp_P(sCommand.c_str(), PSTR("reboot")) == 0)
    {
      addLog(LOG_LEVEL_INFO, F("     : Rebooting..."));
      cmd_within_mainloop = CMD_REBOOT;
    }
   if (strcasecmp_P(sCommand.c_str(), PSTR("reset")) == 0)
    {
      addLog(LOG_LEVEL_INFO, F("     : factory reset..."));
      cmd_within_mainloop = CMD_REBOOT;
      TXBuffer += F("OK. Please wait > 1 min and connect to Acces point.<BR><BR>PW=configesp<BR>URL=<a href='http://192.168.4.1'>192.168.4.1</a>");
      TXBuffer.endStream();
      ExecuteCommand(VALUE_SOURCE_HTTP, sCommand.c_str());
    }

    TXBuffer += "OK";
    TXBuffer.endStream();

  }
}


//********************************************************************************
// Web Interface config page
//********************************************************************************
void handle_config() {

   checkRAM(F("handle_config"));
   if (!isLoggedIn()) return;

   navMenuIndex = MENU_INDEX_CONFIG;
   TXBuffer.startStream();
   sendHeadandTail_stdtemplate(_HEAD);

  if (timerAPoff)
    timerAPoff = millis() + 2000L;  //user has reached the main page - AP can be switched off in 2..3 sec


  String name = WebServer.arg(F("name"));
  //String password = WebServer.arg(F("password"));
  String iprangelow = WebServer.arg(F("iprangelow"));
  String iprangehigh = WebServer.arg(F("iprangehigh"));

  Settings.Delay = getFormItemInt(F("delay"), Settings.Delay);
  Settings.deepSleep = getFormItemInt(F("deepsleep"), Settings.deepSleep);
  String espip = WebServer.arg(F("espip"));
  String espgateway = WebServer.arg(F("espgateway"));
  String espsubnet = WebServer.arg(F("espsubnet"));
  String espdns = WebServer.arg(F("espdns"));
  Settings.Unit = getFormItemInt(F("unit"), Settings.Unit);
  //String apkey = WebServer.arg(F("apkey"));
  String ssid = WebServer.arg(F("ssid"));


  if (ssid[0] != 0)
  {
    if (strcmp(Settings.Name, name.c_str()) != 0) {
      addLog(LOG_LEVEL_INFO, F("Unit Name changed."));
      MQTTclient_should_reconnect = true;
    }
    // Unit name
    safe_strncpy(Settings.Name, name.c_str(), sizeof(Settings.Name));
    Settings.appendUnitToHostname(isFormItemChecked(F("appendunittohostname")));

    // Password
    copyFormPassword(F("password"), SecuritySettings.Password, sizeof(SecuritySettings.Password));

    // SSID 1
    safe_strncpy(SecuritySettings.WifiSSID, ssid.c_str(), sizeof(SecuritySettings.WifiSSID));
    copyFormPassword(F("key"), SecuritySettings.WifiKey, sizeof(SecuritySettings.WifiKey));

    // SSID 2
    strncpy_webserver_arg(SecuritySettings.WifiSSID2, F("ssid2"));
    copyFormPassword(F("key2"), SecuritySettings.WifiKey2, sizeof(SecuritySettings.WifiKey2));

    // Access point password.
    copyFormPassword(F("apkey"), SecuritySettings.WifiAPKey, sizeof(SecuritySettings.WifiAPKey));


    // TD-er Read access control from form.
    SecuritySettings.IPblockLevel = getFormItemInt(F("ipblocklevel"));
    switch (SecuritySettings.IPblockLevel) {
      case LOCAL_SUBNET_ALLOWED:
      {
        IPAddress low, high;
        getSubnetRange(low, high);
        for (byte i=0; i < 4; ++i) {
          SecuritySettings.AllowedIPrangeLow[i] = low[i];
          SecuritySettings.AllowedIPrangeHigh[i] = high[i];
        }
        break;
      }
      case ONLY_IP_RANGE_ALLOWED:
      case ALL_ALLOWED:
        // iprangelow.toCharArray(tmpString, 26);
        str2ip(iprangelow, SecuritySettings.AllowedIPrangeLow);
        // iprangehigh.toCharArray(tmpString, 26);
        str2ip(iprangehigh, SecuritySettings.AllowedIPrangeHigh);
        break;
    }

    Settings.deepSleepOnFail = isFormItemChecked(F("deepsleeponfail"));
    str2ip(espip, Settings.IP);
    str2ip(espgateway, Settings.Gateway);
    str2ip(espsubnet, Settings.Subnet);
    str2ip(espdns, Settings.DNS);
    addHtmlError(SaveSettings());
  }

  html_add_form();
  html_table_class_normal();

  addFormHeader(F("Main Settings"));

  Settings.Name[25] = 0;
  SecuritySettings.Password[25] = 0;
  addFormTextBox( F("Unit Name"), F("name"), Settings.Name, 25);
  addFormNumericBox( F("Unit Number"), F("unit"), Settings.Unit, 0, UNIT_NUMBER_MAX);
  addFormCheckBox(F("Append Unit Number to hostname"), F("appendunittohostname"), Settings.appendUnitToHostname());
  addFormPasswordBox(F("Admin Password"), F("password"), SecuritySettings.Password, 25);

  addFormSubHeader(F("Wifi Settings"));

  addFormTextBox( F("SSID"), F("ssid"), SecuritySettings.WifiSSID, 31);
  addFormPasswordBox(F("WPA Key"), F("key"), SecuritySettings.WifiKey, 63);
  addFormTextBox( F("Fallback SSID"), F("ssid2"), SecuritySettings.WifiSSID2, 31);
  addFormPasswordBox( F("Fallback WPA Key"), F("key2"), SecuritySettings.WifiKey2, 63);
  addFormSeparator(2);
  addFormPasswordBox(F("WPA AP Mode Key"), F("apkey"), SecuritySettings.WifiAPKey, 63);

  // TD-er add IP access box F("ipblocklevel")
  addFormSubHeader(F("Client IP filtering"));
  {
    IPAddress low, high;
    getIPallowedRange(low, high);
    byte iplow[4];
    byte iphigh[4];
    for (byte i = 0; i < 4; ++i) {
      iplow[i] = low[i];
      iphigh[i] = high[i];
    }
    addFormIPaccessControlSelect(F("Client IP block level"), F("ipblocklevel"), SecuritySettings.IPblockLevel);
    addFormIPBox(F("Access IP lower range"), F("iprangelow"), iplow);
    addFormIPBox(F("Access IP upper range"), F("iprangehigh"), iphigh);
  }

  addFormSubHeader(F("IP Settings"));

  addFormIPBox(F("ESP IP"), F("espip"), Settings.IP);
  addFormIPBox(F("ESP GW"), F("espgateway"), Settings.Gateway);
  addFormIPBox(F("ESP Subnet"), F("espsubnet"), Settings.Subnet);
  addFormIPBox(F("ESP DNS"), F("espdns"), Settings.DNS);
  addFormNote(F("Leave empty for DHCP"));


  addFormSubHeader(F("Sleep Mode"));

  addFormNumericBox( F("Sleep awake time"), F("deepsleep"), Settings.deepSleep, 0, 255);
  addUnit(F("sec"));
  addHelpButton(F("SleepMode"));
  addFormNote(F("0 = Sleep Disabled, else time awake from sleep"));

  int dsmax = 4294; // About 71 minutes
#if defined(CORE_2_5_0)
  dsmax = INT_MAX;
  if ((ESP.deepSleepMax()/1000000ULL) <= (uint64_t)INT_MAX)
    dsmax = (int)(ESP.deepSleepMax()/1000000ULL);
#endif
  addFormNumericBox( F("Sleep time"), F("delay"), Settings.Delay, 0, dsmax);   //limited by hardware
  {
    String maxSleeptimeUnit = F("sec (max: ");
    maxSleeptimeUnit += String(dsmax);
    maxSleeptimeUnit += ')';
    addUnit(maxSleeptimeUnit);
  }

  addFormCheckBox(F("Sleep on connection failure"), F("deepsleeponfail"), Settings.deepSleepOnFail);

  addFormSeparator(2);

  html_TR_TD();
  html_TD();
  addSubmitButton();
  html_end_table();
  html_end_form();

  sendHeadandTail_stdtemplate(_TAIL);
  TXBuffer.endStream();
}


//********************************************************************************
// Web Interface controller page
//********************************************************************************
void handle_controllers() {
  checkRAM(F("handle_controllers"));
  if (!isLoggedIn()) return;
  navMenuIndex = MENU_INDEX_CONTROLLERS;
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate(_HEAD);

  struct EventStruct TempEvent;

  byte controllerindex = getFormItemInt(F("index"), 0);
  boolean controllerNotSet = controllerindex == 0;
  --controllerindex;

  String usedns = WebServer.arg(F("usedns"));
  String controllerip = WebServer.arg(F("controllerip"));
  const int controllerport = getFormItemInt(F("controllerport"), 0);
  const int protocol = getFormItemInt(F("protocol"), -1);
  const int minimumsendinterval = getFormItemInt(F("minimumsendinterval"), 100);
  const int maxqueuedepth = getFormItemInt(F("maxqueuedepth"), 10);
  const int maxretry = getFormItemInt(F("maxretry"), 10);
  const int clienttimeout = getFormItemInt(F("clienttimeout"), CONTROLLER_CLIENTTIMEOUT_DFLT);


  //submitted data
  if (protocol != -1 && !controllerNotSet)
  {
    MakeControllerSettings(ControllerSettings);
    //submitted changed protocol
    if (Settings.Protocol[controllerindex] != protocol)
    {

      Settings.Protocol[controllerindex] = protocol;

      //there is a protocol selected?
      if (Settings.Protocol[controllerindex]!=0)
      {
        //reset (some) default-settings
        byte ProtocolIndex = getProtocolIndex(Settings.Protocol[controllerindex]);
        ControllerSettings.Port = Protocol[ProtocolIndex].defaultPort;
        ControllerSettings.MinimalTimeBetweenMessages = CONTROLLER_DELAY_QUEUE_DELAY_DFLT;
        ControllerSettings.ClientTimeout = CONTROLLER_CLIENTTIMEOUT_DFLT;
//        ControllerSettings.MaxQueueDepth = 0;
        if (Protocol[ProtocolIndex].usesTemplate)
          CPluginCall(ProtocolIndex, CPLUGIN_PROTOCOL_TEMPLATE, &TempEvent, dummyString);
        safe_strncpy(ControllerSettings.Subscribe, TempEvent.String1.c_str(), sizeof(ControllerSettings.Subscribe));
        safe_strncpy(ControllerSettings.Publish, TempEvent.String2.c_str(), sizeof(ControllerSettings.Publish));
        safe_strncpy(ControllerSettings.MQTTLwtTopic, TempEvent.String3.c_str(), sizeof(ControllerSettings.MQTTLwtTopic));
        safe_strncpy(ControllerSettings.LWTMessageConnect, TempEvent.String4.c_str(), sizeof(ControllerSettings.LWTMessageConnect));
        safe_strncpy(ControllerSettings.LWTMessageDisconnect, TempEvent.String5.c_str(), sizeof(ControllerSettings.LWTMessageDisconnect));
        TempEvent.String1 = "";
        TempEvent.String2 = "";
        TempEvent.String3 = "";
        TempEvent.String4 = "";
        TempEvent.String5 = "";
        //NOTE: do not enable controller by default, give user a change to enter sensible values first

        //not resetted to default (for convenience)
        //SecuritySettings.ControllerUser[controllerindex]
        //SecuritySettings.ControllerPassword[controllerindex]

        ClearCustomControllerSettings(controllerindex);
      }

    }

    //subitted same protocol
    else
    {
      //there is a protocol selected
      if (Settings.Protocol != 0)
      {
        //copy all settings to conroller settings struct
        byte ProtocolIndex = getProtocolIndex(Settings.Protocol[controllerindex]);
        TempEvent.ControllerIndex = controllerindex;
        TempEvent.ProtocolIndex = ProtocolIndex;
        CPluginCall(ProtocolIndex, CPLUGIN_WEBFORM_SAVE, &TempEvent, dummyString);
        ControllerSettings.UseDNS = usedns.toInt();
        if (ControllerSettings.UseDNS)
        {
          strncpy_webserver_arg(ControllerSettings.HostName, F("controllerhostname"));
          IPAddress IP;
          resolveHostByName(ControllerSettings.HostName, IP);
          for (byte x = 0; x < 4; x++)
            ControllerSettings.IP[x] = IP[x];
        }
        //no protocol selected
        else
        {
          str2ip(controllerip, ControllerSettings.IP);
        }
        //copy settings to struct
        Settings.ControllerEnabled[controllerindex] = isFormItemChecked(F("controllerenabled"));
        ControllerSettings.Port = controllerport;
        strncpy_webserver_arg(SecuritySettings.ControllerUser[controllerindex], F("controlleruser"));
        //safe_strncpy(SecuritySettings.ControllerPassword[controllerindex], controllerpassword.c_str(), sizeof(SecuritySettings.ControllerPassword[0]));
        copyFormPassword(F("controllerpassword"), SecuritySettings.ControllerPassword[controllerindex], sizeof(SecuritySettings.ControllerPassword[0]));
        strncpy_webserver_arg(ControllerSettings.Subscribe, F("controllersubscribe"));
        strncpy_webserver_arg(ControllerSettings.Publish, F("controllerpublish"));
        strncpy_webserver_arg(ControllerSettings.MQTTLwtTopic, F("mqttlwttopic"));
        strncpy_webserver_arg(ControllerSettings.LWTMessageConnect, F("lwtmessageconnect"));
        strncpy_webserver_arg(ControllerSettings.LWTMessageDisconnect, F("lwtmessagedisconnect"));
        ControllerSettings.MinimalTimeBetweenMessages = minimumsendinterval;
        ControllerSettings.MaxQueueDepth = maxqueuedepth;
        ControllerSettings.MaxRetry = maxretry;
        ControllerSettings.DeleteOldest = getFormItemInt(F("deleteoldest"), ControllerSettings.DeleteOldest);
        ControllerSettings.MustCheckReply = getFormItemInt(F("mustcheckreply"), ControllerSettings.MustCheckReply);
        ControllerSettings.ClientTimeout = clienttimeout;


        CPluginCall(ProtocolIndex, CPLUGIN_INIT, &TempEvent, dummyString);
      }
    }
    addHtmlError(SaveControllerSettings(controllerindex, ControllerSettings));
    addHtmlError(SaveSettings());
  }

  html_add_form();

  if (controllerNotSet)
  {
    html_table_class_multirow();
    html_TR();
    html_table_header("", 70);
    html_table_header("Nr", 50);
    html_table_header(F("Enabled"), 100);
    html_table_header(F("Protocol"));
    html_table_header("Host");
    html_table_header("Port");

    MakeControllerSettings(ControllerSettings);
    for (byte x = 0; x < CONTROLLER_MAX; x++)
    {
      LoadControllerSettings(x, ControllerSettings);
      html_TR_TD();
      html_add_button_prefix();
      TXBuffer += F("controllers?index=");
      TXBuffer += x + 1;
      TXBuffer += F("'>Edit</a>");
      html_TD();
      TXBuffer += getControllerSymbol(x);
      html_TD();
      if (Settings.Protocol[x] != 0)
      {
        addEnabled(Settings.ControllerEnabled[x]);

        html_TD();
        byte ProtocolIndex = getProtocolIndex(Settings.Protocol[x]);
        String ProtocolName = "";
        CPluginCall(ProtocolIndex, CPLUGIN_GET_DEVICENAME, 0, ProtocolName);
        TXBuffer += ProtocolName;

        html_TD();
        TXBuffer += ControllerSettings.getHost();
        html_TD();
        TXBuffer += ControllerSettings.Port;
      }
      else {
        html_TD(3);
      }
    }
    html_end_table();
    html_end_form();
  }
  else
  {
    html_table_class_normal();
    addFormHeader(F("Controller Settings"));
    addRowLabel(F("Protocol"));
    byte choice = Settings.Protocol[controllerindex];
    addSelector_Head(F("protocol"), true);
    addSelector_Item(F("- Standalone -"), 0, false, false, "");
    for (byte x = 0; x <= protocolCount; x++)
    {
      String ProtocolName = "";
      CPluginCall(x, CPLUGIN_GET_DEVICENAME, 0, ProtocolName);
      boolean disabled = false;// !((controllerindex == 0) || !Protocol[x].usesMQTT);
      addSelector_Item(ProtocolName,
                       Protocol[x].Number,
                       choice == Protocol[x].Number,
                       disabled,
                       "");
    }
    addSelector_Foot();

    addHelpButton(F("EasyProtocols"));
    if (Settings.Protocol[controllerindex])
    {
      MakeControllerSettings(ControllerSettings);
      LoadControllerSettings(controllerindex, ControllerSettings);
      byte choice = ControllerSettings.UseDNS;
      String options[2];
      options[0] = F("Use IP address");
      options[1] = F("Use Hostname");

      byte choice_delete_oldest = ControllerSettings.DeleteOldest;
      String options_delete_oldest[2];
      options_delete_oldest[0] = F("Ignore New");
      options_delete_oldest[1] = F("Delete Oldest");

      byte choice_mustcheckreply = ControllerSettings.MustCheckReply;
      String options_mustcheckreply[2];
      options_mustcheckreply[0] = F("Ignore Acknowledgement");
      options_mustcheckreply[1] = F("Check Acknowledgement");


      byte ProtocolIndex = getProtocolIndex(Settings.Protocol[controllerindex]);
      if (!Protocol[ProtocolIndex].Custom)
      {

        addFormSelector(F("Locate Controller"), F("usedns"), 2, options, NULL, NULL, choice, true);
        if (ControllerSettings.UseDNS)
        {
          addFormTextBox( F("Controller Hostname"), F("controllerhostname"), ControllerSettings.HostName, sizeof(ControllerSettings.HostName)-1);
        }
        else
        {
          addFormIPBox(F("Controller IP"), F("controllerip"), ControllerSettings.IP);
        }

        addFormNumericBox( F("Controller Port"), F("controllerport"), ControllerSettings.Port, 1, 65535);
        addFormNumericBox( F("Minimum Send Interval"), F("minimumsendinterval"), ControllerSettings.MinimalTimeBetweenMessages, 1, CONTROLLER_DELAY_QUEUE_DELAY_MAX);
        addUnit(F("ms"));
        addFormNumericBox( F("Max Queue Depth"), F("maxqueuedepth"), ControllerSettings.MaxQueueDepth, 1, CONTROLLER_DELAY_QUEUE_DEPTH_MAX);
        addFormNumericBox( F("Max Retries"), F("maxretry"), ControllerSettings.MaxRetry, 1, CONTROLLER_DELAY_QUEUE_RETRY_MAX);
        addFormSelector(F("Full Queue Action"), F("deleteoldest"), 2, options_delete_oldest, NULL, NULL, choice_delete_oldest, true);

        addFormSelector(F("Check Reply"), F("mustcheckreply"), 2, options_mustcheckreply, NULL, NULL, choice_mustcheckreply, true);
        addFormNumericBox( F("Client Timeout"), F("clienttimeout"), ControllerSettings.ClientTimeout, 10, CONTROLLER_CLIENTTIMEOUT_MAX);
        addUnit(F("ms"));

        if (Protocol[ProtocolIndex].usesAccount)
        {
          String protoDisplayName;
          if (!getControllerProtocolDisplayName(ProtocolIndex, CONTROLLER_USER, protoDisplayName)) {
            protoDisplayName = F("Controller User");
          }
          addFormTextBox(protoDisplayName, F("controlleruser"), SecuritySettings.ControllerUser[controllerindex], sizeof(SecuritySettings.ControllerUser[0])-1);
        }
        if (Protocol[ProtocolIndex].usesPassword)
        {
          String protoDisplayName;
          if (getControllerProtocolDisplayName(ProtocolIndex, CONTROLLER_PASS, protoDisplayName)) {
            // It is not a regular password, thus use normal text field.
            addFormTextBox(protoDisplayName, F("controllerpassword"), SecuritySettings.ControllerPassword[controllerindex], sizeof(SecuritySettings.ControllerPassword[0])-1);
          } else {
            addFormPasswordBox(F("Controller Password"), F("controllerpassword"), SecuritySettings.ControllerPassword[controllerindex], sizeof(SecuritySettings.ControllerPassword[0])-1);
          }
        }

        if (Protocol[ProtocolIndex].usesTemplate || Protocol[ProtocolIndex].usesMQTT)
        {
          String protoDisplayName;
          if (!getControllerProtocolDisplayName(ProtocolIndex, CONTROLLER_SUBSCRIBE, protoDisplayName)) {
            protoDisplayName = F("Controller Subscribe");
          }
          addFormTextBox(protoDisplayName, F("controllersubscribe"), ControllerSettings.Subscribe, sizeof(ControllerSettings.Subscribe)-1);
        }

        if (Protocol[ProtocolIndex].usesTemplate || Protocol[ProtocolIndex].usesMQTT)
        {
          String protoDisplayName;
          if (!getControllerProtocolDisplayName(ProtocolIndex, CONTROLLER_PUBLISH, protoDisplayName)) {
            protoDisplayName = F("Controller Publish");
          }
          addFormTextBox(protoDisplayName, F("controllerpublish"), ControllerSettings.Publish, sizeof(ControllerSettings.Publish)-1);
        }

        if (Protocol[ProtocolIndex].usesMQTT)
        {
          String protoDisplayName;
          if (!getControllerProtocolDisplayName(ProtocolIndex, CONTROLLER_LWT_TOPIC, protoDisplayName)) {
            protoDisplayName = F("Controller lwl topic");
          }
          addFormTextBox(protoDisplayName, F("mqttlwttopic"), ControllerSettings.MQTTLwtTopic, sizeof(ControllerSettings.MQTTLwtTopic)-1);
        }

        if (Protocol[ProtocolIndex].usesMQTT)
        {
          String protoDisplayName;
          if (!getControllerProtocolDisplayName(ProtocolIndex, CONTROLLER_LWT_CONNECT_MESSAGE, protoDisplayName)) {
            protoDisplayName = F("LWT Connect Message");
          }
          addFormTextBox(protoDisplayName, F("lwtmessageconnect"), ControllerSettings.LWTMessageConnect, sizeof(ControllerSettings.LWTMessageConnect)-1);
        }

        if (Protocol[ProtocolIndex].usesMQTT)
        {
          String protoDisplayName;
          if (!getControllerProtocolDisplayName(ProtocolIndex, CONTROLLER_LWT_DISCONNECT_MESSAGE, protoDisplayName)) {
            protoDisplayName = F("LWT Disconnect Message");
          }
          addFormTextBox(protoDisplayName, F("lwtmessagedisconnect"), ControllerSettings.LWTMessageDisconnect, sizeof(ControllerSettings.LWTMessageDisconnect)-1);
        }
      }

      addFormCheckBox(F("Enabled"), F("controllerenabled"), Settings.ControllerEnabled[controllerindex]);

      TempEvent.ControllerIndex = controllerindex;
      TempEvent.ProtocolIndex = ProtocolIndex;
      CPluginCall(ProtocolIndex, CPLUGIN_WEBFORM_LOAD, &TempEvent,TXBuffer.buf);

    }

    addFormSeparator(2);
    html_TR_TD();
    html_TD();
    addButton(F("controllers"), F("Close"));
    addSubmitButton();
    html_end_table();
    html_end_form();
  }

  sendHeadandTail_stdtemplate(_TAIL);
  TXBuffer.endStream();
}

//********************************************************************************
// Web Interface notifcations page
//********************************************************************************
#ifndef NOTIFIER_SET_NONE
void handle_notifications() {
  checkRAM(F("handle_notifications"));
  if (!isLoggedIn()) return;
  navMenuIndex = MENU_INDEX_NOTIFICATIONS;
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate(_HEAD);

  struct EventStruct TempEvent;
  // char tmpString[64];


  byte notificationindex = getFormItemInt(F("index"), 0);
  boolean notificationindexNotSet = notificationindex == 0;
  --notificationindex;

  const int notification = getFormItemInt(F("notification"), -1);

  if (notification != -1 && !notificationindexNotSet)
  {
    MakeNotificationSettings(NotificationSettings);
    if (Settings.Notification[notificationindex] != notification)
    {
      Settings.Notification[notificationindex] = notification;
    }
    else
    {
      if (Settings.Notification != 0)
      {
        byte NotificationProtocolIndex = getNotificationProtocolIndex(Settings.Notification[notificationindex]);
        if (NotificationProtocolIndex!=NPLUGIN_NOT_FOUND)
          NPlugin_ptr[NotificationProtocolIndex](NPLUGIN_WEBFORM_SAVE, 0, dummyString);
        NotificationSettings.Port = getFormItemInt(F("port"), 0);
        NotificationSettings.Pin1 = getFormItemInt(F("pin1"), 0);
        NotificationSettings.Pin2 = getFormItemInt(F("pin2"), 0);
        Settings.NotificationEnabled[notificationindex] = isFormItemChecked(F("notificationenabled"));
        strncpy_webserver_arg(NotificationSettings.Domain,   F("domain"));
        strncpy_webserver_arg(NotificationSettings.Server,   F("server"));
        strncpy_webserver_arg(NotificationSettings.Sender,   F("sender"));
        strncpy_webserver_arg(NotificationSettings.Receiver, F("receiver"));
        strncpy_webserver_arg(NotificationSettings.Subject,  F("subject"));
        strncpy_webserver_arg(NotificationSettings.User,     F("user"));
        strncpy_webserver_arg(NotificationSettings.Pass,     F("pass"));
        strncpy_webserver_arg(NotificationSettings.Body,     F("body"));

      }
    }
    // Save the settings.
    addHtmlError(SaveNotificationSettings(notificationindex, (byte*)&NotificationSettings, sizeof(NotificationSettingsStruct)));
    addHtmlError(SaveSettings());
    if (WebServer.hasArg(F("test"))) {
      // Perform tests with the settings in the form.
      byte NotificationProtocolIndex = getNotificationProtocolIndex(Settings.Notification[notificationindex]);
      if (NotificationProtocolIndex != NPLUGIN_NOT_FOUND)
      {
        // TempEvent.NotificationProtocolIndex = NotificationProtocolIndex;
        TempEvent.NotificationIndex = notificationindex;
        schedule_notification_event_timer(NotificationProtocolIndex, NPLUGIN_NOTIFY, &TempEvent);
      }
    }
  }

  html_add_form();

  if (notificationindexNotSet)
  {
    html_table_class_multirow();
    html_TR();
    html_table_header("", 70);
    html_table_header("Nr", 50);
    html_table_header(F("Enabled"), 100);
    html_table_header(F("Service"));
    html_table_header(F("Server"));
    html_table_header("Port");

    MakeNotificationSettings(NotificationSettings);
    for (byte x = 0; x < NOTIFICATION_MAX; x++)
    {
      LoadNotificationSettings(x, (byte*)&NotificationSettings, sizeof(NotificationSettingsStruct));
      html_TR_TD();
      html_add_button_prefix();
      TXBuffer += F("notifications?index=");
      TXBuffer += x + 1;
      TXBuffer += F("'>Edit</a>");
      html_TD();
      TXBuffer += x + 1;
      html_TD();
      if (Settings.Notification[x] != 0)
      {
        addEnabled(Settings.NotificationEnabled[x]);

        html_TD();
        byte NotificationProtocolIndex = getNotificationProtocolIndex(Settings.Notification[x]);
        String NotificationName = F("(plugin not found?)");
        if (NotificationProtocolIndex!=NPLUGIN_NOT_FOUND)
        {
          NPlugin_ptr[NotificationProtocolIndex](NPLUGIN_GET_DEVICENAME, 0, NotificationName);
        }
        TXBuffer += NotificationName;
        html_TD();
        TXBuffer += NotificationSettings.Server;
        html_TD();
        TXBuffer += NotificationSettings.Port;
      }
      else {
        html_TD(3);
      }
    }
    html_end_table();
    html_end_form();
  }
  else
  {
    html_table_class_normal();
    addFormHeader(F("Notification Settings"));
    addRowLabel(F("Notification"));
    byte choice = Settings.Notification[notificationindex];
    addSelector_Head(F("notification"), true);
    addSelector_Item(F("- None -"), 0, false, false, "");
    for (byte x = 0; x <= notificationCount; x++)
    {
      String NotificationName = "";
      NPlugin_ptr[x](NPLUGIN_GET_DEVICENAME, 0, NotificationName);
      addSelector_Item(NotificationName,
                       Notification[x].Number,
                       choice == Notification[x].Number,
                       false,
                       "");
    }
    addSelector_Foot();

    addHelpButton(F("EasyNotifications"));

    if (Settings.Notification[notificationindex])
    {
      MakeNotificationSettings(NotificationSettings);
      LoadNotificationSettings(notificationindex, (byte*)&NotificationSettings, sizeof(NotificationSettingsStruct));

      byte NotificationProtocolIndex = getNotificationProtocolIndex(Settings.Notification[notificationindex]);
      if (NotificationProtocolIndex!=NPLUGIN_NOT_FOUND)
      {

        if (Notification[NotificationProtocolIndex].usesMessaging)
        {
          addFormTextBox(F("Domain"), F("domain"), NotificationSettings.Domain, sizeof(NotificationSettings.Domain)-1);
          addFormTextBox(F("Server"), F("server"), NotificationSettings.Server, sizeof(NotificationSettings.Server)-1);
          addFormNumericBox(F("Port"), F("port"), NotificationSettings.Port, 1, 65535);

          addFormTextBox(F("Sender"), F("sender"), NotificationSettings.Sender, sizeof(NotificationSettings.Sender)-1);
          addFormTextBox(F("Receiver"), F("receiver"), NotificationSettings.Receiver, sizeof(NotificationSettings.Receiver)-1);
          addFormTextBox(F("Subject"), F("subject"), NotificationSettings.Subject, sizeof(NotificationSettings.Subject)-1);

          addFormTextBox(F("User"), F("user"), NotificationSettings.User, sizeof(NotificationSettings.User)-1);
          addFormTextBox(F("Pass"), F("pass"), NotificationSettings.Pass, sizeof(NotificationSettings.Pass)-1);

          addRowLabel(F("Body"));
          TXBuffer += F("<textarea name='body' rows='20' size=512 wrap='off'>");
          TXBuffer += NotificationSettings.Body;
          TXBuffer += F("</textarea>");
        }

        if (Notification[NotificationProtocolIndex].usesGPIO > 0)
        {
          addRowLabel(F("1st GPIO"));
          addPinSelect(false, "pin1", NotificationSettings.Pin1);
        }

        addRowLabel(F("Enabled"));
        addCheckBox(F("notificationenabled"), Settings.NotificationEnabled[notificationindex]);

        TempEvent.NotificationIndex = notificationindex;
        NPlugin_ptr[NotificationProtocolIndex](NPLUGIN_WEBFORM_LOAD, &TempEvent,TXBuffer.buf);
      }
    }

    addFormSeparator(2);

    html_TR_TD();
    html_TD();
    addButton(F("notifications"), F("Close"));
    addSubmitButton();
    addSubmitButton(F("Test"), F("test"));
    html_end_table();
    html_end_form();
  }
  sendHeadandTail_stdtemplate(_TAIL);
  TXBuffer.endStream();
}
#endif // NOTIFIER_SET_NONE

//********************************************************************************
// Web Interface hardware page
//********************************************************************************
void handle_hardware() {
  checkRAM(F("handle_hardware"));
  if (!isLoggedIn()) return;
  navMenuIndex = MENU_INDEX_HARDWARE;
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate(_HEAD);
  if (isFormItem(F("psda")))
  {
    Settings.Pin_status_led  = getFormItemInt(F("pled"));
    Settings.Pin_status_led_Inversed  = isFormItemChecked(F("pledi"));
    Settings.Pin_Reset  = getFormItemInt(F("pres"));
    Settings.Pin_i2c_sda     = getFormItemInt(F("psda"));
    Settings.Pin_i2c_scl     = getFormItemInt(F("pscl"));
    Settings.InitSPI = isFormItemChecked(F("initspi"));      // SPI Init
    Settings.Pin_sd_cs  = getFormItemInt(F("sd"));
    int gpio = 0;
    // FIXME TD-er: Max of 17 is a limit in the Settings.PinBootStates array
    while (gpio < MAX_GPIO  && gpio < 17) {
      if (Settings.UseSerial && (gpio == 1 || gpio == 3)) {
        // do not add the pin state select for these pins.
      } else {
        int pinnr = -1;
        bool input, output, warning;
        if (getGpioInfo(gpio, pinnr, input, output, warning)) {
          String int_pinlabel = "p";
          int_pinlabel += gpio;
          Settings.PinBootStates[gpio] = getFormItemInt(int_pinlabel);
        }
      }
      ++gpio;
    }
    addHtmlError(SaveSettings());
  }

  TXBuffer += F("<form  method='post'>");
  html_table_class_normal();
  addFormHeader(F("Hardware Settings"), F("ESPEasy#Hardware_page"));

  addFormSubHeader(F("Wifi Status LED"));
  addFormPinSelect(formatGpioName_output("LED"), "pled", Settings.Pin_status_led);
  addFormCheckBox(F("Inversed LED"), F("pledi"), Settings.Pin_status_led_Inversed);
  addFormNote(F("Use &rsquo;GPIO-2 (D4)&rsquo; with &rsquo;Inversed&rsquo; checked for onboard LED"));

  addFormSubHeader(F("Reset Pin"));
  addFormPinSelect(formatGpioName_input(F("Switch")), "pres", Settings.Pin_Reset);
  addFormNote(F("Press about 10s for factory reset"));

  addFormSubHeader(F("I2C Interface"));
  addFormPinSelectI2C(formatGpioName_bidirectional("SDA"), F("psda"), Settings.Pin_i2c_sda);
  addFormPinSelectI2C(formatGpioName_output("SCL"), F("pscl"), Settings.Pin_i2c_scl);

  // SPI Init
  addFormSubHeader(F("SPI Interface"));
  addFormCheckBox(F("Init SPI"), F("initspi"), Settings.InitSPI);
  addFormNote(F("CLK=GPIO-14 (D5), MISO=GPIO-12 (D6), MOSI=GPIO-13 (D7)"));
  addFormNote(F("Chip Select (CS) config must be done in the plugin"));
#ifdef FEATURE_SD
  addFormPinSelect(formatGpioName_output("SD Card CS"), "sd", Settings.Pin_sd_cs);
#endif

  addFormSubHeader(F("GPIO boot states"));
  int gpio = 0;
  // FIXME TD-er: Max of 17 is a limit in the Settings.PinBootStates array
  while (gpio < MAX_GPIO  && gpio < 17) {
    bool enabled = true;
    if (Settings.UseSerial && (gpio == 1 || gpio == 3)) {
      // do not add the pin state select for these pins.
      enabled = false;
    }
    int pinnr = -1;
    bool input, output, warning;
    if (getGpioInfo(gpio, pinnr, input, output, warning)) {
      String label;
      label.reserve(32);
      label = F("Pin mode ");
      label += createGPIO_label(gpio, pinnr, input, output, warning);
      String int_pinlabel = "p";
      int_pinlabel += gpio;
      addFormPinStateSelect(label, int_pinlabel, Settings.PinBootStates[gpio], enabled);
    }
    ++gpio;
  }
  addFormSeparator(2);

  html_TR_TD();
  html_TD();
  addSubmitButton();
  html_TR_TD();
  html_end_table();
  html_end_form();

  sendHeadandTail_stdtemplate(_TAIL);
  TXBuffer.endStream();

}

//********************************************************************************
// Add a GPIO pin select dropdown list
//********************************************************************************
void addFormPinStateSelect(const String& label, const String& id, int choice, bool enabled)
{
  addRowLabel(label);
  addPinStateSelect(id, choice, enabled);
}

void addPinStateSelect(const String& name, int choice, bool enabled)
{
  String options[4] = { F("Default"), F("Output Low"), F("Output High"), F("Input") };
  addSelector(name, 4, options, NULL, NULL, choice, false, enabled);
}

//********************************************************************************
// Add a IP Access Control select dropdown list
//********************************************************************************
void addFormIPaccessControlSelect(const String& label, const String& id, int choice)
{
  addRowLabel(label);
  addIPaccessControlSelect(id, choice);
}

void addIPaccessControlSelect(const String& name, int choice)
{
  String options[3] = { F("Allow All"), F("Allow Local Subnet"), F("Allow IP range") };
  addSelector(name, 3, options, NULL, NULL, choice, false);
}




//********************************************************************************
// Web Interface device page
//********************************************************************************
//19480 (11128)

// change of device: cleanup old device and reset default settings
void setTaskDevice_to_TaskIndex(byte taskdevicenumber, byte taskIndex) {
  struct EventStruct TempEvent;
  TempEvent.TaskIndex = taskIndex;
  String dummy;

  //let the plugin do its cleanup by calling PLUGIN_EXIT with this TaskIndex
  PluginCall(PLUGIN_EXIT, &TempEvent, dummy);
  taskClear(taskIndex, false); // clear settings, but do not save
  ClearCustomTaskSettings(taskIndex);

  Settings.TaskDeviceNumber[taskIndex] = taskdevicenumber;
  if (taskdevicenumber != 0) // set default values if a new device has been selected
  {
    //NOTE: do not enable task by default. allow user to enter sensible valus first and let him enable it when ready.
    PluginCall(PLUGIN_GET_DEVICEVALUENAMES, &TempEvent, dummy); //the plugin should populate ExtraTaskSettings with its default values.
  } else {
    // New task is empty task, thus save config now.
    SaveTaskSettings(taskIndex);
    SaveSettings();
  }
}

void setBasicTaskValues(byte taskIndex, unsigned long taskdevicetimer,
                        bool enabled, const String& name, int pin1, int pin2, int pin3) {
    LoadTaskSettings(taskIndex); // Make sure ExtraTaskSettings are up-to-date
    byte DeviceIndex = getDeviceIndex(Settings.TaskDeviceNumber[taskIndex]);
    if (taskdevicetimer > 0) {
      Settings.TaskDeviceTimer[taskIndex] = taskdevicetimer;
    } else {
      if (!Device[DeviceIndex].TimerOptional) // Set default delay, unless it's optional...
        Settings.TaskDeviceTimer[taskIndex] = Settings.Delay;
      else
        Settings.TaskDeviceTimer[taskIndex] = 0;
    }
    Settings.TaskDeviceEnabled[taskIndex] = enabled;
    safe_strncpy(ExtraTaskSettings.TaskDeviceName, name.c_str(), sizeof(ExtraTaskSettings.TaskDeviceName));
    if (pin1 >= 0) Settings.TaskDevicePin1[taskIndex] = pin1;
    if (pin2 >= 0) Settings.TaskDevicePin2[taskIndex] = pin2;
    if (pin3 >= 0) Settings.TaskDevicePin3[taskIndex] = pin3;
    SaveTaskSettings(taskIndex);
}


void handle_devices() {
  checkRAM(F("handle_devices"));
  if (!isLoggedIn()) return;
  navMenuIndex = MENU_INDEX_DEVICES;
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate(_HEAD);


  // char tmpString[41];
  struct EventStruct TempEvent;

  // String taskindex = WebServer.arg(F("index"));

  byte taskdevicenumber;
  if (WebServer.hasArg(F("del")))
    taskdevicenumber=0;
  else
    taskdevicenumber = getFormItemInt(F("TDNUM"), 0);


  unsigned long taskdevicetimer = getFormItemInt(F("TDT"),0);
  // String taskdeviceid[CONTROLLER_MAX];
  // String taskdevicepin1 = WebServer.arg(F("taskdevicepin1"));   // "taskdevicepin*" should not be changed because it is uses by plugins and expected to be saved by this code
  // String taskdevicepin2 = WebServer.arg(F("taskdevicepin2"));
  // String taskdevicepin3 = WebServer.arg(F("taskdevicepin3"));
  // String taskdevicepin1pullup = WebServer.arg(F("TDPPU"));
  // String taskdevicepin1inversed = WebServer.arg(F("TDPI"));
  // String taskdevicename = WebServer.arg(F("TDN"));
  // String taskdeviceport = WebServer.arg(F("TDP"));
  // String taskdeviceformula[VARS_PER_TASK];
  // String taskdevicevaluename[VARS_PER_TASK];
  // String taskdevicevaluedecimals[VARS_PER_TASK];
  // String taskdevicesenddata[CONTROLLER_MAX];
  // String taskdeviceglobalsync = WebServer.arg(F("TDGS"));
  // String taskdeviceenabled = WebServer.arg(F("TDE"));

  // for (byte varNr = 0; varNr < VARS_PER_TASK; varNr++)
  // {
  //   char argc[25];
  //   String arg = F("TDF");
  //   arg += varNr + 1;
  //   arg.toCharArray(argc, 25);
  //   taskdeviceformula[varNr] = WebServer.arg(argc);
  //
  //   arg = F("TDVN");
  //   arg += varNr + 1;
  //   arg.toCharArray(argc, 25);
  //   taskdevicevaluename[varNr] = WebServer.arg(argc);
  //
  //   arg = F("TDVD");
  //   arg += varNr + 1;
  //   arg.toCharArray(argc, 25);
  //   taskdevicevaluedecimals[varNr] = WebServer.arg(argc);
  // }

  // for (byte controllerNr = 0; controllerNr < CONTROLLER_MAX; controllerNr++)
  // {
  //   char argc[25];
  //   String arg = F("TDID");
  //   arg += controllerNr + 1;
  //   arg.toCharArray(argc, 25);
  //   taskdeviceid[controllerNr] = WebServer.arg(argc);
  //
  //   arg = F("TDSD");
  //   arg += controllerNr + 1;
  //   arg.toCharArray(argc, 25);
  //   taskdevicesenddata[controllerNr] = WebServer.arg(argc);
  // }

  byte page = getFormItemInt(F("page"), 0);
  if (page == 0)
    page = 1;
  byte setpage = getFormItemInt(F("setpage"), 0);
  if (setpage > 0)
  {
    if (setpage <= (TASKS_MAX / TASKS_PER_PAGE))
      page = setpage;
    else
      page = TASKS_MAX / TASKS_PER_PAGE;
  }
  const int edit = getFormItemInt(F("edit"), 0);

  // taskIndex in the URL is 1 ... TASKS_MAX
  // For use in other functions, set it to 0 ... (TASKS_MAX - 1)
  byte taskIndex = getFormItemInt(F("index"), 0);
  boolean taskIndexNotSet = taskIndex == 0;
  --taskIndex;

  byte DeviceIndex = 0;
  LoadTaskSettings(taskIndex); // Make sure ExtraTaskSettings are up-to-date
  // FIXME TD-er: Might have to clear any caches here.
  if (edit != 0  && !taskIndexNotSet) // when form submitted
  {
    if (Settings.TaskDeviceNumber[taskIndex] != taskdevicenumber)
    {
      // change of device: cleanup old device and reset default settings
      setTaskDevice_to_TaskIndex(taskdevicenumber, taskIndex);
    }
    else if (taskdevicenumber != 0) //save settings
    {
      Settings.TaskDeviceNumber[taskIndex] = taskdevicenumber;
      int pin1 = -1;
      int pin2 = -1;
      int pin3 = -1;
      update_whenset_FormItemInt(F("taskdevicepin1"), pin1);
      update_whenset_FormItemInt(F("taskdevicepin2"), pin2);
      update_whenset_FormItemInt(F("taskdevicepin3"), pin3);
      setBasicTaskValues(taskIndex, taskdevicetimer,
                         isFormItemChecked(F("TDE")), WebServer.arg(F("TDN")),
                         pin1, pin2, pin3);
      Settings.TaskDevicePort[taskIndex] = getFormItemInt(F("TDP"), 0);

      for (byte controllerNr = 0; controllerNr < CONTROLLER_MAX; controllerNr++)
      {
        Settings.TaskDeviceID[controllerNr][taskIndex] = getFormItemInt(String(F("TDID")) + (controllerNr + 1));
        Settings.TaskDeviceSendData[controllerNr][taskIndex] = isFormItemChecked(String(F("TDSD")) + (controllerNr + 1));
      }

      DeviceIndex = getDeviceIndex(Settings.TaskDeviceNumber[taskIndex]);
      if (Device[DeviceIndex].PullUpOption)
        Settings.TaskDevicePin1PullUp[taskIndex] = isFormItemChecked(F("TDPPU"));

      if (Device[DeviceIndex].InverseLogicOption)
        Settings.TaskDevicePin1Inversed[taskIndex] = isFormItemChecked(F("TDPI"));

      for (byte varNr = 0; varNr < Device[DeviceIndex].ValueCount; varNr++)
      {
        strncpy_webserver_arg(ExtraTaskSettings.TaskDeviceFormula[varNr], String(F("TDF")) + (varNr + 1));
        ExtraTaskSettings.TaskDeviceValueDecimals[varNr] = getFormItemInt(String(F("TDVD")) + (varNr + 1));
        strncpy_webserver_arg(ExtraTaskSettings.TaskDeviceValueNames[varNr], String(F("TDVN")) + (varNr + 1));

        // taskdeviceformula[varNr].toCharArray(tmpString, 41);
        // strcpy(ExtraTaskSettings.TaskDeviceFormula[varNr], tmpString);
        // ExtraTaskSettings.TaskDeviceValueDecimals[varNr] = taskdevicevaluedecimals[varNr].toInt();
        // taskdevicevaluename[varNr].toCharArray(tmpString, 41);

      }

      // // task value names handling.
      // for (byte varNr = 0; varNr < Device[DeviceIndex].ValueCount; varNr++)
      // {
      //   taskdevicevaluename[varNr].toCharArray(tmpString, 41);
      //   strcpy(ExtraTaskSettings.TaskDeviceValueNames[varNr], tmpString);
      // }

      TempEvent.TaskIndex = taskIndex;
      if (ExtraTaskSettings.TaskIndex != TempEvent.TaskIndex) // if field set empty, reload defaults
        PluginCall(PLUGIN_GET_DEVICEVALUENAMES, &TempEvent, dummyString);

      //allow the plugin to save plugin-specific form settings.
      PluginCall(PLUGIN_WEBFORM_SAVE, &TempEvent, dummyString);

      // notify controllers: CPLUGIN_TASK_CHANGE_NOTIFICATION
      for (byte x=0; x < CONTROLLER_MAX; x++)
        {
          TempEvent.ControllerIndex = x;
          if (Settings.TaskDeviceSendData[TempEvent.ControllerIndex][TempEvent.TaskIndex] &&
            Settings.ControllerEnabled[TempEvent.ControllerIndex] && Settings.Protocol[TempEvent.ControllerIndex])
            {
              TempEvent.ProtocolIndex = getProtocolIndex(Settings.Protocol[TempEvent.ControllerIndex]);
              CPluginCall(TempEvent.ProtocolIndex, CPLUGIN_TASK_CHANGE_NOTIFICATION, &TempEvent, dummyString);
            }
        }
    }
    addHtmlError(SaveTaskSettings(taskIndex));

    addHtmlError(SaveSettings());

    if (taskdevicenumber != 0 && Settings.TaskDeviceEnabled[taskIndex])
      PluginCall(PLUGIN_INIT, &TempEvent, dummyString);
  }

  // show all tasks as table
  if (taskIndexNotSet)
  {
    html_add_script(true);
    TXBuffer += DATA_UPDATE_SENSOR_VALUES_DEVICE_PAGE_JS;
    html_add_script_end();
    html_table_class_multirow();
    html_TR();
    html_table_header("", 70);

    if (TASKS_MAX != TASKS_PER_PAGE)
    {
      html_add_button_prefix();
      TXBuffer += F("devices?setpage=");
      if (page > 1)
        TXBuffer += page - 1;
      else
        TXBuffer += page;
      TXBuffer += F("'>&lt;</a>");
      html_add_button_prefix();
      TXBuffer += F("devices?setpage=");
      if (page < (TASKS_MAX / TASKS_PER_PAGE))
        TXBuffer += page + 1;
      else
        TXBuffer += page;
      TXBuffer += F("'>&gt;</a>");
    }

    html_table_header("Task", 50);
    html_table_header(F("Enabled"), 100);
    html_table_header(F("Device"));
    html_table_header("Name");
    html_table_header("Port");
    html_table_header(F("Ctr (IDX)"), 100);
    html_table_header("GPIO", 70);
    html_table_header(F("Values"));

    String deviceName;

    for (byte x = (page - 1) * TASKS_PER_PAGE; x < ((page) * TASKS_PER_PAGE); x++)
    {
      html_TR_TD();
      html_add_button_prefix();
      TXBuffer += F("devices?index=");
      TXBuffer += x + 1;
      TXBuffer += F("&page=");
      TXBuffer += page;
      TXBuffer += F("'>Edit</a>");
      html_TD();
      TXBuffer += x + 1;
      html_TD();

      if (Settings.TaskDeviceNumber[x] != 0)
      {
        LoadTaskSettings(x);
        DeviceIndex = getDeviceIndex(Settings.TaskDeviceNumber[x]);
        TempEvent.TaskIndex = x;
        addEnabled( Settings.TaskDeviceEnabled[x]);

        html_TD();
        TXBuffer += getPluginNameFromDeviceIndex(DeviceIndex);
        html_TD();
        TXBuffer += ExtraTaskSettings.TaskDeviceName;
        html_TD();

        byte customConfig = false;
        customConfig = PluginCall(PLUGIN_WEBFORM_SHOW_CONFIG, &TempEvent,TXBuffer.buf);
        if (!customConfig)
          if (Device[DeviceIndex].Ports != 0)
            TXBuffer += formatToHex_decimal(Settings.TaskDevicePort[x]);

        html_TD();

        if (Device[DeviceIndex].SendDataOption)
        {
          boolean doBR = false;
          for (byte controllerNr = 0; controllerNr < CONTROLLER_MAX; controllerNr++)
          {
            byte ProtocolIndex = getProtocolIndex(Settings.Protocol[controllerNr]);
            if (Settings.TaskDeviceSendData[controllerNr][x])
            {
              if (doBR)
                TXBuffer += F("<BR>");
              TXBuffer += getControllerSymbol(controllerNr);
              if (Protocol[ProtocolIndex].usesID && Settings.Protocol[controllerNr] != 0)
              {
                TXBuffer += " (";
                TXBuffer += Settings.TaskDeviceID[controllerNr][x];
                TXBuffer += ')';
                if (Settings.TaskDeviceID[controllerNr][x] == 0)
                  TXBuffer += F(" " HTML_SYMBOL_WARNING);
              }
              doBR = true;
            }
          }
        }

        html_TD();

        if (Settings.TaskDeviceDataFeed[x] == 0)
        {
          if (Device[DeviceIndex].Type == DEVICE_TYPE_I2C)
          {
            TXBuffer += F("GPIO-");
            TXBuffer += Settings.Pin_i2c_sda;
            TXBuffer += F("<BR>GPIO-");
            TXBuffer += Settings.Pin_i2c_scl;
          }
          if (Device[DeviceIndex].Type == DEVICE_TYPE_ANALOG)
            TXBuffer += F("ADC (TOUT)");

          if (Settings.TaskDevicePin1[x] != -1)
          {
            TXBuffer += F("GPIO-");
            TXBuffer += Settings.TaskDevicePin1[x];
          }

          if (Settings.TaskDevicePin2[x] != -1)
          {
            TXBuffer += F("<BR>GPIO-");
            TXBuffer += Settings.TaskDevicePin2[x];
          }

          if (Settings.TaskDevicePin3[x] != -1)
          {
            TXBuffer += F("<BR>GPIO-");
            TXBuffer += Settings.TaskDevicePin3[x];
          }
        }

        html_TD();
        byte customValues = false;
        customValues = PluginCall(PLUGIN_WEBFORM_SHOW_VALUES, &TempEvent,TXBuffer.buf);
        if (!customValues)
        {
          for (byte varNr = 0; varNr < Device[DeviceIndex].ValueCount; varNr++)
          {
            if (Settings.TaskDeviceNumber[x] != 0)
            {
              if (varNr > 0)
                TXBuffer += F("<div class='div_br'></div>");
              TXBuffer += F("<div class='div_l' id='valuename_");
              TXBuffer  += x;
              TXBuffer  += '_';
              TXBuffer  += varNr;
              TXBuffer  += "'>";
              TXBuffer += ExtraTaskSettings.TaskDeviceValueNames[varNr];
              TXBuffer += F(":</div><div class='div_r' id='value_");
              TXBuffer  += x;
              TXBuffer  += '_';
              TXBuffer  += varNr;
              TXBuffer  += "'>";
              TXBuffer += formatUserVarNoCheck(x, varNr);
              TXBuffer += "</div>";
            }
          }
        }
      }
      else {
        html_TD(6);
      }

    } // next
    html_end_table();
    html_end_form();

  }
  // Show edit form if a specific entry is chosen with the edit button
  else
  {
    LoadTaskSettings(taskIndex);
    DeviceIndex = getDeviceIndex(Settings.TaskDeviceNumber[taskIndex]);
    TempEvent.TaskIndex = taskIndex;

    html_add_form();
    html_table_class_normal();
    addFormHeader(F("Task Settings"));


    TXBuffer += F("<TR><TD style='width:150px;' align='left'>Device:<TD>");

    //no device selected
    if (Settings.TaskDeviceNumber[taskIndex] == 0 )
    {
      //takes lots of memory/time so call this only when needed.
      addDeviceSelect("TDNUM", Settings.TaskDeviceNumber[taskIndex]);   //="taskdevicenumber"

    }
    // device selected
    else
    {
      //remember selected device number
      TXBuffer += F("<input type='hidden' name='TDNUM' value='");
      TXBuffer += Settings.TaskDeviceNumber[taskIndex];
      TXBuffer += "'>";

      //show selected device name and delete button
      TXBuffer += getPluginNameFromDeviceIndex(DeviceIndex);

      addHelpButton(String(F("Plugin")) + Settings.TaskDeviceNumber[taskIndex]);
      addRTDPluginButton(Settings.TaskDeviceNumber[taskIndex]);


      if (Device[DeviceIndex].Number == 3 && taskIndex >= 4) // Number == 3 = PulseCounter Plugin
        {
          addFormNote(F("This plugin is only supported on task 1-4 for now"));
        }

      addFormTextBox( F("Name"), F("TDN"), ExtraTaskSettings.TaskDeviceName, NAME_FORMULA_LENGTH_MAX);   //="taskdevicename"

      addFormCheckBox(F("Enabled"), F("TDE"), Settings.TaskDeviceEnabled[taskIndex]);   //="taskdeviceenabled"

      // section: Sensor / Actuator
      if (!Device[DeviceIndex].Custom && Settings.TaskDeviceDataFeed[taskIndex] == 0 &&
          ((Device[DeviceIndex].Ports != 0) ||
           (Device[DeviceIndex].PullUpOption) ||
           (Device[DeviceIndex].InverseLogicOption) ||
           (Device[DeviceIndex].connectedToGPIOpins())) )
      {
        addFormSubHeader((Device[DeviceIndex].SendDataOption) ? F("Sensor") : F("Actuator"));

        if (Device[DeviceIndex].Ports != 0)
          addFormNumericBox(F("Port"), F("TDP"), Settings.TaskDevicePort[taskIndex]);   //="taskdeviceport"

        if (Device[DeviceIndex].PullUpOption)
        {
          addFormCheckBox(F("Internal PullUp"), F("TDPPU"), Settings.TaskDevicePin1PullUp[taskIndex]);   //="taskdevicepin1pullup"
          if ((Settings.TaskDevicePin1[taskIndex] == 16) || (Settings.TaskDevicePin2[taskIndex] == 16) || (Settings.TaskDevicePin3[taskIndex] == 16))
            addFormNote(F("GPIO-16 (D0) does not support PullUp"));
        }

        if (Device[DeviceIndex].InverseLogicOption)
        {
          addFormCheckBox(F("Inversed Logic"), F("TDPI"), Settings.TaskDevicePin1Inversed[taskIndex]);   //="taskdevicepin1inversed"
          addFormNote(F("Will go into effect on next input change."));
        }

        //get descriptive GPIO-names from plugin
        TempEvent.String1 = F("1st GPIO");
        TempEvent.String2 = F("2nd GPIO");
        TempEvent.String3 = F("3rd GPIO");
        PluginCall(PLUGIN_GET_DEVICEGPIONAMES, &TempEvent, dummyString);

        if (Device[DeviceIndex].connectedToGPIOpins()) {
          if (Device[DeviceIndex].Type >= DEVICE_TYPE_SINGLE)
            addFormPinSelect(TempEvent.String1, F("taskdevicepin1"), Settings.TaskDevicePin1[taskIndex]);
          if (Device[DeviceIndex].Type >= DEVICE_TYPE_DUAL)
            addFormPinSelect(TempEvent.String2, F("taskdevicepin2"), Settings.TaskDevicePin2[taskIndex]);
          if (Device[DeviceIndex].Type == DEVICE_TYPE_TRIPLE)
            addFormPinSelect(TempEvent.String3, F("taskdevicepin3"), Settings.TaskDevicePin3[taskIndex]);
        }
      }

      //add plugins content
      if (Settings.TaskDeviceDataFeed[taskIndex] == 0) { // only show additional config for local connected sensors
        String webformLoadString;
        PluginCall(PLUGIN_WEBFORM_LOAD, &TempEvent,webformLoadString);
        if (webformLoadString.length() > 0) {
          String errorMessage;
          PluginCall(PLUGIN_GET_DEVICENAME, &TempEvent, errorMessage);
          errorMessage += F(": Bug in PLUGIN_WEBFORM_LOAD, should not append to string, use addHtml() instead");
          addHtmlError(errorMessage);
        }
      }

      //section: Data Acquisition
      if (Device[DeviceIndex].SendDataOption)
      {
        addFormSubHeader(F("Data Acquisition"));

        for (byte controllerNr = 0; controllerNr < CONTROLLER_MAX; controllerNr++)
        {
          if (Settings.Protocol[controllerNr] != 0)
          {
            String id = F("TDSD");   //="taskdevicesenddata"
            id += controllerNr + 1;

            html_TR_TD(); TXBuffer += F("Send to Controller ");
            TXBuffer += getControllerSymbol(controllerNr);
            html_TD();
            addCheckBox(id, Settings.TaskDeviceSendData[controllerNr][taskIndex]);

            byte ProtocolIndex = getProtocolIndex(Settings.Protocol[controllerNr]);
            if (Protocol[ProtocolIndex].usesID && Settings.Protocol[controllerNr] != 0)
            {
              addRowLabel(F("IDX"));
              id = F("TDID");   //="taskdeviceid"
              id += controllerNr + 1;
              addNumericBox(id, Settings.TaskDeviceID[controllerNr][taskIndex], 0, DOMOTICZ_MAX_IDX);
            }
          }
        }
      }

      addFormSeparator(2);

      if (Device[DeviceIndex].TimerOption)
      {
        //FIXME: shoudn't the max be ULONG_MAX because Settings.TaskDeviceTimer is an unsigned long? addFormNumericBox only supports ints for min and max specification
        addFormNumericBox( F("Interval"), F("TDT"), Settings.TaskDeviceTimer[taskIndex], 0, 65535);   //="taskdevicetimer"
        addUnit(F("sec"));
        if (Device[DeviceIndex].TimerOptional)
          TXBuffer += F(" (Optional for this Device)");
      }

      //section: Values
      if (!Device[DeviceIndex].Custom && Device[DeviceIndex].ValueCount > 0)
      {
        addFormSubHeader(F("Values"));
        html_end_table();
        html_table_class_normal();

        //table header
        TXBuffer += F("<TR><TH style='width:30px;' align='center'>#");
        html_table_header("Name");

        if (Device[DeviceIndex].FormulaOption)
        {
          html_table_header(F("Formula"), F("EasyFormula"), 0);
        }

        if (Device[DeviceIndex].FormulaOption || Device[DeviceIndex].DecimalsOnly)
        {
          html_table_header(F("Decimals"), 30);
        }

        //table body
        for (byte varNr = 0; varNr < Device[DeviceIndex].ValueCount; varNr++)
        {
          html_TR_TD();
          TXBuffer += varNr + 1;
          html_TD();
          String id = F("TDVN");   //="taskdevicevaluename"
          id += (varNr + 1);
          addTextBox(id, ExtraTaskSettings.TaskDeviceValueNames[varNr], NAME_FORMULA_LENGTH_MAX);

          if (Device[DeviceIndex].FormulaOption)
          {
            html_TD();
            String id = F("TDF");   //="taskdeviceformula"
            id += (varNr + 1);
            addTextBox(id, ExtraTaskSettings.TaskDeviceFormula[varNr], NAME_FORMULA_LENGTH_MAX);
          }

          if (Device[DeviceIndex].FormulaOption || Device[DeviceIndex].DecimalsOnly)
          {
            html_TD();
            String id = F("TDVD");   //="taskdevicevaluedecimals"
            id += (varNr + 1);
            addNumericBox(id, ExtraTaskSettings.TaskDeviceValueDecimals[varNr], 0, 6);
          }
        }
      }
    }

    addFormSeparator(4);

    html_TR_TD();
    TXBuffer += F("<TD colspan='3'>");
    html_add_button_prefix();
    TXBuffer += F("devices?setpage=");
    TXBuffer += page;
    TXBuffer += F("'>Close</a>");
    addSubmitButton();
    TXBuffer += F("<input type='hidden' name='edit' value='1'>");
    TXBuffer += F("<input type='hidden' name='page' value='1'>");

    //if user selected a device, add the delete button
    if (Settings.TaskDeviceNumber[taskIndex] != 0 )
      addSubmitButton(F("Delete"), F("del"));

    html_end_table();
    html_end_form();
  }


  checkRAM(F("handle_devices"));
#ifndef BUILD_NO_DEBUG
  if (loglevelActiveFor(LOG_LEVEL_DEBUG_DEV)) {
    String log = F("DEBUG: String size:");
    log += String(TXBuffer.sentBytes);
    addLog(LOG_LEVEL_DEBUG_DEV, log);
  }
#endif
  sendHeadandTail_stdtemplate(_TAIL);
  TXBuffer.endStream();
}


byte sortedIndex[DEVICES_MAX + 1];
//********************************************************************************
// Add a device select dropdown list
//********************************************************************************
void addDeviceSelect(const String& name,  int choice)
{
  // first get the list in alphabetic order
  for (byte x = 0; x <= deviceCount; x++)
    sortedIndex[x] = x;
  sortDeviceArray();

  String deviceName;

  addSelector_Head(name, true);
  addSelector_Item(F("- None -"), 0, false, false, "");
  for (byte x = 0; x <= deviceCount; x++)
  {
    byte deviceIndex = sortedIndex[x];
    if (Plugin_id[deviceIndex] != 0)
      deviceName = getPluginNameFromDeviceIndex(deviceIndex);

#ifdef PLUGIN_BUILD_DEV
    int num = Plugin_id[deviceIndex];
    String plugin = "P";
    if (num < 10) plugin += '0';
    if (num < 100) plugin += '0';
    plugin += num;
    plugin += F(" - ");
    deviceName = plugin + deviceName;
#endif

    addSelector_Item(deviceName,
                     Device[deviceIndex].Number,
                     choice == Device[deviceIndex].Number,
                     false,
                     "");
  }
  addSelector_Foot();
}

//********************************************************************************
// Device Sort routine, switch array entries
//********************************************************************************
void switchArray(byte value)
{
  byte temp;
  temp = sortedIndex[value - 1];
  sortedIndex[value - 1] = sortedIndex[value];
  sortedIndex[value] = temp;
}


//********************************************************************************
// Device Sort routine, compare two array entries
//********************************************************************************
boolean arrayLessThan(const String& ptr_1, const String& ptr_2)
{
  unsigned int i = 0;
  while (i < ptr_1.length())    // For each character in string 1, starting with the first:
  {
    if (ptr_2.length() < i)    // If string 2 is shorter, then switch them
    {
      return true;
    }
    else
    {
      const char check1 = (char)ptr_1[i];  // get the same char from string 1 and string 2
      const char check2 = (char)ptr_2[i];
      if (check1 == check2) {
        // they're equal so far; check the next char !!
        i++;
      } else {
        return (check2 > check1);
      }
    }
  }
  return false;
}


//********************************************************************************
// Device Sort routine, actual sorting
//********************************************************************************
void sortDeviceArray()
{
  int innerLoop ;
  int mainLoop ;
  for ( mainLoop = 1; mainLoop <= deviceCount; mainLoop++)
  {
    innerLoop = mainLoop;
    while (innerLoop  >= 1)
    {
      if (arrayLessThan(
        getPluginNameFromDeviceIndex(sortedIndex[innerLoop]),
        getPluginNameFromDeviceIndex(sortedIndex[innerLoop - 1])))
      {
        switchArray(innerLoop);
      }
      innerLoop--;
    }
  }
}

void addFormPinSelect(const String& label, const String& id, int choice)
{
  addRowLabel(label, String("tr_")+id);
  addPinSelect(false, id, choice);
}


void addFormPinSelectI2C(const String& label, const String& id, int choice)
{

  addRowLabel(label, String("tr_")+id);
  addPinSelect(true, id, choice);
}


//********************************************************************************
// Add a GPIO pin select dropdown list for 8266, 8285 or ESP32
//********************************************************************************
String createGPIO_label(int gpio, int pinnr, bool input, bool output, bool warning) {
  if (gpio < 0) return F("- None -");
  String result;
  result.reserve(24);
  result = F("GPIO-");
  result += gpio;
  if (pinnr >= 0) {
    result += F(" (D");
    result += pinnr;
    result += ')';
  }
  if (input != output) {
    result += ' ';
    result += input ? F(HTML_SYMBOL_INPUT) : F(HTML_SYMBOL_OUTPUT);
  }
  if (warning) {
    result += ' ';
    result += F(HTML_SYMBOL_WARNING);
  }
  bool serialPinConflict = (Settings.UseSerial && (gpio == 1 || gpio == 3));
  if (serialPinConflict) {
    if (gpio == 1) { result += F(" TX0"); }
    if (gpio == 3) { result += F(" RX0"); }
  }
  return result;
}

void addPinSelect(boolean forI2C, String id,  int choice)
{
  #ifdef ESP32
    #define NR_ITEMS_PIN_DROPDOWN  35 // 34 GPIO + 1
  #else
    #define NR_ITEMS_PIN_DROPDOWN  14 // 13 GPIO + 1
  #endif

  String * gpio_labels = new String[NR_ITEMS_PIN_DROPDOWN];
  int * gpio_numbers = new int[NR_ITEMS_PIN_DROPDOWN];

  // At i == 0 && gpio == -1, add the "- None -" option first
  int i = 0;
  int gpio = -1;
  while (i < NR_ITEMS_PIN_DROPDOWN && gpio <= MAX_GPIO) {
    int pinnr = -1;
    bool input, output, warning;
    if (getGpioInfo(gpio, pinnr, input, output, warning) || i == 0) {
      gpio_labels[i] = createGPIO_label(gpio, pinnr, input, output, warning);
      gpio_numbers[i] = gpio;
      ++i;
    }
    ++gpio;
  }
  renderHTMLForPinSelect(gpio_labels, gpio_numbers, forI2C, id, choice, NR_ITEMS_PIN_DROPDOWN);
  delete[] gpio_numbers;
  delete[] gpio_labels;
  #undef NR_ITEMS_PIN_DROPDOWN
}


//********************************************************************************
// Helper function actually rendering dropdown list for addPinSelect()
//********************************************************************************
void renderHTMLForPinSelect(String options[], int optionValues[], boolean forI2C, const String& id,  int choice, int count) {
  addSelector_Head(id, false);
  for (byte x = 0; x < count; x++)
  {
    boolean disabled = false;

    if (optionValues[x] != -1) // empty selection can never be disabled...
    {
      if (!forI2C && ((optionValues[x] == Settings.Pin_i2c_sda) || (optionValues[x] == Settings.Pin_i2c_scl)))
        disabled = true;
      if (Settings.UseSerial && ((optionValues[x] == 1) || (optionValues[x] == 3)))
        disabled = true;
    }
    addSelector_Item(options[x],
                     optionValues[x],
                     choice == optionValues[x],
                     disabled,
                     "");
  }
  addSelector_Foot();
}


void addFormSelectorI2C(const String& id, int addressCount, const int addresses[], int selectedIndex)
{
  String options[addressCount];
  for (byte x = 0; x < addressCount; x++)
  {
    options[x] = formatToHex_decimal(addresses[x]);
    if (x == 0)
      options[x] += F(" - (default)");
  }
  addFormSelector(F("I2C Address"), id, addressCount, options, addresses, NULL, selectedIndex, false);
}

void addFormSelector(const String& label, const String& id, int optionCount, const String options[], const int indices[], int selectedIndex)
{
  addFormSelector(label, id, optionCount, options, indices, NULL, selectedIndex, false);
}

void addFormSelector(const String& label, const String& id, int optionCount, const String options[], const int indices[], int selectedIndex, bool reloadonchange)
{
  addFormSelector(label, id, optionCount, options, indices, NULL, selectedIndex, reloadonchange);
}

void addFormSelector(const String& label, const String& id, int optionCount, const String options[], const int indices[], const String attr[], int selectedIndex, boolean reloadonchange)
{
  addRowLabel(label);
  addSelector(id, optionCount, options, indices, attr, selectedIndex, reloadonchange);
}

void addFormSelector_script(const String& label, const String& id, int optionCount, const String options[], const int indices[], const String attr[], int selectedIndex, const String& onChangeCall)
{
  addRowLabel(label);
  addSelector_Head(id, onChangeCall, false);
  addSelector_options(optionCount, options, indices, attr, selectedIndex);
  addSelector_Foot();
}

void addSelector(const String& id, int optionCount, const String options[], const int indices[], const String attr[], int selectedIndex, boolean reloadonchange) {
  addSelector(id, optionCount, options, indices, attr, selectedIndex, reloadonchange, true);
}

void addSelector(const String& id, int optionCount, const String options[], const int indices[], const String attr[], int selectedIndex, boolean reloadonchange, bool enabled)
{
  // FIXME TD-er Change boolean to disabled
  addSelector_Head(id, reloadonchange, !enabled);
  addSelector_options(optionCount, options, indices, attr, selectedIndex);
  addSelector_Foot();
}

void addSelector_options(int optionCount, const String options[], const int indices[], const String attr[], int selectedIndex)
{
  int index;
  for (byte x = 0; x < optionCount; x++)
  {
    if (indices)
      index = indices[x];
    else
      index = x;
    TXBuffer += F("<option value=");
    TXBuffer += index;
    if (selectedIndex == index)
      TXBuffer += F(" selected");
    if (attr)
    {
      TXBuffer += ' ';
      TXBuffer += attr[x];
    }
    TXBuffer += '>';
    TXBuffer += options[x];
    TXBuffer += F("</option>");
  }
}

void addSelector_Head(const String& id, boolean reloadonchange) {
  addSelector_Head(id, reloadonchange, false);
}

void addSelector_Head(const String& id, boolean reloadonchange, bool disabled)
{
  if (reloadonchange) {
    addSelector_Head(id, (const String) F("return dept_onchange(frmselect)"), disabled);
  } else {
    addSelector_Head(id, (const String) "", disabled);
  }
}

void addSelector_Head(const String& id, const String& onChangeCall, bool disabled)
{
  TXBuffer += F("<select class='wide' name='");
  TXBuffer += id;
  TXBuffer += F("' id='");
  TXBuffer += id;
  TXBuffer += '\'';
  if (disabled) {
    addDisabled();
  }
  if (onChangeCall.length() > 0) {
    TXBuffer += F(" onchange='");
    TXBuffer += onChangeCall;
    TXBuffer += '\'';
  }
  TXBuffer += '>';
}


void addSelector_Item(const String& option, int index, boolean selected, boolean disabled, const String& attr)
{
  TXBuffer += F("<option value=");
  TXBuffer += index;
  if (selected)
    TXBuffer += F(" selected");
  if (disabled)
    addDisabled();
  if (attr && attr.length() > 0)
  {
    TXBuffer += ' ';
    TXBuffer += attr;
  }
  TXBuffer += '>';
  TXBuffer += option;
  TXBuffer += F("</option>");
}


void addSelector_Foot()
{
  TXBuffer += F("</select>");
}


void addUnit(const String& unit)
{
  TXBuffer += F(" [");
  TXBuffer += unit;
  TXBuffer += "]";
}

void addRowLabel(const String& label)
{
  addRowLabel(label, "");
}

void addRowLabel(const String& label, const String& id)
{
  if (id.length() > 0) {
    TXBuffer += F("<TR id='");
    TXBuffer += id;
    TXBuffer += F("'><TD>");
  } else {
    html_TR_TD();
  }
  TXBuffer += label;
  TXBuffer += ':';
  html_TD();
}

// Add a row label and mark it with copy markers to copy it to clipboard.
void addRowLabel_copy(const String& label) {
  TXBuffer += F("<TR>");
  html_copyText_TD();
  TXBuffer += label;
  TXBuffer += ':';
  html_copyText_marker();
  html_copyText_TD();
}

void addButton(const String &url, const String &label) {
  addButton(url, label, "");
}

void addButton(const String &url, const String &label, const String& classes) {
  addButton(url, label, classes, true);
}

void addButton(const String &url, const String &label, const String& classes, bool enabled)
{
  html_add_button_prefix(classes, enabled);
  TXBuffer += url;
  TXBuffer += "'>";
  TXBuffer += label;
  TXBuffer += F("</a>");
}

void addButton(class StreamingBuffer &buffer, const String &url, const String &label)
{
  addButtonWithSvg(buffer, url, label, "", false);
}

void addButtonWithSvg(class StreamingBuffer &buffer, const String &url, const String &label, const String& svgPath, bool needConfirm) {
  bool hasSVG = svgPath.length() > 0;
  buffer += F("<a class='button link' href='");
  buffer += url;
  if (hasSVG) {
    buffer += F("' alt='");
    buffer += label;
  }
  if (needConfirm) {
    buffer += F("' onclick='return confirm(\"Are you sure?\")");
  }
  buffer += F("'>");
  if (hasSVG) {
    buffer += F("<svg width='24' height='24' viewBox='-1 -1 26 26' style='position: relative; top: 5px;'>");
    buffer += svgPath;
    buffer += F("</svg>");
  } else {
    buffer += label;
  }
  buffer += F("</a>");
}

void addSaveButton(const String &url, const String &label)
{
  addSaveButton(TXBuffer, url, label);
}

void addSaveButton(class StreamingBuffer &buffer, const String &url, const String &label)
{
#ifdef BUILD_MINIMAL_OTA
  addButtonWithSvg(buffer, url, label
     , ""
     , false);
#else
  addButtonWithSvg(buffer, url, label
     , F("<path d='M19 12v7H5v-7H3v7c0 1.1.9 2 2 2h14c1.1 0 2-.9 2-2v-7h-2zm-6 .67l2.59-2.58L17 11.5l-5 5-5-5 1.41-1.41L11 12.67V3h2v9.67z'  stroke='white' fill='white' ></path>")
     , false);
#endif
}

void addDeleteButton(const String &url, const String &label)
{
  addSaveButton(TXBuffer, url, label);
}

void addDeleteButton(class StreamingBuffer &buffer, const String &url, const String &label)
{
#ifdef BUILD_MINIMAL_OTA
  addButtonWithSvg(buffer, url, label
     , ""
     , true);
#else
  addButtonWithSvg(buffer, url, label
    , F("<path fill='none' d='M0 0h24v24H0V0z'></path><path d='M6 19c0 1.1.9 2 2 2h8c1.1 0 2-.9 2-2V7H6v12zM8 9h8v10H8V9zm7.5-5l-1-1h-5l-1 1H5v2h14V4h-3.5z' stroke='white' fill='white' ></path>")
    , true);
#endif
}

void addWideButton(const String &url, const String &label) {
  addWideButton(url, label, "", true);
}

void addWideButton(const String &url, const String &label, const String &classes) {
  addWideButton(url, label, classes, true);
}

void addWideButton(const String &url, const String &label, const String &classes, bool enabled)
{
  html_add_wide_button_prefix(classes, enabled);
  TXBuffer += url;
  TXBuffer += "'>";
  TXBuffer += label;
  TXBuffer += F("</a>");
}

void addSubmitButton()
{
  addSubmitButton(F("Submit"), "");
}

//add submit button with different label and name
void addSubmitButton(const String &value, const String &name) {
  addSubmitButton(value, name, "");
}

void addSubmitButton(const String &value, const String &name, const String &classes)
{
  TXBuffer += F("<input class='button link");
  if (classes.length() > 0) {
    TXBuffer += ' ';
    TXBuffer += classes;
  }
  TXBuffer += F("' type='submit' value='");
  TXBuffer += value;
  if (name.length() > 0) {
    TXBuffer += F("' name='");
    TXBuffer += name;
  }
  TXBuffer += F("'><div id='toastmessage'></div><script type='text/javascript'>toasting();</script>");
}

// add copy to clipboard button
void addCopyButton(const String &value, const String &delimiter, const String &name)
{
  TXBuffer += jsClipboardCopyPart1;
  TXBuffer += value;
  TXBuffer += jsClipboardCopyPart2;
  TXBuffer += delimiter;
  TXBuffer += jsClipboardCopyPart3;
  //Fix HTML
  TXBuffer += F("<button class='button link' onclick='setClipboard()'>");
  TXBuffer += name;
  TXBuffer += " (";
  html_copyText_marker();
  TXBuffer += ')';
  TXBuffer += F("</button>");
}


//********************************************************************************
// Add a header
//********************************************************************************
void addTableSeparator(const String& label, int colspan, int h_size) {
  addTableSeparator(label, colspan, h_size, "");
}

void addTableSeparator(const String& label, int colspan, int h_size, const String& helpButton) {
  TXBuffer += F("<TR><TD colspan=");
  TXBuffer += colspan;
  TXBuffer += "><H";
  TXBuffer += h_size;
  TXBuffer += '>';
  TXBuffer += label;
  if (helpButton.length() > 0)
    addHelpButton(helpButton);
  TXBuffer += "</H";
  TXBuffer += h_size;
  TXBuffer += F("></TD></TR>");
}

void addFormHeader(const String& header, const String& helpButton)
{
  html_TR();
  html_table_header(header, helpButton, 225);
  html_table_header("");
}

void addFormHeader(const String& header)
{
  addFormHeader(header, "");
}


//********************************************************************************
// Add a sub header
//********************************************************************************
void addFormSubHeader(const String& header)
{
  addTableSeparator(header, 2, 3);
}


//********************************************************************************
// Add a note as row start
//********************************************************************************
void addFormNote(const String& text)
{
  html_TR_TD();
  html_TD();
  TXBuffer += F("<div class='note'>Note: ");
  TXBuffer += text;
  TXBuffer += F("</div>");
}


//********************************************************************************
// Add a separator as row start
//********************************************************************************
void addFormSeparator(int clspan)
{
 TXBuffer += F("<TR><TD colspan='");
 TXBuffer += clspan;
 TXBuffer += F("'><hr>");
}


//********************************************************************************
// Add a checkbox
//********************************************************************************
void addCheckBox(const String& id, boolean checked) {
  addCheckBox(id, checked, false);
}

void addCheckBox(const String& id, boolean checked, bool disabled)
{
  TXBuffer += F("<label class='container'>&nbsp;");
  TXBuffer += F("<input type='checkbox' id='");
  TXBuffer += id;
  TXBuffer += F("' name='");
  TXBuffer += id;
  TXBuffer += '\'';
  if (checked)
    TXBuffer += F(" checked");
  if (disabled) addDisabled();
  TXBuffer += F("><span class='checkmark");
  if (disabled) addDisabled();
  TXBuffer += F("'></span></label>");
}

void addFormCheckBox(const String& label, const String& id, boolean checked) {
  addFormCheckBox(label, id, checked, false);
}

void addFormCheckBox_disabled(const String& label, const String& id, boolean checked) {
  addFormCheckBox(label, id, checked, true);
}

void addFormCheckBox(const String& label, const String& id, boolean checked, bool disabled)
{
  addRowLabel(label);
  addCheckBox(id, checked, disabled);
}


//********************************************************************************
// Add a numeric box
//********************************************************************************
void addNumericBox(const String& id, int value, int min, int max)
{
  TXBuffer += F("<input class='widenumber' type='number' name='");
  TXBuffer += id;
  TXBuffer += '\'';
  if (min != INT_MIN)
  {
    TXBuffer += F(" min=");
    TXBuffer += min;
  }
  if (max != INT_MAX)
  {
    TXBuffer += F(" max=");
    TXBuffer += max;
  }
  TXBuffer += F(" value=");
  TXBuffer += value;
  TXBuffer += '>';
}

void addNumericBox(const String& id, int value)
{
  addNumericBox(id, value, INT_MIN, INT_MAX);
}

void addFormNumericBox(const String& label, const String& id, int value, int min, int max)
{
  addRowLabel(label);
  addNumericBox(id, value, min, max);
}

void addFormNumericBox(const String& label, const String& id, int value)
{
  addFormNumericBox(label, id, value, INT_MIN, INT_MAX);
}

void addFloatNumberBox(const String& id, float value, float min, float max)
{
  TXBuffer += F("<input type='number' name='");
  TXBuffer += id;
  TXBuffer += '\'';
  TXBuffer += F(" min=");
  TXBuffer += min;
  TXBuffer += F(" max=");
  TXBuffer += max;
  TXBuffer += F(" step=0.01");
  TXBuffer += F(" style='width:5em;' value=");
  TXBuffer += value;
  TXBuffer += '>';
}

void addFormFloatNumberBox(const String& label, const String& id, float value, float min, float max)
{
  addRowLabel(label);
  addFloatNumberBox(id, value, min, max);
}


void addTextBox(const String& id, const String&  value, int maxlength)
{
  addTextBox(id, value, maxlength, false);
}

void addTextBox(const String& id, const String&  value, int maxlength, bool readonly)
{
  addTextBox(id, value, maxlength, false, false, "");
}

void addTextBox(const String& id, const String&  value, int maxlength, bool readonly, bool required)
{
  addTextBox(id, value, maxlength, false, false, "");
}

void addTextBox(const String& id, const String&  value, int maxlength, bool readonly, bool required, const String& pattern)
{
  TXBuffer += F("<input class='wide' type='text' name='");
  TXBuffer += id;
  TXBuffer += F("' maxlength=");
  TXBuffer += maxlength;
  TXBuffer += F(" value='");
  TXBuffer += value;
  TXBuffer += '\'';
  if(readonly){
    TXBuffer += F(" readonly ");
  }
  if(required){
    TXBuffer += F(" required ");
  }
  if(pattern.length()>0){
    TXBuffer += F("pattern = '");
    TXBuffer += pattern;
    TXBuffer += '\'';
  }
  TXBuffer += '>';
}

void addFormTextBox(const String& label, const String& id, const String&  value, int maxlength)
{
  addRowLabel(label);
  addTextBox(id, value, maxlength);
}

void addFormTextBox(const String& label, const String& id, const String&  value, int maxlength, bool readonly)
{
  addRowLabel(label);
  addTextBox(id, value, maxlength, readonly);
}

void addFormTextBox(const String& label, const String& id, const String&  value, int maxlength, bool readonly, bool required)
{
  addRowLabel(label);
  addTextBox(id, value, maxlength, readonly, required);
}

void addFormTextBox(const String& label, const String& id, const String&  value, int maxlength, bool readonly, bool required, const String& pattern)
{
  addRowLabel(label);
  addTextBox(id, value, maxlength, readonly, required, pattern);
}


void addFormPasswordBox(const String& label, const String& id, const String& password, int maxlength)
{
  addRowLabel(label);
  TXBuffer += F("<input class='wide' type='password' name='");
  TXBuffer += id;
  TXBuffer += F("' maxlength=");
  TXBuffer += maxlength;
  TXBuffer += F(" value='");
  if (password != "")   //no password?
    TXBuffer += F("*****");
  //TXBuffer += password;   //password will not published over HTTP
  TXBuffer += "'>";
}

void copyFormPassword(const String& id, char* pPassword, int maxlength)
{
  String password = WebServer.arg(id);
  if (password == F("*****"))   //no change?
    return;
  safe_strncpy(pPassword, password.c_str(), maxlength);
}

void addFormIPBox(const String& label, const String& id, const byte ip[4])
{
  bool empty_IP =(ip[0] == 0 && ip[1] == 0 && ip[2] == 0 && ip[3] == 0);

  addRowLabel(label);
  TXBuffer += F("<input class='wide' type='text' name='");
  TXBuffer += id;
  TXBuffer += F("' value='");
  if (!empty_IP){
    TXBuffer += formatIP(ip);
  }
  TXBuffer += "'>";
}

// adds a Help Button with points to the the given Wiki Subpage
void addHelpButton(const String& url)
{
  addHtmlLink(
    F("button help"),
    makeDocLink(url, false),
    F("&#10068;"));
}

void addRTDPluginButton(int taskDeviceNumber) {
  String url;
  url.reserve(16);
  url = F("Plugin/P");
  if (taskDeviceNumber < 100) url += '0';
  if (taskDeviceNumber < 10) url += '0';
  url += String(taskDeviceNumber);
  url += F(".html");
  addHtmlLink(
    F("button help"),
    makeDocLink(url, true),
    F("&#8505;"));

  switch (taskDeviceNumber) {
    case 76:
    case 77:
      addHtmlLink(
        F("button help"),
        makeDocLink(F("Reference/Safety.html"), true),
        F("&#9889;")); // High voltage sign
      break;

  }
}

String makeDocLink(const String& url, bool isRTD) {
  String result;
  if (!url.startsWith(F("http"))) {
    if (isRTD) {
      result += F("https://espeasy.readthedocs.io/en/latest/");
    } else {
      result += F("http://www.letscontrolit.com/wiki/index.php/");
    }
  }
  result += url;
  return result;
}

void addHtmlLink(const String& htmlclass, const String& url, const String& label) {
  TXBuffer += F(" <a class='");
  TXBuffer += htmlclass;
  TXBuffer += F("' href='");
  TXBuffer += url;
  TXBuffer += F("' target='_blank'>");
  TXBuffer += label;
  TXBuffer += F("</a>");
}

void addEnabled(boolean enabled)
{
  TXBuffer += F("<span class='enabled ");
  if (enabled)
    TXBuffer += F("on'>&#10004;");
  else
    TXBuffer += F("off'>&#10060;");
  TXBuffer += F("</span>");
}


//********************************************************************************
// HTML string re-use to keep the executable smaller
// Flash strings are not checked for duplication.
//********************************************************************************
void wrap_html_tag(const String& tag, const String& text) {
  TXBuffer += '<';
  TXBuffer += tag;
  TXBuffer += '>';
  TXBuffer += text;
  TXBuffer += "</";
  TXBuffer += tag;
  TXBuffer += '>';
}

void html_B(const String& text) {
  wrap_html_tag("b", text);
}

void html_I(const String& text) {
  wrap_html_tag("i", text);
}

void html_U(const String& text) {
  wrap_html_tag("u", text);
}

void html_TR_TD_highlight() {
  TXBuffer += F("<TR class=\"highlight\">");
  html_TD();
}

void html_TR_TD() {
  html_TR();
  html_TD();
}

void html_BR() {
  TXBuffer += F("<BR>");
}

void html_TR() {
  TXBuffer += F("<TR>");
}

void html_TR_TD_height(int height) {
  html_TR();
  TXBuffer += F("<TD HEIGHT=\"");
  TXBuffer += height;
  TXBuffer += "\">";
}

void html_TD() {
  html_TD(1);
}

void html_TD(int td_cnt) {
  for (int i = 0; i < td_cnt; ++i) {
    TXBuffer += F("<TD>");
  }
}

static int copyTextCounter = 0;

void html_reset_copyTextCounter() {
  copyTextCounter = 0;
}

void html_copyText_TD() {
  ++copyTextCounter;
  TXBuffer += F("<TD id='copyText_");
  TXBuffer += copyTextCounter;
  TXBuffer += "'>";
}

// Add some recognizable token to show which parts will be copied.
void html_copyText_marker() {
  TXBuffer += F("&#x022C4;"); //   &diam; &diamond; &Diamond; &#x022C4; &#8900;
}

void html_add_estimate_symbol() {
  TXBuffer += F(" &#8793; "); //   &#8793;  &#x2259;  &wedgeq;
}

void html_table_class_normal() {
  html_table(F("normal"));
}

void html_table_class_multirow() {
  html_table(F("multirow"), true);
}

void html_table_class_multirow_noborder() {
  html_table(F("multirow"), false);
}

void html_table(const String& tableclass) {
  html_table(tableclass, false);
}

void html_table(const String& tableclass, bool boxed) {
  TXBuffer += F("<table class='");
  TXBuffer += tableclass;
  TXBuffer += '\'';
  if (boxed) {
    TXBuffer += F("' border=1px frame='box' rules='all'");
  }
  TXBuffer += '>';
}

void html_table_header(const String& label) {
  html_table_header(label, 0);
}

void html_table_header(const String& label, int width) {
  html_table_header(label, "", width);
}

void html_table_header(const String& label, const String& helpButton, int width) {
  TXBuffer += F("<TH");
  if (width > 0) {
    TXBuffer += F(" style='width:");
    TXBuffer += String(width);
    TXBuffer += F("px;'");
  }
  TXBuffer += '>';
  TXBuffer += label;
  if (helpButton.length() > 0)
    addHelpButton(helpButton);
  TXBuffer += F("</TH>");
}

void html_end_table() {
  TXBuffer += F("</table>");
}

void html_end_form() {
  TXBuffer += F("</form>");
}

void html_add_button_prefix() {
  html_add_button_prefix("", true);
}

void html_add_button_prefix(const String& classes, bool enabled) {
  TXBuffer += F(" <a class='button link");
  if (classes.length() > 0) {
    TXBuffer += ' ';
    TXBuffer += classes;
  }
  if (!enabled) {
    addDisabled();
  }
  TXBuffer += '\'';
  if (!enabled) {
    addDisabled();
  }
  TXBuffer += F(" href='");
}

void html_add_wide_button_prefix() {
  html_add_wide_button_prefix("", true);
}

void html_add_wide_button_prefix(const String& classes, bool enabled) {
  String wide_classes;
  wide_classes.reserve(classes.length() + 5);
  wide_classes = F("wide ");
  wide_classes += classes;
  html_add_button_prefix(wide_classes, enabled);
}

void html_add_form() {
  TXBuffer += F("<form name='frmselect' method='post'>");
}


void html_add_autosubmit_form() {
  TXBuffer += F("<script><!--\n"
           "function dept_onchange(frmselect) {frmselect.submit();}"
           "\n//--></script>");
}

void html_add_script(const String& script, bool defer) {
  html_add_script(defer);
  addHtml(script);
  html_add_script_end();
}

void html_add_script(bool defer) {
  TXBuffer += F("<script");
  if (defer) {
    TXBuffer += F(" defer");
  }
  TXBuffer += F(" type='text/JavaScript'>");
}

void html_add_script_end() {
  TXBuffer += F("</script>");
}


//********************************************************************************
// Add a task select dropdown list
//********************************************************************************
void addTaskSelect(const String& name,  int choice)
{
  String deviceName;

  TXBuffer += F("<select id='selectwidth' name='");
  TXBuffer += name;
  TXBuffer += F("' onchange='return dept_onchange(frmselect)'>");

  for (byte x = 0; x < TASKS_MAX; x++)
  {
    deviceName = "";
    if (Settings.TaskDeviceNumber[x] != 0 )
    {
      byte DeviceIndex = getDeviceIndex(Settings.TaskDeviceNumber[x]);

      if (Plugin_id[DeviceIndex] != 0)
        deviceName = getPluginNameFromDeviceIndex(DeviceIndex);
    }
    LoadTaskSettings(x);
    TXBuffer += F("<option value='");
    TXBuffer += x;
    TXBuffer += '\'';
    if (choice == x)
      TXBuffer += F(" selected");
    if (Settings.TaskDeviceNumber[x] == 0)
      addDisabled();
    TXBuffer += '>';
    TXBuffer += x + 1;
    TXBuffer += F(" - ");
    TXBuffer += deviceName;
    TXBuffer += F(" - ");
    TXBuffer += ExtraTaskSettings.TaskDeviceName;
    TXBuffer += F("</option>");
  }
}



//********************************************************************************
// Add a Value select dropdown list, based on TaskIndex
//********************************************************************************
void addTaskValueSelect(const String& name, int choice, byte TaskIndex)
{
  TXBuffer += F("<select id='selectwidth' name='");
  TXBuffer += name;
  TXBuffer += "'>";

  byte DeviceIndex = getDeviceIndex(Settings.TaskDeviceNumber[TaskIndex]);

  for (byte x = 0; x < Device[DeviceIndex].ValueCount; x++)
  {
    TXBuffer += F("<option value='");
    TXBuffer += x;
    TXBuffer += '\'';
    if (choice == x)
      TXBuffer += F(" selected");
    TXBuffer += '>';
    TXBuffer += ExtraTaskSettings.TaskDeviceValueNames[x];
    TXBuffer += F("</option>");
  }
}



//********************************************************************************
// Web Interface log page
//********************************************************************************
void handle_log() {
  if (!isLoggedIn()) return;
  navMenuIndex = MENU_INDEX_TOOLS;
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate(_HEAD);

  html_table_class_normal();
  TXBuffer += F("<TR><TH id=\"headline\" align=\"left\">Log");
  addCopyButton(F("copyText"), "", F("Copy log to clipboard"));
  TXBuffer += F("</TR></table><div  id='current_loglevel' style='font-weight: bold;'>Logging: </div><div class='logviewer' id='copyText_1'></div>");
  TXBuffer += F("Autoscroll: ");
  addCheckBox(F("autoscroll"), true);
  TXBuffer += F("<BR></body>");

  html_add_script(true);
  TXBuffer += DATA_FETCH_AND_PARSE_LOG_JS;
  html_add_script_end();

  sendHeadandTail_stdtemplate(_TAIL);
  TXBuffer.endStream();
  }

//********************************************************************************
// Web Interface JSON log page
//********************************************************************************
void handle_log_JSON() {
  if (!isLoggedIn()) return;
  TXBuffer.startJsonStream();
  String webrequest = WebServer.arg(F("view"));
  TXBuffer += F("{\"Log\": {");
  if (webrequest == F("legend")) {
    TXBuffer += F("\"Legend\": [");
    for (byte i = 0; i < LOG_LEVEL_NRELEMENTS; ++i) {
      if (i != 0)
        TXBuffer += ',';
      TXBuffer += '{';
      int loglevel;
      stream_next_json_object_value(F("label"), getLogLevelDisplayStringFromIndex(i, loglevel));
      stream_last_json_object_value(F("loglevel"), String(loglevel));
    }
    TXBuffer += F("],\n");
  }
  TXBuffer += F("\"Entries\": [");
  bool logLinesAvailable = true;
  int nrEntries = 0;
  unsigned long firstTimeStamp = 0;
  unsigned long lastTimeStamp = 0;
  while (logLinesAvailable) {
    String reply = Logging.get_logjson_formatted(logLinesAvailable, lastTimeStamp);
    if (reply.length() > 0) {
      TXBuffer += reply;
      if (nrEntries == 0) {
        firstTimeStamp = lastTimeStamp;
      }
      ++nrEntries;
    }
    // Do we need to do something here and maybe limit number of lines at once?
  }
  TXBuffer += F("],\n");
  long logTimeSpan = timeDiff(firstTimeStamp, lastTimeStamp);
  long refreshSuggestion = 1000;
  long newOptimum = 1000;
  if (nrEntries > 2 && logTimeSpan > 1) {
    // May need to lower the TTL for refresh when time needed
    // to fill half the log is lower than current TTL
    newOptimum = logTimeSpan * (LOG_STRUCT_MESSAGE_LINES / 2);
    newOptimum = newOptimum / (nrEntries - 1);
  }
  if (newOptimum < refreshSuggestion) refreshSuggestion = newOptimum;
  if (refreshSuggestion < 100) {
    // Reload times no lower than 100 msec.
    refreshSuggestion = 100;
  }
  stream_next_json_object_value(F("TTL"), String(refreshSuggestion));
  stream_next_json_object_value(F("timeHalfBuffer"), String(newOptimum));
  stream_next_json_object_value(F("nrEntries"), String(nrEntries));
  stream_next_json_object_value(F("SettingsWebLogLevel"), String(Settings.WebLogLevel));
  stream_last_json_object_value(F("logTimeSpan"), String(logTimeSpan));
  TXBuffer += F("}\n");
  TXBuffer.endStream();
  updateLogLevelCache();
}

//********************************************************************************
// Web Interface debug page
//********************************************************************************
void addWideButtonPlusDescription(const String& url, const String& buttonText, const String& description)
{
  html_TR_TD_height(30);
  addWideButton(url, buttonText);
  html_TD();
  TXBuffer += description;
}

void handle_tools() {
  if (!isLoggedIn()) return;
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

  printToWeb = true;
  printWebString = "";

  if (webrequest.length() > 0)
  {
    struct EventStruct TempEvent;
    webrequest=parseTemplate(webrequest,webrequest.length());  //@giig1967g: parseTemplate before executing the command
    parseCommandString(&TempEvent, webrequest);
    TempEvent.Source = VALUE_SOURCE_WEB_FRONTEND;
    if (!PluginCall(PLUGIN_WRITE, &TempEvent, webrequest))
      ExecuteCommand(VALUE_SOURCE_WEB_FRONTEND, webrequest.c_str());
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
  addWideButtonPlusDescription(F("timingstats"),  F("Timing stats"),     F("Open timing statistics of system"));
  addWideButtonPlusDescription(F("pinstates"),    F("Pin state buffer"), F("Show Pin state buffer"));
  addWideButtonPlusDescription(F("sysvars"),      F("System Variables"), F("Show all system variables and conversions"));

  addFormSubHeader(F("Wifi"));

  addWideButtonPlusDescription(F("/?cmd=wificonnect"),    F("Connect"),    F("Connects to known Wifi network"));
  addWideButtonPlusDescription(F("/?cmd=wifidisconnect"), F("Disconnect"), F("Disconnect from wifi network"));
  addWideButtonPlusDescription(F("wifiscanner"),          F("Scan"),       F("Scan for wifi networks"));

  addFormSubHeader(F("Interfaces"));

  addWideButtonPlusDescription(F("i2cscanner"), F("I2C Scan"), F("Scan for I2C devices"));

  addFormSubHeader(F("Settings"));

  addWideButtonPlusDescription(F("upload"),   F("Load"), F("Loads a settings file"));
  addFormNote(F("(File MUST be renamed to \"config.dat\" before upload!)"));
  addWideButtonPlusDescription(F("download"), F("Save"), F("Saves a settings file"));

#if defined(ESP8266)
  {
    {
      uint32_t maxSketchSize;
      bool use2step;
      bool otaEnabled = OTA_possible(maxSketchSize, use2step);
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
  }
#endif

  addFormSubHeader(F("Filesystem"));

  addWideButtonPlusDescription(F("filelist"),      F("Flash"),         F("Show files on internal flash"));
  addWideButtonPlusDescription(F("/factoryreset"), F("Factory Reset"), F("Select pre-defined configuration or full erase of settings"));
#ifdef FEATURE_SD
  addWideButtonPlusDescription(F("SDfilelist"),    F("SD Card"),       F("Show files on SD-Card"));
#endif

  html_end_table();
  html_end_form();
  sendHeadandTail_stdtemplate(_TAIL);
  TXBuffer.endStream();
  printWebString = "";
  printToWeb = false;
}


//********************************************************************************
// Web Interface pin state list
//********************************************************************************
void handle_pinstates_json() {
  checkRAM(F("handle_pinstates"));
  if (!isLoggedIn()) return;
  navMenuIndex = MENU_INDEX_TOOLS;
  TXBuffer.startJsonStream();

  bool comma_between = false;
  TXBuffer += F("[{");
  for (std::map<uint32_t,portStatusStruct>::iterator it=globalMapPortStatus.begin(); it!=globalMapPortStatus.end(); ++it)
  {
    if( comma_between ) {
      TXBuffer += ",{";
    } else {
      comma_between=true;
    }

    const uint16_t plugin = getPluginFromKey(it->first);
    const uint16_t port = getPortFromKey(it->first);

    stream_next_json_object_value(F("plugin"), String(plugin));
    stream_next_json_object_value(F("port"), String(port));
    stream_next_json_object_value(F("state"), String(it->second.state));
    stream_next_json_object_value(F("task"), String(it->second.task));
    stream_next_json_object_value(F("monitor"), String(it->second.monitor));
    stream_next_json_object_value(F("command"), String(it->second.command));
    stream_last_json_object_value(F("init"), String(it->second.init));
  }

  TXBuffer += F("]");


/*
  html_table_header(F("Plugin"), F("Official_plugin_list"), 0);
  html_table_header("GPIO");
  html_table_header("Mode");
  html_table_header(F("Value/State"));
  for (byte x = 0; x < PINSTATE_TABLE_MAX; x++)
    if (pinStates[x].plugin != 0)
    {
      html_TR_TD(); TXBuffer += "P";
      if (pinStates[x].plugin < 100)
      {
        TXBuffer += '0';
      }
      if (pinStates[x].plugin < 10)
      {
        TXBuffer += '0';
      }
      TXBuffer += pinStates[x].plugin;
      html_TD();
      TXBuffer += pinStates[x].index;
      html_TD();
      byte mode = pinStates[x].mode;
      TXBuffer += getPinModeString(mode);
      html_TD();
      TXBuffer += pinStates[x].value;
    }
*/

    TXBuffer.endStream();
}

void handle_pinstates() {
  checkRAM(F("handle_pinstates"));
  if (!isLoggedIn()) return;
  navMenuIndex = MENU_INDEX_TOOLS;
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate(_HEAD);

  //addFormSubHeader(F("Pin state table<TR>"));

  html_table_class_multirow();
  html_TR();
  html_table_header(F("Plugin"), F("Official_plugin_list"), 0);
  html_table_header("GPIO");
  html_table_header("Mode");
  html_table_header(F("Value/State"));
  html_table_header(F("Task"));
  html_table_header(F("Monitor"));
  html_table_header(F("Command"));
  html_table_header("Init");
  for (std::map<uint32_t,portStatusStruct>::iterator it=globalMapPortStatus.begin(); it!=globalMapPortStatus.end(); ++it)
  {
    html_TR_TD(); TXBuffer += "P";
    const uint16_t plugin = getPluginFromKey(it->first);
    const uint16_t port = getPortFromKey(it->first);

    if (plugin < 100)
    {
      TXBuffer += '0';
    }
    if (plugin < 10)
    {
      TXBuffer += '0';
    }
    TXBuffer += plugin;
    html_TD();
    TXBuffer += port;
    html_TD();
    TXBuffer += getPinModeString(it->second.mode);
    html_TD();
    TXBuffer += it->second.state;
    html_TD();
    TXBuffer += it->second.task;
    html_TD();
    TXBuffer += it->second.monitor;
    html_TD();
    TXBuffer += it->second.command;
    html_TD();
    TXBuffer += it->second.init;
  }


/*
  html_table_header(F("Plugin"), F("Official_plugin_list"), 0);
  html_table_header("GPIO");
  html_table_header("Mode");
  html_table_header(F("Value/State"));
  for (byte x = 0; x < PINSTATE_TABLE_MAX; x++)
    if (pinStates[x].plugin != 0)
    {
      html_TR_TD(); TXBuffer += "P";
      if (pinStates[x].plugin < 100)
      {
        TXBuffer += '0';
      }
      if (pinStates[x].plugin < 10)
      {
        TXBuffer += '0';
      }
      TXBuffer += pinStates[x].plugin;
      html_TD();
      TXBuffer += pinStates[x].index;
      html_TD();
      byte mode = pinStates[x].mode;
      TXBuffer += getPinModeString(mode);
      html_TD();
      TXBuffer += pinStates[x].value;
    }
*/
    html_end_table();
    sendHeadandTail_stdtemplate(_TAIL);
    TXBuffer.endStream();
}


//********************************************************************************
// Web Interface I2C scanner
//********************************************************************************
void handle_i2cscanner_json() {
  checkRAM(F("handle_i2cscanner"));
  if (!isLoggedIn()) return;
  navMenuIndex = MENU_INDEX_TOOLS;
  TXBuffer.startJsonStream();
  TXBuffer += "[{";

  char *TempString = (char*)malloc(80);
  bool firstentry = true;
  byte error, address;
  for (address = 1; address <= 127; address++ )
  {
    if (firstentry) {
      firstentry = false;
    } else {
      TXBuffer += ",{";
    }

    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    stream_next_json_object_value(F("addr"), String(formatToHex(address)));
    stream_last_json_object_value(F("status"), String(error));
  }
  TXBuffer += "]";
  TXBuffer.endStream();
  free(TempString);
}

void handle_i2cscanner() {
  checkRAM(F("handle_i2cscanner"));
  if (!isLoggedIn()) return;
  navMenuIndex = MENU_INDEX_TOOLS;
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate(_HEAD);

  char *TempString = (char*)malloc(80);

  html_table_class_multirow();
  html_table_header(F("I2C Addresses in use"));
  html_table_header(F("Supported devices"));

  byte error, address;
  int nDevices;
  nDevices = 0;
  for (address = 1; address <= 127; address++ )
  {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0)
    {
      TXBuffer += "<TR><TD>";
      TXBuffer += formatToHex(address);
      TXBuffer += "<TD>";
      switch (address)
      {
        case 0x20:
        case 0x21:
        case 0x22:
        case 0x25:
        case 0x26:
        case 0x27:
          TXBuffer += F("PCF8574<BR>MCP23017<BR>LCD");
          break;
        case 0x23:
          TXBuffer += F("PCF8574<BR>MCP23017<BR>LCD<BR>BH1750");
          break;
        case 0x24:
          TXBuffer += F("PCF8574<BR>MCP23017<BR>LCD<BR>PN532");
          break;
        case 0x29:
          TXBuffer += F("TSL2561");
          break;
        case 0x38:
        case 0x3A:
        case 0x3B:
        case 0x3E:
        case 0x3F:
          TXBuffer += F("PCF8574A");
          break;
        case 0x39:
          TXBuffer += F("PCF8574A<BR>TSL2561<BR>APDS9960");
          break;
        case 0x3C:
        case 0x3D:
          TXBuffer += F("PCF8574A<BR>OLED");
          break;
        case 0x40:
          TXBuffer += F("SI7021<BR>HTU21D<BR>INA219<BR>PCA9685");
          break;
        case 0x41:
        case 0x42:
        case 0x43:
          TXBuffer += F("INA219");
          break;
        case 0x44:
        case 0x45:
          TXBuffer += F("SHT30/31/35");
          break;
        case 0x48:
        case 0x4A:
        case 0x4B:
          TXBuffer += F("PCF8591<BR>ADS1115<BR>LM75A");
          break;
        case 0x49:
          TXBuffer += F("PCF8591<BR>ADS1115<BR>TSL2561<BR>LM75A");
          break;
        case 0x4C:
        case 0x4E:
        case 0x4F:
          TXBuffer += F("PCF8591<BR>LM75A");
          break;
        case 0x4D:
          TXBuffer += F("PCF8591<BR>MCP3221<BR>LM75A");
          break;
        case 0x5A:
          TXBuffer += F("MLX90614<BR>MPR121");
          break;
        case 0x5B:
          TXBuffer += F("MPR121");
          break;
        case 0x5C:
          TXBuffer += F("DHT12<BR>AM2320<BR>BH1750<BR>MPR121");
          break;
        case 0x5D:
          TXBuffer += F("MPR121");
          break;
        case 0x60:
          TXBuffer += F("Adafruit Motorshield v2<BR>SI1145");
          break;
        case 0x70:
          TXBuffer += F("Adafruit Motorshield v2 (Catchall)<BR>HT16K33");
          break;
        case 0x71:
        case 0x72:
        case 0x73:
        case 0x74:
        case 0x75:
          TXBuffer += F("HT16K33");
          break;
        case 0x76:
          TXBuffer += F("BME280<BR>BMP280<BR>MS5607<BR>MS5611<BR>HT16K33");
          break;
        case 0x77:
          TXBuffer += F("BMP085<BR>BMP180<BR>BME280<BR>BMP280<BR>MS5607<BR>MS5611<BR>HT16K33");
          break;
        case 0x7f:
          TXBuffer += F("Arduino PME");
          break;
      }
      nDevices++;
    }
    else if (error == 4)
    {
      html_TR_TD(); TXBuffer += F("Unknown error at address ");
      TXBuffer += formatToHex(address);
    }
  }

  if (nDevices == 0)
    TXBuffer += F("<TR>No I2C devices found");

  html_end_table();
  sendHeadandTail_stdtemplate(_TAIL);
  TXBuffer.endStream();
  free(TempString);
}


//********************************************************************************
// Web Interface Wifi scanner
//********************************************************************************
void handle_wifiscanner_json() {
  checkRAM(F("handle_wifiscanner"));
  if (!isLoggedIn()) return;
  navMenuIndex = MENU_INDEX_TOOLS;
  TXBuffer.startJsonStream();
  TXBuffer += "[{";
  bool firstentry = true;
  int n = WiFi.scanNetworks(false, true);
  for (int i = 0; i < n; ++i)
  {
    if (firstentry) firstentry = false;
    else TXBuffer += ",{";

    stream_next_json_object_value(F("ssid"), WiFi.SSID(i));
    stream_next_json_object_value(F("bssid"), WiFi.BSSIDstr(i));
    stream_next_json_object_value(F("channel"), String(WiFi.channel(i)));
    stream_next_json_object_value(F("rssi"), String(WiFi.RSSI(i)));
    switch (WiFi.encryptionType(i)) {
    #ifdef ESP32
      case WIFI_AUTH_OPEN: stream_last_json_object_value(F("auth"), F("open")); break;
      case WIFI_AUTH_WEP:  stream_last_json_object_value(F("auth"), F("WEP")); break;
      case WIFI_AUTH_WPA_PSK: stream_last_json_object_value(F("auth"), F("WPA/PSK")); break;
      case WIFI_AUTH_WPA2_PSK: stream_last_json_object_value(F("auth"), F("WPA2/PSK")); break;
      case WIFI_AUTH_WPA_WPA2_PSK: stream_last_json_object_value(F("auth"), F("WPA/WPA2/PSK")); break;
      case WIFI_AUTH_WPA2_ENTERPRISE: stream_last_json_object_value(F("auth"), F("WPA2 Enterprise")); break;
    #else
      case ENC_TYPE_WEP: stream_last_json_object_value(F("auth"), F("WEP")); break;
      case ENC_TYPE_TKIP: stream_last_json_object_value(F("auth"), F("WPA/PSK")); break;
      case ENC_TYPE_CCMP: stream_last_json_object_value(F("auth"), F("WPA2/PSK")); break;
      case ENC_TYPE_NONE: stream_last_json_object_value(F("auth"), F("open")); break;
      case ENC_TYPE_AUTO: stream_last_json_object_value(F("auth"), F("WPA/WPA2/PSK")); break;
    #endif
      default:
        break;
    }
  }
  TXBuffer += "]";
  TXBuffer.endStream();
}

void handle_wifiscanner() {
  checkRAM(F("handle_wifiscanner"));
  if (!isLoggedIn()) return;
  navMenuIndex = MENU_INDEX_TOOLS;
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate(_HEAD);
  html_table_class_multirow();
  html_TR();
  html_table_header("SSID");
  html_table_header(F("BSSID"));
  html_table_header("info");

  int n = WiFi.scanNetworks(false, true);
  if (n == 0)
    TXBuffer += F("No Access Points found");
  else
  {
    for (int i = 0; i < n; ++i)
    {
      html_TR_TD();
      TXBuffer += formatScanResult(i, "<TD>");
    }
  }

  html_end_table();
  sendHeadandTail_stdtemplate(_TAIL);
  TXBuffer.endStream();
}


//********************************************************************************
// Web Interface login page
//********************************************************************************
void handle_login() {
  checkRAM(F("handle_login"));
  if (!clientIPallowed()) return;
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate(_HEAD);

  String webrequest = WebServer.arg(F("password"));
  TXBuffer += F("<form method='post'>");
  html_table_class_normal();
  TXBuffer += F("<TR><TD>Password<TD>");
  TXBuffer += F("<input class='wide' type='password' name='password' value='");
  TXBuffer += webrequest;
  TXBuffer += "'>";
  html_TR_TD();
  html_TD();
  addSubmitButton();
  html_TR_TD();
  html_end_table();
  html_end_form();

  if (webrequest.length() != 0)
  {
    char command[80];
    command[0] = 0;
    webrequest.toCharArray(command, 80);

    // compare with stored password and set timer if there's a match
    if ((strcasecmp(command, SecuritySettings.Password) == 0) || (SecuritySettings.Password[0] == 0))
    {
      WebLoggedIn = true;
      WebLoggedInTimer = 0;
      TXBuffer = F("<script>window.location = '.'</script>");
    }
    else
    {
      TXBuffer += F("Invalid password!");
      if (Settings.UseRules)
      {
        String event = F("Login#Failed");
        rulesProcessing(event);
      }
    }
  }

  sendHeadandTail_stdtemplate(_TAIL);
  TXBuffer.endStream();
  printWebString = "";
  printToWeb = false;
}


//********************************************************************************
// Web Interface control page (no password!)
//********************************************************************************
void handle_control() {
  checkRAM(F("handle_control"));
  if (!clientIPallowed()) return;
  //TXBuffer.startStream(true); // true= json
  // sendHeadandTail_stdtemplate(_HEAD);
  String webrequest = WebServer.arg(F("cmd"));

  // in case of event, store to buffer and return...
  String command = parseString(webrequest, 1);
  addLog(LOG_LEVEL_INFO,String(F("HTTP: ")) + webrequest);
  webrequest=parseTemplate(webrequest,webrequest.length());
#ifndef BUILD_NO_DEBUG
  addLog(LOG_LEVEL_DEBUG,String(F("HTTP after parseTemplate: ")) + webrequest);
#endif

  bool handledCmd = false;
  if (command == F("event"))
  {
    eventBuffer = webrequest.substring(6);
    handledCmd = true;
  }
  else if (command.equalsIgnoreCase(F("taskrun")) ||
           command.equalsIgnoreCase(F("taskvalueset")) ||
           command.equalsIgnoreCase(F("taskvaluetoggle")) ||
           command.equalsIgnoreCase(F("let")) ||
           command.equalsIgnoreCase(F("logPortStatus")) ||
           command.equalsIgnoreCase(F("jsonportstatus")) ||
           command.equalsIgnoreCase(F("rules"))) {
    ExecuteCommand(VALUE_SOURCE_HTTP,webrequest.c_str());
    handledCmd = true;
  }

  if (handledCmd) {
    TXBuffer.startStream("*");
    TXBuffer += "OK";
    TXBuffer.endStream();
	  return;
  }

  struct EventStruct TempEvent;
  parseCommandString(&TempEvent, webrequest);
  TempEvent.Source = VALUE_SOURCE_HTTP;

  printToWeb = true;
  printWebString = "";

  bool unknownCmd = false;
  if (PluginCall(PLUGIN_WRITE, &TempEvent, webrequest));
  else if (remoteConfig(&TempEvent, webrequest));
  else unknownCmd = true;

  if (printToWebJSON) // it is setted in PLUGIN_WRITE (SendStatus)
    TXBuffer.startJsonStream();
  else
    TXBuffer.startStream();

  if (unknownCmd)
	TXBuffer += F("Unknown or restricted command!");
  else
	TXBuffer += printWebString;

  TXBuffer.endStream();

  printWebString = "";
  printToWeb = false;
  printToWebJSON = false;
}

/*********************************************************************************************\
   Streaming versions directly to TXBuffer
  \*********************************************************************************************/

void stream_to_json_object_value(const String& object, const String& value) {
  TXBuffer += '\"';
  TXBuffer += object;
  TXBuffer += "\":";
  if (value.length() == 0 || !isFloat(value)) {
    TXBuffer += '\"';
    TXBuffer += value;
    TXBuffer += '\"';
  } else {
    TXBuffer += value;
  }
}

String jsonBool(bool value) {
  return toString(value);
}

// Add JSON formatted data directly to the TXbuffer, including a trailing comma.
void stream_next_json_object_value(const String& object, const String& value) {
  TXBuffer += to_json_object_value(object, value);
  TXBuffer += ",\n";
}

// Add JSON formatted data directly to the TXbuffer, including a closing '}'
void stream_last_json_object_value(const String& object, const String& value) {
  TXBuffer += to_json_object_value(object, value);
  TXBuffer += "\n}";
}


//********************************************************************************
// Web Interface JSON page (no password!)
//********************************************************************************
void handle_json()
{
  const int taskNr = getFormItemInt(F("tasknr"), -1);
  const bool showSpecificTask = taskNr > 0;
  bool showSystem = true;
  bool showWifi = true;
  bool showDataAcquisition = true;
  bool showTaskDetails = true;
  bool showNodes = true;
  {
    String view = WebServer.arg("view");
    if (view.length() != 0) {
      if (view == F("sensorupdate")) {
        showSystem = false;
        showWifi = false;
        showDataAcquisition = false;
        showTaskDetails = false;
        showNodes =false;
      }
    }
  }
  TXBuffer.startJsonStream();
  if (!showSpecificTask)
  {
    TXBuffer += '{';
    if (showSystem) {
      TXBuffer += F("\"System\":{\n");
      stream_next_json_object_value(F("Build"), String(BUILD));
      stream_next_json_object_value(F("Git Build"), String(BUILD_GIT));
      stream_next_json_object_value(F("System libraries"), getSystemLibraryString());
      stream_next_json_object_value(F("Plugins"), String(deviceCount + 1));
      stream_next_json_object_value(F("Plugin description"), getPluginDescriptionString());
      stream_next_json_object_value(F("Local time"), getDateTimeString('-',':',' '));
      stream_next_json_object_value(F("Unit"), String(Settings.Unit));
      stream_next_json_object_value(F("Name"), String(Settings.Name));
      stream_next_json_object_value(F("Uptime"), String(wdcounter / 2));
      stream_next_json_object_value(F("Last boot cause"), getLastBootCauseString());
      stream_next_json_object_value(F("Reset Reason"), getResetReasonString());

      if (wdcounter > 0)
      {
          stream_next_json_object_value(F("Load"), String(getCPUload()));
          stream_next_json_object_value(F("Load LC"), String(getLoopCountPerSec()));
      }

      stream_last_json_object_value(F("Free RAM"), String(ESP.getFreeHeap()));
      TXBuffer += ",\n";
    }
    if (showWifi) {
      TXBuffer += F("\"WiFi\":{\n");
      #if defined(ESP8266)
        stream_next_json_object_value(F("Hostname"), WiFi.hostname());
      #endif
      stream_next_json_object_value(F("IP config"), useStaticIP() ? F("Static") : F("DHCP"));
      stream_next_json_object_value(F("IP"), WiFi.localIP().toString());
      stream_next_json_object_value(F("Subnet Mask"), WiFi.subnetMask().toString());
      stream_next_json_object_value(F("Gateway IP"), WiFi.gatewayIP().toString());
      stream_next_json_object_value(F("MAC address"), WiFi.macAddress());
      stream_next_json_object_value(F("DNS 1"), WiFi.dnsIP(0).toString());
      stream_next_json_object_value(F("DNS 2"), WiFi.dnsIP(1).toString());
      stream_next_json_object_value(F("SSID"), WiFi.SSID());
      stream_next_json_object_value(F("BSSID"), WiFi.BSSIDstr());
      stream_next_json_object_value(F("Channel"), String(WiFi.channel()));
      stream_next_json_object_value(F("Connected msec"), String(timeDiff(lastConnectMoment, millis())));
      stream_next_json_object_value(F("Last Disconnect Reason"), String(lastDisconnectReason));
      stream_next_json_object_value(F("Last Disconnect Reason str"), getLastDisconnectReason());
      stream_next_json_object_value(F("Number reconnects"), String(wifi_reconnects));
      stream_last_json_object_value(F("RSSI"), String(WiFi.RSSI()));
      TXBuffer += ",\n";
    }
    if(showNodes) {
      bool comma_between=false;
      for (NodesMap::iterator it = Nodes.begin(); it != Nodes.end(); ++it)
      {
        if (it->second.ip[0] != 0)
        {
          if( comma_between ) {
            TXBuffer += ',';
          } else {
            comma_between=true;
            TXBuffer += F("\"nodes\":[\n"); // open json array if >0 nodes
          }

          TXBuffer += '{';
          stream_next_json_object_value(F("nr"), String(it->first));
          stream_next_json_object_value(F("name"),
              (it->first != Settings.Unit) ? it->second.nodeName : Settings.Name);

          if (it->second.build) {
            stream_next_json_object_value(F("build"), String(it->second.build));
          }

          if (it->second.nodeType) {
            String platform = getNodeTypeDisplayString(it->second.nodeType);
            if (platform.length() > 0)
              stream_next_json_object_value(F("platform"), platform);
          }
          stream_next_json_object_value(F("ip"), it->second.ip.toString());
          stream_last_json_object_value(F("age"),  String( it->second.age ));
        } // if node info exists
      } // for loop
      if(comma_between) {
        TXBuffer += F("],\n"); // close array if >0 nodes
      }
    }
  }

  byte firstTaskIndex = 0;
  byte lastTaskIndex = TASKS_MAX - 1;
  if (showSpecificTask)
  {
    firstTaskIndex = taskNr - 1;
    lastTaskIndex = taskNr - 1;
  }
  byte lastActiveTaskIndex = 0;
  for (byte TaskIndex = firstTaskIndex; TaskIndex <= lastTaskIndex; TaskIndex++) {
    if (Settings.TaskDeviceNumber[TaskIndex])
      lastActiveTaskIndex = TaskIndex;
  }

  if (!showSpecificTask) TXBuffer += F("\"Sensors\":[\n");
  unsigned long ttl_json = 60; // The shortest interval per enabled task (with output values) in seconds
  for (byte TaskIndex = firstTaskIndex; TaskIndex <= lastActiveTaskIndex; TaskIndex++)
  {
    if (Settings.TaskDeviceNumber[TaskIndex])
    {
      byte DeviceIndex = getDeviceIndex(Settings.TaskDeviceNumber[TaskIndex]);
      const unsigned long taskInterval = Settings.TaskDeviceTimer[TaskIndex];
      LoadTaskSettings(TaskIndex);
      TXBuffer += F("{\n");
      // For simplicity, do the optional values first.
      if (Device[DeviceIndex].ValueCount != 0) {
        if (ttl_json > taskInterval && taskInterval > 0 && Settings.TaskDeviceEnabled[TaskIndex]) {
          ttl_json = taskInterval;
        }
        TXBuffer += F("\"TaskValues\": [\n");
        for (byte x = 0; x < Device[DeviceIndex].ValueCount; x++)
        {
          TXBuffer += '{';
          stream_next_json_object_value(F("ValueNumber"), String(x + 1));
          stream_next_json_object_value(F("Name"), String(ExtraTaskSettings.TaskDeviceValueNames[x]));
          stream_next_json_object_value(F("NrDecimals"), String(ExtraTaskSettings.TaskDeviceValueDecimals[x]));
          stream_last_json_object_value(F("Value"), formatUserVarNoCheck(TaskIndex, x));
          if (x < (Device[DeviceIndex].ValueCount - 1))
            TXBuffer += ",\n";
        }
        TXBuffer += F("],\n");
      }
      if (showSpecificTask) {
        stream_next_json_object_value(F("TTL"), String(ttl_json * 1000));
      }
      if (showDataAcquisition) {
        TXBuffer += F("\"DataAcquisition\": [\n");
        for (byte x = 0; x < CONTROLLER_MAX; x++)
        {
          TXBuffer += '{';
          stream_next_json_object_value(F("Controller"), String(x + 1));
          stream_next_json_object_value(F("IDX"), String(Settings.TaskDeviceID[x][TaskIndex]));
          stream_last_json_object_value(F("Enabled"), jsonBool(Settings.TaskDeviceSendData[x][TaskIndex]));
          if (x < (CONTROLLER_MAX - 1))
            TXBuffer += ",\n";
        }
        TXBuffer += F("],\n");
      }
      if (showTaskDetails) {
        stream_next_json_object_value(F("TaskInterval"), String(taskInterval));
        stream_next_json_object_value(F("Type"), getPluginNameFromDeviceIndex(DeviceIndex));
        stream_next_json_object_value(F("TaskName"), String(ExtraTaskSettings.TaskDeviceName));
      }
      stream_next_json_object_value(F("TaskEnabled"), jsonBool(Settings.TaskDeviceEnabled[TaskIndex]));
      stream_last_json_object_value(F("TaskNumber"), String(TaskIndex + 1));
      if (TaskIndex != lastActiveTaskIndex)
        TXBuffer += ',';
      TXBuffer += '\n';
    }
  }
  if (!showSpecificTask) {
    TXBuffer += F("],\n");
    stream_last_json_object_value(F("TTL"), String(ttl_json * 1000));
  }

  TXBuffer.endStream();
}

//********************************************************************************
// JSON formatted timing statistics
//********************************************************************************

void stream_timing_stats_json(unsigned long count, unsigned long minVal, unsigned long maxVal, float avg) {
  stream_next_json_object_value(F("count"), String(count));
  stream_next_json_object_value(F("min"), String(minVal));
  stream_next_json_object_value(F("max"), String(maxVal));
  stream_next_json_object_value(F("avg"), String(avg));
}

void stream_plugin_function_timing_stats_json(
      const String& object,
      unsigned long count, unsigned long minVal, unsigned long maxVal, float avg) {
  TXBuffer += "{\"";
  TXBuffer += object;
  TXBuffer += "\":{";
  stream_timing_stats_json(count, minVal, maxVal, avg);
  stream_last_json_object_value(F("unit"), F("usec"));
}

void stream_plugin_timing_stats_json(int pluginId) {
  String P_name = "";
  Plugin_ptr[pluginId](PLUGIN_GET_DEVICENAME, NULL, P_name);
  TXBuffer += '{';
  stream_next_json_object_value(F("name"), P_name);
  stream_next_json_object_value(F("id"), String(pluginId));
  stream_json_start_array(F("function"));
}

void stream_json_start_array(const String& label) {
  TXBuffer += '\"';
  TXBuffer += label;
  TXBuffer += F("\": [\n");
}

void stream_json_end_array_element(bool isLast) {
  if (isLast) {
    TXBuffer += "]\n";
  } else {
    TXBuffer += ",\n";
  }
}

void stream_json_end_object_element(bool isLast) {
  TXBuffer += '}';
  if (!isLast) {
    TXBuffer += ',';
  }
  TXBuffer += '\n';
}


void handle_timingstats_json() {
  TXBuffer.startJsonStream();
  TXBuffer += '{';
  jsonStatistics(false);
  TXBuffer += '}';
  TXBuffer.endStream();
}

//********************************************************************************
// HTML table formatted timing statistics
//********************************************************************************
void format_using_threshhold(unsigned long value) {
  float value_msec = value / 1000.0;
  if (value > TIMING_STATS_THRESHOLD) {
    html_B(String(value_msec, 3));
  } else {
    TXBuffer += String(value_msec, 3);
  }
}

void stream_html_timing_stats(const TimingStats& stats, long timeSinceLastReset) {
    unsigned long minVal, maxVal;
    unsigned int c = stats.getMinMax(minVal, maxVal);

    html_TD();
    TXBuffer += c;
    html_TD();
    float call_per_sec = static_cast<float>(c) / static_cast<float>(timeSinceLastReset) * 1000.0;
    TXBuffer += call_per_sec;
    html_TD();
    format_using_threshhold(minVal);
    html_TD();
    format_using_threshhold(stats.getAvg());
    html_TD();
    format_using_threshhold(maxVal);
}



long stream_timing_statistics(bool clearStats) {
  long timeSinceLastReset = timePassedSince(timingstats_last_reset);
  for (auto& x: pluginStats) {
      if (!x.second.isEmpty()) {
          const int pluginId = x.first/256;
          String P_name = "";
          Plugin_ptr[pluginId](PLUGIN_GET_DEVICENAME, NULL, P_name);
          if (x.second.thresholdExceeded(TIMING_STATS_THRESHOLD)) {
            html_TR_TD_highlight();
          } else {
            html_TR_TD();
          }
          TXBuffer += F("P_");
          TXBuffer += Device[pluginId].Number;
          TXBuffer += '_';
          TXBuffer += P_name;
          html_TD();
          TXBuffer += getPluginFunctionName(x.first%256);
          stream_html_timing_stats(x.second, timeSinceLastReset);
          if (clearStats) x.second.reset();
      }
  }
  for (auto& x: controllerStats) {
      if (!x.second.isEmpty()) {
          const int pluginId = x.first/256;
          String C_name = "";
          CPluginCall(pluginId, CPLUGIN_GET_DEVICENAME, NULL, C_name);
          if (x.second.thresholdExceeded(TIMING_STATS_THRESHOLD)) {
            html_TR_TD_highlight();
          } else {
            html_TR_TD();
          }
          TXBuffer += F("C_");
          TXBuffer += Protocol[pluginId].Number;
          TXBuffer += '_';
          TXBuffer += C_name;
          html_TD();
          TXBuffer += getCPluginCFunctionName(x.first%256);
          stream_html_timing_stats(x.second, timeSinceLastReset);
          if (clearStats) x.second.reset();
      }
  }
  for (auto& x: miscStats) {
      if (!x.second.isEmpty()) {
          if (x.second.thresholdExceeded(TIMING_STATS_THRESHOLD)) {
            html_TR_TD_highlight();
          } else {
            html_TR_TD();
          }
          TXBuffer += getMiscStatsName(x.first);
          html_TD();
          stream_html_timing_stats(x.second, timeSinceLastReset);
          if (clearStats) x.second.reset();
      }
  }
  if (clearStats) {
    timediff_calls = 0;
    timediff_cpu_cycles_total = 0;
    timingstats_last_reset = millis();
  }
  return timeSinceLastReset;
}

void handle_timingstats() {
  checkRAM(F("handle_timingstats"));
  navMenuIndex = MENU_INDEX_TOOLS;
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate(_HEAD);
  html_table_class_multirow();
  html_TR();
  html_table_header(F("Description"));
  html_table_header(F("Function"));
  html_table_header(F("#calls"));
  html_table_header(F("call/sec"));
  html_table_header(F("min (ms)"));
  html_table_header(F("Avg (ms)"));
  html_table_header(F("max (ms)"));

  long timeSinceLastReset = stream_timing_statistics(true);
  html_end_table();

  html_table_class_normal();
  const float timespan = timeSinceLastReset / 1000.0;
  addFormHeader(F("Statistics"));
  addRowLabel(F("Start Period"));
  struct tm startPeriod = addSeconds(tm, -1.0 * timespan, false);
  TXBuffer += getDateTimeString(startPeriod, '-', ':', ' ', false);
  addRowLabel(F("Local Time"));
  TXBuffer += getDateTimeString('-', ':', ' ');
  addRowLabel(F("Time span"));
  TXBuffer += String(timespan);
  TXBuffer += " sec";
  html_end_table();

  sendHeadandTail_stdtemplate(_TAIL);
  TXBuffer.endStream();
}

//********************************************************************************
// Web Interface config page
//********************************************************************************
void handle_advanced() {
  checkRAM(F("handle_advanced"));
  if (!isLoggedIn()) return;
  navMenuIndex = MENU_INDEX_TOOLS;
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate();

  int timezone = getFormItemInt(F("timezone"));
  int dststartweek = getFormItemInt(F("dststartweek"));
  int dststartdow = getFormItemInt(F("dststartdow"));
  int dststartmonth = getFormItemInt(F("dststartmonth"));
  int dststarthour = getFormItemInt(F("dststarthour"));
  int dstendweek = getFormItemInt(F("dstendweek"));
  int dstenddow = getFormItemInt(F("dstenddow"));
  int dstendmonth = getFormItemInt(F("dstendmonth"));
  int dstendhour = getFormItemInt(F("dstendhour"));
  String edit = WebServer.arg(F("edit"));


  if (edit.length() != 0)
  {
    Settings.MessageDelay = getFormItemInt(F("messagedelay"));
    Settings.IP_Octet = WebServer.arg(F("ip")).toInt();
    strncpy_webserver_arg(Settings.NTPHost, F("ntphost"));
    Settings.TimeZone = timezone;
    TimeChangeRule dst_start(dststartweek, dststartdow, dststartmonth, dststarthour, timezone);
    if (dst_start.isValid()) { Settings.DST_Start = dst_start.toFlashStoredValue(); }
    TimeChangeRule dst_end(dstendweek, dstenddow, dstendmonth, dstendhour, timezone);
    if (dst_end.isValid()) { Settings.DST_End = dst_end.toFlashStoredValue(); }
    str2ip(WebServer.arg(F("syslogip")).c_str(), Settings.Syslog_IP);
    Settings.UDPPort = getFormItemInt(F("udpport"));

    Settings.SyslogFacility = getFormItemInt(F("syslogfacility"));
    Settings.UseSerial = isFormItemChecked(F("useserial"));
    setLogLevelFor(LOG_TO_SYSLOG, getFormItemInt(F("sysloglevel")));
    setLogLevelFor(LOG_TO_SERIAL, getFormItemInt(F("serialloglevel")));
    setLogLevelFor(LOG_TO_WEBLOG, getFormItemInt(F("webloglevel")));
    setLogLevelFor(LOG_TO_SDCARD, getFormItemInt(F("sdloglevel")));
    Settings.UseValueLogger = isFormItemChecked(F("valuelogger"));
    Settings.BaudRate = getFormItemInt(F("baudrate"));
    Settings.UseNTP = isFormItemChecked(F("usentp"));
    Settings.DST = isFormItemChecked(F("dst"));
    Settings.WDI2CAddress = getFormItemInt(F("wdi2caddress"));
    Settings.UseSSDP = isFormItemChecked(F("usessdp"));
    Settings.WireClockStretchLimit = getFormItemInt(F("wireclockstretchlimit"));
    Settings.UseRules = isFormItemChecked(F("userules"));
    Settings.ConnectionFailuresThreshold = getFormItemInt(F("cft"));
    Settings.MQTTRetainFlag = isFormItemChecked(F("mqttretainflag"));
    Settings.ArduinoOTAEnable = isFormItemChecked(F("arduinootaenable"));
    Settings.UseRTOSMultitasking = isFormItemChecked(F("usertosmultitasking"));
    Settings.MQTTUseUnitNameAsClientId = isFormItemChecked(F("mqttuseunitnameasclientid"));
    Settings.uniqueMQTTclientIdReconnect(isFormItemChecked(F("uniquemqttclientidreconnect")));
    Settings.Latitude = getFormItemFloat(F("latitude"));
    Settings.Longitude = getFormItemFloat(F("longitude"));
    Settings.OldRulesEngine(isFormItemChecked(F("oldrulesengine")));
    Settings.ForceWiFi_bg_mode(isFormItemChecked(F("forcewifi_bg")));
    Settings.WiFiRestart_connection_lost(isFormItemChecked(F("wifi_restart_conn_lost")));

    addHtmlError(SaveSettings());
    if (systemTimePresent())
      initTime();
  }

  TXBuffer += F("<form  method='post'>");
  html_table_class_normal();

  addFormHeader(F("Advanced Settings"));

  addFormSubHeader(F("Rules Settings"));

  addFormCheckBox(F("Rules"), F("userules"), Settings.UseRules);
  addFormCheckBox(F("Old Engine"), F("oldrulesengine"), Settings.OldRulesEngine());

  addFormSubHeader(F("Controller Settings"));

  addFormCheckBox(F("MQTT Retain Msg"), F("mqttretainflag"), Settings.MQTTRetainFlag);
  addFormNumericBox( F("Message Interval"), F("messagedelay"), Settings.MessageDelay, 0, INT_MAX);
  addUnit(F("ms"));
  addFormCheckBox(F("MQTT use unit name as ClientId"), F("mqttuseunitnameasclientid"), Settings.MQTTUseUnitNameAsClientId);
  addFormCheckBox(F("MQTT change ClientId at reconnect"), F("uniquemqttclientidreconnect"), Settings.uniqueMQTTclientIdReconnect());

  addFormSubHeader(F("NTP Settings"));

  addFormCheckBox(F("Use NTP"), F("usentp"), Settings.UseNTP);
  addFormTextBox( F("NTP Hostname"), F("ntphost"), Settings.NTPHost, 63);

  addFormSubHeader(F("DST Settings"));
  addFormDstSelect(true, Settings.DST_Start);
  addFormDstSelect(false, Settings.DST_End);
  addFormNumericBox(F("Timezone Offset (UTC +)"), F("timezone"), Settings.TimeZone, -720, 840);   // UTC-12H ... UTC+14h
  addUnit(F("minutes"));
  addFormCheckBox(F("DST"), F("dst"), Settings.DST);

  addFormSubHeader(F("Location Settings"));
  addFormFloatNumberBox(F("Latitude"), F("latitude"), Settings.Latitude, -90.0, 90.0);
  addUnit(F("&deg;"));
  addFormFloatNumberBox(F("Longitude"), F("longitude"), Settings.Longitude, -180.0, 180.0);
  addUnit(F("&deg;"));

  addFormSubHeader(F("Log Settings"));

  addFormIPBox(F("Syslog IP"), F("syslogip"), Settings.Syslog_IP);
  addFormLogLevelSelect(F("Syslog Level"),      F("sysloglevel"),    Settings.SyslogLevel);
  addFormLogFacilitySelect(F("Syslog Facility"),F("syslogfacility"), Settings.SyslogFacility);
  addFormLogLevelSelect(F("Serial log Level"),  F("serialloglevel"), Settings.SerialLogLevel);
  addFormLogLevelSelect(F("Web log Level"),     F("webloglevel"),    Settings.WebLogLevel);

#ifdef FEATURE_SD
  addFormLogLevelSelect(F("SD Card log Level"), F("sdloglevel"),     Settings.SDLogLevel);

  addFormCheckBox(F("SD Card Value Logger"), F("valuelogger"), Settings.UseValueLogger);
#endif


  addFormSubHeader(F("Serial Settings"));

  addFormCheckBox(F("Enable Serial port"), F("useserial"), Settings.UseSerial);
  addFormNumericBox(F("Baud Rate"), F("baudrate"), Settings.BaudRate, 0, 1000000);


  addFormSubHeader(F("Inter-ESPEasy Network"));

  addFormNumericBox(F("UDP port"), F("udpport"), Settings.UDPPort, 0, 65535);


  //TODO sort settings in groups or move to other pages/groups
  addFormSubHeader(F("Special and Experimental Settings"));

  addFormNumericBox(F("Fixed IP Octet"), F("ip"), Settings.IP_Octet, 0, 255);

  addFormNumericBox(F("WD I2C Address"), F("wdi2caddress"), Settings.WDI2CAddress, 0, 127);
  TXBuffer += F(" (decimal)");

  addFormCheckBox_disabled(F("Use SSDP"), F("usessdp"), Settings.UseSSDP);

  addFormNumericBox(F("Connection Failure Threshold"), F("cft"), Settings.ConnectionFailuresThreshold, 0, 100);
#ifdef ESP8266
  addFormCheckBox(F("Force WiFi B/G"), F("forcewifi_bg"), Settings.ForceWiFi_bg_mode());
#endif
#ifdef ESP32
  // Disabled for now, since it is not working properly.
  addFormCheckBox_disabled(F("Force WiFi B/G"), F("forcewifi_bg"), Settings.ForceWiFi_bg_mode());
#endif

  addFormCheckBox(F("Restart WiFi on lost conn."), F("wifi_restart_conn_lost"), Settings.WiFiRestart_connection_lost());

  addFormNumericBox(F("I2C ClockStretchLimit"), F("wireclockstretchlimit"), Settings.WireClockStretchLimit);   //TODO define limits
  #if defined(FEATURE_ARDUINO_OTA)
  addFormCheckBox(F("Enable Arduino OTA"), F("arduinootaenable"), Settings.ArduinoOTAEnable);
  #endif
  #if defined(ESP32)
    addFormCheckBox_disabled(F("Enable RTOS Multitasking"), F("usertosmultitasking"), Settings.UseRTOSMultitasking);
  #endif

  addFormSeparator(2);

  html_TR_TD();
  html_TD();
  addSubmitButton();
  TXBuffer += F("<input type='hidden' name='edit' value='1'>");
  html_end_table();
  html_end_form();
  sendHeadandTail_stdtemplate(true);
  TXBuffer.endStream();
}

void addFormDstSelect(bool isStart, uint16_t choice) {
  String weekid  = isStart ? F("dststartweek")  : F("dstendweek");
  String dowid   = isStart ? F("dststartdow")   : F("dstenddow");
  String monthid = isStart ? F("dststartmonth") : F("dstendmonth");
  String hourid  = isStart ? F("dststarthour")  : F("dstendhour");

  String weeklabel  = isStart ? F("Start (week, dow, month)")  : F("End (week, dow, month)");
  String hourlabel  = isStart ? F("Start (localtime, e.g. 2h&rarr;3h)")  : F("End (localtime, e.g. 3h&rarr;2h)");

  String week[5] = {F("Last"), F("1st"), F("2nd"), F("3rd"), F("4th")};
  int weekValues[5] = {0, 1, 2, 3, 4};
  String dow[7] = {F("Sun"), F("Mon"), F("Tue"), F("Wed"), F("Thu"), F("Fri"), F("Sat")};
  int dowValues[7] = {1, 2, 3, 4, 5, 6, 7};
  String month[12] = {F("Jan"), F("Feb"), F("Mar"), F("Apr"), F("May"), F("Jun"), F("Jul"), F("Aug"), F("Sep"), F("Oct"), F("Nov"), F("Dec")};
  int monthValues[12] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

  uint16_t tmpstart(choice);
  uint16_t tmpend(choice);
  if (!TimeChangeRule(choice, 0).isValid()) {
    getDefaultDst_flash_values(tmpstart, tmpend);
  }
  TimeChangeRule rule(isStart ? tmpstart : tmpend, 0);
  addRowLabel(weeklabel);
  addSelector(weekid, 5, week, weekValues, NULL, rule.week, false);
  TXBuffer += F("<BR>");
  addSelector(dowid, 7, dow, dowValues, NULL, rule.dow, false);
  TXBuffer += F("<BR>");
  addSelector(monthid, 12, month, monthValues, NULL, rule.month, false);

  addFormNumericBox(hourlabel, hourid, rule.hour, 0, 23);
  addUnit(isStart ? F("hour &#x21b7;") : F("hour &#x21b6;"));
}

void addFormLogLevelSelect(const String& label, const String& id, int choice)
{
  addRowLabel(label);
  addLogLevelSelect(id, choice);
}

void addLogLevelSelect(const String& name, int choice)
{
  String options[LOG_LEVEL_NRELEMENTS + 1];
  int optionValues[LOG_LEVEL_NRELEMENTS + 1] = {0};
  options[0] = getLogLevelDisplayString(0);
  optionValues[0] = 0;
  for (int i = 0; i < LOG_LEVEL_NRELEMENTS; ++i) {
    options[i + 1] = getLogLevelDisplayStringFromIndex(i, optionValues[i + 1]);
  }
  addSelector(name, LOG_LEVEL_NRELEMENTS + 1, options, optionValues, NULL, choice, false);
}

void addFormLogFacilitySelect(const String& label, const String& id, int choice)
{
  addRowLabel(label);
  addLogFacilitySelect(id, choice);
}

void addLogFacilitySelect(const String& name, int choice)
{
  String options[12] = { F("Kernel"), F("User"), F("Daemon"), F("Message"), F("Local0"), F("Local1"), F("Local2"), F("Local3"), F("Local4"), F("Local5"), F("Local6"), F("Local7")};
  int optionValues[12] = { 0, 1, 3, 5, 16, 17, 18, 19, 20, 21, 22, 23 };
  addSelector(name, 12, options, optionValues, NULL, choice, false);
}


//********************************************************************************
// Login state check
//********************************************************************************
boolean isLoggedIn()
{
  if (!clientIPallowed()) return false;
  if (SecuritySettings.Password[0] == 0)
    WebLoggedIn = true;

  if (!WebLoggedIn)
  {
    WebServer.sendContent(F("HTTP/1.1 302 \r\nLocation: /login\r\n"));
  }
  else
  {
    WebLoggedInTimer = 0;
  }

  return WebLoggedIn;
}


//********************************************************************************
// Web Interface download page
//********************************************************************************
void handle_download()
{
  checkRAM(F("handle_download"));
  if (!isLoggedIn()) return;
  navMenuIndex = MENU_INDEX_TOOLS;
//  TXBuffer.startStream();
//  sendHeadandTail_stdtemplate();


  fs::File dataFile = SPIFFS.open(F(FILE_CONFIG), "r");
  if (!dataFile)
    return;

  String str = F("attachment; filename=config_");
  str += Settings.Name;
  str += "_U";
  str += Settings.Unit;
  str += F("_Build");
  str += BUILD;
  str += '_';
  if (systemTimePresent())
  {
    str += getDateTimeString('\0', '\0', '\0');
  }
  str += F(".dat");

  WebServer.sendHeader(F("Content-Disposition"), str);
  WebServer.streamFile(dataFile, F("application/octet-stream"));
  dataFile.close();
}


//********************************************************************************
// Web Interface upload page
//********************************************************************************
byte uploadResult = 0;
void handle_upload() {
  if (!isLoggedIn()) return;
  navMenuIndex = MENU_INDEX_TOOLS;
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate();

  TXBuffer += F("<form enctype='multipart/form-data' method='post'><p>Upload settings file:<br><input type='file' name='datafile' size='40'></p><div><input class='button link' type='submit' value='Upload'></div><input type='hidden' name='edit' value='1'></form>");
  sendHeadandTail_stdtemplate(true);
  TXBuffer.endStream();
  printWebString = "";
  printToWeb = false;
}


//********************************************************************************
// Web Interface upload page
//********************************************************************************
void handle_upload_post() {
  checkRAM(F("handle_upload_post"));
  if (!isLoggedIn()) return;

  navMenuIndex = MENU_INDEX_TOOLS;
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate();



  if (uploadResult == 1)
  {
    TXBuffer += F("Upload OK!<BR>You may need to reboot to apply all settings...");
    LoadSettings();
  }

  if (uploadResult == 2)
    TXBuffer += F("<font color=\"red\">Upload file invalid!</font>");

  if (uploadResult == 3)
    TXBuffer += F("<font color=\"red\">No filename!</font>");


  TXBuffer += F("Upload finished");
  sendHeadandTail_stdtemplate(true);
  TXBuffer.endStream();
  printWebString = "";
  printToWeb = false;
}


//********************************************************************************
// Web Interface upload handler
//********************************************************************************
fs::File uploadFile;
void handleFileUpload() {
  checkRAM(F("handleFileUpload"));
  if (!isLoggedIn()) return;

  static boolean valid = false;

  HTTPUpload& upload = WebServer.upload();

  if (upload.filename.c_str()[0] == 0)
  {
    uploadResult = 3;
    return;
  }

  if (upload.status == UPLOAD_FILE_START)
  {
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = F("Upload: START, filename: ");
      log += upload.filename;
      addLog(LOG_LEVEL_INFO, log);
    }
    valid = false;
    uploadResult = 0;
  }
  else if (upload.status == UPLOAD_FILE_WRITE)
  {
    // first data block, if this is the config file, check PID/Version
    if (upload.totalSize == 0)
    {
      if (strcasecmp(upload.filename.c_str(), FILE_CONFIG) == 0)
      {
        struct TempStruct {
          unsigned long PID;
          int Version;
        } Temp;
        for (unsigned int x = 0; x < sizeof(struct TempStruct); x++)
        {
          byte b = upload.buf[x];
          memcpy((byte*)&Temp + x, &b, 1);
        }
        if (Temp.Version == VERSION && Temp.PID == ESP_PROJECT_PID)
          valid = true;
      }
      else
      {
        // other files are always valid...
        valid = true;
      }
      if (valid)
      {
        // once we're safe, remove file and create empty one...
        SPIFFS.remove((char *)upload.filename.c_str());
        uploadFile = SPIFFS.open(upload.filename.c_str(), "w");
        // dont count manual uploads: flashCount();
      }
    }
    if (uploadFile) uploadFile.write(upload.buf, upload.currentSize);
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = F("Upload: WRITE, Bytes: ");
      log += upload.currentSize;
      addLog(LOG_LEVEL_INFO, log);
    }
  }
  else if (upload.status == UPLOAD_FILE_END)
  {
    if (uploadFile) uploadFile.close();
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = F("Upload: END, Size: ");
      log += upload.totalSize;
      addLog(LOG_LEVEL_INFO, log);
    }
  }

  if (valid)
    uploadResult = 1;
  else
    uploadResult = 2;

}


//********************************************************************************
// Web Interface server web file from SPIFFS
//********************************************************************************
bool loadFromFS(boolean spiffs, String path) {
  // path is a deepcopy, since it will be changed here.
  checkRAM(F("loadFromFS"));
  if (!isLoggedIn()) return false;

  statusLED(true);

  String dataType = F("text/plain");
  if (path.endsWith("/")) path += F("index.htm");

  if (path.endsWith(F(".src"))) path = path.substring(0, path.lastIndexOf("."));
  else if (path.endsWith(F(".htm"))) dataType = F("text/html");
  else if (path.endsWith(F(".css"))) dataType = F("text/css");
  else if (path.endsWith(F(".js"))) dataType = F("application/javascript");
  else if (path.endsWith(F(".png"))) dataType = F("image/png");
  else if (path.endsWith(F(".gif"))) dataType = F("image/gif");
  else if (path.endsWith(F(".jpg"))) dataType = F("image/jpeg");
  else if (path.endsWith(F(".ico"))) dataType = F("image/x-icon");
  else if (path.endsWith(F(".txt")) ||
           path.endsWith(F(".dat"))) dataType = F("application/octet-stream");
  else if (path.endsWith(F(".esp"))) return handle_custom(path);
#ifndef BUILD_NO_DEBUG
  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log = F("HTML : Request file ");
    log += path;
    addLog(LOG_LEVEL_DEBUG, log);
  }
#endif

  path = path.substring(1);
  if (spiffs)
  {
    fs::File dataFile = SPIFFS.open(path.c_str(), "r");
    if (!dataFile)
      return false;

    //prevent reloading stuff on every click
    WebServer.sendHeader(F("Cache-Control"), F("max-age=3600, public"));
    WebServer.sendHeader(F("Vary"),"*");
    WebServer.sendHeader(F("ETag"), F("\"2.0.0\""));

    if (path.endsWith(F(".dat")))
      WebServer.sendHeader(F("Content-Disposition"), F("attachment;"));
    WebServer.streamFile(dataFile, dataType);
    dataFile.close();
  }
  else
  {
#ifdef FEATURE_SD
    File dataFile = SD.open(path.c_str());
    if (!dataFile)
      return false;
    if (path.endsWith(F(".DAT")))
      WebServer.sendHeader(F("Content-Disposition"), F("attachment;"));
    WebServer.streamFile(dataFile, dataType);
    dataFile.close();
#endif
  }
  statusLED(true);
  return true;
}

//********************************************************************************
// Web Interface custom page handler
//********************************************************************************
boolean handle_custom(String path) {
  // path is a deepcopy, since it will be changed.
  checkRAM(F("handle_custom"));
  if (!clientIPallowed()) return false;
  path = path.substring(1);

  // create a dynamic custom page, parsing task values into [<taskname>#<taskvalue>] placeholders and parsing %xx% system variables
  fs::File dataFile = SPIFFS.open(path.c_str(), "r");
  const bool dashboardPage = path.startsWith(F("dashboard"));
  if (!dataFile && !dashboardPage) {
    return false; // unknown file that does not exist...
  }

  if (dashboardPage) // for the dashboard page, create a default unit dropdown selector
  {
    // handle page redirects to other unit's as requested by the unit dropdown selector
    byte unit = getFormItemInt(F("unit"));
    byte btnunit = getFormItemInt(F("btnunit"));
    if(!unit) unit = btnunit; // unit element prevails, if not used then set to btnunit
    if (unit && unit != Settings.Unit)
    {
      NodesMap::iterator it = Nodes.find(unit);
      if (it != Nodes.end()) {
        TXBuffer.startStream();
        sendHeadandTail(F("TmplDsh"),_HEAD);
        TXBuffer += F("<meta http-equiv=\"refresh\" content=\"0; URL=http://");
        TXBuffer += it->second.ip.toString();
        TXBuffer += F("/dashboard.esp\">");
        sendHeadandTail(F("TmplDsh"),_TAIL);
        TXBuffer.endStream();
        return true;
      }
    }

    TXBuffer.startStream();
    sendHeadandTail(F("TmplDsh"),_HEAD);
    html_add_autosubmit_form();
    html_add_form();

    // create unit selector dropdown
    addSelector_Head(F("unit"), true);
    byte choice = Settings.Unit;
    for (NodesMap::iterator it = Nodes.begin(); it != Nodes.end(); ++it)
    {
      if (it->second.ip[0] != 0 || it->first == Settings.Unit)
      {
        String name = String(it->first) + F(" - ");
        if (it->first != Settings.Unit)
          name += it->second.nodeName;
        else
          name += Settings.Name;
        addSelector_Item(name, it->first, choice == it->first, false, "");
      }
    }
    addSelector_Foot();

    // create <> navigation buttons
    byte prev=Settings.Unit;
    byte next=Settings.Unit;
    NodesMap::iterator it;
    for (byte x = Settings.Unit-1; x > 0; x--) {
      it = Nodes.find(x);
      if (it != Nodes.end()) {
        if (it->second.ip[0] != 0) {prev = x; break;}
      }
    }
    for (byte x = Settings.Unit+1; x < UNIT_MAX; x++) {
      it = Nodes.find(x);
      if (it != Nodes.end()) {
        if (it->second.ip[0] != 0) {next = x; break;}
      }
    }

    html_add_button_prefix();
    TXBuffer += path;
    TXBuffer += F("?btnunit=");
    TXBuffer += prev;
    TXBuffer += F("'>&lt;</a>");
    html_add_button_prefix();
    TXBuffer += path;
    TXBuffer += F("?btnunit=");
    TXBuffer += next;
    TXBuffer += F("'>&gt;</a>");
  }

  // handle commands from a custom page
  String webrequest = WebServer.arg(F("cmd"));
  if (webrequest.length() > 0 ){
    struct EventStruct TempEvent;
    parseCommandString(&TempEvent, webrequest);
    TempEvent.Source = VALUE_SOURCE_HTTP;

    if (PluginCall(PLUGIN_WRITE, &TempEvent, webrequest));
    else if (remoteConfig(&TempEvent, webrequest));
    else if (webrequest.startsWith(F("event")))
      ExecuteCommand(VALUE_SOURCE_HTTP, webrequest.c_str());

    // handle some update processes first, before returning page update...
    PluginCall(PLUGIN_TEN_PER_SECOND, 0, dummyString);
  }


  if (dataFile)
  {
    String page = "";
    page.reserve(dataFile.size());
    while (dataFile.available())
      page += ((char)dataFile.read());

    TXBuffer += parseTemplate(page,0);
    dataFile.close();
  }
  else // if the requestef file does not exist, create a default action in case the page is named "dashboard*"
  {
    if (dashboardPage)
    {
      // if the custom page does not exist, create a basic task value overview page in case of dashboard request...
      TXBuffer += F("<meta name='viewport' content='width=width=device-width, initial-scale=1'><STYLE>* {font-family:sans-serif; font-size:16pt;}.button {margin:4px; padding:4px 16px; background-color:#07D; color:#FFF; text-decoration:none; border-radius:4px}</STYLE>");
      html_table_class_normal();
      for (byte x = 0; x < TASKS_MAX; x++)
      {
        if (Settings.TaskDeviceNumber[x] != 0)
          {
            LoadTaskSettings(x);
            byte DeviceIndex = getDeviceIndex(Settings.TaskDeviceNumber[x]);
            html_TR_TD();
            TXBuffer += ExtraTaskSettings.TaskDeviceName;
            for (byte varNr = 0; varNr < VARS_PER_TASK; varNr++)
              {
                if ((Settings.TaskDeviceNumber[x] != 0) && (varNr < Device[DeviceIndex].ValueCount) && ExtraTaskSettings.TaskDeviceValueNames[varNr][0] !=0)
                {
                  if (varNr > 0)
                    html_TR_TD();
                  html_TD();
                  TXBuffer += ExtraTaskSettings.TaskDeviceValueNames[varNr];
                  html_TD();
                  TXBuffer += String(UserVar[x * VARS_PER_TASK + varNr], ExtraTaskSettings.TaskDeviceValueDecimals[varNr]);
                }
              }
          }
      }
    }
  }
  sendHeadandTail(F("TmplDsh"),_TAIL);
  TXBuffer.endStream();
  return true;
}



//********************************************************************************
// Web Interface file list
//********************************************************************************
void handle_filelist_json() {
  checkRAM(F("handle_filelist"));
  if (!clientIPallowed()) return;
  navMenuIndex = MENU_INDEX_TOOLS;
  TXBuffer.startJsonStream();

  String fdelete = WebServer.arg(F("delete"));

  if (fdelete.length() > 0)
  {
    SPIFFS.remove(fdelete);
    #if defined(ESP32)
    // flashCount();
    #endif
    #if defined(ESP8266)
    checkRuleSets();
    #endif
  }

  const int pageSize = 25;
  int startIdx = 0;

  String fstart = WebServer.arg(F("start"));
  if (fstart.length() > 0)
  {
    startIdx = atoi(fstart.c_str());
  }
  int endIdx = startIdx + pageSize - 1;

  TXBuffer += "[{";
  bool firstentry = true;
  #if defined(ESP32)
    File root = SPIFFS.open("/");
    File file = root.openNextFile();
    int count = -1;
    while (file and count < endIdx)
    {
      if(!file.isDirectory()){
        ++count;

        if (count >= startIdx)
        {
          if (firstentry) {
            firstentry = false;
          } else {
            TXBuffer += ",{";
          }
          stream_next_json_object_value(F("fileName"), String(file.name()));
          stream_next_json_object_value(F("index"), String(startIdx));
          stream_last_json_object_value(F("size"), String(file.size()));
        }
      }
      file = root.openNextFile();
    }
  #endif
  #if defined(ESP8266)
  fs::Dir dir = SPIFFS.openDir("");

  int count = -1;
  while (dir.next())
  {
    ++count;

    if (count < startIdx)
    {
      continue;
    }

    if (firstentry) {
      firstentry = false;
    } else {
      TXBuffer += ",{";
    }

    stream_next_json_object_value(F("fileName"), String(dir.fileName()));

    fs::File f = dir.openFile("r");
    if (f) {
      stream_next_json_object_value(F("size"), String(f.size()));
      f.close();
    }

    stream_last_json_object_value(F("index"), String(startIdx));

    if (count >= endIdx)
    {
      break;
    }
  }

  #endif
  TXBuffer += "]";
  TXBuffer.endStream();
}

void handle_filelist() {
  checkRAM(F("handle_filelist"));
  if (!clientIPallowed()) return;
  navMenuIndex = MENU_INDEX_TOOLS;
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate();

#if defined(ESP8266)

  String fdelete = WebServer.arg(F("delete"));

  if (fdelete.length() > 0)
  {
    SPIFFS.remove(fdelete);
    checkRuleSets();
  }

  const int pageSize = 25;
  int startIdx = 0;

  String fstart = WebServer.arg(F("start"));
  if (fstart.length() > 0)
  {
    startIdx = atoi(fstart.c_str());
  }
  int endIdx = startIdx + pageSize - 1;

  html_table_class_multirow();
  html_table_header("", 50);
  html_table_header(F("Filename"));
  html_table_header(F("Size"), 80);

  fs::Dir dir = SPIFFS.openDir("");

  int count = -1;
  while (dir.next())
  {
    ++count;

    if (count < startIdx)
    {
      continue;
    }

    html_TR_TD();
    if (dir.fileName() != F(FILE_CONFIG) && dir.fileName() != F(FILE_SECURITY) && dir.fileName() != F(FILE_NOTIFICATION))
    {
      html_add_button_prefix();
      TXBuffer += F("filelist?delete=");
      TXBuffer += dir.fileName();
      if (startIdx > 0)
      {
        TXBuffer += F("&start=");
        TXBuffer += startIdx;
      }
      TXBuffer += F("'>Del</a>");
    }

    TXBuffer += F("<TD><a href=\"");
    TXBuffer += dir.fileName();
    TXBuffer += "\">";
    TXBuffer += dir.fileName();
    TXBuffer += F("</a>");
    fs::File f = dir.openFile("r");
    html_TD();
    if (f) {
      TXBuffer += f.size();
      f.close();
    }
    if (count >= endIdx)
    {
      break;
    }
  }
  html_end_table();
  html_end_form();
  html_BR();
  addButton(F("/upload"), F("Upload"));
  if (startIdx > 0)
  {
    html_add_button_prefix();
    TXBuffer += F("/filelist?start=");
    TXBuffer += std::max(0, startIdx - pageSize);
    TXBuffer += F("'>Previous</a>");
  }
  if (count >= endIdx and dir.next())
  {
    html_add_button_prefix();
    TXBuffer += F("/filelist?start=");
    TXBuffer += endIdx + 1;
    TXBuffer += F("'>Next</a>");
  }
  TXBuffer += F("<BR><BR>");
  sendHeadandTail_stdtemplate(true);
  TXBuffer.endStream();
#endif
#if defined(ESP32)
  String fdelete = WebServer.arg(F("delete"));

  if (fdelete.length() > 0)
  {
    SPIFFS.remove(fdelete);
    // flashCount();
  }

  const int pageSize = 25;
  int startIdx = 0;

  String fstart = WebServer.arg(F("start"));
  if (fstart.length() > 0)
  {
    startIdx = atoi(fstart.c_str());
  }
  int endIdx = startIdx + pageSize - 1;

  html_table_class_multirow();
  html_table_header("");
  html_table_header(F("Filename"));
  html_table_header("Size");

  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  int count = -1;
  while (file and count < endIdx)
  {
    if(!file.isDirectory()){
      ++count;

      if (count >= startIdx)
      {
        html_TR_TD();
        if (strcmp(file.name(), FILE_CONFIG) != 0 && strcmp(file.name(), FILE_SECURITY) != 0 && strcmp(file.name(), FILE_NOTIFICATION) != 0)
        {
          html_add_button_prefix();
          TXBuffer += F("filelist?delete=");
          TXBuffer += file.name();
          if (startIdx > 0)
          {
            TXBuffer += F("&start=");
            TXBuffer += startIdx;
          }
          TXBuffer += F("'>Del</a>");
        }

        TXBuffer += F("<TD><a href=\"");
        TXBuffer += file.name();
        TXBuffer += "\">";
        TXBuffer += file.name();
        TXBuffer += F("</a>");
        html_TD();
        TXBuffer += file.size();
      }
    }
    file = root.openNextFile();
  }
  html_end_table();
  html_end_form();
  html_BR();
  addButton(F("/upload"), F("Upload"));
  if (startIdx > 0)
  {
    html_add_button_prefix();
    TXBuffer += F("/filelist?start=");
    TXBuffer += startIdx < pageSize ? 0 : startIdx - pageSize;
    TXBuffer += F("'>Previous</a>");
  }
  if (count >= endIdx and file)
  {
    html_add_button_prefix();
    TXBuffer += F("/filelist?start=");
    TXBuffer += endIdx + 1;
    TXBuffer += F("'>Next</a>");
  }
  TXBuffer += F("<BR><BR>");
    sendHeadandTail_stdtemplate(true);
    TXBuffer.endStream();
#endif
}


//********************************************************************************
// Web Interface SD card file and directory list
//********************************************************************************
#ifdef FEATURE_SD
void handle_SDfilelist() {
  checkRAM(F("handle_SDfilelist"));
  if (!clientIPallowed()) return;
  navMenuIndex = MENU_INDEX_TOOLS;
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate();


  String fdelete = "";
  String ddelete = "";
  String change_to_dir = "";
  String current_dir = "";
  String parent_dir = "";

  for (uint8_t i = 0; i < WebServer.args(); i++) {
    if (WebServer.argName(i) == F("delete"))
    {
      fdelete = WebServer.arg(i);
    }
    if (WebServer.argName(i) == F("deletedir"))
    {
      ddelete = WebServer.arg(i);
    }
    if (WebServer.argName(i) == F("chgto"))
    {
      change_to_dir = WebServer.arg(i);
    }
  }

  if (fdelete.length() > 0)
  {
    SD.remove((char*)fdelete.c_str());
  }
  if (ddelete.length() > 0)
  {
    SD.rmdir((char*)ddelete.c_str());
  }
  if (change_to_dir.length() > 0)
  {
    current_dir = change_to_dir;
  }
  else
  {
    current_dir = "/";
  }

  File root = SD.open(current_dir.c_str());
  root.rewindDirectory();
  File entry = root.openNextFile();
  parent_dir = current_dir;
  if (!current_dir.equals("/"))
  {
    /* calculate the position to remove
    /
    / current_dir = /dir1/dir2/   =>   parent_dir = /dir1/
    /                     ^ position to remove, second last index of "/" + 1
    /
    / current_dir = /dir1/   =>   parent_dir = /
    /                ^ position to remove, second last index of "/" + 1
    */
    parent_dir.remove(parent_dir.lastIndexOf("/", parent_dir.lastIndexOf("/") - 1) + 1);
  }



  String subheader = "SD Card: " + current_dir;
  addFormSubHeader(subheader);
  html_BR();
  html_table_class_multirow();
  html_table_header("", 50);
  html_table_header("Name");
  html_table_header("Size");
  html_TR_TD();
  TXBuffer += F("<TD><a href=\"SDfilelist?chgto=");
  TXBuffer += parent_dir;
  TXBuffer += F("\">..");
  TXBuffer += F("</a>");
  html_TD();
  while (entry)
  {
    if (entry.isDirectory())
    {
      char SDcardChildDir[80];
      html_TR_TD();
      // take a look in the directory for entries
      String child_dir = current_dir + entry.name();
      child_dir.toCharArray(SDcardChildDir, child_dir.length()+1);
      File child = SD.open(SDcardChildDir);
      File dir_has_entry = child.openNextFile();
      // when the directory is empty, display the button to delete them
      if (!dir_has_entry)
      {
        TXBuffer += F("<a class='button link' onclick=\"return confirm('Delete this directory?')\" href=\"SDfilelist?deletedir=");
        TXBuffer += current_dir;
        TXBuffer += entry.name();
        TXBuffer += '/';
        TXBuffer += F("&chgto=");
        TXBuffer += current_dir;
        TXBuffer += F("\">Del</a>");
      }
      TXBuffer += F("<TD><a href=\"SDfilelist?chgto=");
      TXBuffer += current_dir;
      TXBuffer += entry.name();
      TXBuffer += '/';
      TXBuffer += "\">";
      TXBuffer += entry.name();
      TXBuffer += F("</a>");
      html_TD();
      TXBuffer += F("dir");
      dir_has_entry.close();
    }
    else
    {
      html_TR_TD();
      if (entry.name() != String(F(FILE_CONFIG)).c_str() && entry.name() != String(F(FILE_SECURITY)).c_str())
      {
        TXBuffer += F("<a class='button link' onclick=\"return confirm('Delete this file?')\" href=\"SDfilelist?delete=");
        TXBuffer += current_dir;
        TXBuffer += entry.name();
        TXBuffer += F("&chgto=");
        TXBuffer += current_dir;
        TXBuffer += F("\">Del</a>");
      }
      TXBuffer += F("<TD><a href=\"");
      TXBuffer += current_dir;
      TXBuffer += entry.name();
      TXBuffer += "\">";
      TXBuffer += entry.name();
      TXBuffer += F("</a>");
      html_TD();
      TXBuffer += entry.size();
    }
    entry.close();
    entry = root.openNextFile();
  }
  root.close();
  html_end_table();
  html_end_form();
  //TXBuffer += F("<BR><a class='button link' href=\"/upload\">Upload</a>");
  sendHeadandTail_stdtemplate(true);
  TXBuffer.endStream();
}
#endif


//********************************************************************************
// Web Interface handle other requests
//********************************************************************************
void handleNotFound() {
  checkRAM(F("handleNotFound"));

  if (wifiSetup)
  {
    WebServer.send(200, F("text/html"), F("<meta HTTP-EQUIV='REFRESH' content='0; url=/setup'>"));
    return;
  }

  if (!isLoggedIn()) return;
  if (handle_rules_edit(WebServer.uri())) return;
  if (loadFromFS(true, WebServer.uri())) return;
  if (loadFromFS(false, WebServer.uri())) return;
  String message = F("URI: ");
  message += WebServer.uri();
  message += F("\nMethod: ");
  message += (WebServer.method() == HTTP_GET) ? F("GET") : F("POST");
  message += F("\nArguments: ");
  message += WebServer.args();
  message += "\n";
  for (uint8_t i = 0; i < WebServer.args(); i++) {
    message += F(" NAME:");
    message += WebServer.argName(i);
    message += F("\n VALUE:");
    message += WebServer.arg(i);
    message += '\n';
  }
  WebServer.send(404, F("text/plain"), message);
}


//********************************************************************************
// Web Interface Setup Wizard
//********************************************************************************
void handle_setup() {
  checkRAM(F("handle_setup"));
  // Do not check client IP range allowed.
  TXBuffer.startStream();
  sendHeadandTail(F("TmplAP"));

  addHeader(false,TXBuffer.buf);

  if (WiFiConnected())
  {
    addHtmlError(SaveSettings());
    String host = formatIP(WiFi.localIP());
    TXBuffer += F("<BR>ESP is connected and using IP Address: <BR><h1>");
    TXBuffer += host;
    TXBuffer += F("</h1><BR><BR>Connect your laptop / tablet / phone<BR>back to your main Wifi network and<BR><BR>");
    TXBuffer += F("<a class='button' href='http://");
    TXBuffer += host;
    TXBuffer += F("/config'>Proceed to main config</a><BR><BR>");

    sendHeadandTail(F("TmplAP"),true);
    TXBuffer.endStream();

    wifiSetup = false;
    //setWifiMode(WIFI_STA);  //this forces the iPhone to exit safari and this page was never displayed
    timerAPoff = millis() + 60000L;  //switch the AP off in 1 minute
    return;
  }

  static byte status = 0;
  static int n = 0;
  static byte refreshCount = 0;
  String ssid = WebServer.arg(F("ssid"));
  String other = WebServer.arg(F("other"));
  String password = WebServer.arg(F("pass"));

  if (other.length() != 0)
  {
    ssid = other;
  }

  // if ssid config not set and params are both provided
  if (status == 0 && ssid.length() != 0 /*&& strcasecmp(SecuritySettings.WifiSSID, "ssid") == 0 */)
  {
    safe_strncpy(SecuritySettings.WifiKey, password.c_str(), sizeof(SecuritySettings.WifiKey));
    safe_strncpy(SecuritySettings.WifiSSID, ssid.c_str(), sizeof(SecuritySettings.WifiSSID));
    wifiSetupConnect = true;
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String reconnectlog = F("WIFI : Credentials Changed, retry connection. SSID: ");
      reconnectlog += ssid;
      addLog(LOG_LEVEL_INFO, reconnectlog);
    }
    status = 1;
    refreshCount = 0;
  }

  TXBuffer += F("<BR><h1>Wifi Setup wizard</h1>");
  html_add_form();

  if (status == 0)  // first step, scan and show access points within reach...
  {
    WiFiMode_t cur_wifimode = WiFi.getMode();
    if (n == 0)
      n = WiFi.scanNetworks(false, true);
    setWifiMode(cur_wifimode);
    if (n == 0)
      TXBuffer += F("No Access Points found");
    else
    {
      html_table_class_multirow();
      html_TR();
      html_table_header(F("Pick"), 50);
      html_table_header(F("Network info"));
      for (int i = 0; i < n; ++i)
      {
        html_TR_TD(); TXBuffer += F("<label class='container2'>");
        TXBuffer += F("<input type='radio' name='ssid' value='");
        {
          String escapeBuffer = WiFi.SSID(i);
          htmlStrongEscape(escapeBuffer);
          TXBuffer += escapeBuffer;
        }
        TXBuffer += '\'';
        if (WiFi.SSID(i) == ssid)
          TXBuffer += F(" checked ");
        TXBuffer += F("><span class='dotmark'></span></label><TD>");
        TXBuffer += formatScanResult(i, "<BR>");
        TXBuffer += "";
      }
      html_end_table();
    }

    TXBuffer += F("<BR><label class='container2'>other SSID:<input type='radio' name='ssid' id='other_ssid' value='other' ><span class='dotmark'></span></label>");
    TXBuffer += F("<input class='wide' type ='text' name='other' value='");
    TXBuffer += other;
    TXBuffer += F("'><BR><BR>");

    addFormSeparator (2);

    TXBuffer += F("<BR>Password:<BR><input class='wide' type ='text' name='pass' value='");
    TXBuffer += password;
    TXBuffer += F("'><BR><BR>");

    addSubmitButton(F("Connect"),"");
  }

  if (status == 1)  // connecting stage...
  {
    if (refreshCount > 0)
    {
      status = 0;
//      safe_strncpy(SecuritySettings.WifiSSID, "ssid", sizeof(SecuritySettings.WifiSSID));
//      SecuritySettings.WifiKey[0] = 0;
      TXBuffer += F("<a class='button' href='setup'>Back to Setup</a><BR><BR>");
    }
    else
    {
      int wait = 20;
      if (refreshCount != 0)
        wait = 3;
      TXBuffer += F("Please wait for <h1 id='countdown'>20..</h1>");
      TXBuffer += F("<script type='text/JavaScript'>");
      TXBuffer += F("function timedRefresh(timeoutPeriod) {");
      TXBuffer += F(   "var timer = setInterval(function() {");
      TXBuffer += F(   "if (timeoutPeriod > 0) {");
      TXBuffer += F(       "timeoutPeriod -= 1;");
      TXBuffer += F(       "document.getElementById('countdown').innerHTML = timeoutPeriod + '..' + '<br />';");
      TXBuffer += F(   "} else {");
      TXBuffer += F(       "clearInterval(timer);");
      TXBuffer += F(            "window.location.href = window.location.href;");
      TXBuffer += F(       "};");
      TXBuffer += F(   "}, 1000);");
      TXBuffer += F("};");
      TXBuffer += F("timedRefresh(");
      TXBuffer += wait;
      TXBuffer += F(");");
      html_add_script_end();
      TXBuffer += F("seconds while trying to connect");
    }
    refreshCount++;
  }

  html_end_form();
  sendHeadandTail(F("TmplAP"),true);
  TXBuffer.endStream();
  delay(10);
}

//********************************************************************************
// Create pre-defined config selector
//********************************************************************************
void addPreDefinedConfigSelector() {
  DeviceModel active_model = ResetFactoryDefaultPreference.getDeviceModel();
  addSelector_Head("fdm", true);
  for (byte x = 0; x < DeviceModel_MAX; ++x) {
    DeviceModel model = static_cast<DeviceModel>(x);
    addSelector_Item(
      getDeviceModelString(model),
      x,
      model == active_model,
      !modelMatchingFlashSize(model),
      ""
    );
  }
  addSelector_Foot();
}

//********************************************************************************
// Web Interface Factory Reset
//********************************************************************************
void handle_factoryreset() {
  checkRAM(F("handle_factoryreset"));
  if (!isLoggedIn()) return;
  navMenuIndex = MENU_INDEX_TOOLS;
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate(_HEAD);
  html_add_form();
  html_table_class_normal();
  html_TR();
  addFormHeader(F("Factory Reset"));

  if (WebServer.hasArg("fdm")) {
    DeviceModel model = static_cast<DeviceModel>(getFormItemInt("fdm"));
    if (modelMatchingFlashSize(model)) {
      setFactoryDefault(model);
    }
  }
  if (WebServer.hasArg("kun")) {
    ResetFactoryDefaultPreference.keepUnitName(isFormItemChecked("kun"));
  }
  if (WebServer.hasArg("kw")) {
    ResetFactoryDefaultPreference.keepWiFi(isFormItemChecked("kw"));
  }
  if (WebServer.hasArg("knet")) {
    ResetFactoryDefaultPreference.keepNetwork(isFormItemChecked("knet"));
  }
  if (WebServer.hasArg("kntp")) {
    ResetFactoryDefaultPreference.keepNTP(isFormItemChecked("kntp"));
  }
  if (WebServer.hasArg("klog")) {
    ResetFactoryDefaultPreference.keepLogSettings(isFormItemChecked("klog"));
  }

  if (WebServer.hasArg(F("savepref"))) {
    // User choose a pre-defined config and wants to save it as the new default.
    applyFactoryDefaultPref();
    addHtmlError(SaveSettings());
  }
  if (WebServer.hasArg(F("performfactoryreset"))) {
      // User confirmed to really perform the reset.
      applyFactoryDefaultPref();
      // No need to call SaveSettings(); ResetFactory() will save the new settings.
      ResetFactory();
  } else {
    // Nothing chosen yet, show options.
    addTableSeparator(F("Settings to keep"), 2, 3);

    addRowLabel(F("Keep Unit/Name"));
    addCheckBox("kun", ResetFactoryDefaultPreference.keepUnitName());

    addRowLabel(F("Keep WiFi config"));
    addCheckBox("kw", ResetFactoryDefaultPreference.keepWiFi());

    addRowLabel(F("Keep Network config"));
    addCheckBox("knet", ResetFactoryDefaultPreference.keepNetwork());

    addRowLabel(F("Keep NTP/DST config"));
    addCheckBox("kntp", ResetFactoryDefaultPreference.keepNTP());

    addRowLabel(F("Keep log config"));
    addCheckBox("klog", ResetFactoryDefaultPreference.keepLogSettings());

    addTableSeparator(F("Pre-defined configurations"), 2, 3);
    addRowLabel(F("Pre-defined config"));
    addPreDefinedConfigSelector();


    html_TR_TD();
    html_TD();
    addSubmitButton(F("Save Preferences"), F("savepref"));


    html_TR_TD_height(30);

    addTableSeparator(F("Immediate full reset"), 2, 3);
    addRowLabel(F("Erase settings files"));
    addSubmitButton(F("Factory Reset"), F("performfactoryreset"), F("red"));
  }

  html_end_table();
  html_end_form();
  sendHeadandTail_stdtemplate(_TAIL);
  TXBuffer.endStream();

}

//********************************************************************************
// Web Interface rules page
//********************************************************************************
void handle_rules() {
  checkRAM(F("handle_rules"));
  if (!isLoggedIn() || !Settings.UseRules) return;
  navMenuIndex = MENU_INDEX_RULES;
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate();
  static byte currentSet = 1;

  const byte rulesSet = getFormItemInt(F("set"), 1);

  #if defined(ESP8266)
    String fileName = F("rules");
  #endif
  #if defined(ESP32)
    String fileName = F("/rules");
  #endif
  fileName += rulesSet;
  fileName += F(".txt");


  checkRAM(F("handle_rules"));



  if (WebServer.args() > 0)
  {
    String log = F("Rules : Save rulesSet: ");
    log += rulesSet;
    log += F(" currentSet: ");
    log += currentSet;

    if (currentSet == rulesSet) // only save when the dropbox was not used to change set
    {
      String rules = WebServer.arg(F("rules"));
      log += F(" rules.length(): ");
      log += rules.length();
      if (rules.length() > RULES_MAX_SIZE)
        TXBuffer += F("<span style=\"color:red\">Data was not saved, exceeds web editor limit!</span>");
      else
      {

        // if (RTC.flashDayCounter > MAX_FLASHWRITES_PER_DAY)
        // {
        //   String log = F("FS   : Daily flash write rate exceeded! (powercyle to reset this)");
        //   addLog(LOG_LEVEL_ERROR, log);
        //   TXBuffer += F("<span style=\"color:red\">Error saving to flash!</span>");
        // }
        // else
        // {
          fs::File f = SPIFFS.open(fileName, "w");
          if (f)
          {
            log += F(" Write to file: ");
            log += fileName;
            f.print(rules);
            f.close();
            // flashCount();
          }
        // }
      }
    }
    else // changed set, check if file exists and create new
    {
      if (!SPIFFS.exists(fileName))
      {
        log += F(" Create new file: ");
        log += fileName;
        fs::File f = SPIFFS.open(fileName, "w");
        if (f) f.close();
      }
    }
    addLog(LOG_LEVEL_INFO, log);

    log = F(" Webserver args:");
    for (int i = 0; i < WebServer.args(); ++i) {
      log += ' ';
      log += i;
      log += F(": '");
      log += WebServer.argName(i);
      log += F("' length: ");
      log += WebServer.arg(i).length();
    }
    addLog(LOG_LEVEL_INFO, log);
  }

  if (rulesSet != currentSet)
    currentSet = rulesSet;

  TXBuffer += F("<form name = 'frmselect' method = 'post'>");
  html_table_class_normal();
  html_TR();
  html_table_header(F("Rules"));

  byte choice = rulesSet;
  String options[RULESETS_MAX];
  int optionValues[RULESETS_MAX];
  for (byte x = 0; x < RULESETS_MAX; x++)
  {
    options[x] = F("Rules Set ");
    options[x] += x + 1;
    optionValues[x] = x + 1;
  }

   html_TR_TD();
  addSelector(F("set"), RULESETS_MAX, options, optionValues, NULL, choice, true);
  addHelpButton(F("Tutorial_Rules"));

  // load form data from flash

  int size = 0;
  fs::File f = SPIFFS.open(fileName, "r+");
  if (f)
  {
    size = f.size();
    if (size > RULES_MAX_SIZE)
       TXBuffer += F("<span style=\"color:red\">Filesize exceeds web editor limit!</span>");
    else
    {
       html_TR_TD(); TXBuffer += F("<textarea name='rules' rows='30' wrap='off'>");
      while (f.available())
      {
        String c((char)f.read());
        htmlEscape(c);
         TXBuffer += c;
      }
       TXBuffer += F("</textarea>");
    }
    f.close();
  }

  html_TR_TD(); TXBuffer += F("Current size: ");
  TXBuffer += size;
  TXBuffer += F(" characters (Max ");
  TXBuffer += RULES_MAX_SIZE;
  TXBuffer += ')';

  addFormSeparator(2);

  html_TR_TD();
  addSubmitButton();
  addButton(fileName, F("Download to file"));
  html_end_table();
  html_end_form();
  sendHeadandTail_stdtemplate(true);
  TXBuffer.endStream();

  checkRuleSets();
}


//********************************************************************************
// Web Interface sysinfo page
//********************************************************************************
void handle_sysinfo() {
  checkRAM(F("handle_sysinfo"));
  if (!isLoggedIn()) return;
  navMenuIndex = MENU_INDEX_TOOLS;
  html_reset_copyTextCounter();
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate();

  int freeMem = ESP.getFreeHeap();

  addHeader(true,  TXBuffer.buf);
  TXBuffer += printWebString;
  TXBuffer += F("<form>");

  // the table header
  html_table_class_normal();

  // Not using addFormHeader() to get the copy button on the same header line as 2nd column
  html_TR();
  html_table_header(F("System Info"), 225);
  TXBuffer += "<TH>"; // Needed to get the copy button on the same header line.
  addCopyButton(F("copyText"), F("\\n"), F("Copy info to clipboard") );

  TXBuffer += githublogo;
  html_add_script(false);
  TXBuffer += DATA_GITHUB_CLIPBOARD_JS;
  html_add_script_end();

  addRowLabel(F("Unit"));
  TXBuffer += Settings.Unit;

  if (systemTimePresent())
  {
     addRowLabel(F("Local Time"));
     TXBuffer += getDateTimeString('-', ':', ' ');
  }

  addRowLabel(F("Uptime"));
  {
    char strUpTime[40];
    int minutes = wdcounter / 2;
    int days = minutes / 1440;
    minutes = minutes % 1440;
    int hrs = minutes / 60;
    minutes = minutes % 60;
    sprintf_P(strUpTime, PSTR("%d days %d hours %d minutes"), days, hrs, minutes);
    TXBuffer += strUpTime;
  }

  addRowLabel(F("Load"));
  if (wdcounter > 0)
  {
     TXBuffer += getCPUload();
     TXBuffer += F("% (LC=");
     TXBuffer += getLoopCountPerSec();
     TXBuffer += ')';
  }

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

  addRowLabel(F("Boot"));
  TXBuffer += getLastBootCauseString();
  TXBuffer += " (";
  TXBuffer += RTC.bootCounter;
  TXBuffer += ')';
  addRowLabel(F("Reset Reason"));
  TXBuffer += getResetReasonString();

  addTableSeparator(F("Network"), 2, 3, F("Wifi"));

  if (WiFiConnected())
  {
     addRowLabel(F("Wifi"));
    #if defined(ESP8266)
      byte PHYmode = wifi_get_phy_mode();
    #endif
    #if defined(ESP32)
      byte PHYmode = 3; // wifi_get_phy_mode();
    #endif
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
  addRowLabel(F("IP config"));
  TXBuffer += useStaticIP() ? F("Static") : F("DHCP");

  addRowLabel(F("IP / subnet"));
  TXBuffer += formatIP(WiFi.localIP());
  TXBuffer += F(" / ");
  TXBuffer += formatIP(WiFi.subnetMask());

  addRowLabel(F("GW"));
  TXBuffer += formatIP(WiFi.gatewayIP());

  {
    addRowLabel(F("Client IP"));
    WiFiClient client(WebServer.client());
    TXBuffer += formatIP(client.remoteIP());
  }

  addRowLabel(F("DNS"));
  TXBuffer += formatIP(WiFi.dnsIP(0));
  TXBuffer += F(" / ");
  TXBuffer += formatIP(WiFi.dnsIP(1));

  addRowLabel(F("Allowed IP Range"));
  TXBuffer += describeAllowedIPrange();

  addRowLabel(F("STA MAC"));

  {
    uint8_t mac[] = {0, 0, 0, 0, 0, 0};
    uint8_t* macread = WiFi.macAddress(mac);
    char macaddress[20];
    formatMAC(macread, macaddress);
    TXBuffer += macaddress;

    addRowLabel(F("AP MAC"));
    macread = WiFi.softAPmacAddress(mac);
    formatMAC(macread, macaddress);
    TXBuffer += macaddress;
  }

  addRowLabel(F("SSID"));
  TXBuffer += WiFi.SSID();
  TXBuffer += " (";
  TXBuffer += WiFi.BSSIDstr();
  TXBuffer += ')';

  addRowLabel(F("Channel"));
  TXBuffer += WiFi.channel();

  addRowLabel(F("Connected"));
  TXBuffer += format_msec_duration(timeDiff(lastConnectMoment, millis()));

  addRowLabel(F("Last Disconnect Reason"));
  TXBuffer += getLastDisconnectReason();

  addRowLabel(F("Number reconnects"));
  TXBuffer += wifi_reconnects;

  addTableSeparator(F("Firmware"), 2, 3);

  addRowLabel_copy(F("Build"));
  TXBuffer += BUILD;
  TXBuffer += ' ';
  TXBuffer += F(BUILD_NOTES);

  addRowLabel_copy(F("Libraries"));
  TXBuffer += getSystemLibraryString();

  addRowLabel_copy(F("GIT version"));
  TXBuffer += BUILD_GIT;

  addRowLabel_copy(F("Plugins"));
  TXBuffer += deviceCount + 1;
  TXBuffer += getPluginDescriptionString();

  bool filenameDummy = String(CRCValues.binaryFilename).startsWith(F("ThisIsTheDummy"));
  if (!filenameDummy) {
    addRowLabel(F("Build Md5"));
    for (byte i = 0; i<16; i++)    TXBuffer += String(CRCValues.compileTimeMD5[i],HEX);

     addRowLabel(F("Md5 check"));
    if (! CRCValues.checkPassed())
       TXBuffer += F("<font color = 'red'>fail !</font>");
    else  TXBuffer += F("passed.");
  }
  addRowLabel_copy(F("Build time"));
  TXBuffer += String(CRCValues.compileDate);
  TXBuffer += ' ';
  TXBuffer += String(CRCValues.compileTime);

  addRowLabel_copy(F("Binary filename"));
  if (filenameDummy) {
    TXBuffer += F("<b>Self built!</b>");
  } else {
    TXBuffer += String(CRCValues.binaryFilename);
  }

  addTableSeparator(F("System Status"), 2, 3);
  {
    // Actual Loglevel
    addRowLabel(F("Syslog Log Level"));
    TXBuffer += getLogLevelDisplayString(Settings.SyslogLevel);
    addRowLabel(F("Serial Log Level"));
    TXBuffer += getLogLevelDisplayString(getSerialLogLevel());
    addRowLabel(F("Web Log Level"));
    TXBuffer += getLogLevelDisplayString(getWebLogLevel());
    #ifdef FEATURE_SD
    addRowLabel(F("SD Log Level"));
    TXBuffer += getLogLevelDisplayString(Settings.SDLogLevel);
    #endif
  }


  addTableSeparator(F("ESP board"), 2, 3);

  addRowLabel(F("ESP Chip ID"));
  #if defined(ESP8266)
    TXBuffer += ESP.getChipId();
    TXBuffer += F(" (0x");
    String espChipId(ESP.getChipId(), HEX);
    espChipId.toUpperCase();
    TXBuffer += espChipId;
    TXBuffer += ')';

    addRowLabel(F("ESP Chip Freq"));
    TXBuffer += ESP.getCpuFreqMHz();
    TXBuffer += F(" MHz");
  #endif
  #if defined(ESP32)
    TXBuffer += F(" (0x");
    uint64_t chipid=ESP.getEfuseMac();   //The chip ID is essentially its MAC address(length: 6 bytes).
    uint32_t ChipId1 = (uint16_t)(chipid>>32);
    String espChipIdS(ChipId1, HEX);
    espChipIdS.toUpperCase();
    TXBuffer += espChipIdS;
    ChipId1 = (uint32_t)chipid;
    String espChipIdS1(ChipId1, HEX);
    espChipIdS1.toUpperCase();
    TXBuffer += espChipIdS1;
    TXBuffer += ')';

    addRowLabel(F("ESP Chip Freq"));
    TXBuffer += ESP.getCpuFreqMHz();
    TXBuffer += F(" MHz");
  #endif
  #ifdef ARDUINO_BOARD
  addRowLabel(F("ESP Board Name"));
  TXBuffer += ARDUINO_BOARD;
  #endif

  addTableSeparator(F("Storage"), 2, 3);

  addRowLabel(F("Flash Chip ID"));
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
  #endif
  uint32_t realSize = getFlashRealSizeInBytes();
  uint32_t ideSize = ESP.getFlashChipSize();

  addRowLabel(F("Flash Chip Real Size"));
  TXBuffer += realSize / 1024;
  TXBuffer += F(" kB");

  addRowLabel(F("Flash IDE Size"));
  TXBuffer += ideSize / 1024;
  TXBuffer += F(" kB");

  // Please check what is supported for the ESP32
  #if defined(ESP8266)
    addRowLabel(F("Flash IDE speed"));
    TXBuffer += ESP.getFlashChipSpeed() / 1000000;
    TXBuffer += F(" MHz");

    FlashMode_t ideMode = ESP.getFlashChipMode();
    addRowLabel(F("Flash IDE mode"));
    switch (ideMode) {
      case FM_QIO:   TXBuffer += F("QIO");  break;
      case FM_QOUT:  TXBuffer += F("QOUT"); break;
      case FM_DIO:   TXBuffer += F("DIO");  break;
      case FM_DOUT:  TXBuffer += F("DOUT"); break;
      default:
          TXBuffer += getUnknownString(); break;
    }
  #endif

   addRowLabel(F("Flash Writes"));
   TXBuffer += RTC.flashDayCounter;
   TXBuffer += F(" daily / ");
   TXBuffer += RTC.flashCounter;
   TXBuffer += F(" boot");

   addRowLabel(F("Sketch Size"));
  #if defined(ESP8266)
   TXBuffer += ESP.getSketchSize() / 1024;
   TXBuffer += F(" kB (");
   TXBuffer += ESP.getFreeSketchSpace() / 1024;
   TXBuffer += F(" kB free)");
  #endif

  addRowLabel(F("SPIFFS Size"));
  {
  #if defined(ESP8266)
    fs::FSInfo fs_info;
    SPIFFS.info(fs_info);
    TXBuffer += fs_info.totalBytes / 1024;
    TXBuffer += F(" kB (");
    TXBuffer += (fs_info.totalBytes - fs_info.usedBytes) / 1024;
    TXBuffer += F(" kB free)");
  #endif
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
#endif

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
  #endif

  html_end_table();
  html_end_form();
  sendHeadandTail_stdtemplate(true);
  TXBuffer.endStream();
}

void addSysVar_html(const String& input) {
  html_TR_TD();
  TXBuffer += F("<pre>"); // Make monospaced (<tt> tag?)
  TXBuffer += F("<xmp>"); // Make sure HTML code is escaped. Tag depricated??
  TXBuffer += input;
  TXBuffer += F("</xmp>");
  TXBuffer += F("</pre>");
  html_TD();
  String replacement(input); // Make deepcopy for replacement
  parseSystemVariables(replacement, false); // Not URL encoded
  parseStandardConversions(replacement, false);
  TXBuffer += replacement;
  html_TD();
  replacement = input;
  parseSystemVariables(replacement, true); // URL encoded
  parseStandardConversions(replacement, true);
  TXBuffer += replacement;
  delay(0);
}


//********************************************************************************
// Web Interface sysvars showing all system vars and their value.
//********************************************************************************
void handle_sysvars() {
  checkRAM(F("handle_sysvars"));
  if (!isLoggedIn()) return;
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate();

  addHeader(true,  TXBuffer.buf);

  html_BR();
  TXBuffer += F("<p>This page may load slow.<BR>Do not load too often, since it may affect performance of the node.</p>");
  html_BR();

  // the table header
  html_table_class_normal();
  html_TR();
  html_table_header(F("System Variables"));
  html_table_header(F("Normal"));
  html_table_header(F("URL encoded"), F("ESPEasy_System_Variables"), 0);

  addTableSeparator(F("Constants"), 3, 3);
  addSysVar_html(F("%CR%"));
  addSysVar_html(F("%LF%"));
  addSysVar_html(F("%SP%"));
  addSysVar_html(F("%R%"));
  addSysVar_html(F("%N%"));

  addTableSeparator(F("Network"), 3, 3);
  addSysVar_html(F("%mac%"));
#if defined(ESP8266)
  addSysVar_html(F("%mac_int%"));
#endif
  addSysVar_html(F("%ip4%"));
  addSysVar_html(F("%ip%"));
  addSysVar_html(F("%rssi%"));
  addSysVar_html(F("%ssid%"));
  addSysVar_html(F("%bssid%"));
  addSysVar_html(F("%wi_ch%"));

  addTableSeparator(F("System"), 3, 3);
  addSysVar_html(F("%unit%"));
  addSysVar_html(F("%sysload%"));
  addSysVar_html(F("%sysheap%"));
  addSysVar_html(F("%sysname%"));
#if FEATURE_ADC_VCC
  addSysVar_html(F("%vcc%"));
#endif

  addTableSeparator(F("System status"), 3, 3);

  addSysVar_html(F("%iswifi%"));
  addSysVar_html(F("%isntp%"));
  addSysVar_html(F("%ismqtt%"));
#ifdef USES_P037
  addSysVar_html(F("%ismqttimp%"));
#endif // USES_P037

  addTableSeparator(F("Time"), 3, 3);
  addSysVar_html(F("%lcltime%"));
  addSysVar_html(F("%lcltime_am%"));
  addSysVar_html(F("%systm_hm%"));
  addSysVar_html(F("%systm_hm_am%"));
  addSysVar_html(F("%systime%"));
  addSysVar_html(F("%systime_am%"));
  addSysVar_html(F("%sysbuild_date%"));
  addSysVar_html(F("%sysbuild_time%"));
  addTableSeparator(F("System"), 3, 3);
  addSysVar_html(F("%sysyear%  // %sysyear_0%"));
  addSysVar_html(F("%sysyears%"));
  addSysVar_html(F("%sysmonth% // %sysmonth_0%"));
  addSysVar_html(F("%sysday%   // %sysday_0%"));
  addSysVar_html(F("%syshour%  // %syshour_0%"));
  addSysVar_html(F("%sysmin%   // %sysmin_0%"));
  addSysVar_html(F("%syssec%   // %syssec_0%"));
  addSysVar_html(F("%syssec_d%"));
  addSysVar_html(F("%sysweekday%"));
  addSysVar_html(F("%sysweekday_s%"));
  addTableSeparator(F("System"), 3, 3);
  addSysVar_html(F("%uptime%"));
  addSysVar_html(F("%unixtime%"));
  addSysVar_html(F("%sunset%"));
  addSysVar_html(F("%sunset-1h%"));
  addSysVar_html(F("%sunrise%"));
  addSysVar_html(F("%sunrise+10m%"));

  addTableSeparator(F("Custom Variables"), 3, 3);
  for (byte i = 0; i < CUSTOM_VARS_MAX; ++i) {
    addSysVar_html("%v"+toString(i+1,0)+'%');
  }

  addTableSeparator(F("Special Characters"), 3, 2);
  addTableSeparator(F("Degree"), 3, 3);
  addSysVar_html(F("{D}"));
  addSysVar_html(F("&deg;"));

  addTableSeparator(F("Angle quotes"), 3, 3);
  addSysVar_html(F("{<<}"));
  addSysVar_html(F("&laquo;"));
  addFormSeparator(3);
  addSysVar_html(F("{>>}"));
  addSysVar_html(F("&raquo;"));
  addTableSeparator(F("Greek letter Mu"), 3, 3);
  addSysVar_html(F("{u}"));
  addSysVar_html(F("&micro;"));
  addTableSeparator(F("Currency"), 3, 3);
  addSysVar_html(F("{E}"));
  addSysVar_html(F("&euro;"));
  addFormSeparator(3);
  addSysVar_html(F("{Y}"));
  addSysVar_html(F("&yen;"));
  addFormSeparator(3);
  addSysVar_html(F("{P}"));
  addSysVar_html(F("&pound;"));
  addFormSeparator(3);
  addSysVar_html(F("{c}"));
  addSysVar_html(F("&cent;"));

  addTableSeparator(F("Math symbols"), 3, 3);
  addSysVar_html(F("{^1}"));
  addSysVar_html(F("&sup1;"));
  addFormSeparator(3);
  addSysVar_html(F("{^2}"));
  addSysVar_html(F("&sup2;"));
  addFormSeparator(3);
  addSysVar_html(F("{^3}"));
  addSysVar_html(F("&sup3;"));
  addFormSeparator(3);
  addSysVar_html(F("{1_4}"));
  addSysVar_html(F("&frac14;"));
  addFormSeparator(3);
  addSysVar_html(F("{1_2}"));
  addSysVar_html(F("&frac12;"));
  addFormSeparator(3);
  addSysVar_html(F("{3_4}"));
  addSysVar_html(F("&frac34;"));
  addFormSeparator(3);
  addSysVar_html(F("{+-}"));
  addSysVar_html(F("&plusmn;"));
  addFormSeparator(3);
  addSysVar_html(F("{x}"));
  addSysVar_html(F("&times;"));
  addFormSeparator(3);
  addSysVar_html(F("{..}"));
  addSysVar_html(F("&divide;"));

  addTableSeparator(F("Standard Conversions"), 3, 2);

  addSysVar_html(F("Wind Dir.:    %c_w_dir%(123.4)"));
  addSysVar_html(F("{D}C to {D}F: %c_c2f%(20.4)"));
  addSysVar_html(F("m/s to Bft:   %c_ms2Bft%(5.1)"));
  addSysVar_html(F("Dew point(T,H): %c_dew_th%(18.6,67)"));
  addFormSeparator(3);
  addSysVar_html(F("cm to imperial: %c_cm2imp%(190)"));
  addSysVar_html(F("mm to imperial: %c_mm2imp%(1900)"));
  addFormSeparator(3);
  addSysVar_html(F("Mins to days: %c_m2day%(1900)"));
  addSysVar_html(F("Mins to dh:   %c_m2dh%(1900)"));
  addSysVar_html(F("Mins to dhm:  %c_m2dhm%(1900)"));
  addSysVar_html(F("Secs to dhms: %c_s2dhms%(100000)"));

  html_end_table();
  html_end_form();
  sendHeadandTail_stdtemplate(true);
  TXBuffer.endStream();
}

//********************************************************************************
// URNEncode char string to string object
//********************************************************************************
String URLEncode(const char* msg)
{
  const char *hex = "0123456789abcdef";
  String encodedMsg = "";

  while (*msg != '\0') {
    if ( ('a' <= *msg && *msg <= 'z')
         || ('A' <= *msg && *msg <= 'Z')
         || ('0' <= *msg && *msg <= '9')
         || ('-' == *msg) || ('_' == *msg)
         || ('.' == *msg) || ('~' == *msg) ) {
      encodedMsg += *msg;
    } else {
      encodedMsg += '%';
      encodedMsg += hex[*msg >> 4];
      encodedMsg += hex[*msg & 15];
    }
    msg++;
  }
  return encodedMsg;
}


String getControllerSymbol(byte index)
{
  String ret = F("<p style='font-size:20px'>&#");
  ret += 10102 + index;
  ret += F(";</p>");
  return ret;
}

String getValueSymbol(byte index)
{
  String ret = F("&#");
  ret += 10112 + index;
  ret += ';';
  return ret;
}


void handle_favicon() {
  checkRAM(F("handle_favicon"));
  WebServer.send_P(200, PSTR("image/x-icon"), favicon_8b_ico, favicon_8b_ico_len);
}

void createSvgRectPath(unsigned int color, int xoffset, int yoffset, int size, int height, int range, float SVG_BAR_WIDTH) {
  float width = SVG_BAR_WIDTH * size / range;
  if (width < 2) width = 2;
  TXBuffer += formatToHex(color, F("<path fill=\"#"));
  TXBuffer += F("\" d=\"M");
  TXBuffer += toString(SVG_BAR_WIDTH * xoffset / range, 2);
  TXBuffer += ' ';
  TXBuffer += yoffset;
  TXBuffer += 'h';
  TXBuffer += toString(width, 2);
  TXBuffer += 'v';
  TXBuffer += height;
  TXBuffer += 'H';
  TXBuffer += toString(SVG_BAR_WIDTH * xoffset / range, 2);
  TXBuffer += F("z\"/>\n");
}

void createSvgTextElement(const String& text, float textXoffset, float textYoffset) {
  TXBuffer += F("<text style=\"line-height:1.25\" x=\"");
  TXBuffer += toString(textXoffset, 2);
  TXBuffer += F("\" y=\"");
  TXBuffer += toString(textYoffset, 2);
  TXBuffer += F("\" stroke-width=\".3\" font-family=\"sans-serif\" font-size=\"8\" letter-spacing=\"0\" word-spacing=\"0\">\n");
  TXBuffer += F("<tspan x=\"");
  TXBuffer += toString(textXoffset, 2);
  TXBuffer += F("\" y=\"");
  TXBuffer += toString(textYoffset, 2);
  TXBuffer += "\">";
  TXBuffer += text;
  TXBuffer += F("</tspan>\n</text>");
}

unsigned int getSettingsTypeColor(SettingsType settingsType) {
  switch (settingsType) {
    case BasicSettings_Type:
      return 0x5F0A87;
    case TaskSettings_Type:
      return 0xEE6352;
    case CustomTaskSettings_Type:
      return 0x59CD90;
    case ControllerSettings_Type:
      return 0x3FA7D6;
    case CustomControllerSettings_Type:
      return 0xFAC05E;
    case NotificationSettings_Type:
      return 0xF79D84;
    default:
      break;
  }
  return 0;
}

#define SVG_BAR_HEIGHT 16
#define SVG_BAR_WIDTH 400

void write_SVG_image_header(int width, int height) {
  write_SVG_image_header(width, height, false);
}

void write_SVG_image_header(int width, int height, bool useViewbox) {
  TXBuffer += F("<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"");
  TXBuffer += width;
  TXBuffer += F("\" height=\"");
  TXBuffer += height;
  TXBuffer += F("\" version=\"1.1\"");
  if (useViewbox) {
    TXBuffer += F(" viewBox=\"0 0 100 100\"");
  }
  TXBuffer += '>';
}

/*
void getESPeasyLogo(int width_pixels) {
  write_SVG_image_header(width_pixels, width_pixels, true);
  TXBuffer += F("<g transform=\"translate(-33.686 -7.8142)\">");
  TXBuffer += F("<rect x=\"49\" y=\"23.1\" width=\"69.3\" height=\"69.3\" fill=\"#2c72da\" stroke=\"#2c72da\" stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-width=\"30.7\"/>");
  TXBuffer += F("<g transform=\"matrix(3.3092 0 0 3.3092 -77.788 -248.96)\">");
  TXBuffer += F("<path d=\"m37.4 89 7.5-7.5M37.4 96.5l15-15M37.4 96.5l15-15M37.4 104l22.5-22.5M44.9 104l15-15\" fill=\"none\" stroke=\"#fff\" stroke-linecap=\"round\" stroke-width=\"2.6\"/>");
  TXBuffer += F("<circle cx=\"58\" cy=\"102.1\" r=\"3\" fill=\"#fff\"/>");
  TXBuffer += F("</g></g></svg>");
}
*/

#ifndef BUILD_MINIMAL_OTA
void getConfig_dat_file_layout() {
  const int shiftY = 2;
  float yOffset = shiftY;
  write_SVG_image_header(SVG_BAR_WIDTH + 250, SVG_BAR_HEIGHT + shiftY);

  int max_index, offset, max_size;
  int struct_size = 0;

  // background
  const uint32_t realSize = getFileSize(TaskSettings_Type);
  createSvgRectPath(0xcdcdcd, 0, yOffset, realSize, SVG_BAR_HEIGHT - 2, realSize, SVG_BAR_WIDTH);

  for (int st = 0; st < SettingsType_MAX; ++st) {
    SettingsType settingsType = static_cast<SettingsType>(st);
    if (settingsType != NotificationSettings_Type) {
      unsigned int color = getSettingsTypeColor(settingsType);
      getSettingsParameters(settingsType, 0, max_index, offset, max_size, struct_size);
      for (int i = 0; i < max_index; ++i) {
        getSettingsParameters(settingsType, i, offset, max_size);
        // Struct position
        createSvgRectPath(color, offset, yOffset, max_size, SVG_BAR_HEIGHT - 2, realSize, SVG_BAR_WIDTH);
      }
    }
  }
  // Text labels
  float textXoffset = SVG_BAR_WIDTH + 2;
  float textYoffset = yOffset + 0.9 * SVG_BAR_HEIGHT;
  createSvgTextElement(F("Config.dat"), textXoffset, textYoffset);
  TXBuffer += F("</svg>\n");
}

void getStorageTableSVG(SettingsType settingsType) {
  uint32_t realSize = getFileSize(settingsType);
  unsigned int color = getSettingsTypeColor(settingsType);
  const int shiftY = 2;

  int max_index, offset, max_size;
  int struct_size = 0;
  getSettingsParameters(settingsType, 0, max_index, offset, max_size, struct_size);
  if (max_index == 0) return;
  // One more to add bar indicating struct size vs. reserved space.
  write_SVG_image_header(SVG_BAR_WIDTH + 250, (max_index + 1) * SVG_BAR_HEIGHT + shiftY);
  float yOffset = shiftY;
  for (int i = 0; i < max_index; ++i) {
    getSettingsParameters(settingsType, i, offset, max_size);
    // background
    createSvgRectPath(0xcdcdcd, 0, yOffset, realSize, SVG_BAR_HEIGHT - 2, realSize, SVG_BAR_WIDTH);
    // Struct position
    createSvgRectPath(color, offset, yOffset, max_size, SVG_BAR_HEIGHT - 2, realSize, SVG_BAR_WIDTH);
    // Text labels
    float textXoffset = SVG_BAR_WIDTH + 2;
    float textYoffset = yOffset + 0.9 * SVG_BAR_HEIGHT;
    createSvgTextElement(formatHumanReadable(offset, 1024), textXoffset, textYoffset);
    textXoffset = SVG_BAR_WIDTH + 60;
    createSvgTextElement(formatHumanReadable(max_size, 1024), textXoffset, textYoffset);
    textXoffset = SVG_BAR_WIDTH + 130;
    createSvgTextElement(String(i), textXoffset, textYoffset);
    yOffset += SVG_BAR_HEIGHT;
  }
  // usage
  createSvgRectPath(0xcdcdcd, 0, yOffset, max_size, SVG_BAR_HEIGHT - 2, max_size, SVG_BAR_WIDTH);
  // Struct size (used part of the reserved space)
  if (struct_size != 0) {
    createSvgRectPath(color, 0, yOffset, struct_size, SVG_BAR_HEIGHT - 2, max_size, SVG_BAR_WIDTH);
  }
  // Text labels
  float textXoffset = SVG_BAR_WIDTH + 2;
  float textYoffset = yOffset + 0.9 * SVG_BAR_HEIGHT;
  if (struct_size != 0) {
    String text = formatHumanReadable(struct_size, 1024);
    text += '/';
    text += formatHumanReadable(max_size, 1024);
    text += F(" per item");
    createSvgTextElement(text, textXoffset, textYoffset);
  } else {
    createSvgTextElement(F("Variable size"), textXoffset, textYoffset);
  }
  TXBuffer += F("</svg>\n");
}
#endif

#ifdef ESP32


int getPartionCount(byte pType) {
  esp_partition_type_t partitionType = static_cast<esp_partition_type_t>(pType);
  esp_partition_iterator_t _mypartiterator = esp_partition_find(partitionType, ESP_PARTITION_SUBTYPE_ANY, NULL);
  int nrPartitions = 0;
  if (_mypartiterator) {
    do {
      ++nrPartitions;
    } while ((_mypartiterator = esp_partition_next(_mypartiterator)) != NULL);
  }
  esp_partition_iterator_release(_mypartiterator);
  return nrPartitions;
}

void getPartitionTableSVG(byte pType, unsigned int partitionColor) {
  int nrPartitions = getPartionCount(pType);
  if (nrPartitions == 0) return;
  const int shiftY = 2;

  uint32_t realSize = getFlashRealSizeInBytes();
  esp_partition_type_t partitionType = static_cast<esp_partition_type_t>(pType);
  const esp_partition_t * _mypart;
  esp_partition_iterator_t _mypartiterator = esp_partition_find(partitionType, ESP_PARTITION_SUBTYPE_ANY, NULL);
  write_SVG_image_header(SVG_BAR_WIDTH + 250, nrPartitions * SVG_BAR_HEIGHT + shiftY);
  float yOffset = shiftY;
  if (_mypartiterator) {
    do {
      _mypart = esp_partition_get(_mypartiterator);
      createSvgRectPath(0xcdcdcd, 0, yOffset, realSize, SVG_BAR_HEIGHT - 2, realSize, SVG_BAR_WIDTH);
      createSvgRectPath(partitionColor, _mypart->address, yOffset, _mypart->size, SVG_BAR_HEIGHT - 2, realSize, SVG_BAR_WIDTH);
      float textXoffset = SVG_BAR_WIDTH + 2;
      float textYoffset = yOffset + 0.9 * SVG_BAR_HEIGHT;
      createSvgTextElement(formatHumanReadable(_mypart->size, 1024), textXoffset, textYoffset);
      textXoffset = SVG_BAR_WIDTH + 60;
      createSvgTextElement(_mypart->label, textXoffset, textYoffset);
      textXoffset = SVG_BAR_WIDTH + 130;
      createSvgTextElement(getPartitionType(_mypart->type, _mypart->subtype), textXoffset, textYoffset);
      yOffset += SVG_BAR_HEIGHT;
    } while ((_mypartiterator = esp_partition_next(_mypartiterator)) != NULL);
  }
  TXBuffer += F("</svg>\n");
  esp_partition_iterator_release(_mypartiterator);
}

#endif
