#ifndef HELPERS_PRINTTOLOG_H
#define HELPERS_PRINTTOLOG_H

#include "../../ESPEasy_common.h"

class PrintToLog : public Print
{
public:

  PrintToLog(uint8_t logLevel);

  PrintToLog(uint8_t       logLevel,
             const String& logPrefix);

  PrintToLog(uint8_t                    logLevel,
             const __FlashStringHelper *logPrefix);

  virtual ~PrintToLog();

  // Check if loglevel is active
  operator bool() const;

  void   flush() override;

  void   sendToLog();

  void   clear();

  size_t length() const;

  bool   reserve(unsigned int size);

  size_t write(uint8_t c) override;

  String get() const;

private:

  String _prefix;

  String _str;
  uint8_t _logLevel{};

  // Cached loglevel to speed up write.
  bool _logLevelActive{};
}; // class PrintToLog


#endif // ifndef HELPERS_PRINTTOLOG_H
