#include "../DataStructs/Web_StreamingBuffer.h"

#include "../DataStructs/tcp_cleanup.h"
#include "../DataTypes/ESPEasyTimeSource.h"
#include "../ESPEasyCore/ESPEasy_Log.h"

// FIXME TD-er: Should keep a pointer to the webserver as a member, not use the global defined one.
#include "../Globals/Services.h"

#include "../Helpers/ESPEasy_time_calc.h"



#define CHUNKED_BUFFER_SIZE          400

Web_StreamingBuffer::Web_StreamingBuffer(void) : lowMemorySkip(false),
  initialRam(0), beforeTXRam(0), duringTXRam(0), finalRam(0), maxCoreUsage(0),
  maxServerUsage(0), sentBytes(0), flashStringCalls(0), flashStringData(0)
{
  buf.reserve(CHUNKED_BUFFER_SIZE + 50);
  buf = "";
}

Web_StreamingBuffer Web_StreamingBuffer::operator=(String& a)                 {
  flush(); return addString(a);
}

Web_StreamingBuffer Web_StreamingBuffer::operator=(const String& a)           {
  flush(); return addString(a);
}

Web_StreamingBuffer Web_StreamingBuffer::operator+=(char a)                   {
  return addString(String(a));
}

Web_StreamingBuffer Web_StreamingBuffer::operator+=(long unsigned int a)     {
  return addString(String(a));
}

Web_StreamingBuffer Web_StreamingBuffer::operator+=(float a)                  {
  return addString(String(a));
}

Web_StreamingBuffer Web_StreamingBuffer::operator+=(int a)                    {
  return addString(String(a));
}

Web_StreamingBuffer Web_StreamingBuffer::operator+=(uint32_t a)               {
  return addString(String(a));
}

Web_StreamingBuffer Web_StreamingBuffer::operator+=(const String& a)          {
  return addString(a);
}

