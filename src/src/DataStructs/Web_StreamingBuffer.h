#ifndef DATASTRUCTS_WEB_STREAMINGBUFFER_H
#define DATASTRUCTS_WEB_STREAMINGBUFFER_H

#include <map>
#include "../../ESPEasy_common.h"


// ********************************************************************************
// Core part of WebServer, the chunked streaming buffer
// ********************************************************************************


class Web_StreamingBuffer {
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

private:

  String buf;

public:

  Web_StreamingBuffer(void);

  Web_StreamingBuffer& operator+=(char a);

  Web_StreamingBuffer& operator+=(uint64_t a);
  Web_StreamingBuffer& operator+=(int64_t a);

  Web_StreamingBuffer& operator+=(const float& a);
  Web_StreamingBuffer& operator+=(const double& a);

template <typename T>
  Web_StreamingBuffer& operator+=(T a) {
    return addString(String(a));
  }

  Web_StreamingBuffer& operator+=(const String& a);
  Web_StreamingBuffer& operator+=(PGM_P str);
  Web_StreamingBuffer& operator+=(const __FlashStringHelper* str);

  Web_StreamingBuffer& addFlashString(PGM_P str, int length = -1);
  
private:
  Web_StreamingBuffer& addString(const String& a);

public:
  void flush();

  void checkFull();

  void startStream(int httpCode = 200);

  void startStream(const __FlashStringHelper * origin, int httpCode = 200);

  void startStream(const __FlashStringHelper * content_type, const __FlashStringHelper * origin, int httpCode = 200);

  void startJsonStream();

private:

  void startStream(bool allowOriginAll, 
                   const __FlashStringHelper * content_type, 
                   const __FlashStringHelper * origin,
                   int httpCode = 200);

  void trackTotalMem();

public:

  void trackCoreMem();

  void endStream();

private: 

  void sendContentBlocking(String& data);
  void sendHeaderBlocking(bool          allowOriginAll,
                          const String& content_type,
                          const String& origin,
                          int httpCode);

};

#endif // DATASTRUCTS_WEB_STREAMINGBUFFER_H
