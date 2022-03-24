#include "../DataStructs/LogStruct.h"

#include "../Helpers/ESPEasy_time_calc.h"
#include "../Helpers/StringConverter.h"

void LogStruct::add_end(const uint8_t loglevel) {
  timeStamp[write_idx] = millis();
  log_level[write_idx] = loglevel;
  if (isFull()) {
    read_idx  = nextIndex(read_idx);
  }
  write_idx = nextIndex(write_idx);
  is_full = (write_idx == read_idx);
}

void LogStruct::add(const uint8_t loglevel, const String& line) {
  if (line.length() > 0)
  {
    #ifdef USE_SECOND_HEAP
    // Allow to store the logs in 2nd heap if present.
    HeapSelectIram ephemeral;
    #endif

    if (line.length() > LOG_STRUCT_MESSAGE_SIZE - 1) {
      Message[write_idx] = std::move(line.substring(0, LOG_STRUCT_MESSAGE_SIZE - 1));
    } else {
      Message[write_idx] = line;
    }
    add_end(loglevel);
  }
}

void LogStruct::add(const uint8_t loglevel, String&& line) {
  if (line.length() > 0)
  {
    if (line.length() > LOG_STRUCT_MESSAGE_SIZE - 1) {
      #ifdef USE_SECOND_HEAP
      // Need to make a substring, which is a new allocation, on the 2nd heap
      HeapSelectIram ephemeral;
      #endif
      Message[write_idx] = std::move(line.substring(0, LOG_STRUCT_MESSAGE_SIZE - 1));
    } else {
      #ifdef USE_SECOND_HEAP
      // Allow to store the logs in 2nd heap if present.
      HeapSelectIram ephemeral;

      if (!mmu_is_iram(&(line[0]))) {
        // The log entry was not allocated on the 2nd heap, so copy instead of move
        Message[write_idx] = line;
      } else {
        Message[write_idx] = std::move(line);
      }
      #else
      Message[write_idx] = std::move(line);
      #endif
    }
    add_end(loglevel);
  }
}


bool LogStruct::getNext(bool& logLinesAvailable, unsigned long& timestamp, String& message, uint8_t& loglevel) {
  lastReadTimeStamp = millis();
  logLinesAvailable = false;

  if (isEmpty()) {
    return false;
  }
  timestamp = timeStamp[read_idx];
  message = std::move(Message[read_idx]);
  loglevel = log_level[read_idx];
  clearOldest();
  if (!isEmpty()) { 
    logLinesAvailable = true;
  }
  return true;
}

bool LogStruct::isEmpty() const {
  return !is_full && (write_idx == read_idx);
}

bool LogStruct::isFull() const {
  return is_full;
}

bool LogStruct::logActiveRead() {
  clearExpiredEntries();
  return timePassedSince(lastReadTimeStamp) < LOG_BUFFER_EXPIRE;
}

void LogStruct::clearExpiredEntries() {
  unsigned int maxLoops = LOG_STRUCT_MESSAGE_LINES;
  while (maxLoops > 0) {
    --maxLoops;
    if (isEmpty() ||  // Nothing left
        (timePassedSince(timeStamp[read_idx]) < LOG_BUFFER_EXPIRE)) // Expired
    {
      return;
    }
    clearOldest();
  }
}

void LogStruct::clearOldest() {
  if (!isEmpty()) {
    is_full = false;
    Message[read_idx] = String();
    timeStamp[read_idx] = 0;
    log_level[read_idx] = 0;
    read_idx  = nextIndex(read_idx);
  }
}