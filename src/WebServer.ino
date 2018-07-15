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
void sendHeaderBlocking(bool json);

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
    startStream(false);
  }

  void startJsonStream() {
    startStream(true);
  }

private:
  void startStream(bool json) {
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
      sendHeaderBlocking(json);
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
      String log = String("Ram usage: Webserver only: ") + maxServerUsage +
                   " including Core: " + maxCoreUsage +
                   " flashStringCalls: " + flashStringCalls +
                   " flashStringData: " + flashStringData;
      addLog(LOG_LEVEL_DEBUG, log);
      */
    } else {
      String log = String("Webpage skipped: low memory: ") + finalRam;
      addLog(LOG_LEVEL_DEBUG, log);
      lowMemorySkip = false;
    }
  }
} TXBuffer;

void sendContentBlocking(String& data) {
  checkRAM(F("sendContentBlocking"));
  uint32_t freeBeforeSend = ESP.getFreeHeap();
  const uint32_t length = data.length();
  String log = String("sendcontent free: ") + freeBeforeSend + " chunk size:" + length;
  addLog(LOG_LEVEL_DEBUG_DEV, log);
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
  yield();
}

void sendHeaderBlocking(bool json) {
  checkRAM(F("sendHeaderBlocking"));
  WebServer.client().flush();
#if defined(ESP8266) && defined(ARDUINO_ESP8266_RELEASE_2_3_0)
  WebServer.setContentLength(CONTENT_LENGTH_UNKNOWN);
  WebServer.sendHeader(F("Content-Type"), json ? F("application/json") : F("text/html"), true);
  WebServer.sendHeader(F("Accept-Ranges"), F("none"));
  WebServer.sendHeader(F("Cache-Control"), F("no-cache"));
  WebServer.sendHeader(F("Transfer-Encoding"), F("chunked"));
  if (json)
    WebServer.sendHeader(F("Access-Control-Allow-Origin"),"*");
  WebServer.send(200);
#else
  unsigned int timeout = 0;
  uint32_t freeBeforeSend = ESP.getFreeHeap();
  if (freeBeforeSend < 5000) timeout = 100;
  if (freeBeforeSend < 4000) timeout = 1000;
  const uint32_t beginWait = millis();
  WebServer.setContentLength(CONTENT_LENGTH_UNKNOWN);
  WebServer.sendHeader(F("Content-Type"), json ? F("application/json") : F("text/html"), true);
  WebServer.sendHeader(F("Cache-Control"), F("no-cache"));
  if (json)
    WebServer.sendHeader(F("Access-Control-Allow-Origin"),"*");
  WebServer.send(200);
  // dont wait on 2.3.0. Memory returns just too slow.
  while ((ESP.getFreeHeap() < freeBeforeSend) &&
         !timeOutReached(beginWait + timeout)) {
    checkRAM(F("duringHeaderTX"));
    delay(1);
  }
#endif
  yield();
}