Web_StreamingBuffer Web_StreamingBuffer::operator+=(PGM_P str) {
  ++flashStringCalls;

  if (!str) { return *this; // return if the pointer is void
  }

  if (lowMemorySkip) { return *this; }
  int flush_step = CHUNKED_BUFFER_SIZE - this->buf.length();

  if (flush_step < 1) { flush_step = 0; }
  unsigned int pos          = 0;
  const unsigned int length = strlen_P((PGM_P)str);

  if (length == 0) { return *this; }
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

Web_StreamingBuffer Web_StreamingBuffer::addString(const String& a) {
  if (lowMemorySkip) { return *this; }
  int flush_step = CHUNKED_BUFFER_SIZE - this->buf.length();

  if (flush_step < 1) { flush_step = 0; }
  int pos          = 0;
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

void Web_StreamingBuffer::flush() {
  if (lowMemorySkip) {
    this->buf = "";
  } else {
    sendContentBlocking(this->buf);
  }
}

void Web_StreamingBuffer::checkFull(void) {
  if (lowMemorySkip) { this->buf = ""; }

  if (this->buf.length() > CHUNKED_BUFFER_SIZE) {
    trackTotalMem();
    sendContentBlocking(this->buf);
  }
}

void Web_StreamingBuffer::startStream() {
  startStream(false, "");
}

void Web_StreamingBuffer::startStream(const String& origin) {
  startStream(false, origin);
}

void Web_StreamingBuffer::startJsonStream() {
  startStream(true, "*");
}

void Web_StreamingBuffer::startStream(bool json, const String& origin) {
  maxCoreUsage = maxServerUsage = 0;
  initialRam   = ESP.getFreeHeap();
  beforeTXRam  = initialRam;
  sentBytes    = 0;
  buf          = "";

  if (beforeTXRam < 3000) {
    lowMemorySkip = true;
    web_server.send(200, "text/plain", "Low memory. Cannot display webpage :-(");
      #if defined(ESP8266)
    tcpCleanup();
      #endif // if defined(ESP8266)
    return;
  } else {
    sendHeaderBlocking(json, origin);
  }
}

void Web_StreamingBuffer::trackTotalMem() {
  beforeTXRam = ESP.getFreeHeap();

  if ((initialRam - beforeTXRam) > maxServerUsage) {
    maxServerUsage = initialRam - beforeTXRam;
  }
}

void Web_StreamingBuffer::trackCoreMem() {
  duringTXRam = ESP.getFreeHeap();

  if ((initialRam - duringTXRam) > maxCoreUsage) {
    maxCoreUsage = (initialRam - duringTXRam);
  }
}

void Web_StreamingBuffer::endStream(void) {
  if (!lowMemorySkip) {
    if (buf.length() > 0) { sendContentBlocking(buf); }
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




void Web_StreamingBuffer::sendContentBlocking(String& data) {
  checkRAM(F("sendContentBlocking"));
  uint32_t freeBeforeSend = ESP.getFreeHeap();
  const uint32_t length   = data.length();
#ifndef BUILD_NO_DEBUG
  addLog(LOG_LEVEL_DEBUG_DEV, String("sendcontent free: ") + freeBeforeSend + " chunk size:" + length);
#endif // ifndef BUILD_NO_DEBUG
  freeBeforeSend = ESP.getFreeHeap();

  if (beforeTXRam > freeBeforeSend) {
    beforeTXRam = freeBeforeSend;
  }
  duringTXRam = freeBeforeSend;
#if defined(ESP8266) && defined(ARDUINO_ESP8266_RELEASE_2_3_0)
  String size = formatToHex(length) + "\r\n";

  // do chunked transfer encoding ourselves (WebServer doesn't support it)
  web_server.sendContent(size);

  if (length > 0) { web_server.sendContent(data); }
  web_server.sendContent("\r\n");
#else // ESP8266 2.4.0rc2 and higher and the ESP32 webserver supports chunked http transfer
  unsigned int timeout = 0;

  if (freeBeforeSend < 5000) { timeout = 100; }

  if (freeBeforeSend < 4000) { timeout = 1000; }
  const uint32_t beginWait = millis();
  web_server.sendContent(data);

  while ((ESP.getFreeHeap() < freeBeforeSend) &&
         !timeOutReached(beginWait + timeout)) {
    if (ESP.getFreeHeap() < duringTXRam) {
      duringTXRam = ESP.getFreeHeap();
    }
    trackCoreMem();
    checkRAM(F("duringDataTX"));
    delay(1);
  }
#endif // if defined(ESP8266) && defined(ARDUINO_ESP8266_RELEASE_2_3_0)

  sentBytes += length;
  data                = "";
  delay(0);
}

void Web_StreamingBuffer::sendHeaderBlocking(bool json, const String& origin) {
  checkRAM(F("sendHeaderBlocking"));
  web_server.client().flush();
  String contenttype;

  if (json) {
    contenttype = F("application/json");
  }
  else {
    contenttype = F("text/html");
  }

#if defined(ESP8266) && defined(ARDUINO_ESP8266_RELEASE_2_3_0)
  web_server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  web_server.sendHeader(F("Accept-Ranges"),     F("none"));
  web_server.sendHeader(F("Cache-Control"),     F("no-cache"));
  web_server.sendHeader(F("Transfer-Encoding"), F("chunked"));

  if (json) {
    web_server.sendHeader(F("Access-Control-Allow-Origin"), "*");
  }
  web_server.send(200, contenttype, "");
#else // if defined(ESP8266) && defined(ARDUINO_ESP8266_RELEASE_2_3_0)
  unsigned int timeout        = 0;
  uint32_t     freeBeforeSend = ESP.getFreeHeap();

  if (freeBeforeSend < 5000) { timeout = 100; }

  if (freeBeforeSend < 4000) { timeout = 1000; }
  const uint32_t beginWait = millis();
  web_server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  web_server.sendHeader(F("Cache-Control"), F("no-cache"));

  if (origin.length() > 0) {
    web_server.sendHeader(F("Access-Control-Allow-Origin"), origin);
  }
  web_server.send(200, contenttype, "");

  // dont wait on 2.3.0. Memory returns just too slow.
  while ((ESP.getFreeHeap() < freeBeforeSend) &&
         !timeOutReached(beginWait + timeout)) {
    checkRAM(F("duringHeaderTX"));
    delay(1);
  }
#endif // if defined(ESP8266) && defined(ARDUINO_ESP8266_RELEASE_2_3_0)
  delay(0);
}
