#include "../DataStructs/LogStruct.h"

#include "../Helpers/ESPEasy_time_calc.h"
#include "../Helpers/StringConverter.h"

void LogStruct::add_end() {
  if (isFull()) {
    read_idx = nextIndex(read_idx);
  }
  write_idx = nextIndex(write_idx);
  is_full   = (write_idx == read_idx);
}

void LogStruct::add(const uint8_t loglevel, const String& line) {
  if (Message[write_idx].add(loglevel, line)) {
    add_end();
  }
}

void LogStruct::add(const uint8_t loglevel, String&& line) {
  if (Message[write_idx].add(loglevel, std::move(line))) {
    add_end();
  }
}

bool LogStruct::getNext(bool& logLinesAvailable, unsigned long& timestamp, String& message, uint8_t& loglevel) {
  lastReadTimeStamp = millis();
  logLinesAvailable = false;

  if (isEmpty()) {
    return false;
  }
  timestamp = Message[read_idx]._timestamp;
  message   = std::move(Message[read_idx]._message);
  loglevel  = Message[read_idx]._loglevel;
  clearOldest();

  if (!isEmpty()) {
    logLinesAvailable = true;
  }
  return true;
}


bool LogStruct::logActiveRead() {
  clearExpiredEntries();
  return timePassedSince(lastReadTimeStamp) < LOG_BUFFER_ACTIVE_READ_TIMEOUT;
}

void LogStruct::clearExpiredEntries() {
  unsigned int maxLoops = LOG_STRUCT_MESSAGE_LINES;

  while (maxLoops > 0) {
    --maxLoops;

    if (isEmpty() || // Nothing left
        !Message[read_idx].isExpired())
    {
      return;
    }
    clearOldest();
  }
}

void LogStruct::clearOldest() {
  if (!isEmpty()) {
    is_full = false;
    Message[read_idx].clear();
    read_idx = nextIndex(read_idx);
  }
}