void sendHeadandTail(const String& tmplName, boolean Tail = false) {
  String pageTemplate = "";
  int indexStart, indexEnd;
  String varName;  //, varValue;
  String fileName = tmplName;
  fileName += F(".htm");
  fs::File f = SPIFFS.open(fileName, "r+");

  if (f) {
    pageTemplate.reserve(f.size());
    while (f.available()) pageTemplate += (char)f.read();
    f.close();
  } else {
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
    while ((indexStart = pageTemplate.indexOf(F("{{"))) >= 0) {
      TXBuffer += pageTemplate.substring(0, indexStart);
      pageTemplate = pageTemplate.substring(indexStart);
      if ((indexEnd = pageTemplate.indexOf(F("}}"))) > 0) {
        varName = pageTemplate.substring(2, indexEnd);
        pageTemplate = pageTemplate.substring(indexEnd + 2);
        varName.toLowerCase();

        if (varName == F("content")) {  // is var == page content?
          break;  // send first part of result only
        } else if (varName == F("error")) {
          getErrorNotifications();
        } else {
          getWebPageTemplateVar(varName);
        }
      } else {  // no closing "}}"
        pageTemplate = pageTemplate.substring(2);  // eat "{{"
      }
    }
  }
  if (shouldReboot) {
    //we only add this here as a seperate chucnk to prevent using too much memory at once
    TXBuffer += jsReboot;
  }
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
  response += F(" Allowed: ");
  response += formatIP(low);
  response += F(" - ");
  response += formatIP(high);
  addLog(LOG_LEVEL_ERROR, response);
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
void addHtmlError(String error){
  if (error.length()>0)
  {
    TXBuffer += F("<div class=\"alert\"><span class=\"closebtn\" onclick=\"this.parentElement.style.display='none';\">&times;</span>");
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

void addHtml(const String html) {
  TXBuffer += html;
}

void WebServerInit()
{
  // Prepare webserver pages
  WebServer.on(F("/"), handle_root);
  WebServer.on(F("/config"), handle_config);
  WebServer.on(F("/controllers"), handle_controllers);
  WebServer.on(F("/hardware"), handle_hardware);
  WebServer.on(F("/devices"), handle_devices);
  WebServer.on(F("/notifications"), handle_notifications);
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
  WebServer.on(F("/rules"), handle_rules);
  WebServer.on(F("/sysinfo"), handle_sysinfo);
  WebServer.on(F("/pinstates"), handle_pinstates);
  WebServer.on(F("/favicon.ico"), handle_favicon);

  #if defined(ESP8266)
    if (getFlashRealSizeInBytes() > 524288)
      httpUpdater.setup(&WebServer);
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
  if (tmplName == F("TmplAP"))
  {
    tmpl += F(
              "<!DOCTYPE html><html lang='en'>"
              "<head>"
              "<meta charset='utf-8'/>"
              "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
              "<title>{{name}}</title>"
              "{{css}}"
              "</head>"
              "<body>"
              "<header class='apheader'>"
              "<h1>Welcome to ESP Easy Mega AP</h1>"
              "</header>"
              "<section>"
              "<span class='message error'>"
              "{{error}}"
              "</span>"
              "{{content}}"
              "</section>"
              "<footer>"
                "<br>"
                "<h6>Powered by <a href='http://www.letscontrolit.com' style='font-size: 15px; text-decoration: none'>www.letscontrolit.com</a></h6>"
              "</footer>"
              "</body>"            );
  }
  else if (tmplName == F("TmplMsg"))
  {
    tmpl += F(
              "<!DOCTYPE html><html lang='en'>"
              "<head>"
              "<meta charset='utf-8'/>"
              "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
              "<title>{{name}}</title>"
              "{{css}}"
              "</head>"
              "<body>"
              "<header class='headermenu'>"
              "<h1>ESP Easy Mega: {{name}}</h1><div class='menu_button'>&#9776;</div><BR>"
              "</header>"
              "<section>"
              "<span class='message error'>"
              "{{error}}"
              "</span>"
              "{{content}}"
              "</section>"
              "<footer>"
                "<br>"
                "<h6>Powered by <a href='http://www.letscontrolit.com' style='font-size: 15px; text-decoration: none'>www.letscontrolit.com</a></h6>"
              "</footer>"
              "</body>"
            );
  }
  else if (tmplName == F("TmplDsh"))
  {
    tmpl += F(
      "<!DOCTYPE html><html lang='en'>"
      "<head>"
        "<meta charset='utf-8'/>"
        "<title>{{name}}</title>"
        "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
        "{{js}}"
        "{{css}}"
        "</head>"
        "<body>"
        "{{content}}"
        "</body></html>"
            );
  }
  else   //all other template names e.g. TmplStd
  {
    tmpl += F(
      "<!DOCTYPE html><html lang='en'>"
      "<head>"
        "<meta charset='utf-8'/>"
        "<title>{{name}}</title>"
        "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
        "{{js}}"
        "{{css}}"
      "</head>"
      "<body class='bodymenu'>"
        "<span class='message' id='rbtmsg'></span>"
        "<header class='headermenu'>"
          "<h1>ESP Easy Mega: {{name}} {{logo}}</h1><div class='menu_button'>&#9776;</div><BR>"
          "{{menu}}"
        "</header>"
        "<section>"
        "<span class='message error'>"
        "{{error}}"
        "</span>"
        "{{content}}"
        "</section>"
        "<footer>"
          "<br>"
          "<h6>Powered by <a href='http://www.letscontrolit.com' style='font-size: 15px; text-decoration: none'>www.letscontrolit.com</a></h6>"
        "</footer>"
      "</body></html>"
            );
  }
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


static byte navMenuIndex = 0;

void getWebPageTemplateVar(const String& varName )
{
 // Serial.print(varName); Serial.print(" : free: "); Serial.print(ESP.getFreeHeap());   Serial.print("var len before:  "); Serial.print (varValue.length()) ;Serial.print("after:  ");
 //varValue = F("");

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
    static const __FlashStringHelper* gpMenu[8][2] = {
      F("Main"), F("."),                      //0
      F("Config"), F("config"),               //1
      F("Controllers"), F("controllers"),     //2
      F("Hardware"), F("hardware"),           //3
      F("Devices"), F("devices"),             //4
      F("Rules"), F("rules"),                 //5
      F("Notifications"), F("notifications"), //6
      F("Tools"), F("tools"),                 //7
    };

    TXBuffer += F("<div class='menubar'>");

    for (byte i = 0; i < 8; i++)
    {
      if (i == 5 && !Settings.UseRules)   //hide rules menu item
        continue;

      TXBuffer += F("<a class='menu");
      if (i == navMenuIndex)
        TXBuffer += F(" active");
      TXBuffer += F("' href='");
      TXBuffer += gpMenu[i][1];
      TXBuffer += F("'>");
      TXBuffer += gpMenu[i][0];
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
      TXBuffer += pgDefaultCSS;
      TXBuffer += F("</style>");
    }
  }


  else if (varName == F("js"))
  {
    TXBuffer += F(
                  "<script><!--\n"
                  "function dept_onchange(frmselect) {frmselect.submit();}"
                  "\n//--></script>");
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
    String log = F("Templ: Unknown Var : ");
    log += varName;
    addLog(LOG_LEVEL_ERROR, log);
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
      String log = F("CSS  : Writing default CSS file to SPIFFS (");
      log += defaultCSS.length();
      log += F(" bytes)");
      addLog(LOG_LEVEL_INFO, log);
      defaultCSS= PGMT(pgDefaultCSS);
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
void addFooter(String& str)
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
   sendHeadandTail(F("TmplStd"),_HEAD);

  int freeMem = ESP.getFreeHeap();
  String sCommand = WebServer.arg(F("cmd"));

  if ((strcasecmp_P(sCommand.c_str(), PSTR("wifidisconnect")) != 0) && (strcasecmp_P(sCommand.c_str(), PSTR("reboot")) != 0)&& (strcasecmp_P(sCommand.c_str(), PSTR("reset")) != 0))
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
    TXBuffer += F("<table class='normal'><TH style='width:150px;' align='left'>System Info<TH align='left'>Value");

    TXBuffer += F("<TR><TD>Unit:<TD>");
    TXBuffer += String(Settings.Unit);

    TXBuffer += F("<TR><TD>GIT version:<TD>");
    TXBuffer += BUILD_GIT;

    TXBuffer += F("<TR><TD>Local Time:<TD>");
    if (Settings.UseNTP)
    {
      TXBuffer += getDateTimeString('-', ':', ' ');
    }
    else
      TXBuffer += F("<font color='red'>NTP disabled</font>");

    TXBuffer += F("<TR><TD>Uptime:<TD>");
    char strUpTime[40];
    int minutes = wdcounter / 2;
    int days = minutes / 1440;
    minutes = minutes % 1440;
    int hrs = minutes / 60;
    minutes = minutes % 60;
    sprintf_P(strUpTime, PSTR("%d days %d hours %d minutes"), days, hrs, minutes);
    TXBuffer += strUpTime;

    TXBuffer += F("<TR><TD>Load:<TD>");
    if (wdcounter > 0)
    {
      TXBuffer += String(getCPUload());
      TXBuffer += F("% (LC=");
      TXBuffer += String(getLoopCountPerSec());
      TXBuffer += F(")");
    }

    TXBuffer += F("<TR><TD>Free Mem:<TD>");
    TXBuffer += String(freeMem);
    TXBuffer += F(" (");
    TXBuffer += String(lowestRAM);
    TXBuffer += F(" - ");
    TXBuffer += String(lowestRAMfunction);
    TXBuffer += F(")");

    TXBuffer += F("<TR><TD>IP:<TD>");
    TXBuffer += formatIP(ip);

    TXBuffer += F("<TR><TD>Wifi RSSI:<TD>");
    if (wifiStatus == ESPEASY_WIFI_SERVICES_INITIALIZED)
    {
      TXBuffer += String(WiFi.RSSI());
      TXBuffer += F(" dB");
    }

    #ifdef FEATURE_MDNS
      TXBuffer += F("<TR><TD>mDNS:<TD><a href='http://");
      TXBuffer += WifiGetHostname();
      TXBuffer += F(".local'>");
      TXBuffer += WifiGetHostname();
      TXBuffer += F(".local</a><TD><TD><TD>");
    #endif
    TXBuffer += F("<TR><TD><TD>");
    addButton(F("sysinfo"), F("More info"));

    TXBuffer += F("</table><BR><BR><table class='multirow'><TR><TH>Node List:<TH>Name<TH>Build<TH>Type<TH>IP<TH>Age");
    for (byte x = 0; x < UNIT_MAX; x++)
    {
      if (Nodes[x].ip[0] != 0)
      {
        char url[80];
        sprintf_P(url, PSTR("<a class='button link' href='http://%u.%u.%u.%u'>%u.%u.%u.%u</a>"), Nodes[x].ip[0], Nodes[x].ip[1], Nodes[x].ip[2], Nodes[x].ip[3], Nodes[x].ip[0], Nodes[x].ip[1], Nodes[x].ip[2], Nodes[x].ip[3]);
        TXBuffer += F("<TR><TD>Unit ");
        TXBuffer += String(x);
        TXBuffer += F("<TD>");
        if (x != Settings.Unit)
          TXBuffer += Nodes[x].nodeName;
        else
          TXBuffer += Settings.Name;
        TXBuffer += F("<TD>");
        if (Nodes[x].build)
          TXBuffer += String(Nodes[x].build);
        TXBuffer += F("<TD>");
        if (Nodes[x].nodeType)
          switch (Nodes[x].nodeType)
          {
            case NODE_TYPE_ID_ESP_EASY_STD:
              TXBuffer += F("ESP Easy");
              break;
            case NODE_TYPE_ID_ESP_EASYM_STD:
              TXBuffer += F("ESP Easy Mega");
              break;
            case NODE_TYPE_ID_ESP_EASY32_STD:
              TXBuffer += F("ESP Easy 32");
              break;
            case NODE_TYPE_ID_ARDUINO_EASY_STD:
              TXBuffer += F("Arduino Easy");
              break;
            case NODE_TYPE_ID_NANO_EASY_STD:
              TXBuffer += F("Nano Easy");
              break;
          }
        TXBuffer += F("<TD>");
        TXBuffer += url;
        TXBuffer += F("<TD>");
        TXBuffer += String( Nodes[x].age);
      }
    }

    TXBuffer += F("</table></form>");

    printWebString = "";
    printToWeb = false;
    sendHeadandTail(F("TmplStd"),_TAIL);
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
      String log = F("WIFI : Disconnecting...");
      addLog(LOG_LEVEL_INFO, log);
      cmd_within_mainloop = CMD_WIFI_DISCONNECT;
    }

    if (strcasecmp_P(sCommand.c_str(), PSTR("reboot")) == 0)
    {
      String log = F("     : Rebooting...");
      addLog(LOG_LEVEL_INFO, log);
      cmd_within_mainloop = CMD_REBOOT;
    }
   if (strcasecmp_P(sCommand.c_str(), PSTR("reset")) == 0)
    {
      String log = F("     : factory reset...");
      addLog(LOG_LEVEL_INFO, log);
      cmd_within_mainloop = CMD_REBOOT;
      TXBuffer+= F("OK. Please wait > 1 min and connect to Acces point.<BR><BR>PW=configesp<BR>URL=<a href='http://192.168.4.1'>192.168.4.1</a>");
      TXBuffer.endStream();
      ExecuteCommand(VALUE_SOURCE_HTTP, sCommand.c_str());
    }

    TXBuffer+= "OK";
    TXBuffer.endStream();

  }
}


//********************************************************************************
// Web Interface config page
//********************************************************************************
void handle_config() {

   checkRAM(F("handle_config"));
   if (!isLoggedIn()) return;

   navMenuIndex = 1;
   TXBuffer.startStream();
   sendHeadandTail(F("TmplStd"),_HEAD);

  if (timerAPoff)
    timerAPoff = millis() + 2000L;  //user has reached the main page - AP can be switched off in 2..3 sec


  String name = WebServer.arg(F("name"));
  //String password = WebServer.arg(F("password"));
  String ssid = WebServer.arg(F("ssid"));
  //String key = WebServer.arg(F("key"));
  String ssid2 = WebServer.arg(F("ssid2"));
  //String key2 = WebServer.arg(F("key2"));
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


  if (ssid[0] != 0)
  {
    if (strcmp(Settings.Name, name.c_str()) != 0) {
      addLog(LOG_LEVEL_INFO, F("Unit Name changed."));
      MQTTclient_should_reconnect = true;
    }
    strncpy(Settings.Name, name.c_str(), sizeof(Settings.Name));
    //strncpy(SecuritySettings.Password, password.c_str(), sizeof(SecuritySettings.Password));
    copyFormPassword(F("password"), SecuritySettings.Password, sizeof(SecuritySettings.Password));
    strncpy(SecuritySettings.WifiSSID, ssid.c_str(), sizeof(SecuritySettings.WifiSSID));
    //strncpy(SecuritySettings.WifiKey, key.c_str(), sizeof(SecuritySettings.WifiKey));
    copyFormPassword(F("key"), SecuritySettings.WifiKey, sizeof(SecuritySettings.WifiKey));
    strncpy(SecuritySettings.WifiSSID2, ssid2.c_str(), sizeof(SecuritySettings.WifiSSID2));
    //strncpy(SecuritySettings.WifiKey2, key2.c_str(), sizeof(SecuritySettings.WifiKey2));
    copyFormPassword(F("key2"), SecuritySettings.WifiKey2, sizeof(SecuritySettings.WifiKey2));
    //strncpy(SecuritySettings.WifiAPKey, apkey.c_str(), sizeof(SecuritySettings.WifiAPKey));
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

  TXBuffer += F("<form name='frmselect' method='post'><table class='normal'>");

  addFormHeader(F("Main Settings"));

  Settings.Name[25] = 0;
  SecuritySettings.Password[25] = 0;
  addFormTextBox( F("Unit Name"), F("name"), Settings.Name, 25);
  addFormNumericBox( F("Unit Number"), F("unit"), Settings.Unit, 0, 9999);
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

  addFormNumericBox( F("Sleep time"), F("delay"), Settings.Delay, 0, 4294);   //limited by hardware to ~1.2h
  addUnit(F("sec"));

  addFormCheckBox(F("Sleep on connection failure"), F("deepsleeponfail"), Settings.deepSleepOnFail);

  addFormSeparator(2);

  TXBuffer += F("<TR><TD style='width:150px;' align='left'><TD>");
  addSubmitButton();
  TXBuffer += F("</table></form>");

  sendHeadandTail(F("TmplStd"),_TAIL);
  TXBuffer.endStream();
}


//********************************************************************************
// Web Interface controller page
//********************************************************************************
void handle_controllers() {
  checkRAM(F("handle_controllers"));
  if (!isLoggedIn()) return;
  navMenuIndex = 2;
  TXBuffer.startStream();
  sendHeadandTail(F("TmplStd"),_HEAD);

  struct EventStruct TempEvent;

  byte controllerindex = getFormItemInt(F("index"), 0);
  boolean controllerNotSet = controllerindex == 0;
  --controllerindex;

  String usedns = WebServer.arg(F("usedns"));
  String controllerip = WebServer.arg(F("controllerip"));
  String controllerhostname = WebServer.arg(F("controllerhostname"));
  const int controllerport = getFormItemInt(F("controllerport"), 0);
  const int protocol = getFormItemInt(F("protocol"), -1);
  String controlleruser = WebServer.arg(F("controlleruser"));
  String controllerpassword = WebServer.arg(F("controllerpassword"));
  String controllersubscribe = WebServer.arg(F("controllersubscribe"));
  String controllerpublish = WebServer.arg(F("controllerpublish"));
  String MQTTLwtTopic = WebServer.arg(F("mqttlwttopic"));
  String lwtmessageconnect = WebServer.arg(F("lwtmessageconnect"));
  String lwtmessagedisconnect = WebServer.arg(F("lwtmessagedisconnect"));


  //submitted data
  if (protocol != -1 && !controllerNotSet)
  {
    ControllerSettingsStruct ControllerSettings;
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
        if (Protocol[ProtocolIndex].usesTemplate)
          CPlugin_ptr[ProtocolIndex](CPLUGIN_PROTOCOL_TEMPLATE, &TempEvent, dummyString);
        strncpy(ControllerSettings.Subscribe, TempEvent.String1.c_str(), sizeof(ControllerSettings.Subscribe));
        strncpy(ControllerSettings.Publish, TempEvent.String2.c_str(), sizeof(ControllerSettings.Publish));
        strncpy(ControllerSettings.MQTTLwtTopic, TempEvent.String3.c_str(), sizeof(ControllerSettings.MQTTLwtTopic));
        strncpy(ControllerSettings.LWTMessageConnect, TempEvent.String4.c_str(), sizeof(ControllerSettings.LWTMessageConnect));
        strncpy(ControllerSettings.LWTMessageDisconnect, TempEvent.String5.c_str(), sizeof(ControllerSettings.LWTMessageDisconnect));
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
        CPlugin_ptr[ProtocolIndex](CPLUGIN_WEBFORM_SAVE, &TempEvent, dummyString);
        ControllerSettings.UseDNS = usedns.toInt();
        if (ControllerSettings.UseDNS)
        {
          strncpy(ControllerSettings.HostName, controllerhostname.c_str(), sizeof(ControllerSettings.HostName));
          IPAddress IP;
          WiFi.hostByName(ControllerSettings.HostName, IP);
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
        strncpy(SecuritySettings.ControllerUser[controllerindex], controlleruser.c_str(), sizeof(SecuritySettings.ControllerUser[0]));
        //strncpy(SecuritySettings.ControllerPassword[controllerindex], controllerpassword.c_str(), sizeof(SecuritySettings.ControllerPassword[0]));
        copyFormPassword(F("controllerpassword"), SecuritySettings.ControllerPassword[controllerindex], sizeof(SecuritySettings.ControllerPassword[0]));
        strncpy(ControllerSettings.Subscribe, controllersubscribe.c_str(), sizeof(ControllerSettings.Subscribe));
        strncpy(ControllerSettings.Publish, controllerpublish.c_str(), sizeof(ControllerSettings.Publish));
        strncpy(ControllerSettings.MQTTLwtTopic, MQTTLwtTopic.c_str(), sizeof(ControllerSettings.MQTTLwtTopic));
        strncpy(ControllerSettings.LWTMessageConnect, lwtmessageconnect.c_str(), sizeof(ControllerSettings.LWTMessageConnect));
        strncpy(ControllerSettings.LWTMessageDisconnect, lwtmessagedisconnect.c_str(), sizeof(ControllerSettings.LWTMessageDisconnect));

        CPlugin_ptr[ProtocolIndex](CPLUGIN_INIT, &TempEvent, dummyString);
      }
    }
    addHtmlError(SaveControllerSettings(controllerindex, (byte*)&ControllerSettings, sizeof(ControllerSettings)));
    addHtmlError(SaveSettings());
  }

  TXBuffer += F("<form name='frmselect' method='post'>");

  if (controllerNotSet)
  {
    TXBuffer += F("<table class='multirow' border=1px frame='box' rules='all'><TR><TH style='width:70px;'>");
    TXBuffer += F("<TH style='width:50px;'>Nr<TH style='width:100px;'>Enabled<TH>Protocol<TH>Host<TH>Port");

    ControllerSettingsStruct ControllerSettings;
    for (byte x = 0; x < CONTROLLER_MAX; x++)
    {
      LoadControllerSettings(x, (byte*)&ControllerSettings, sizeof(ControllerSettings));
      TXBuffer += F("<TR><TD>");
      TXBuffer += F("<a class='button link' href=\"controllers?index=");
      TXBuffer += x + 1;
      TXBuffer += F("\">Edit</a>");
      TXBuffer += F("<TD>");
      TXBuffer += getControllerSymbol(x);
      TXBuffer += F("<TD>");
      if (Settings.Protocol[x] != 0)
      {
        addEnabled(Settings.ControllerEnabled[x]);

        TXBuffer += F("<TD>");
        byte ProtocolIndex = getProtocolIndex(Settings.Protocol[x]);
        String ProtocolName = "";
        CPlugin_ptr[ProtocolIndex](CPLUGIN_GET_DEVICENAME, 0, ProtocolName);
        TXBuffer += ProtocolName;

        TXBuffer += F("<TD>");
        TXBuffer += ControllerSettings.getHost();
        TXBuffer += F("<TD>");
        TXBuffer += ControllerSettings.Port;
      }
      else
        TXBuffer += F("<TD><TD><TD>");
    }
    TXBuffer += F("</table></form>");
  }
  else
  {
    TXBuffer += F("<table class='normal'><TR><TH style='width:150px;' align='left'>Controller Settings<TH>");
    TXBuffer += F("<TR><TD>Protocol:");
    byte choice = Settings.Protocol[controllerindex];
    TXBuffer += F("<TD>");
    addSelector_Head(F("protocol"), true);
    addSelector_Item(F("- Standalone -"), 0, false, false, F(""));
    for (byte x = 0; x <= protocolCount; x++)
    {
      String ProtocolName = "";
      CPlugin_ptr[x](CPLUGIN_GET_DEVICENAME, 0, ProtocolName);
      boolean disabled = false;// !((controllerindex == 0) || !Protocol[x].usesMQTT);
      addSelector_Item(ProtocolName,
                       Protocol[x].Number,
                       choice == Protocol[x].Number,
                       disabled,
                       F(""));
    }
    addSelector_Foot();

    addHelpButton(F("EasyProtocols"));
      // char str[20];

    if (Settings.Protocol[controllerindex])
    {
      ControllerSettingsStruct ControllerSettings;
      LoadControllerSettings(controllerindex, (byte*)&ControllerSettings, sizeof(ControllerSettings));
      byte choice = ControllerSettings.UseDNS;
      String options[2];
      options[0] = F("Use IP address");
      options[1] = F("Use Hostname");

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
      CPlugin_ptr[ProtocolIndex](CPLUGIN_WEBFORM_LOAD, &TempEvent,TXBuffer.buf);

    }

    addFormSeparator(2);

    TXBuffer += F("<TR><TD><TD><a class='button link' href=\"controllers\">Close</a>");
    addSubmitButton();
    TXBuffer += F("</table></form>");
  }

  sendHeadandTail(F("TmplStd"),_TAIL);
  TXBuffer.endStream();
}


//********************************************************************************
// Web Interface notifcations page
//********************************************************************************
void handle_notifications() {
  checkRAM(F("handle_notifications"));
  if (!isLoggedIn()) return;
  navMenuIndex = 6;
  TXBuffer.startStream();
  sendHeadandTail(F("TmplStd"),_HEAD);

  struct EventStruct TempEvent;
  // char tmpString[64];


  byte notificationindex = getFormItemInt(F("index"), 0);
  boolean notificationindexNotSet = notificationindex == 0;
  --notificationindex;

  const int notification = getFormItemInt(F("notification"), -1);
  String domain = WebServer.arg(F("domain"));
  String server = WebServer.arg(F("server"));
  String sender = WebServer.arg(F("sender"));
  String receiver = WebServer.arg(F("receiver"));
  String subject = WebServer.arg(F("subject"));
  String user = WebServer.arg(F("user"));
  String pass = WebServer.arg(F("pass"));
  String body = WebServer.arg(F("body"));
  String notificationenabled = WebServer.arg(F("notificationenabled"));




  if (notification != -1 && !notificationindexNotSet)
  {
    NotificationSettingsStruct NotificationSettings;
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
        strncpy(NotificationSettings.Domain, domain.c_str(), sizeof(NotificationSettings.Domain));
        strncpy(NotificationSettings.Server, server.c_str(), sizeof(NotificationSettings.Server));
        strncpy(NotificationSettings.Sender, sender.c_str(), sizeof(NotificationSettings.Sender));
        strncpy(NotificationSettings.Receiver, receiver.c_str(), sizeof(NotificationSettings.Receiver));
        strncpy(NotificationSettings.Subject, subject.c_str(), sizeof(NotificationSettings.Subject));
        strncpy(NotificationSettings.User, user.c_str(), sizeof(NotificationSettings.User));
        strncpy(NotificationSettings.Pass, pass.c_str(), sizeof(NotificationSettings.Pass));
        strncpy(NotificationSettings.Body, body.c_str(), sizeof(NotificationSettings.Body));
      }
    }
    // Save the settings.
    addHtmlError(SaveNotificationSettings(notificationindex, (byte*)&NotificationSettings, sizeof(NotificationSettings)));
    addHtmlError(SaveSettings());
    if (WebServer.hasArg(F("test"))) {
      // Perform tests with the settings in the form.
      byte NotificationProtocolIndex = getNotificationProtocolIndex(Settings.Notification[notificationindex]);
      if (NotificationProtocolIndex != NPLUGIN_NOT_FOUND)
      {
        // TempEvent.NotificationProtocolIndex = NotificationProtocolIndex;
        TempEvent.NotificationIndex = notificationindex;
        NPlugin_ptr[NotificationProtocolIndex](NPLUGIN_NOTIFY, &TempEvent, dummyString);
      }
    }
  }

  TXBuffer += F("<form name='frmselect' method='post'>");

  if (notificationindexNotSet)
  {
    TXBuffer += F("<table class='multirow' border=1px frame='box' rules='all'><TR><TH style='width:70px;'>");
    TXBuffer += F("<TH style='width:50px;'>Nr<TH style='width:100px;'>Enabled<TH>Service<TH>Server<TH>Port");

    NotificationSettingsStruct NotificationSettings;
    for (byte x = 0; x < NOTIFICATION_MAX; x++)
    {
      LoadNotificationSettings(x, (byte*)&NotificationSettings, sizeof(NotificationSettings));
      TXBuffer += F("<TR><TD>");
      TXBuffer += F("<a class='button link' href=\"notifications?index=");
      TXBuffer += x + 1;
      TXBuffer += F("\">Edit</a>");
      TXBuffer += F("<TD>");
      TXBuffer += x + 1;
      TXBuffer += F("<TD>");
      if (Settings.Notification[x] != 0)
      {
        addEnabled(Settings.NotificationEnabled[x]);

        TXBuffer += F("<TD>");
        byte NotificationProtocolIndex = getNotificationProtocolIndex(Settings.Notification[x]);
        String NotificationName = F("(plugin not found?)");
        if (NotificationProtocolIndex!=NPLUGIN_NOT_FOUND)
        {
          NPlugin_ptr[NotificationProtocolIndex](NPLUGIN_GET_DEVICENAME, 0, NotificationName);
        }
        TXBuffer += NotificationName;
        TXBuffer += F("<TD>");
        TXBuffer += NotificationSettings.Server;
        TXBuffer += F("<TD>");
        TXBuffer += NotificationSettings.Port;
      }
      else
        TXBuffer += F("<TD><TD><TD>");
    }
    TXBuffer += F("</table></form>");
  }
  else
  {
    TXBuffer += F("<table class='normal'><TR><TH style='width:150px;' align='left'>Notification Settings<TH>");
    TXBuffer += F("<TR><TD>Notification:");
    byte choice = Settings.Notification[notificationindex];
    TXBuffer += F("<TD>");
    addSelector_Head(F("notification"), true);
    addSelector_Item(F("- None -"), 0, false, false, F(""));
    for (byte x = 0; x <= notificationCount; x++)
    {
      String NotificationName = "";
      NPlugin_ptr[x](NPLUGIN_GET_DEVICENAME, 0, NotificationName);
      addSelector_Item(NotificationName,
                       Notification[x].Number,
                       choice == Notification[x].Number,
                       false,
                       F(""));
    }
    addSelector_Foot();

    addHelpButton(F("EasyNotifications"));


    // char str[20];

    if (Settings.Notification[notificationindex])
    {
      NotificationSettingsStruct NotificationSettings;
      LoadNotificationSettings(notificationindex, (byte*)&NotificationSettings, sizeof(NotificationSettings));

      byte NotificationProtocolIndex = getNotificationProtocolIndex(Settings.Notification[notificationindex]);
      if (NotificationProtocolIndex!=NPLUGIN_NOT_FOUND)
      {

        if (Notification[NotificationProtocolIndex].usesMessaging)
        {
          TXBuffer += F("<TR><TD>Domain:<TD><input class='wide' type='text' name='domain' size=64 value='");
          TXBuffer += NotificationSettings.Domain;
          TXBuffer += F("'>");

          TXBuffer += F("<TR><TD>Server:<TD><input class='wide' type='text' name='server' size=64 value='");
          TXBuffer += NotificationSettings.Server;
          TXBuffer += F("'>");

          TXBuffer += F("<TR><TD>Port:<TD><input class='wide' type='text' name='port' value='");
          TXBuffer += NotificationSettings.Port;
          TXBuffer += F("'>");

          TXBuffer += F("<TR><TD>Sender:<TD><input class='wide' type='text' name='sender' size=64 value='");
          TXBuffer += NotificationSettings.Sender;
          TXBuffer += F("'>");

          TXBuffer += F("<TR><TD>Receiver:<TD><input class='wide' type='text' name='receiver' size=64 value='");
          TXBuffer += NotificationSettings.Receiver;
          TXBuffer += F("'>");

          TXBuffer += F("<TR><TD>Subject:<TD><input class='wide' type='text' name='subject' size=64 value='");
          TXBuffer += NotificationSettings.Subject;
          TXBuffer += F("'>");

          TXBuffer += F("<TR><TD>User:<TD><input class='wide' type='text' name='user' size=48 value='");
          TXBuffer += NotificationSettings.User;
          TXBuffer += F("'>");

          TXBuffer += F("<TR><TD>Pass:<TD><input class='wide' type='text' name='pass' size=32 value='");
          TXBuffer += NotificationSettings.Pass;
          TXBuffer += F("'>");

          TXBuffer += F("<TR><TD>Body:<TD><textarea name='body' rows='20' size=512 wrap='off'>");
          TXBuffer += NotificationSettings.Body;
          TXBuffer += F("</textarea>");
        }

        if (Notification[NotificationProtocolIndex].usesGPIO > 0)
        {
          TXBuffer += F("<TR><TD>1st GPIO:<TD>");
          addPinSelect(false, "pin1", NotificationSettings.Pin1);
        }

        TXBuffer += F("<TR><TD>Enabled:<TD>");
        addCheckBox(F("notificationenabled"), Settings.NotificationEnabled[notificationindex]);

        TempEvent.NotificationIndex = notificationindex;
        NPlugin_ptr[NotificationProtocolIndex](NPLUGIN_WEBFORM_LOAD, &TempEvent,TXBuffer.buf);
      }
    }

    addFormSeparator(2);

    TXBuffer += F("<TR><TD><TD><a class='button link' href=\"notifications\">Close</a>");
    addSubmitButton();
    addSubmitButton(F("Test"), F("test"));
    TXBuffer += F("</table></form>");
  }
  sendHeadandTail(F("TmplStd"),_TAIL);
  TXBuffer.endStream();
}


