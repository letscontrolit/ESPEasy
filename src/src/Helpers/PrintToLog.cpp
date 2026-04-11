#include "../Helpers/PrintToLog.h"

#include "../Helpers/StringConverter.h"

PrintToLog::PrintToLog(uint8_t logLevel) : _logLevel(logLevel)
{
  _logLevelActive = loglevelActiveFor(_logLevel);
}

PrintToLog::PrintToLog(uint8_t logLevel, const String& logPrefix)
  : _prefix(logPrefix), _logLevel(logLevel)
{
  _logLevelActive = loglevelActiveFor(_logLevel);
}

PrintToLog::PrintToLog(uint8_t logLevel, const __FlashStringHelper *logPrefix)
  : _prefix(logPrefix), _logLevel(logLevel)
{
  _logLevelActive = loglevelActiveFor(_logLevel);
}

PrintToLog::~PrintToLog()
{
  sendToLog();
}

PrintToLog::operator bool() const
{
  return loglevelActiveFor(_logLevel);
}

void PrintToLog::flush()
{
  // For compatibility
  sendToLog();
}

void PrintToLog::sendToLog()
{
  // Update cached loglevel
  _logLevelActive = loglevelActiveFor(_logLevel);

  if (_logLevelActive) {
    if (!_str.isEmpty()) {
      if (_prefix.isEmpty()) {
        addLogMove(_logLevel, _str);
      } else {
        addLog(_logLevel, concat(_prefix, _str));
      }
    }
  }
  _str.clear();
}

void PrintToLog::clear() {
  _str.clear();
  _logLevelActive = loglevelActiveFor(_logLevel);
}

size_t PrintToLog::length() const
{
  return _str.length();
}

bool   PrintToLog::reserve(unsigned int size) { return _str.reserve(size); }

size_t PrintToLog::write(uint8_t c)
{
  if (!_logLevelActive) { return 0; }
  _str += (char)c;
  return 1;
}

String PrintToLog::get() const
{
  if (!_str.isEmpty() && !_prefix.isEmpty()) {
    return concat(_prefix, _str);
  }
  return _str;
}
