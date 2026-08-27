#pragma once

#include "../../ESPEasy_common.h"

#include "../DataStructs/LogBuffer.h"

#include "../DataTypes/LogLevels.h"

class LogHelper
{
public:

  LogHelper() {}

  void addLogEntry(LogEntry_t&& logEntry);

  bool getNext(LogDestination logDestination,
               uint32_t     & timestamp,
               String       & message,
               uint8_t      & loglevel);

  bool hasMessages(LogDestination logDestination);

  void loop(bool serialOnly);

  bool logActiveRead(LogDestination logDestination);


  // Append to internal buffer, which will only be flushed on consolePrintln
  void consolePrint(const __FlashStringHelper *text);
  void consolePrint(const String& text);

  void consolePrintln(const __FlashStringHelper *text);
  void consolePrintln(const String& text);
  void consolePrintln(String&& text);

  void consolePrintln();

private:

  String _tmpConsoleOutput;

  LogBuffer _logBuffer{};
}; // class LogHelper