//********************************************************************************
// Web Interface hardware page
//********************************************************************************
void handle_hardware() {
  checkRAM(F("handle_hardware"));
  if (!isLoggedIn()) return;
  navMenuIndex = 3;
  TXBuffer.startStream();
  sendHeadandTail(F("TmplStd"),_HEAD);
  if (isFormItem(F("psda")))
  {
    Settings.Pin_status_led  = getFormItemInt(F("pled"));
    Settings.Pin_status_led_Inversed  = isFormItemChecked(F("pledi"));
    Settings.Pin_Reset  = getFormItemInt(F("pres"));
    Settings.Pin_i2c_sda     = getFormItemInt(F("psda"));
    Settings.Pin_i2c_scl     = getFormItemInt(F("pscl"));
    Settings.InitSPI = isFormItemChecked(F("initspi"));      // SPI Init
    Settings.Pin_sd_cs  = getFormItemInt(F("sd"));
    Settings.PinBootStates[0]  =  getFormItemInt(F("p0"));
    Settings.PinBootStates[2]  =  getFormItemInt(F("p2"));
    Settings.PinBootStates[4]  =  getFormItemInt(F("p4"));
    Settings.PinBootStates[5]  =  getFormItemInt(F("p5"));
    Settings.PinBootStates[9]  =  getFormItemInt(F("p9"));
    Settings.PinBootStates[10] =  getFormItemInt(F("p10"));
    Settings.PinBootStates[12] =  getFormItemInt(F("p12"));
    Settings.PinBootStates[13] =  getFormItemInt(F("p13"));
    Settings.PinBootStates[14] =  getFormItemInt(F("p14"));
    Settings.PinBootStates[15] =  getFormItemInt(F("p15"));
    Settings.PinBootStates[16] =  getFormItemInt(F("p16"));

    addHtmlError(SaveSettings());
  }

  TXBuffer += F("<form  method='post'><table class='normal'><TR><TH style='width:150px;' align='left'>Hardware Settings<TH align='left'>");
  addHelpButton(F("ESPEasy#Hardware_page"));

  addFormSubHeader(F("Wifi Status LED"));
  addFormPinSelect(F("GPIO &rarr; LED"), "pled", Settings.Pin_status_led);
  addFormCheckBox(F("Inversed LED"), F("pledi"), Settings.Pin_status_led_Inversed);
  addFormNote(F("Use &rsquo;GPIO-2 (D4)&rsquo; with &rsquo;Inversed&rsquo; checked for onboard LED"));

  addFormSubHeader(F("Reset Pin"));
  addFormPinSelect(F("GPIO &larr; Switch"), "pres", Settings.Pin_Reset);
  addFormNote(F("Press about 10s for factory reset"));

  addFormSubHeader(F("I2C Interface"));
  addFormPinSelectI2C(F("GPIO &#8703; SDA"), F("psda"), Settings.Pin_i2c_sda);
  addFormPinSelectI2C(F("GPIO &#8702; SCL"), F("pscl"), Settings.Pin_i2c_scl);

  // SPI Init
  addFormSubHeader(F("SPI Interface"));
  addFormCheckBox(F("Init SPI"), F("initspi"), Settings.InitSPI);
  addFormNote(F("CLK=GPIO-14 (D5), MISO=GPIO-12 (D6), MOSI=GPIO-13 (D7)"));
  addFormNote(F("Chip Select (CS) config must be done in the plugin"));
#ifdef FEATURE_SD
  addFormPinSelect(F("GPIO &rarr; SD Card CS"), "sd", Settings.Pin_sd_cs);
#endif

  addFormSubHeader(F("GPIO boot states"));
  addFormPinStateSelect(F("Pin mode 0 (D3)"), F("p0"), Settings.PinBootStates[0]);
  addFormPinStateSelect(F("Pin mode 2 (D4)"), F("p2"), Settings.PinBootStates[2]);
  addFormPinStateSelect(F("Pin mode 4 (D2)"), F("p4"), Settings.PinBootStates[4]);
  addFormPinStateSelect(F("Pin mode 5 (D1)"), F("p5"), Settings.PinBootStates[5]);
  addFormPinStateSelect(F("Pin mode 9 (D11)"), F("p9"), Settings.PinBootStates[9]);
  addFormPinStateSelect(F("Pin mode 10 (D12)"), F("p10"), Settings.PinBootStates[10]);
  addFormPinStateSelect(F("Pin mode 12 (D6)"), F("p12"), Settings.PinBootStates[12]);
  addFormPinStateSelect(F("Pin mode 13 (D7)"), F("p13"), Settings.PinBootStates[13]);
  addFormPinStateSelect(F("Pin mode 14 (D5)"), F("p14"), Settings.PinBootStates[14]);
  addFormPinStateSelect(F("Pin mode 15 (D8)"), F("p15"), Settings.PinBootStates[15]);
  addFormPinStateSelect(F("Pin mode 16 (D0)"), F("p16"), Settings.PinBootStates[16]);
  addFormSeparator(2);

  TXBuffer += F("<TR><TD><TD>");
  addSubmitButton();
  TXBuffer += F("<TR><TD></table></form>");

  sendHeadandTail(F("TmplStd"),_TAIL);
  TXBuffer.endStream();

}

//********************************************************************************
// Add a GPIO pin select dropdown list
//********************************************************************************
void addFormPinStateSelect(const String& label, const String& id, int choice)
{
  addRowLabel(label);
  addPinStateSelect(id, choice);
}

void addPinStateSelect(String name, int choice)
{
  String options[4] = { F("Default"), F("Output Low"), F("Output High"), F("Input") };
  addSelector(name, 4, options, NULL, NULL, choice, false);
}

//********************************************************************************
// Add a IP Access Control select dropdown list
//********************************************************************************
void addFormIPaccessControlSelect(const String& label, const String& id, int choice)
{
  addRowLabel(label);
  addIPaccessControlSelect(id, choice);
}

void addIPaccessControlSelect(String name, int choice)
{
  String options[3] = { F("Allow All"), F("Allow Local Subnet"), F("Allow IP range") };
  addSelector(name, 3, options, NULL, NULL, choice, false);
}




