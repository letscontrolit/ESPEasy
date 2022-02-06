#include "../DataStructs/Web_StreamingBuffer.h"

#include "../DataStructs/tcp_cleanup.h"
#include "../DataTypes/ESPEasyTimeSource.h"
#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../ESPEasyCore/ESPEasyNetwork.h"

// FIXME TD-er: Should keep a pointer to the webserver as a member, not use the global defined one.
#include "../Globals/Services.h"

#include "../Helpers/ESPEasy_time_calc.h"
#include "../Helpers/Convert.h"
#include "../Helpers/StringConverter.h"

#include "../../ESPEasy_common.h"

#ifdef ESP8266
#define CHUNKED_BUFFER_SIZE         512
#else 
#define CHUNKED_BUFFER_SIZE         1360
#endif

Web_StreamingBuffer::Web_StreamingBuffer(void) : lowMemorySkip(false),
  initialRam(0), beforeTXRam(0), duringTXRam(0), finalRam(0), maxCoreUsage(0),
  maxServerUsage(0), sentBytes(0), flashStringCalls(0), flashStringData(0)
{
  buf.reserve(CHUNKED_BUFFER_SIZE + 50);
  buf.clear();
}

Web_StreamingBuffer& Web_StreamingBuffer::operator+=(char a)                   {
  if (this->buf.length() >= CHUNKED_BUFFER_SIZE) {
    flush();
  }
  this->buf += a;
  return *this;
}

Web_StreamingBuffer& Web_StreamingBuffer::operator+=(uint64_t a) {
  return addString(ull2String(a));
}

Web_StreamingBuffer& Web_StreamingBuffer::operator+=(int64_t a) {
  return addString(ll2String(a));
}

Web_StreamingBuffer& Web_StreamingBuffer::operator+=(const float& a)           {
  return addString(toString(a, 2));
}

Web_StreamingBuffer& Web_StreamingBuffer::operator+=(const double& a)          {
  return addString(doubleToString(a));
}

Web_StreamingBuffer& Web_StreamingBuffer::operator+=(const String& a)          {
  return addString(a);
}

Web_StreamingBuffer& Web_StreamingBuffer::operator+=(PGM_P str) {
  return addFlashString(str);
}

Web_StreamingBuffer& Web_StreamingBuffer::operator+=(const __FlashStringHelper* str) {
  return addFlashString((PGM_P)str);
}

Web_StreamingBuffer& Web_StreamingBuffer::addFlashString(PGM_P str) {
  #ifdef USE_SECOND_HEAP
  HeapSelectDram ephemeral;
  #endif


  if (!str) { 
    return *this; // return if the pointer is void
  }

  #ifdef USE_SECOND_HEAP
  if (mmu_is_iram(str)) {
    // Have to copy the string using mmu_get functions
    // This is not a flash string.
    bool done = false;
    const char* cur_char = str;
    while (!done) {
      const uint8_t ch = mmu_get_uint8(cur_char++);
      if (ch == 0) return *this;
      if (this->buf.length() >= CHUNKED_BUFFER_SIZE) {
        flush();
      }
      this->buf += (char)ch;
    }
  }
  #endif

  ++flashStringCalls;

  if (lowMemorySkip) { return *this; }
  const unsigned int length = strlen_P((PGM_P)str);

  if (length == 0) { return *this; }
  flashStringData += length;

  int flush_step = CHUNKED_BUFFER_SIZE - this->buf.length();
  if (flush_step < 1) { flush_step = 0; }

  if (length < static_cast<unsigned int>(flush_step)) {
    // Just use the faster String operator to copy flash strings.
    this->buf += str;
    return *this;
  }

  // FIXME TD-er: Not sure what happens, but streaming large flash chunks does cause allocation issues.
  const bool stream_P = ESP.getFreeHeap() > 4000 && length < (2 * CHUNKED_BUFFER_SIZE);

  if (stream_P && ((this->buf.length() + length) > CHUNKED_BUFFER_SIZE)) {
    // Do not copy to the internal buffer, but stream immediately.
    flush();
    web_server.sendContent_P(str);
  } else {
    // Copy to internal buffer and send in chunks
    unsigned int pos          = 0;

    const char * cur_char = &str[pos];
    while (pos < length) {
      if (flush_step == 0) {
        flush();
        flush_step = CHUNKED_BUFFER_SIZE;
      }
      this->buf += (char)pgm_read_byte(cur_char);
      ++pos;
      ++cur_char;
      --flush_step;
    }
    checkFull();
  }
  return *this;
}

