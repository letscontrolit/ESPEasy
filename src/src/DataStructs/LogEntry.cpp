#include "../DataStructs/LogEntry.h"

#include "../Helpers/ESPEasy_time_calc.h"

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
    #ifdef USE_SECOND_HEAP

  // Allow to store the logs in 2nd heap if present.
  HeapSelectIram ephemeral;
    #endif // ifdef USE_SECOND_HEAP

  if (line.length() > LOG_STRUCT_MESSAGE_SIZE - 1) {
    _message = std::move(line.substring(0, LOG_STRUCT_MESSAGE_SIZE - 1));
  } else {
    _message = line;
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
    _message = std::move(line.substring(0, LOG_STRUCT_MESSAGE_SIZE - 1));
  } else {
      #ifdef USE_SECOND_HEAP

    // Allow to store the logs in 2nd heap if present.
    HeapSelectIram ephemeral;

    if (!mmu_is_iram(&(line[0]))) {
      // The log entry was not allocated on the 2nd heap, so copy instead of move
      _message = line;
    } else {
      _message = std::move(line);
    }
      #else // ifdef USE_SECOND_HEAP
    _message = std::move(line);
      #endif // ifdef USE_SECOND_HEAP
  }
  _loglevel  = loglevel;
  _timestamp = millis();
  return true;
}

void LogEntry_t::clear()
{
  _message   = String();
  _timestamp = 0;
  _loglevel  = 0;
}

bool LogEntry_t::isExpired() const
{
    return timePassedSince(_timestamp) >= LOG_BUFFER_EXPIRE;

}