//********************************************************************************
// Web Interface device page
//********************************************************************************
//19480 (11128)
void handle_devices() {
  checkRAM(F("handle_devices"));
  if (!isLoggedIn()) return;
  navMenuIndex = 4;
  TXBuffer.startStream();
  sendHeadandTail(F("TmplStd"),_HEAD);


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

  byte taskIndex = getFormItemInt(F("index"), 0);
  boolean taskIndexNotSet = taskIndex == 0;
  --taskIndex;

  byte DeviceIndex = 0;

  if (edit != 0  && !taskIndexNotSet) // when form submitted
  {
    if (Settings.TaskDeviceNumber[taskIndex] != taskdevicenumber) // change of device: cleanup old device and reset default settings
    {
      //let the plugin do its cleanup by calling PLUGIN_EXIT with this TaskIndex
      TempEvent.TaskIndex = taskIndex;
      PluginCall(PLUGIN_EXIT, &TempEvent, dummyString);

      taskClear(taskIndex, false); // clear settings, but do not save

      Settings.TaskDeviceNumber[taskIndex] = taskdevicenumber;
      if (taskdevicenumber != 0) // set default values if a new device has been selected
      {
        //NOTE: do not enable task by default. allow user to enter sensible valus first and let him enable it when ready.
        if (ExtraTaskSettings.TaskDeviceValueNames[0][0] == 0) // if field set empty, reload defaults
          PluginCall(PLUGIN_GET_DEVICEVALUENAMES, &TempEvent, dummyString); //the plugin should populate ExtraTaskSettings with its default values.

          ClearCustomTaskSettings(taskIndex);
      }
    }
    else if (taskdevicenumber != 0) //save settings
    {
      Settings.TaskDeviceNumber[taskIndex] = taskdevicenumber;
      DeviceIndex = getDeviceIndex(Settings.TaskDeviceNumber[taskIndex]);

      if (taskdevicetimer > 0)
        Settings.TaskDeviceTimer[taskIndex] = taskdevicetimer;
      else
      {
        if (!Device[DeviceIndex].TimerOptional) // Set default delay, unless it's optional...
          Settings.TaskDeviceTimer[taskIndex] = Settings.Delay;
        else
          Settings.TaskDeviceTimer[taskIndex] = 0;
      }

      Settings.TaskDeviceEnabled[taskIndex] = isFormItemChecked(F("TDE"));
      strcpy(ExtraTaskSettings.TaskDeviceName, WebServer.arg(F("TDN")).c_str());
      Settings.TaskDevicePort[taskIndex] =  getFormItemInt(F("TDP"), 0);

      for (byte controllerNr = 0; controllerNr < CONTROLLER_MAX; controllerNr++)
      {

        Settings.TaskDeviceID[controllerNr][taskIndex] = getFormItemInt(String(F("TDID")) + (controllerNr + 1));
        Settings.TaskDeviceSendData[controllerNr][taskIndex] = isFormItemChecked(String(F("TDSD")) + (controllerNr + 1));
      }

      update_whenset_FormItemInt(F("taskdevicepin1"), Settings.TaskDevicePin1[taskIndex]);
      update_whenset_FormItemInt(F("taskdevicepin2"), Settings.TaskDevicePin2[taskIndex]);
      update_whenset_FormItemInt(F("taskdevicepin3"), Settings.TaskDevicePin3[taskIndex]);

      if (Device[DeviceIndex].PullUpOption)
        Settings.TaskDevicePin1PullUp[taskIndex] = isFormItemChecked(F("TDPPU"));

      if (Device[DeviceIndex].InverseLogicOption)
        Settings.TaskDevicePin1Inversed[taskIndex] = isFormItemChecked(F("TDPI"));

      for (byte varNr = 0; varNr < Device[DeviceIndex].ValueCount; varNr++)
      {

        strcpy(ExtraTaskSettings.TaskDeviceFormula[varNr], WebServer.arg(String(F("TDF")) + (varNr + 1)).c_str());
        ExtraTaskSettings.TaskDeviceValueDecimals[varNr] = getFormItemInt(String(F("TDVD")) + (varNr + 1));
        strcpy(ExtraTaskSettings.TaskDeviceValueNames[varNr], WebServer.arg(String(F("TDVN")) + (varNr + 1)).c_str());

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
      if (ExtraTaskSettings.TaskDeviceValueNames[0][0] == 0) // if field set empty, reload defaults
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
              CPlugin_ptr[TempEvent.ProtocolIndex](CPLUGIN_TASK_CHANGE_NOTIFICATION, &TempEvent, dummyString);
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
    TXBuffer += jsUpdateSensorValuesDevicePage;

    TXBuffer += F("<table class='multirow' border=1px frame='box' rules='all'><TR><TH style='width:70px;'>");

    if (TASKS_MAX != TASKS_PER_PAGE)
    {
      TXBuffer += F("<a class='button link' href=\"devices?setpage=");
      if (page > 1)
        TXBuffer += page - 1;
      else
        TXBuffer += page;
      TXBuffer += F("\">&lt;</a>");
      TXBuffer += F("<a class='button link' href=\"devices?setpage=");
      if (page < (TASKS_MAX / TASKS_PER_PAGE))
        TXBuffer += page + 1;
      else
        TXBuffer += page;
      TXBuffer += F("\">&gt;</a>");
    }

    TXBuffer += F("<TH style='width:50px;'>Task<TH style='width:100px;'>Enabled<TH>Device<TH>Name<TH>Port<TH style='width:100px;'>Ctr (IDX)<TH style='width:70px;'>GPIO<TH>Values");

    String deviceName;

    for (byte x = (page - 1) * TASKS_PER_PAGE; x < ((page) * TASKS_PER_PAGE); x++)
    {
      TXBuffer += F("<TR><TD>");
      TXBuffer += F("<a class='button link' href=\"devices?index=");
      TXBuffer += x + 1;
      TXBuffer += F("&page=");
      TXBuffer += page;
      TXBuffer += F("\">Edit</a>");
      TXBuffer += F("<TD>");
      TXBuffer += x + 1;
      TXBuffer += F("<TD>");

      if (Settings.TaskDeviceNumber[x] != 0)
      {
        LoadTaskSettings(x);
        DeviceIndex = getDeviceIndex(Settings.TaskDeviceNumber[x]);
        TempEvent.TaskIndex = x;
        addEnabled( Settings.TaskDeviceEnabled[x]);

        TXBuffer += F("<TD>");
        TXBuffer += getPluginNameFromDeviceIndex(DeviceIndex);
        TXBuffer += F("<TD>");
        TXBuffer += ExtraTaskSettings.TaskDeviceName;
        TXBuffer += F("<TD>");

        byte customConfig = false;
        customConfig = PluginCall(PLUGIN_WEBFORM_SHOW_CONFIG, &TempEvent,TXBuffer.buf);
        if (!customConfig)
          if (Device[DeviceIndex].Ports != 0)
            TXBuffer += formatToHex_decimal(Settings.TaskDevicePort[x]);

        TXBuffer += F("<TD>");

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
                TXBuffer += F(" (");
                TXBuffer += Settings.TaskDeviceID[controllerNr][x];
                TXBuffer += F(")");
                if (Settings.TaskDeviceID[controllerNr][x] == 0)
                  TXBuffer += F(" " HTML_SYMBOL_WARNING);
              }
              doBR = true;
            }
          }
        }

        TXBuffer += F("<TD>");

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

        TXBuffer += F("<TD>");
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
              TXBuffer  += F("'>");
              TXBuffer += ExtraTaskSettings.TaskDeviceValueNames[varNr];
              TXBuffer += F(":</div><div class='div_r' id='value_");
              TXBuffer  += x;
              TXBuffer  += '_';
              TXBuffer  += varNr;
              TXBuffer  += F("'>");
              TXBuffer += formatUserVarNoCheck(x, varNr);
              TXBuffer += "</div>";
            }
          }
        }
      }
      else
        TXBuffer += F("<TD><TD><TD><TD><TD><TD>");

    } // next
    TXBuffer += F("</table></form>");

  }
  // Show edit form if a specific entry is chosen with the edit button
  else
  {
    LoadTaskSettings(taskIndex);
    DeviceIndex = getDeviceIndex(Settings.TaskDeviceNumber[taskIndex]);
    TempEvent.TaskIndex = taskIndex;

    TXBuffer += F("<form name='frmselect' method='post'><table class='normal'>");
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
      TXBuffer += F("'>");

      //show selected device name and delete button
      TXBuffer += getPluginNameFromDeviceIndex(DeviceIndex);

      addHelpButton(String(F("Plugin")) + Settings.TaskDeviceNumber[taskIndex]);

      if (Device[DeviceIndex].Number == 3 && taskIndex >= 4) // Number == 3 = PulseCounter Plugin
        {
          addFormNote(F("This plugin is only supported on task 1-4 for now"));
        }

      addFormTextBox( F("Name"), F("TDN"), ExtraTaskSettings.TaskDeviceName, 40);   //="taskdevicename"

      addFormCheckBox(F("Enabled"), F("TDE"), Settings.TaskDeviceEnabled[taskIndex]);   //="taskdeviceenabled"

      // section: Sensor / Actuator
      if (!Device[DeviceIndex].Custom && Settings.TaskDeviceDataFeed[taskIndex] == 0 &&
          ((Device[DeviceIndex].Ports != 0) || (Device[DeviceIndex].PullUpOption) || (Device[DeviceIndex].InverseLogicOption) || (Device[DeviceIndex].Type >= DEVICE_TYPE_SINGLE && Device[DeviceIndex].Type <= DEVICE_TYPE_TRIPLE)) )
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

        if (Device[DeviceIndex].Type >= DEVICE_TYPE_SINGLE && Device[DeviceIndex].Type <= DEVICE_TYPE_TRIPLE)
          addFormPinSelect(TempEvent.String1, F("taskdevicepin1"), Settings.TaskDevicePin1[taskIndex]);
        if (Device[DeviceIndex].Type >= DEVICE_TYPE_DUAL && Device[DeviceIndex].Type <= DEVICE_TYPE_TRIPLE)
          addFormPinSelect( TempEvent.String2, F("taskdevicepin2"), Settings.TaskDevicePin2[taskIndex]);
        if (Device[DeviceIndex].Type == DEVICE_TYPE_TRIPLE)
          addFormPinSelect(TempEvent.String3, F("taskdevicepin3"), Settings.TaskDevicePin3[taskIndex]);
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

            TXBuffer += F("<TR><TD>Send to Controller ");
            TXBuffer += getControllerSymbol(controllerNr);
            TXBuffer += F("<TD>");
            addCheckBox(id, Settings.TaskDeviceSendData[controllerNr][taskIndex]);

            byte ProtocolIndex = getProtocolIndex(Settings.Protocol[controllerNr]);
            if (Protocol[ProtocolIndex].usesID && Settings.Protocol[controllerNr] != 0)
            {
              TXBuffer += F("<TR><TD>IDX:<TD>");
              id = F("TDID");   //="taskdeviceid"
              id += controllerNr + 1;
              addNumericBox(id, Settings.TaskDeviceID[controllerNr][taskIndex], 0, 999999999); // Looks like it is an unsigned int, so could be up to 4 bln.
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
        TXBuffer += F("</table><table class='normal'>");

        //table header
        TXBuffer += F("<TR><TH style='width:30px;' align='center'>#");
        TXBuffer += F("<TH align='left'>Name");

        if (Device[DeviceIndex].FormulaOption)
        {
          TXBuffer += F("<TH align='left'>Formula");
          addHelpButton(F("EasyFormula"));
        }

        if (Device[DeviceIndex].FormulaOption || Device[DeviceIndex].DecimalsOnly)
        {
          TXBuffer += F("<TH style='width:30px;' align='left'>Decimals");
        }

        //table body
        for (byte varNr = 0; varNr < Device[DeviceIndex].ValueCount; varNr++)
        {
          TXBuffer += F("<TR><TD>");
          TXBuffer += varNr + 1;
          TXBuffer += F("<TD>");
          String id = F("TDVN");   //="taskdevicevaluename"
          id += (varNr + 1);
          addTextBox(id, ExtraTaskSettings.TaskDeviceValueNames[varNr], 40);

          if (Device[DeviceIndex].FormulaOption)
          {
            TXBuffer += F("<TD>");
            String id = F("TDF");   //="taskdeviceformula"
            id += (varNr + 1);
            addTextBox(id, ExtraTaskSettings.TaskDeviceFormula[varNr], 40);
          }

          if (Device[DeviceIndex].FormulaOption || Device[DeviceIndex].DecimalsOnly)
          {
            TXBuffer += F("<TD>");
            String id = F("TDVD");   //="taskdevicevaluedecimals"
            id += (varNr + 1);
            addNumericBox(id, ExtraTaskSettings.TaskDeviceValueDecimals[varNr], 0, 6);
          }
        }
      }
    }

    addFormSeparator(4);

    TXBuffer += F("<TR><TD><TD colspan='3'><a class='button link' href=\"devices?setpage=");
    TXBuffer += page;
    TXBuffer += F("\">Close</a>");
    addSubmitButton();
    TXBuffer += F("<input type='hidden' name='edit' value='1'>");
    TXBuffer += F("<input type='hidden' name='page' value='1'>");

    //if user selected a device, add the delete button
    if (Settings.TaskDeviceNumber[taskIndex] != 0 )
      addSubmitButton(F("Delete"), F("del"));



    TXBuffer += F("</table></form>");
  }


  checkRAM(F("handle_devices"));
  String log = F("DEBUG: String size:");
  log += String(TXBuffer.sentBytes);
  addLog(LOG_LEVEL_DEBUG_DEV, log);
  sendHeadandTail(F("TmplStd"),_TAIL);
  TXBuffer.endStream();
}


byte sortedIndex[DEVICES_MAX + 1];
//********************************************************************************
// Add a device select dropdown list
//********************************************************************************
void addDeviceSelect(String name,  int choice)
{
  // first get the list in alphabetic order
  for (byte x = 0; x <= deviceCount; x++)
    sortedIndex[x] = x;
  sortDeviceArray();

  String deviceName;

  addSelector_Head(name, true);
  addSelector_Item(F("- None -"), 0, false, false, F(""));
  for (byte x = 0; x <= deviceCount; x++)
  {
    byte deviceIndex = sortedIndex[x];
    if (Plugin_id[deviceIndex] != 0)
      deviceName = getPluginNameFromDeviceIndex(deviceIndex);

#ifdef PLUGIN_BUILD_DEV
    int num = Plugin_id[deviceIndex];
    String plugin = F("P");
    if (num < 10) plugin += F("0");
    if (num < 100) plugin += F("0");
    plugin += num;
    plugin += F(" - ");
    deviceName = plugin + deviceName;
#endif

    addSelector_Item(deviceName,
                     Device[deviceIndex].Number,
                     choice == Device[deviceIndex].Number,
                     false,
                     F(""));
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
  addRowLabel(label);
  addPinSelect(false, id, choice);
}


void addFormPinSelectI2C(const String& label, const String& id, int choice)
{
  addRowLabel(label);
  addPinSelect(true, id, choice);
}


//********************************************************************************
// Add a GPIO pin select dropdown list for both 8266 and 8285
//********************************************************************************
#if defined(ESP8285)
// Code for the ESP8285

//********************************************************************************
// Add a GPIO pin select dropdown list
//********************************************************************************
void addPinSelect(boolean forI2C, String name,  int choice)
{
  String options[18];
  options[0] = F("- None -");
  options[1] = F("GPIO-0 (D3)");
  options[2] = F("GPIO-1 (D10)");
  options[3] = F("GPIO-2 (D4)");
  options[4] = F("GPIO-3 (D9)");
  options[5] = F("GPIO-4 (D2)");
  options[6] = F("GPIO-5 (D1)");
  options[7] = F("GPIO-6");
  options[8] = F("GPIO-7");
  options[9] = F("GPIO-8");
  options[10] = F("GPIO-9 (D11)");
  options[11] = F("GPIO-10 (D12)");
  options[12] = F("GPIO-11");
  options[13] = F("GPIO-12 (D6)");
  options[14] = F("GPIO-13 (D7)");
  options[15] = F("GPIO-14 (D5)");
  options[16] = F("GPIO-15 (D8)");
  options[17] = F("GPIO-16 (D0)");
  int optionValues[18];
  optionValues[0] = -1;
  optionValues[1] = 0;
  optionValues[2] = 1;
  optionValues[3] = 2;
  optionValues[4] = 3;
  optionValues[5] = 4;
  optionValues[6] = 5;
  optionValues[7] = 6;
  optionValues[8] = 7;
  optionValues[9] = 8;
  optionValues[10] = 9;
  optionValues[11] = 10;
  optionValues[12] = 11;
  optionValues[13] = 12;
  optionValues[14] = 13;
  optionValues[15] = 14;
  optionValues[16] = 15;
  optionValues[17] = 16;
  renderHTMLForPinSelect(options, optionValues, forI2C, name, choice, 18);

}

#else
#if defined(ESP8266)
// Code for the ESP8266

//********************************************************************************
// Add a GPIO pin select dropdown list
//********************************************************************************
void addPinSelect(boolean forI2C, String name,  int choice)
{
  String options[14];
  options[0] = F("- None -");
  options[1] = F("GPIO-0 (D3)");
  options[2] = F("GPIO-1 (D10)");
  options[3] = F("GPIO-2 (D4)");
  options[4] = F("GPIO-3 (D9)");
  options[5] = F("GPIO-4 (D2)");
  options[6] = F("GPIO-5 (D1)");
  options[7] = F("GPIO-9 (D11) " HTML_SYMBOL_WARNING);
  options[8] = F("GPIO-10 (D12)");
  options[9] = F("GPIO-12 (D6)");
  options[10] = F("GPIO-13 (D7)");
  options[11] = F("GPIO-14 (D5)");
  options[12] = F("GPIO-15 (D8)");
  options[13] = F("GPIO-16 (D0)");
  int optionValues[14];
  optionValues[0] = -1;
  optionValues[1] = 0;
  optionValues[2] = 1;
  optionValues[3] = 2;
  optionValues[4] = 3;
  optionValues[5] = 4;
  optionValues[6] = 5;
  optionValues[7] = 9;
  optionValues[8] = 10;
  optionValues[9] = 12;
  optionValues[10] = 13;
  optionValues[11] = 14;
  optionValues[12] = 15;
  optionValues[13] = 16;
  renderHTMLForPinSelect(options, optionValues, forI2C, name, choice, 14);
}
#endif

#if defined(ESP32)
//********************************************************************************
// Add a GPIO pin select dropdown list
//********************************************************************************
void addPinSelect(boolean forI2C, String name,  int choice)
{
  String options[PIN_D_MAX+1];
  int optionValues[PIN_D_MAX+1];
  options[0] = F("- None -");
  optionValues[0] = -1;
  for(byte x=1; x < PIN_D_MAX+1; x++)
  {
    options[x] = F("GPIO-");
    options[x] += x;
    optionValues[x] = x;
  }
  renderHTMLForPinSelect(options, optionValues, forI2C, name, choice, PIN_D_MAX+1);
}
#endif


#endif

//********************************************************************************
// Helper function actually rendering dropdown list for addPinSelect()
//********************************************************************************
void renderHTMLForPinSelect(String options[], int optionValues[], boolean forI2C, String name,  int choice, int count) {
  addSelector_Head(name, false);
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
                     F(""));
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

void addFormSelector(const String& label, const String& id, int optionCount, const String options[], const int indices[], const String attr[], int selectedIndex, boolean reloadonchange)
{
  addRowLabel(label);
  addSelector(id, optionCount, options, indices, attr, selectedIndex, reloadonchange);
}

void addSelector(const String& id, int optionCount, const String options[], const int indices[], const String attr[], int selectedIndex, boolean reloadonchange)
{
  int index;

  TXBuffer += F("<select id='selectwidth' name='");
  TXBuffer += id;
  TXBuffer += F("'");
  if (reloadonchange)
    TXBuffer += F(" onchange='return dept_onchange(frmselect)'>");
  TXBuffer += F(">");
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
      TXBuffer += F(" ");
      TXBuffer += attr[x];
    }
    TXBuffer += ">";
    TXBuffer += options[x];
    TXBuffer += F("</option>");
  }
  TXBuffer += F("</select>");
}