Web_StreamingBuffer& Web_StreamingBuffer::addString(const String& a) {
  if (lowMemorySkip) { return *this; }
  int flush_step = CHUNKED_BUFFER_SIZE - this->buf.length();

  if (flush_step < 1) { flush_step = 0; }

  const unsigned int length = a.length();

  if (length < static_cast<unsigned int>(flush_step)) {
    // Just use the faster String operator to copy flash strings.
    this->buf += a;
    return *this;
  }

  unsigned int pos = 0;
  while (pos < length) {
    if (flush_step == 0) {
      flush();
      flush_step = CHUNKED_BUFFER_SIZE;
    } else {
      // Just copy per byte instead of using substring as substring needs to allocate memory.
      this->buf += a[pos];
      ++pos;
      --flush_step;
    }
  }
  checkFull();
  return *this;
}

void Web_StreamingBuffer::flush() {
  if (lowMemorySkip) {
    this->buf.clear();
  } else {
    if (this->buf.length() > 0) {
      sendContentBlocking(this->buf);
    }
  }
}

void Web_StreamingBuffer::checkFull() {
  if (lowMemorySkip) { this->buf.clear(); }

  if (this->buf.length() >= CHUNKED_BUFFER_SIZE) {
    trackTotalMem();
    flush();
  }
}

void Web_StreamingBuffer::startStream() {
  startStream(false, F("text/html"), F(""));
}

void Web_StreamingBuffer::startStream(const String& origin) {
  startStream(false, F("text/html"), origin);
}

void Web_StreamingBuffer::startStream(const String& content_type, const String& origin) {
  startStream(false, content_type, origin);
}


void Web_StreamingBuffer::startJsonStream() {
  startStream(true, F("application/json"), F("*"));
}

void Web_StreamingBuffer::startStream(bool allowOriginAll, 
                                      const String& content_type, 
                                      const String& origin) {
  #ifdef USE_SECOND_HEAP
  HeapSelectDram ephemeral;
  #endif

  maxCoreUsage = maxServerUsage = 0;
  initialRam   = ESP.getFreeHeap();
  beforeTXRam  = initialRam;
  sentBytes    = 0;
  buf.clear();
  buf.reserve(CHUNKED_BUFFER_SIZE);
  
  if (beforeTXRam < 3000) {
    lowMemorySkip = true;
    web_server.send(200, F("text/plain"), F("Low memory. Cannot display webpage :-("));
      #if defined(ESP8266)
    tcpCleanup();
      #endif // if defined(ESP8266)
    return;
  } else {
    sendHeaderBlocking(allowOriginAll, content_type, origin);
  }
}

void Web_StreamingBuffer::trackTotalMem() {
  #ifdef USE_SECOND_HEAP
  HeapSelectDram ephemeral;
  #endif

  beforeTXRam = ESP.getFreeHeap();

  if ((initialRam - beforeTXRam) > maxServerUsage) {
    maxServerUsage = initialRam - beforeTXRam;
  }
}

void Web_StreamingBuffer::trackCoreMem() {
  #ifdef USE_SECOND_HEAP
  HeapSelectDram ephemeral;
  #endif

  duringTXRam = ESP.getFreeHeap();

  if ((initialRam - duringTXRam) > maxCoreUsage) {
    maxCoreUsage = (initialRam - duringTXRam);
  }
}

