#ifndef DATASTRUCTS_TXBUFFER_STRUCT_H
#define DATASTRUCTS_TXBUFFER_STRUCT_H

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

//  Web_StreamingBuffer operator=(String& a);
//  Web_StreamingBuffer operator=(const String& a);
  Web_StreamingBuffer operator+=(char a);
  Web_StreamingBuffer operator+=(long unsigned int a);
  Web_StreamingBuffer operator+=(float a);
  Web_StreamingBuffer operator+=(int a);
  Web_StreamingBuffer operator+=(uint32_t a);
  Web_StreamingBuffer operator+=(const String& a);
  Web_StreamingBuffer operator+=(PGM_P str);
  Web_StreamingBuffer operator+=(const __FlashStringHelper* str);

//private:
  Web_StreamingBuffer addFlashString(PGM_P str);
  Web_StreamingBuffer addString(const String& a);

public:
  void flush();

  void checkFull();

  void startStream();

  void startStream(const String& origin);

  void startJsonStream();

private:

  void startStream(bool json, const String& origin);

  void trackTotalMem();

public:

  void trackCoreMem();

  void endStream();

private: 

  void sendContentBlocking(String& data);
  void sendHeaderBlocking(bool          json,
                          const String& origin = "");

};

#endif // DATASTRUCTS_TXBUFFER_STRUCT_H