void addSelector_Head(const String& id, boolean reloadonchange)
{
  TXBuffer += F("<select id='selectwidth' name='");
  TXBuffer += id;
  TXBuffer += F("'");
  if (reloadonchange)
    TXBuffer += F(" onchange='return dept_onchange(frmselect)'>");
  TXBuffer += F(">");
}

void addSelector_Item(const String& option, int index, boolean selected, boolean disabled, const String& attr)
{
  TXBuffer += F("<option value=");
  TXBuffer += index;
  if (selected)
    TXBuffer += F(" selected");
  if (disabled)
    TXBuffer += F(" disabled");
  if (attr && attr.length() > 0)
  {
    TXBuffer += F(" ");
    TXBuffer += attr;
  }
  TXBuffer += ">";
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
  TXBuffer += F("]");
}


void addRowLabel(const String& label)
{
  TXBuffer += F("<TR><TD>");
  TXBuffer += label;
  TXBuffer += F(":<TD>");
}

void addButton(const String &url, const String &label)
{
  TXBuffer += F("<a class='button link' href='");
  TXBuffer += url;
  TXBuffer += F("'>");
  TXBuffer += label;
  TXBuffer += F("</a>");
}

void addWideButton(const String &url, const String &label, const String &color)
{
  TXBuffer += F("<a class='button link wide");
  TXBuffer += color;
  TXBuffer += F("' href='");
  TXBuffer += url;
  TXBuffer += F("'>");
  TXBuffer += label;
  TXBuffer += F("</a>");
}

void addSubmitButton()
{
  TXBuffer += F("<input class='button link' type='submit' value='Submit'><div id='toastmessage'></div><script type='text/javascript'>toasting();</script>");
}

//add submit button with different label and name
void addSubmitButton(const String &value, const String &name)
{
  TXBuffer += F("<input class='button link' type='submit' value='");
  TXBuffer += value;
  TXBuffer += F("' name='");
  TXBuffer += name;
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
  TXBuffer += F("</button>");
}


//********************************************************************************
// Add a header
//********************************************************************************
void addFormHeader(const String& header1, const String& header2)
{
  TXBuffer += F("<TR><TH>");
  TXBuffer += header1;
  TXBuffer += F("<TH>");
  TXBuffer += header2;
  TXBuffer += F("");
}

void addFormHeader(const String& header)
{
  TXBuffer += F("<TR><TD colspan='2'><h2>");
  TXBuffer += header;
  TXBuffer += F("</h2>");
}


//********************************************************************************
// Add a sub header
//********************************************************************************
void addFormSubHeader(const String& header)
{
  TXBuffer += F("<TR><TD colspan='2'><h3>");
  TXBuffer += header;
  TXBuffer += F("</h3>");
}


//********************************************************************************
// Add a note as row start
//********************************************************************************
void addFormNote(const String& text)
{
  TXBuffer += F("<TR><TD><TD><div class='note'>Note: ");
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
void addCheckBox(const String& id, boolean checked)
{
  TXBuffer += F("<label class='container'>&nbsp;");
  TXBuffer += F("<input type='checkbox' id='");
  TXBuffer += id;
  TXBuffer += F("' name='");
  TXBuffer += id;
  TXBuffer += F("'");
  if (checked)
    TXBuffer += F(" checked");
  TXBuffer += F("><span class='checkmark'></span></label>");
}

void addFormCheckBox(const String& label, const String& id, boolean checked)
{
  addRowLabel(label);
  addCheckBox(id, checked);
}


//********************************************************************************
// Add a numeric box
//********************************************************************************
void addNumericBox(const String& id, int value, int min, int max)
{
  TXBuffer += F("<input class='widenumber' type='number' name='");
  TXBuffer += id;
  TXBuffer += F("'");
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
  TXBuffer += F(">");
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
  TXBuffer += F("'");
  TXBuffer += F(" min=");
  TXBuffer += min;
  TXBuffer += F(" max=");
  TXBuffer += max;
  TXBuffer += F(" step=0.01");
  TXBuffer += F(" style='width:5em;' value=");
  TXBuffer += value;
  TXBuffer += F(">");
}

void addFormFloatNumberBox(const String& label, const String& id, float value, float min, float max)
{
  addRowLabel(label);
  addFloatNumberBox(id, value, min, max);
}


void addTextBox(const String& id, const String&  value, int maxlength)
{
  TXBuffer += F("<input class='wide' type='text' name='");
  TXBuffer += id;
  TXBuffer += F("' maxlength=");
  TXBuffer += maxlength;
  TXBuffer += F(" value='");
  TXBuffer += value;
  TXBuffer += F("'>");
}

void addFormTextBox(const String& label, const String& id, const String&  value, int maxlength)
{
  addRowLabel(label);
  addTextBox(id, value, maxlength);
}


void addFormPasswordBox(const String& label, const String& id, const String& password, int maxlength)
{
  addRowLabel(label);
  TXBuffer += F("<input class='wide' type='password' name='");
  TXBuffer += id;
  TXBuffer += F("' maxlength=");
  TXBuffer += maxlength;
  TXBuffer += F(" value='");
  if (password != F(""))   //no password?
    TXBuffer += F("*****");
  //TXBuffer += password;   //password will not published over HTTP
  TXBuffer += F("'>");
}

void copyFormPassword(const String& id, char* pPassword, int maxlength)
{
  String password = WebServer.arg(id);
  if (password == F("*****"))   //no change?
    return;
  strncpy(pPassword, password.c_str(), maxlength);
}

void addFormIPBox(const String& label, const String& id, const byte ip[4])
{
  char strip[20];
  if (ip[0] == 0 && ip[1] == 0 && ip[2] == 0 && ip[3] == 0)
    strip[0] = 0;
  else {
    formatIP(ip, strip);
  }

  addRowLabel(label);
  TXBuffer += F("<input class='wide' type='text' name='");
  TXBuffer += id;
  TXBuffer += F("' value='");
  TXBuffer += strip;
  TXBuffer += F("'>");
}

// adds a Help Button with points to the the given Wiki Subpage
void addHelpButton(const String& url)
{
  TXBuffer += F(" <a class='button help' href='");
  if (!url.startsWith(F("http"))) {
    TXBuffer += F("http://www.letscontrolit.com/wiki/index.php/");
  }
  TXBuffer += url;
  TXBuffer += F("' target='_blank'>&#10068;</a>");
}

void addEnabled(boolean enabled)
{
  if (enabled)
    TXBuffer += F("<span class='enabled on'>&#10004;</span>");
  else
    TXBuffer += F("<span class='enabled off'>&#10060;</span>");
}


//********************************************************************************
// Add a task select dropdown list
//********************************************************************************
void addTaskSelect(String name,  int choice)
{
  struct EventStruct TempEvent;
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
    TXBuffer += "'";
    if (choice == x)
      TXBuffer += F(" selected");
    if (Settings.TaskDeviceNumber[x] == 0)
      TXBuffer += F(" disabled");
    TXBuffer += ">";
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
void addTaskValueSelect(String name, int choice, byte TaskIndex)
{
  TXBuffer += F("<select id='selectwidth' name='");
  TXBuffer += name;
  TXBuffer += "'>";

  byte DeviceIndex = getDeviceIndex(Settings.TaskDeviceNumber[TaskIndex]);

  for (byte x = 0; x < Device[DeviceIndex].ValueCount; x++)
  {
    TXBuffer += F("<option value='");
    TXBuffer += x;
    TXBuffer += "'";
    if (choice == x)
      TXBuffer += F(" selected");
    TXBuffer += ">";
    TXBuffer += ExtraTaskSettings.TaskDeviceValueNames[x];
    TXBuffer += F("</option>");
  }
}



//********************************************************************************
// Web Interface log page
//********************************************************************************
void handle_log() {
  if (!isLoggedIn()) return;
  navMenuIndex = 7;
  TXBuffer.startStream();
  sendHeadandTail(F("TmplStd"),_HEAD);

  TXBuffer += F("<table class=\"normal\"><TR><TH id=\"headline\" align=\"left\">Log");
  addCopyButton(F("copyText"), F(""), F("Copy log to clipboard"));
  TXBuffer += F("</TR></table><div  id='current_loglevel' style='font-weight: bold;'>Logging: </div><div class='logviewer' id='copyText_1'></div>");
  TXBuffer += F("Autoscroll: ");
  addCheckBox(F("autoscroll"), true);
  TXBuffer += F("<BR></body>");

  TXBuffer += jsFetchAndParseLog;

  sendHeadandTail(F("TmplStd"),_TAIL);
  TXBuffer.endStream();
  }

//********************************************************************************
// Web Interface JSON log page
//********************************************************************************
void handle_log_JSON() {
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
      stream_next_json_object_value(F("label"), getLogLevelDisplayString(i, loglevel));
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
}

//********************************************************************************
// Web Interface debug page
//********************************************************************************
void handle_tools() {
  if (!isLoggedIn()) return;
  navMenuIndex = 7;
  TXBuffer.startStream();
  sendHeadandTail(F("TmplStd"),_HEAD);


  String webrequest = WebServer.arg(F("cmd"));

  TXBuffer += F("<form>");
  TXBuffer += F("<table class='normal'>");

  addFormHeader(F("Tools"));

  addFormSubHeader(F("Command"));
    TXBuffer += F("<TR><TD style='width: 180px'>");
    TXBuffer += F("<input class='wide' type='text' name='cmd' value='");
    TXBuffer += webrequest;
    TXBuffer += F("'>");
    TXBuffer += F("<TD>");
    addSubmitButton();
    addHelpButton(F("ESPEasy_Command_Reference"));
    TXBuffer += F("<TR><TD>");

    printToWeb = true;
    printWebString = "";

    if (webrequest.length() > 0)
    {
      struct EventStruct TempEvent;
      parseCommandString(&TempEvent, webrequest);
      TempEvent.Source = VALUE_SOURCE_HTTP;
      if (!PluginCall(PLUGIN_WRITE, &TempEvent, webrequest))
        ExecuteCommand(VALUE_SOURCE_HTTP, webrequest.c_str());
    }

    if (printWebString.length() > 0)
    {
      TXBuffer += F("<TR><TD colspan='2'>Command Output<BR><textarea readonly rows='10' wrap='on'>");
      TXBuffer += printWebString;
      TXBuffer += F("</textarea>");
    }

  addFormSubHeader(F("System"));

  TXBuffer += F("<TR><TD HEIGHT=\"30\">");
  addWideButton(F("/?cmd=reboot"), F("Reboot"), F(""));
  TXBuffer += F("<TD>");
  TXBuffer += F("Reboots ESP");

  TXBuffer += F("<TR><TD HEIGHT=\"30\">");
  addWideButton(F("log"), F("Log"), F(""));
  TXBuffer += F("<TD>");
  TXBuffer += F("Open log output");

  TXBuffer += F("<TR><TD HEIGHT=\"30\">");
  addWideButton(F("sysinfo"), F("Info"), F(""));
  TXBuffer += F("<TD>");
  TXBuffer += F("Open system info page");

  TXBuffer += F("<TR><TD HEIGHT=\"30\">");
  addWideButton(F("advanced"), F("Advanced"), F(""));
  TXBuffer += F("<TD>");
  TXBuffer += F("Open advanced settings");

  TXBuffer += F("<TR><TD HEIGHT=\"30\">");
  addWideButton(F("json"), F("Show JSON"), F(""));
  TXBuffer += F("<TD>");
  TXBuffer += F("Open JSON output");

  TXBuffer += F("<TR><TD HEIGHT=\"30\">");
  addWideButton(F("pinstates"), F("Pin state buffer"), F(""));
  TXBuffer += F("<TD>");
  TXBuffer += F("Show Pin state buffer");

  addFormSubHeader(F("Wifi"));

  TXBuffer += F("<TR><TD HEIGHT=\"30\">");
  addWideButton(F("/?cmd=wificonnect"), F("Connect"), F(""));
  TXBuffer += F("<TD>");
  TXBuffer += F("Connects to known Wifi network");

  TXBuffer += F("<TR><TD HEIGHT=\"30\">");
  addWideButton(F("/?cmd=wifidisconnect"), F("Disconnect"), F(""));
  TXBuffer += F("<TD>");
  TXBuffer += F("Disconnect from wifi network");

  TXBuffer += F("<TR><TD HEIGHT=\"30\">");
  addWideButton(F("wifiscanner"), F("Scan"), F(""));
  TXBuffer += F("<TD>");
  TXBuffer += F("Scan for wifi networks");

  addFormSubHeader(F("Interfaces"));

  TXBuffer += F("<TR><TD HEIGHT=\"30\">");
  addWideButton(F("i2cscanner"), F("I2C Scan"), F(""));
  TXBuffer += F("<TD>");
  TXBuffer += F("Scan for I2C devices");

  addFormSubHeader(F("Settings"));

  TXBuffer += F("<TR><TD HEIGHT=\"30\">");
  addWideButton(F("upload"), F("Load"), F(""));
  TXBuffer += F("<TD>");
  TXBuffer += F("Loads a settings file");
  addFormNote(F("(File MUST be renamed to \"config.dat\" before upload!)"));

  TXBuffer += F("<TR><TD HEIGHT=\"30\">");
  addWideButton(F("download"), F("Save"), F(""));
  TXBuffer += F("<TD>");
  TXBuffer += F("Saves a settings file");

#if defined(ESP8266)
  {
    const uint32_t flashSize = getFlashRealSizeInBytes();
    if (flashSize > 524288)
    {
      addFormSubHeader(F("Firmware"));
      TXBuffer += F("<TR><TD HEIGHT=\"30\">");
      addWideButton(F("update"), F("Load"), F(""));
      addHelpButton(F("EasyOTA"));
      TXBuffer += F("<TD>");
      TXBuffer += F("Load a new firmware");
      if (flashSize <= 1048576) {
        TXBuffer += F(" <b>WARNING</b> only use 2-step OTA update and sketch < 604 kB");
      }
    }
  }
#endif

  addFormSubHeader(F("Filesystem"));

  TXBuffer += F("<TR><TD HEIGHT=\"30\">");
  addWideButton(F("filelist"), F("Flash"), F(""));
  TXBuffer += F("<TD>");
  TXBuffer += F("Show files on internal flash");

  TXBuffer += F("<TR><TD HEIGHT=\"30\">");
  addWideButton(F("/?cmd=reset"), F("Factory Reset"), F(" red"));
  TXBuffer += F("<TD>");
  TXBuffer += F("Erase all settings files");

#ifdef FEATURE_SD
  TXBuffer += F("<TR><TD HEIGHT=\"30\">");
  addWideButton(F("SDfilelist"), F("SD Card"), F(""));
  TXBuffer += F("<TD>");
  TXBuffer += F("Show files on SD-Card");
#endif

  TXBuffer += F("</table></form>");
  sendHeadandTail(F("TmplStd"),_TAIL);
  TXBuffer.endStream();
  printWebString = "";
  printToWeb = false;
}


//********************************************************************************
// Web Interface pin state list
//********************************************************************************
void handle_pinstates() {
  checkRAM(F("handle_pinstates"));
  if (!isLoggedIn()) return;
  navMenuIndex = 7;
  TXBuffer.startStream();
  sendHeadandTail(F("TmplStd"),_HEAD);





  //addFormSubHeader(F("Pin state table<TR>"));

  TXBuffer += F("<table class='multirow' border=1px frame='box' rules='all'><TH>Plugin");
  addHelpButton(F("Official_plugin_list"));
  TXBuffer += F("<TH>GPIO<TH>Mode<TH>Value/State");
  for (byte x = 0; x < PINSTATE_TABLE_MAX; x++)
    if (pinStates[x].plugin != 0)
    {
      TXBuffer += F("<TR><TD>P");
      if (pinStates[x].plugin < 100)
      {
        TXBuffer += F("0");
      }
      if (pinStates[x].plugin < 10)
      {
        TXBuffer += F("0");
      }
      TXBuffer += pinStates[x].plugin;
      TXBuffer += F("<TD>");
      TXBuffer += pinStates[x].index;
      TXBuffer += F("<TD>");
      byte mode = pinStates[x].mode;
      switch (mode)
      {
        case PIN_MODE_UNDEFINED:
          TXBuffer += F("undefined");
          break;
        case PIN_MODE_INPUT:
          TXBuffer += F("input");
          break;
        case PIN_MODE_OUTPUT:
          TXBuffer += F("output");
          break;
        case PIN_MODE_PWM:
          TXBuffer += F("PWM");
          break;
        case PIN_MODE_SERVO:
          TXBuffer += F("servo");
          break;
      }
      TXBuffer += F("<TD>");
      TXBuffer += pinStates[x].value;
    }

  TXBuffer += F("</table>");
    sendHeadandTail(F("TmplStd"),_TAIL);
    TXBuffer.endStream();
}


//********************************************************************************
// Web Interface I2C scanner
//********************************************************************************
void handle_i2cscanner() {
  checkRAM(F("handle_i2cscanner"));
  if (!isLoggedIn()) return;
  navMenuIndex = 7;
  TXBuffer.startStream();
  sendHeadandTail(F("TmplStd"),_HEAD);

  char *TempString = (char*)malloc(80);



  TXBuffer += F("<table class='multirow' border=1px frame='box' rules='all'><TH>I2C Addresses in use<TH>Supported devices");

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
      TXBuffer += F("<TR><TD>Unknown error at address ");
      TXBuffer += formatToHex(address);
    }
  }

  if (nDevices == 0)
    TXBuffer += F("<TR>No I2C devices found");

  TXBuffer += F("</table>");
  sendHeadandTail(F("TmplStd"),_TAIL);
  TXBuffer.endStream();
  free(TempString);
}