void Web_StreamingBuffer::endStream() {
  #ifdef USE_SECOND_HEAP
  HeapSelectDram ephemeral;
  #endif

  if (!lowMemorySkip) {
    if (buf.length() > 0) { sendContentBlocking(buf); }
    buf.clear();
    sendContentBlocking(buf);
    #ifdef ESP8266
    web_server.client().flush(100);
    #endif
    #ifdef ESP32
    web_server.client().flush();
    #endif
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
  #ifdef USE_SECOND_HEAP
  HeapSelectDram ephemeral;
  #endif

  delay(0); // Try to prevent WDT reboots

  const uint32_t length   = data.length();
#ifndef BUILD_NO_DEBUG
  if (loglevelActiveFor(LOG_LEVEL_DEBUG_DEV)) {
    addLog(LOG_LEVEL_DEBUG_DEV, String(F("sendcontent free: ")) + ESP.getFreeHeap() + F(" chunk size:") + length);
  }
#endif // ifndef BUILD_NO_DEBUG
  const uint32_t freeBeforeSend = ESP.getFreeHeap();
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("sendContentBlocking"));
  #endif

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
  unsigned int timeout = 1;

  if (freeBeforeSend < 5000) { timeout = 100; }

  if (freeBeforeSend < 4000) { timeout = 300; }
  web_server.sendContent(data);

  data.clear();
  const uint32_t beginWait = millis();
  while ((!data.reserve(CHUNKED_BUFFER_SIZE) || (ESP.getFreeHeap() < 4000 /*freeBeforeSend*/ )) &&
         !timeOutReached(beginWait + timeout)) {
    if (ESP.getFreeHeap() < duringTXRam) {
      duringTXRam = ESP.getFreeHeap();
    }
    trackCoreMem();
    #ifndef BUILD_NO_RAM_TRACKER
    checkRAM(F("duringDataTX"));
    #endif

    delay(1);
  }
#endif // if defined(ESP8266) && defined(ARDUINO_ESP8266_RELEASE_2_3_0)

  sentBytes += length;
  delay(0);
}

void Web_StreamingBuffer::sendHeaderBlocking(bool allowOriginAll, 
                                             const String& content_type, 
                                             const String& origin) {
  #ifdef USE_SECOND_HEAP
  HeapSelectDram ephemeral;
  #endif

  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("sendHeaderBlocking"));
  #endif
  
  web_server.client().flush();

#if defined(ESP8266) && defined(ARDUINO_ESP8266_RELEASE_2_3_0)
  web_server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  web_server.sendHeader(F("Accept-Ranges"),     F("none"));
  web_server.sendHeader(F("Cache-Control"),     F("no-cache"));
  web_server.sendHeader(F("Transfer-Encoding"), F("chunked"));

  if (allowOriginAll) {
    web_server.sendHeader(F("Access-Control-Allow-Origin"), "*");
  }
  web_server.send(200, content_type, EMPTY_STRING);
#else // if defined(ESP8266) && defined(ARDUINO_ESP8266_RELEASE_2_3_0)
  unsigned int timeout          = 0;
  const uint32_t freeBeforeSend = ESP.getFreeHeap();

  if (freeBeforeSend < 5000) { timeout = 100; }

  if (freeBeforeSend < 4000) { timeout = 1000; }
  const uint32_t beginWait = millis();
  web_server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  web_server.sendHeader(F("Cache-Control"), F("no-cache"));

  if (origin.length() > 0) {
    web_server.sendHeader(F("Access-Control-Allow-Origin"), origin);
  }
  web_server.send(200, content_type, EMPTY_STRING);

  // dont wait on 2.3.0. Memory returns just too slow.
  while ((ESP.getFreeHeap() < freeBeforeSend) &&
         !timeOutReached(beginWait + timeout)) {
    #ifndef BUILD_NO_RAM_TRACKER
    checkRAM(F("duringHeaderTX"));
    #endif
    delay(1);
  }
#endif // if defined(ESP8266) && defined(ARDUINO_ESP8266_RELEASE_2_3_0)
  delay(0);
}
