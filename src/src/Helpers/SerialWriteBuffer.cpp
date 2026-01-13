#include "../Helpers/SerialWriteBuffer.h"

#include "../Helpers/StringConverter.h"


String SerialWriteBuffer_t::colorize(const String& str) const {
#if FEATURE_COLORIZE_CONSOLE_LOGS
  const __FlashStringHelper *format = F("%s");

  switch (_loglevel)
  {
    case LOG_LEVEL_NONE: format = F("\033[31;1m%s\033[0m");     // Red + Bold or increased intensity
      break;
    case LOG_LEVEL_INFO: format = F("\033[36m%s\033[0m");       // Cyan
      break;
    case LOG_LEVEL_ERROR: format = F("\033[31;1m%s\033[0m");    // Red + Bold or increased intensity
      break;
# ifndef BUILD_NO_DEBUG
    case LOG_LEVEL_DEBUG: format = F("\033[32m%s\033[0m");      // Green
      break;
    case LOG_LEVEL_DEBUG_MORE: format = F("\033[35m%s\033[0m"); // Purple
      break;
    case LOG_LEVEL_DEBUG_DEV: format = F("\033[33m%s\033[0m");  // Yellow
      break;
# endif // ifndef BUILD_NO_DEBUG
    default:
      return str;

  }
  return strformat(format, str.c_str());
#else // if FEATURE_COLORIZE_CONSOLE_LOGS
  return str;
#endif // if FEATURE_COLORIZE_CONSOLE_LOGS
}


size_t SerialWriteBuffer_t::write_skipping(Stream& stream)
{
  size_t bytesWritten{};

  // Mark with empty line we skipped the rest of the message.
  bytesWritten += stream.println(F(" ..."));
  bytesWritten += stream.println();
  return bytesWritten;
}

void SerialWriteBuffer_t::prepare_prefix()
{
  // Prepare prefix
  _prefix = format_msec_duration(_timestamp);

  if (_loglevel == LOG_LEVEL_NONE) {
    _prefix += colorize(F(" : >  "));
  } else {
      #ifndef LIMIT_BUILD_SIZE
    _prefix += strformat(F(" : (%d) "), FreeMem());
      #else
    _prefix += ' ';
      #endif // ifndef LIMIT_BUILD_SIZE
    {
      String loglevelDisplayString = getLogLevelDisplayString(_loglevel);

      while (loglevelDisplayString.length() < LOG_LEVEL_MAX_STRING_LENGTH) {
        loglevelDisplayString += ' ';
      }
      _prefix += colorize(loglevelDisplayString);
    }
    _prefix += F(" | ");
  }

}