//********************************************************************************
// Web Interface Wifi scanner
//********************************************************************************
void handle_wifiscanner() {
  checkRAM(F("handle_wifiscanner"));
  if (!isLoggedIn()) return;
  navMenuIndex = 7;
  TXBuffer.startStream();
  sendHeadandTail(F("TmplStd"),_HEAD);
  TXBuffer += F("<table class='multirow'><TR><TH>SSID<TH>BSSID<TH>info");

  int n = WiFi.scanNetworks(false, true);
  if (n == 0)
    TXBuffer += F("No Access Points found");
  else
  {
    for (int i = 0; i < n; ++i)
    {
      TXBuffer += F("<TR><TD>");
      TXBuffer += formatScanResult(i, "<TD>");
    }
  }

  TXBuffer += F("</table>");
  sendHeadandTail(F("TmplStd"),_TAIL);
  TXBuffer.endStream();
}


//********************************************************************************
// Web Interface login page
//********************************************************************************
void handle_login() {
  checkRAM(F("handle_login"));
  if (!clientIPallowed()) return;
  TXBuffer.startStream();
  sendHeadandTail(F("TmplStd"),_HEAD);

  String webrequest = WebServer.arg(F("password"));
  char command[80];
  command[0] = 0;
  webrequest.toCharArray(command, 80);


  TXBuffer += F("<form method='post'>");
  TXBuffer += F("<table class='normal'><TR><TD>Password<TD>");
  TXBuffer += F("<input class='wide' type='password' name='password' value='");
  TXBuffer += webrequest;
  TXBuffer += F("'><TR><TD><TD>");
  addSubmitButton();
  TXBuffer += F("<TR><TD>");
  TXBuffer += F("</table></form>");

  if (webrequest.length() != 0)
  {
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

  sendHeadandTail(F("TmplStd"),_TAIL);
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
  // sendHeadandTail(F("TmplStd"),_HEAD);
  String webrequest = WebServer.arg(F("cmd"));

  // in case of event, store to buffer and return...
  String command = parseString(webrequest, 1);
  if (command == F("event"))
  {
    eventBuffer = webrequest.substring(6);
    WebServer.send(200, "text/html", "OK");
    return;
  }
  else if (command.equalsIgnoreCase(F("taskrun")) ||
           command.equalsIgnoreCase(F("taskvalueset")) ||
           command.equalsIgnoreCase(F("rules"))) {
    addLog(LOG_LEVEL_INFO,String(F("HTTP : ")) + webrequest);
    ExecuteCommand(VALUE_SOURCE_HTTP,webrequest.c_str());
    WebServer.send(200, "text/html", "OK");
    return;
  }

  struct EventStruct TempEvent;
  parseCommandString(&TempEvent, webrequest);
  TempEvent.Source = VALUE_SOURCE_HTTP;

  printToWeb = true;
  printWebString = "";

  if (printToWebJSON)
    TXBuffer.startJsonStream();
  else
    TXBuffer.startStream();

  if (PluginCall(PLUGIN_WRITE, &TempEvent, webrequest));
  else if (remoteConfig(&TempEvent, webrequest));
  else
    TXBuffer += F("Unknown or restricted command!");

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
  TXBuffer += F("\"");
  TXBuffer += object;
  TXBuffer += F("\":");
  if (value.length() == 0 || !isFloat(value)) {
    TXBuffer += F("\"");
    TXBuffer += value;
    TXBuffer += F("\"");
  } else {
    TXBuffer += value;
  }
}

String jsonBool(bool value) {
  return value ? F("true") : F("false");
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
  {
    String view = WebServer.arg("view");
    if (view.length() != 0) {
      if (view == F("sensorupdate")) {
        showSystem = false;
        showWifi = false;
        showDataAcquisition = false;
        showTaskDetails = false;
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
      TXBuffer += F(",\n");
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
      TXBuffer += F(",\n");
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
          TXBuffer += F("{");
          stream_next_json_object_value(F("ValueNumber"), String(x + 1));
          stream_next_json_object_value(F("Name"), String(ExtraTaskSettings.TaskDeviceValueNames[x]));
          stream_next_json_object_value(F("NrDecimals"), String(ExtraTaskSettings.TaskDeviceValueDecimals[x]));
          stream_last_json_object_value(F("Value"), formatUserVarNoCheck(TaskIndex, x));
          if (x < (Device[DeviceIndex].ValueCount - 1))
            TXBuffer += F(",\n");
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
          TXBuffer += F("{");
          stream_next_json_object_value(F("Controller"), String(x + 1));
          stream_next_json_object_value(F("IDX"), String(Settings.TaskDeviceID[x][TaskIndex]));
          stream_last_json_object_value(F("Enabled"), jsonBool(Settings.TaskDeviceSendData[x][TaskIndex]));
          if (x < (CONTROLLER_MAX - 1))
            TXBuffer += F(",\n");
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
        TXBuffer += F(",");
      TXBuffer += F("\n");
    }
  }
  if (!showSpecificTask) {
    TXBuffer += F("],\n");
    stream_last_json_object_value(F("TTL"), String(ttl_json * 1000));
  }

  TXBuffer.endStream();
}

//********************************************************************************
// Web Interface config page
//********************************************************************************
void handle_advanced() {
  checkRAM(F("handle_advanced"));
  if (!isLoggedIn()) return;
  navMenuIndex = 7;
  TXBuffer.startStream();
  sendHeadandTail(F("TmplStd"));

  char tmpString[81];

  String ip = WebServer.arg(F("ip"));
  String syslogip = WebServer.arg(F("syslogip"));
  String ntphost = WebServer.arg(F("ntphost"));
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
    Settings.IP_Octet = ip.toInt();
    ntphost.toCharArray(tmpString, 64);
    strcpy(Settings.NTPHost, tmpString);
    Settings.TimeZone = timezone;
    TimeChangeRule dst_start(dststartweek, dststartdow, dststartmonth, dststarthour, timezone);
    if (dst_start.isValid()) { Settings.DST_Start = dst_start.toFlashStoredValue(); }
    TimeChangeRule dst_end(dstendweek, dstenddow, dstendmonth, dstendhour, timezone);
    if (dst_end.isValid()) { Settings.DST_End = dst_end.toFlashStoredValue(); }
    str2ip(syslogip.c_str(), Settings.Syslog_IP);
    Settings.UDPPort = getFormItemInt(F("udpport"));
    Settings.SyslogLevel = getFormItemInt(F("sysloglevel"));
    Settings.SyslogFacility = getFormItemInt(F("syslogfacility"));
    Settings.UseSerial = isFormItemChecked(F("useserial"));
    Settings.SerialLogLevel = getFormItemInt(F("serialloglevel"));
    Settings.WebLogLevel = getFormItemInt(F("webloglevel"));
    Settings.SDLogLevel = getFormItemInt(F("sdloglevel"));
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
    Settings.Latitude = getFormItemFloat(F("latitude"));
    Settings.Longitude = getFormItemFloat(F("longitude"));

    addHtmlError(SaveSettings());
    if (Settings.UseNTP)
      initTime();
  }

  // char str[20];

  TXBuffer += F("<form  method='post'><table class='normal'>");

  addFormHeader(F("Advanced Settings"));

  addFormCheckBox(F("Rules"), F("userules"), Settings.UseRules);

  addFormSubHeader(F("Controller Settings"));

  addFormCheckBox(F("MQTT Retain Msg"), F("mqttretainflag"), Settings.MQTTRetainFlag);
  addFormNumericBox( F("Message Interval"), F("messagedelay"), Settings.MessageDelay, 0, INT_MAX);
  addUnit(F("ms"));
  addFormCheckBox(F("MQTT usage unit name as ClientId"), F("mqttuseunitnameasclientid"), Settings.MQTTUseUnitNameAsClientId);

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

  addFormCheckBox(F("Use SSDP"), F("usessdp"), Settings.UseSSDP);

  addFormNumericBox(F("Connection Failure Threshold"), F("cft"), Settings.ConnectionFailuresThreshold, 0, 100);

  addFormNumericBox(F("I2C ClockStretchLimit"), F("wireclockstretchlimit"), Settings.WireClockStretchLimit);   //TODO define limits
  #if defined(FEATURE_ARDUINO_OTA)
  addFormCheckBox(F("Enable Arduino OTA"), F("arduinootaenable"), Settings.ArduinoOTAEnable);
  #endif
  #if defined(ESP32)
    addFormCheckBox(F("Enable RTOS Multitasking"), F("usertosmultitasking"), Settings.UseRTOSMultitasking);
  #endif

  addFormSeparator(2);

  TXBuffer += F("<TR><TD style='width:150px;' align='left'><TD>");
  addSubmitButton();
  TXBuffer += F("<input type='hidden' name='edit' value='1'>");
  TXBuffer += F("</table></form>");
  sendHeadandTail(F("TmplStd"),true);
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

void addLogLevelSelect(String name, int choice)
{
  String options[LOG_LEVEL_NRELEMENTS + 1];
  int optionValues[LOG_LEVEL_NRELEMENTS + 1] = {0};
  options[0] = F("None");
  optionValues[0] = 0;
  for (int i = 0; i < LOG_LEVEL_NRELEMENTS; ++i) {
    options[i + 1] = getLogLevelDisplayString(i, optionValues[i + 1]);
  }
  addSelector(name, LOG_LEVEL_NRELEMENTS + 1, options, optionValues, NULL, choice, false);
}

void addFormLogFacilitySelect(const String& label, const String& id, int choice)
{
  addRowLabel(label);
  addLogFacilitySelect(id, choice);
}

void addLogFacilitySelect(String name, int choice)
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
  navMenuIndex = 7;
//  TXBuffer.startStream();
//  sendHeadandTail(F("TmplStd"));


  fs::File dataFile = SPIFFS.open(F(FILE_CONFIG), "r");
  if (!dataFile)
    return;

  String str = F("attachment; filename=config_");
  str += Settings.Name;
  str += "_U";
  str += Settings.Unit;
  str += F("_Build");
  str += BUILD;
  str += F("_");
  if (Settings.UseNTP)
  {
    str += getDateTimeString('\0', '\0', '\0');
  }
  str += F(".dat");

  WebServer.sendHeader(F("Content-Disposition"), str);
  WebServer.streamFile(dataFile, F("application/octet-stream"));
}


//********************************************************************************
// Web Interface upload page
//********************************************************************************
byte uploadResult = 0;
void handle_upload() {
  if (!isLoggedIn()) return;
  navMenuIndex = 7;
  TXBuffer.startStream();
  sendHeadandTail(F("TmplStd"));

  TXBuffer += F("<form enctype='multipart/form-data' method='post'><p>Upload settings file:<br><input type='file' name='datafile' size='40'></p><div><input class='button link' type='submit' value='Upload'></div><input type='hidden' name='edit' value='1'></form>");
  sendHeadandTail(F("TmplStd"),true);
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

  navMenuIndex = 7;
  TXBuffer.startStream();
  sendHeadandTail(F("TmplStd"));



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
  sendHeadandTail(F("TmplStd"),true);
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
  String log = "";

  HTTPUpload& upload = WebServer.upload();

  if (upload.filename.c_str()[0] == 0)
  {
    uploadResult = 3;
    return;
  }

  if (upload.status == UPLOAD_FILE_START)
  {
    log = F("Upload: START, filename: ");
    log += upload.filename;
    addLog(LOG_LEVEL_INFO, log);
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
    log = F("Upload: WRITE, Bytes: ");
    log += upload.currentSize;
    addLog(LOG_LEVEL_INFO, log);
  }
  else if (upload.status == UPLOAD_FILE_END)
  {
    if (uploadFile) uploadFile.close();
    log = F("Upload: END, Size: ");
    log += upload.totalSize;
    addLog(LOG_LEVEL_INFO, log);
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
  checkRAM(F("loadFromFS"));
  if (!isLoggedIn()) return false;

  statusLED(true);

  String dataType = F("text/plain");
  if (path.endsWith(F("/"))) path += F("index.htm");

  if (path.endsWith(F(".src"))) path = path.substring(0, path.lastIndexOf("."));
  else if (path.endsWith(F(".htm"))) dataType = F("text/html");
  else if (path.endsWith(F(".css"))) dataType = F("text/css");
  else if (path.endsWith(F(".js"))) dataType = F("application/javascript");
  else if (path.endsWith(F(".png"))) dataType = F("image/png");
  else if (path.endsWith(F(".gif"))) dataType = F("image/gif");
  else if (path.endsWith(F(".jpg"))) dataType = F("image/jpeg");
  else if (path.endsWith(F(".ico"))) dataType = F("image/x-icon");
  else if (path.endsWith(F(".txt"))) dataType = F("application/octet-stream");
  else if (path.endsWith(F(".dat"))) dataType = F("application/octet-stream");
  else if (path.endsWith(F(".esp"))) return handle_custom(path);
  String log = F("HTML : Request file ");
  log += path;

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

  addLog(LOG_LEVEL_DEBUG, log);
  return true;
}

//********************************************************************************
// Web Interface custom page handler
//********************************************************************************
boolean handle_custom(String path) {
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
      TXBuffer.startStream();
      sendHeadandTail(F("TmplDsh"),_HEAD);
      char url[40];
      sprintf_P(url, PSTR("http://%u.%u.%u.%u/dashboard.esp"), Nodes[unit].ip[0], Nodes[unit].ip[1], Nodes[unit].ip[2], Nodes[unit].ip[3]);
      TXBuffer += F("<meta http-equiv=\"refresh\" content=\"0; URL=");
      TXBuffer += url;
      TXBuffer += F("\">");
      sendHeadandTail(F("TmplDsh"),_TAIL);
      TXBuffer.endStream();
      return true;
    }

    TXBuffer.startStream();
    sendHeadandTail(F("TmplDsh"),_HEAD);
    TXBuffer += F("<script><!--\n"
             "function dept_onchange(frmselect) {frmselect.submit();}"
             "\n//--></script>");

    TXBuffer += F("<form name='frmselect' method='post'>");

    // create unit selector dropdown
    addSelector_Head(F("unit"), true);
    byte choice = Settings.Unit;
    for (byte x = 0; x < UNIT_MAX; x++)
    {
      if (Nodes[x].ip[0] != 0 || x == Settings.Unit)
      {
        String name = String(x) + F(" - ");
        if (x != Settings.Unit)
          name += Nodes[x].nodeName;
        else
          name += Settings.Name;
        addSelector_Item(name, x, choice == x, false, F(""));
      }
    }
    addSelector_Foot();

    // create <> navigation buttons
    byte prev=Settings.Unit;
    byte next=Settings.Unit;
    for (byte x = Settings.Unit-1; x > 0; x--)
      if (Nodes[x].ip[0] != 0) {prev = x; break;}
    for (byte x = Settings.Unit+1; x < UNIT_MAX; x++)
      if (Nodes[x].ip[0] != 0) {next = x; break;}

    TXBuffer += F("<a class='button link' href=");
    TXBuffer += path;
    TXBuffer += F("?btnunit=");
    TXBuffer += prev;
    TXBuffer += F(">&lt;</a>");
    TXBuffer += F("<a class='button link' href=");
    TXBuffer += path;
    TXBuffer += F("?btnunit=");
    TXBuffer += next;
    TXBuffer += F(">&gt;</a>");
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
      TXBuffer += F("<table class='normal'>");
      for (byte x = 0; x < TASKS_MAX; x++)
      {
        if (Settings.TaskDeviceNumber[x] != 0)
          {
            LoadTaskSettings(x);
            byte DeviceIndex = getDeviceIndex(Settings.TaskDeviceNumber[x]);
            TXBuffer += F("<TR><TD>");
            TXBuffer += ExtraTaskSettings.TaskDeviceName;
            for (byte varNr = 0; varNr < VARS_PER_TASK; varNr++)
              {
                if ((Settings.TaskDeviceNumber[x] != 0) && (varNr < Device[DeviceIndex].ValueCount) && ExtraTaskSettings.TaskDeviceValueNames[varNr][0] !=0)
                {
                  if (varNr > 0)
                    TXBuffer += F("<TR><TD>");
                  TXBuffer += F("<TD>");
                  TXBuffer += ExtraTaskSettings.TaskDeviceValueNames[varNr];
                  TXBuffer += F("<TD>");
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
void handle_filelist() {
  checkRAM(F("handle_filelist"));
  if (!clientIPallowed()) return;
  navMenuIndex = 7;
  TXBuffer.startStream();
  sendHeadandTail(F("TmplStd"));

#if defined(ESP8266)

  String fdelete = WebServer.arg(F("delete"));

  if (fdelete.length() > 0)
  {
    SPIFFS.remove(fdelete);
    checkRuleSets();
  }



  TXBuffer += F("<table class='multirow' border=1px frame='box' rules='all'><TH style='width:50px;'><TH>Filename<TH style='width:80px;'>Size");

  fs::Dir dir = SPIFFS.openDir("");
  while (dir.next())
  {
    TXBuffer += F("<TR><TD>");
    if (dir.fileName() != F(FILE_CONFIG) && dir.fileName() != F(FILE_SECURITY) && dir.fileName() != F(FILE_NOTIFICATION))
    {
      TXBuffer += F("<a class='button link' href=\"filelist?delete=");
      TXBuffer += dir.fileName();
      TXBuffer += F("\">Del</a>");
    }

    TXBuffer += F("<TD><a href=\"");
    TXBuffer += dir.fileName();
    TXBuffer += F("\">");
    TXBuffer += dir.fileName();
    TXBuffer += F("</a>");
    fs::File f = dir.openFile("r");
    TXBuffer += F("<TD>");
    TXBuffer += f.size();
  }
  TXBuffer += F("</table></form>");
  TXBuffer += F("<BR><a class='button link' href=\"/upload\">Upload</a><BR><BR>");
    sendHeadandTail(F("TmplStd"),true);
    TXBuffer.endStream();
#endif
#if defined(ESP32)
  String fdelete = WebServer.arg(F("delete"));

  if (fdelete.length() > 0)
  {
    SPIFFS.remove(fdelete);
    // flashCount();
  }



  TXBuffer += F("<table class='multirow' border=1px frame='box' rules='all'><TH><TH>Filename<TH>Size");

  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  while (file)
  {
    if(!file.isDirectory()){
      TXBuffer += F("<TR><TD>");
      if (strcmp(file.name(), FILE_CONFIG) != 0 && strcmp(file.name(), FILE_SECURITY) != 0 && strcmp(file.name(), FILE_NOTIFICATION) != 0)
      {
        TXBuffer += F("<a class='button link' href=\"filelist?delete=");
        TXBuffer += file.name();
        TXBuffer += F("\">Del</a>");
      }

      TXBuffer += F("<TD><a href=\"");
      TXBuffer += file.name();
      TXBuffer += F("\">");
      TXBuffer += file.name();
      TXBuffer += F("</a>");
      TXBuffer += F("<TD>");
      TXBuffer += file.size();
      file = root.openNextFile();
    }
  }
  TXBuffer += F("</table></form>");
  TXBuffer += F("<BR><a class='button link' href=\"/upload\">Upload</a><BR><BR>");
    sendHeadandTail(F("TmplStd"),true);
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
  navMenuIndex = 7;
  TXBuffer.startStream();
  sendHeadandTail(F("TmplStd"));


  String fdelete = "";
  String ddelete = "";
  String change_to_dir = "";
  String current_dir = "";
  String parent_dir = "";
  char SDcardDir[80];

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

  current_dir.toCharArray(SDcardDir, current_dir.length()+1);
  File root = SD.open(SDcardDir);
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
  TXBuffer += F("<BR>");
  TXBuffer += F("<table class='multirow' border=1px frame='box' rules='all'><TH style='width:50px;'><TH>Name<TH>Size");
  TXBuffer += F("<TR><TD>");
  TXBuffer += F("<TD><a href=\"SDfilelist?chgto=");
  TXBuffer += parent_dir;
  TXBuffer += F("\">..");
  TXBuffer += F("</a>");
  TXBuffer += F("<TD>");
  while (entry)
  {
    if (entry.isDirectory())
    {
      char SDcardChildDir[80];
      TXBuffer += F("<TR><TD>");
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
        TXBuffer += F("/");
        TXBuffer += F("&chgto=");
        TXBuffer += current_dir;
        TXBuffer += F("\">Del</a>");
      }
      TXBuffer += F("<TD><a href=\"SDfilelist?chgto=");
      TXBuffer += current_dir;
      TXBuffer += entry.name();
      TXBuffer += F("/");
      TXBuffer += F("\">");
      TXBuffer += entry.name();
      TXBuffer += F("</a>");
      TXBuffer += F("<TD>");
      TXBuffer += F("dir");
      dir_has_entry.close();
    }
    else
    {
      TXBuffer += F("<TR><TD>");
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
      TXBuffer += F("\">");
      TXBuffer += entry.name();
      TXBuffer += F("</a>");
      TXBuffer += F("<TD>");
      TXBuffer += entry.size();
    }
    entry.close();
    entry = root.openNextFile();
  }
  root.close();
  TXBuffer += F("</table></form>");
  //TXBuffer += F("<BR><a class='button link' href=\"/upload\">Upload</a>");
     sendHeadandTail(F("TmplStd"),true);
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
    message += F("\n");
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

  if (wifiStatus == ESPEASY_WIFI_SERVICES_INITIALIZED)
  {
    addHtmlError(SaveSettings());
    const IPAddress ip = WiFi.localIP();
    char host[20];
    formatIP(ip, host);
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
    strncpy(SecuritySettings.WifiKey, password.c_str(), sizeof(SecuritySettings.WifiKey));
    strncpy(SecuritySettings.WifiSSID, ssid.c_str(), sizeof(SecuritySettings.WifiSSID));
    wifiSetupConnect = true;
    String reconnectlog = F("WIFI : Credentials Changed, retry connection. SSID: ");
    reconnectlog += ssid;
    addLog(LOG_LEVEL_INFO, reconnectlog);
    status = 1;
    refreshCount = 0;
  }

  TXBuffer += F("<BR><h1>Wifi Setup wizard</h1>");
  TXBuffer += F("<form name='frmselect' method='post'>");

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
      TXBuffer += F("<table class='multirow' border=1px frame='box'><TR><TH style='width:50px;'>Pick<TH>Network info");
      for (int i = 0; i < n; ++i)
      {
        TXBuffer += F("<TR><TD><label class='container2'>");
        TXBuffer += F("<input type='radio' name='ssid' value='");
        TXBuffer += WiFi.SSID(i);
        TXBuffer += F("'");
        if (WiFi.SSID(i) == ssid)
          TXBuffer += F(" checked ");
        TXBuffer += F("><span class='dotmark'></span></label><TD>");
        TXBuffer += formatScanResult(i, "<BR>");
        TXBuffer += F("");
      }
      TXBuffer += F("</table>");
    }

    TXBuffer += F("<BR><label class='container2'>other SSID:<input type='radio' name='ssid' id='other_ssid' value='other' ><span class='dotmark'></span></label>");
    TXBuffer += F("<input class='wide' type ='text' name='other' value='");
    TXBuffer += other;
    TXBuffer += F("'><BR><BR>");

    addFormSeparator (2);

    TXBuffer += F("<BR>Password:<BR><input class='wide' type ='text' name='pass' value='");
    TXBuffer += password;
    TXBuffer += F("'><BR><BR>");

    addSubmitButton(F("Connect"),F(""));
  }

  if (status == 1)  // connecting stage...
  {
    if (refreshCount > 0)
    {
      status = 0;
//      strncpy(SecuritySettings.WifiSSID, "ssid", sizeof(SecuritySettings.WifiSSID));
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
      TXBuffer += F("   var timer = setInterval(function() {");
      TXBuffer += F("   if (timeoutPeriod > 0) {");
      TXBuffer += F("       timeoutPeriod -= 1;");
      TXBuffer += F("       document.getElementById('countdown').innerHTML = timeoutPeriod + '..' + '<br />';");
      TXBuffer += F("   } else {");
      TXBuffer += F("       clearInterval(timer);");
      TXBuffer += F("            window.location.href = window.location.href;");
      TXBuffer += F("       };");
      TXBuffer += F("   }, 1000);");
      TXBuffer += F("};");
      TXBuffer += F("timedRefresh(");
      TXBuffer += wait;
      TXBuffer += F(");");
      TXBuffer += F("</script>");
      TXBuffer += F("seconds while trying to connect");
    }
    refreshCount++;
  }

  TXBuffer += F("</form>");
   sendHeadandTail(F("TmplAP"),true);
  TXBuffer.endStream();
  delay(10);
}


//********************************************************************************
// Web Interface rules page
//********************************************************************************
void handle_rules() {
  checkRAM(F("handle_rules"));
  if (!isLoggedIn()) return;
  navMenuIndex = 5;
  TXBuffer.startStream();
  sendHeadandTail(F("TmplStd"));
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
        f.close();
      }
    }
    addLog(LOG_LEVEL_INFO, log);

    log = F(" Webserver args:");
    for (int i = 0; i < WebServer.args(); ++i) {
      log += F(" ");
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

  TXBuffer += F("<form name = 'frmselect' method = 'post'><table class='normal'><TR><TH align='left'>Rules");

  byte choice = rulesSet;
  String options[RULESETS_MAX];
  int optionValues[RULESETS_MAX];
  for (byte x = 0; x < RULESETS_MAX; x++)
  {
    options[x] = F("Rules Set ");
    options[x] += x + 1;
    optionValues[x] = x + 1;
  }

   TXBuffer += F("<TR><TD>");
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
       TXBuffer += F("<TR><TD><textarea name='rules' rows='30' wrap='off'>");
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

   TXBuffer += F("<TR><TD>Current size: ");
   TXBuffer += size;
   TXBuffer += F(" characters (Max ");
   TXBuffer += RULES_MAX_SIZE;
   TXBuffer += F(")");

  addFormSeparator(2);

   TXBuffer += F("<TR><TD>");
  addSubmitButton();
  addButton(fileName, F("Download to file"));
   TXBuffer += F("</table></form>");
  sendHeadandTail(F("TmplStd"),true);
  TXBuffer.endStream();

  checkRuleSets();
}


//********************************************************************************
// Web Interface sysinfo page
//********************************************************************************
void handle_sysinfo() {
  checkRAM(F("handle_sysinfo"));
  if (!isLoggedIn()) return;
  TXBuffer.startStream();
  sendHeadandTail(F("TmplStd"));

  int freeMem = ESP.getFreeHeap();

  addHeader(true,  TXBuffer.buf);
   TXBuffer += printWebString;
   TXBuffer += F("<form>");

   // the table header
   TXBuffer += F("<table class='normal'><TR><TH style='width:150px;' align='left'>System Info<TH align='left'>");

   addCopyButton(F("copyText"), F("\\n"), F("Copy info to clipboard") );

   TXBuffer += githublogo;

   TXBuffer += F("<TR><TD>Unit<TD>");
   TXBuffer += Settings.Unit;

  if (Settings.UseNTP)
  {

     TXBuffer += F("<TR><TD>Local Time<TD>");
     TXBuffer += getDateTimeString('-', ':', ' ');
  }

   TXBuffer += F("<TR><TD>Uptime<TD>");
  char strUpTime[40];
  int minutes = wdcounter / 2;
  int days = minutes / 1440;
  minutes = minutes % 1440;
  int hrs = minutes / 60;
  minutes = minutes % 60;
  sprintf_P(strUpTime, PSTR("%d days %d hours %d minutes"), days, hrs, minutes);
   TXBuffer += strUpTime;

   TXBuffer += F("<TR><TD>Load<TD>");
  if (wdcounter > 0)
  {
     TXBuffer += getCPUload();
     TXBuffer += F("% (LC=");
     TXBuffer += getLoopCountPerSec();
     TXBuffer += F(")");
  }

   TXBuffer += F("<TR><TD>Free Mem<TD>");
   TXBuffer += freeMem;
   TXBuffer += F(" (");
   TXBuffer += lowestRAM;
   TXBuffer += F(" - ");
   TXBuffer += lowestRAMfunction;
   TXBuffer += F(")");

   TXBuffer += F("<TR><TD>Boot<TD>");
   TXBuffer += getLastBootCauseString();
   TXBuffer += F(" (");
   TXBuffer += RTC.bootCounter;
   TXBuffer += F(")");
   TXBuffer += F("<TR><TD>Reset Reason<TD>");
   TXBuffer += getResetReasonString();

   TXBuffer += F("<TR><TD colspan=2><H3>Network");
   addHelpButton(F("Wifi"));
   TXBuffer += F("</H3></TD></TR>");

  if (wifiStatus == ESPEASY_WIFI_SERVICES_INITIALIZED)
  {
     TXBuffer += F("<TR><TD>Wifi<TD>");
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
  TXBuffer += F("<TR><TD>IP config<TD>");
  TXBuffer += useStaticIP() ? F("Static") : F("DHCP");

   TXBuffer += F("<TR><TD>IP / subnet<TD>");
   TXBuffer += formatIP(WiFi.localIP());
   TXBuffer += F(" / ");
   TXBuffer += formatIP(WiFi.subnetMask());

   TXBuffer += F("<TR><TD>GW<TD>");
   TXBuffer += formatIP(WiFi.gatewayIP());

  {
    TXBuffer += F("<TR><TD>Client IP<TD>");
    WiFiClient client(WebServer.client());
    TXBuffer += formatIP(client.remoteIP());
  }

  TXBuffer += F("<TR><TD>DNS<TD>");
  TXBuffer += formatIP(WiFi.dnsIP(0));
  TXBuffer += F(" / ");
  TXBuffer += formatIP(WiFi.dnsIP(1));

  TXBuffer += F("<TR><TD>Allowed IP Range<TD>");
  TXBuffer += describeAllowedIPrange();

  TXBuffer += F("<TR><TD>Serial Port available:<TD>");
  TXBuffer += String(SerialAvailableForWrite());
  TXBuffer += F(" (");
  #if defined(ESP8266)
    TXBuffer += Serial.availableForWrite();
  #endif
  TXBuffer += F(" , ");
  TXBuffer += Serial.available();
  TXBuffer += F(")");

  TXBuffer += F("<TR><TD>STA MAC<TD>");

  uint8_t mac[] = {0, 0, 0, 0, 0, 0};
  uint8_t* macread = WiFi.macAddress(mac);
  char macaddress[20];
  formatMAC(macread, macaddress);
  TXBuffer += macaddress;

  TXBuffer += F("<TR><TD>AP MAC<TD>");
  macread = WiFi.softAPmacAddress(mac);
  formatMAC(macread, macaddress);
  TXBuffer += macaddress;

  TXBuffer += F("<TR><TD>SSID<TD>");
  TXBuffer += WiFi.SSID();
  TXBuffer += F(" (");
  TXBuffer += WiFi.BSSIDstr();
  TXBuffer += F(")");

  TXBuffer += F("<TR><TD>Channel<TD>");
  TXBuffer += WiFi.channel();

  TXBuffer += F("<TR><TD>Connected<TD>");
  TXBuffer += format_msec_duration(timeDiff(lastConnectMoment, millis()));

  TXBuffer += F("<TR><TD>Last Disconnect Reason<TD>");
  TXBuffer += getLastDisconnectReason();

  TXBuffer += F("<TR><TD>Number reconnects<TD>");
  TXBuffer += wifi_reconnects;

  TXBuffer += F("<TR><TD colspan=2><H3>Firmware</H3></TD></TR>");

  TXBuffer += F("<TR><TD id='copyText_1'>Build<TD id='copyText_2'>");
  TXBuffer += BUILD;
  TXBuffer += F(" ");
  TXBuffer += F(BUILD_NOTES);

  TXBuffer += F("<TR><TD id='copyText_3'>Libraries<TD id='copyText_4'>");
  TXBuffer += getSystemLibraryString();

  TXBuffer += F("<TR><TD id='copyText_5'>GIT version<TD id='copyText_6'>");
  TXBuffer += BUILD_GIT;

  TXBuffer += F("<TR><TD id='copyText_7'>Plugins<TD id='copyText_8'>");
  TXBuffer += deviceCount + 1;
  TXBuffer += getPluginDescriptionString();

  TXBuffer += F("<TR><TD>Build Md5<TD>");
  for (byte i = 0; i<16; i++)    TXBuffer += String(CRCValues.compileTimeMD5[i],HEX);

   TXBuffer += F("<TR><TD>Md5 check<TD>");
  if (! CRCValues.checkPassed())
     TXBuffer += F("<font color = 'red'>fail !</font>");
  else  TXBuffer += F("passed.");

   TXBuffer += F("<TR><TD id='copyText_9'>Build time<TD id='copyText_10'>");
   TXBuffer += String(CRCValues.compileDate);
   TXBuffer += " ";
   TXBuffer += String(CRCValues.compileTime);

   TXBuffer += F("<TR><TD id='copyText_11'>Binary filename<TD id='copyText_12'>");
   TXBuffer += String(CRCValues.binaryFilename);

   TXBuffer += F("<TR><TD colspan=2><H3>ESP board</H3></TD></TR>");

   TXBuffer += F("<TR><TD>ESP Chip ID<TD>");
  #if defined(ESP8266)
     TXBuffer += ESP.getChipId();
     TXBuffer += F(" (0x");
    String espChipId(ESP.getChipId(), HEX);
    espChipId.toUpperCase();
     TXBuffer += espChipId;
     TXBuffer += F(")");

     TXBuffer += F("<TR><TD>ESP Chip Freq:<TD>");
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
     TXBuffer += F(")");

     TXBuffer += F("<TR><TD>ESP Chip Freq:<TD>");
     TXBuffer += ESP.getCpuFreqMHz();
     TXBuffer += F(" MHz");
  #endif



   TXBuffer += F("<TR><TD colspan=2><H3>Storage</H3></TD></TR>");

   TXBuffer += F("<TR><TD>Flash Chip ID<TD>");
  #if defined(ESP8266)
    uint32_t flashChipId = ESP.getFlashChipId();
    // Set to HEX may be something like 0x1640E0.
    // Where manufacturer is 0xE0 and device is 0x4016.
     TXBuffer += F("Vendor: ");
     TXBuffer += formatToHex(flashChipId & 0xFF);
     TXBuffer += F(" Device: ");
     uint32_t flashDevice = (flashChipId & 0xFF00) | ((flashChipId >> 16) & 0xFF);
     TXBuffer += formatToHex(flashDevice);
  #endif
  uint32_t realSize = getFlashRealSizeInBytes();
  uint32_t ideSize = ESP.getFlashChipSize();

   TXBuffer += F("<TR><TD>Flash Chip Real Size:<TD>");
   TXBuffer += realSize / 1024;
   TXBuffer += F(" kB");

   TXBuffer += F("<TR><TD>Flash IDE Size:<TD>");
   TXBuffer += ideSize / 1024;
   TXBuffer += F(" kB");

  // Please check what is supported for the ESP32
  #if defined(ESP8266)
     TXBuffer += F("<TR><TD>Flash IDE speed:<TD>");
     TXBuffer += ESP.getFlashChipSpeed() / 1000000;
     TXBuffer += F(" MHz");

    FlashMode_t ideMode = ESP.getFlashChipMode();
     TXBuffer += F("<TR><TD>Flash IDE mode:<TD>");
    switch (ideMode) {
      case FM_QIO:   TXBuffer += F("QIO");  break;
      case FM_QOUT:  TXBuffer += F("QOUT"); break;
      case FM_DIO:   TXBuffer += F("DIO");  break;
      case FM_DOUT:  TXBuffer += F("DOUT"); break;
      default:
          TXBuffer += F("Unknown"); break;
    }
  #endif

   TXBuffer += F("<TR><TD>Flash Writes<TD>");
   TXBuffer += RTC.flashDayCounter;
   TXBuffer += F(" daily / ");
   TXBuffer += RTC.flashCounter;
   TXBuffer += F(" boot");

   TXBuffer += F("<TR><TD>Sketch Size<TD>");
  #if defined(ESP8266)
   TXBuffer += ESP.getSketchSize() / 1024;
   TXBuffer += F(" kB (");
   TXBuffer += ESP.getFreeSketchSpace() / 1024;
   TXBuffer += F(" kB free)");
  #endif

  #ifdef ESP32
   TXBuffer += F("<TR><TD colspan=2><H3>Partitions");
   addHelpButton(F("https://dl.espressif.com/doc/esp-idf/latest/api-guides/partition-tables.html"));
   TXBuffer += F("</H3></TD></TR>");

   TXBuffer += F("<TR><TD>Data Partition Table<TD>");
//   TXBuffer += getPartitionTableHeader(F(" - "), F("<BR>"));
//   TXBuffer += getPartitionTable(ESP_PARTITION_TYPE_DATA, F(" - "), F("<BR>"));
   getPartitionTableSVG(ESP_PARTITION_TYPE_DATA, 0x5856e6);

   TXBuffer += F("<TR><TD>App Partition Table<TD>");
//   TXBuffer += getPartitionTableHeader(F(" - "), F("<BR>"));
//   TXBuffer += getPartitionTable(ESP_PARTITION_TYPE_APP , F(" - "), F("<BR>"));
   getPartitionTableSVG(ESP_PARTITION_TYPE_APP, 0xab56e6);
  #endif

   TXBuffer += F("</table></form>");
   sendHeadandTail(F("TmplStd"),true);
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
  ret += F(";");
  return ret;
}


void handle_favicon() {
  checkRAM(F("handle_favicon"));
  WebServer.send_P(200, PSTR("image/x-icon"), favicon_8b_ico, favicon_8b_ico_len);
}

#ifdef ESP32

void createSvgRectPath(unsigned int color, int xoffset, int yoffset, int size, int height, int range, float svgBarWidth) {
  float width = svgBarWidth * size / range;
  if (width < 2) width = 2;
  TXBuffer += formatToHex(color, F("<path fill=\"#"));
  TXBuffer += F("\" d=\"M");
  TXBuffer += toString(svgBarWidth * xoffset / range, 2);
  TXBuffer += ' ';
  TXBuffer += yoffset;
  TXBuffer += 'h';
  TXBuffer += toString(width, 2);
  TXBuffer += 'v';
  TXBuffer += height;
  TXBuffer += 'H';
  TXBuffer += toString(svgBarWidth * xoffset / range, 2);
  TXBuffer += F("z\"/>\n");
}

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
  TXBuffer += F("\">");
  TXBuffer += text;
  TXBuffer += F("</tspan>\n</text>");
}

void getPartitionTableSVG(byte pType, unsigned int partitionColor) {
  int nrPartitions = getPartionCount(pType);
  if (nrPartitions == 0) return;

  const int barHeight = 16;
  const int svgBarWidth = 200;
  const int shiftY = 2;

  uint32_t realSize = getFlashRealSizeInBytes();
  esp_partition_type_t partitionType = static_cast<esp_partition_type_t>(pType);
  const esp_partition_t * _mypart;
  esp_partition_iterator_t _mypartiterator = esp_partition_find(partitionType, ESP_PARTITION_SUBTYPE_ANY, NULL);
  TXBuffer += F("<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"500\" height=\"");
  TXBuffer += nrPartitions * barHeight + shiftY;
  TXBuffer += F("\">");
  int partNr = 0;
  if (_mypartiterator) {
    do {
      _mypart = esp_partition_get(_mypartiterator);
      float yOffset = partNr * barHeight + shiftY;
      createSvgRectPath(0xcdcdcd, 0, yOffset, realSize, barHeight - 2, realSize, svgBarWidth);
      createSvgRectPath(partitionColor, _mypart->address, yOffset, _mypart->size, barHeight - 2, realSize, svgBarWidth);
      float textXoffset = svgBarWidth + 2;
      float textYoffset = yOffset + 0.9 * barHeight;
      createSvgTextElement(formatHumanReadable(_mypart->size, 1024), textXoffset, textYoffset);
      textXoffset = svgBarWidth + 60;
      createSvgTextElement(_mypart->label, textXoffset, textYoffset);
      textXoffset = svgBarWidth + 130;
      createSvgTextElement(getPartitionType(_mypart->type, _mypart->subtype), textXoffset, textYoffset);
      ++partNr;
    } while ((_mypartiterator = esp_partition_next(_mypartiterator)) != NULL);
  }
  TXBuffer += F("</svg>\n");
  esp_partition_iterator_release(_mypartiterator);
}

#endif
