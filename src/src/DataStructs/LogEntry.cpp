#include "../DataStructs/LogEntry.h"

#include "../Helpers/ESPEasy_time_calc.h"
#include "../Helpers/StringConverter.h"

#define LOG_STRUCT_MESSAGE_SIZE 128

#ifdef ESP32
  #define LOG_BUFFER_EXPIRE         30000  // Time after which a buffered log item is considered expired.
#else
  #define LOG_BUFFER_EXPIRE         5000  // Time after which a buffered log item is considered expired.
#endif


bool LogEntry_t::add(const uint8_t loglevel, const String& line)
{
  if (line.length() == 0) {
    return false;
  }

  {
    if (line.length() > LOG_STRUCT_MESSAGE_SIZE - 1) {
      #ifdef USE_SECOND_HEAP

      // Need to make a substring or a copy, which is a new allocation, on the 2nd heap
      HeapSelectIram ephemeral;
      #endif // ifdef USE_SECOND_HEAP
      move_special(_message, line.substring(0, LOG_STRUCT_MESSAGE_SIZE - 1));
    } else {
      reserve_special(_message, line.length());
      _message = line;
    }
  }
  _loglevel  = loglevel;
  _timestamp = millis();
  return true;
}

bool LogEntry_t::add(const uint8_t loglevel, String&& line)
{
  if (line.length() == 0) {
    return false;
  }

  if (line.length() > LOG_STRUCT_MESSAGE_SIZE - 1) {
      #ifdef USE_SECOND_HEAP

    // Need to make a substring, which is a new allocation, on the 2nd heap
    HeapSelectIram ephemeral;
      #endif // ifdef USE_SECOND_HEAP
    move_special(_message, line.substring(0, LOG_STRUCT_MESSAGE_SIZE - 1));
  } else {
    move_special(_message, std::move(line));
  }
  _loglevel  = loglevel;
  _timestamp = millis();
  return true;
}

void LogEntry_t::clear()
{
  free_string(_message);
  _timestamp = 0;
  _loglevel  = 0;
}

bool LogEntry_t::isExpired() const
{
    return timePassedSince(_timestamp) >= LOG_BUFFER_EXPIRE;

}