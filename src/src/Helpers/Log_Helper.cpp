#include "../Helpers/Log_Helper.h"


#include "../Globals/ESPEasy_Console.h"
#include "../Helpers/Networking.h"
#include "../Helpers/StringConverter.h"


#include <FS.h>

#if FEATURE_SD
# include <SD.h>
# include "../Helpers/ESPEasy_Storage.h"
#endif // if FEATURE_SD

#if FEATURE_SD
void addToSDLog(uint8_t logLevel, const String& str)
{
  if (!str.isEmpty() && loglevelActiveFor(LOG_TO_SDCARD, logLevel)) {
    String   logName = patch_fname(F("log.txt"));
    fs::File logFile = SD.open(logName, "a+");

    if (logFile) {
      const size_t stringLength = str.length();

      for (size_t i = 0; i < stringLength; ++i) {
        logFile.print(str[i]);
      }
      logFile.println();
    }
    logFile.close();
  }
}
#endif // if FEATURE_SD

void LogHelper::addLogEntry(LogEntry_t&& logEntry)
{
      #ifdef ESP32

  if (xPortInIsrContext()) {
    // When called from an ISR, you should not send out logs.
    // Allocating memory from within an ISR is a big no-no.
    // Also long-time blocking like sending logs (especially to a syslog server)
    // is also really not a good idea from an ISR call.
    return;
  }
  #endif // ifdef ESP32

  logEntry.setSubscribers();

  if (!logEntry) { return; }

  _logBuffer.add(std::move(logEntry));

  loop();
}

bool LogHelper::getNext(LogDestination logDestination, uint32_t& timestamp, String& message, uint8_t& loglevel)
{
  return _logBuffer.getNext(logDestination, timestamp, message, loglevel);
}

uint32_t LogHelper::getNrMessages(LogDestination logDestination) const
{
  return _logBuffer.getNrMessages(logDestination);
}

void LogHelper::loop()
{
#if FEATURE_SD
    String   message;
    uint32_t timestamp{};
    uint8_t  loglevel{};

    if (_logBuffer.getNext(LOG_TO_SDCARD, timestamp, message, loglevel))
    {
      addToSDLog(loglevel, message);
    }
#endif
  _logBuffer.clearExpiredEntries();
}

bool LogHelper::logActiveRead(LogDestination logDestination)
{ 
  return _logBuffer.logActiveRead(logDestination); 
}

void LogHelper::consolePrint(const __FlashStringHelper *text) { _tmpConsoleOutput += text; }

void LogHelper::consolePrint(const String& text)              { _tmpConsoleOutput += text; }

void LogHelper::consolePrintln(const __FlashStringHelper *text)
{
  if (_tmpConsoleOutput.isEmpty()) {
    // A complete console line, thus send directly to _logBuffer
    addLogEntry({ LOG_LEVEL_NONE, text });
  } else {
    _tmpConsoleOutput += text;
    consolePrintln();
  }
}

void LogHelper::consolePrintln(const String& text)
{
  if (_tmpConsoleOutput.isEmpty()) {
    // A complete console line, thus send directly to _logBuffer
    addLogEntry({ LOG_LEVEL_NONE, text });
  } else {
    _tmpConsoleOutput += text;
    consolePrintln();
  }
}

void LogHelper::consolePrintln(String&& text)
{
  if (_tmpConsoleOutput.isEmpty()) {
    // A complete console line, thus send directly to _logBuffer
    addLogEntry({ LOG_LEVEL_NONE, std::move(text) });
  } else {
    _tmpConsoleOutput += text;
    consolePrintln();
  }
}

void LogHelper::consolePrintln()
{
  if (!_tmpConsoleOutput.isEmpty()) {
    addLogEntry({ LOG_LEVEL_NONE, std::move(_tmpConsoleOutput) });
  }
}
