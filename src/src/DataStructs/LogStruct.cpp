#include "../DataStructs/LogStruct.h"

#include "../Helpers/ESPEasy_time_calc.h"
#include "../Helpers/StringConverter.h"


void LogStruct::add(const uint8_t loglevel, const String& line) {
  write_idx = (write_idx + 1) % LOG_STRUCT_MESSAGE_LINES;

  if (write_idx == read_idx) {
    // Buffer full, move read_idx to overwrite oldest entry.
    read_idx = (read_idx + 1) % LOG_STRUCT_MESSAGE_LINES;
  }
  timeStamp[write_idx] = millis();
  log_level[write_idx] = loglevel;

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
  }
}

// Read the next item and append it to the given string.
// Returns whether new lines are available.
bool LogStruct::get(String& output, const String& lineEnd) {
  lastReadTimeStamp = millis();

  if (!isEmpty()) {
    #ifdef USE_SECOND_HEAP
    // Fetch the log line and make sure it is allocated on the DRAM heap, not the 2nd heap
    // Otherwise checks like strnlen_P may crash on it.
    HeapSelectDram ephemeral;
    #endif
    read_idx = (read_idx + 1) % LOG_STRUCT_MESSAGE_LINES;
    output  += formatLine(read_idx, lineEnd);
  }
  return !isEmpty();
}

bool LogStruct::getNext(bool& logLinesAvailable, unsigned long& timestamp, String& message, uint8_t& loglevel) {
  lastReadTimeStamp = millis();
  logLinesAvailable = false;

  if (isEmpty()) {
    return false;
  }
  read_idx  = (read_idx + 1) % LOG_STRUCT_MESSAGE_LINES;
  timestamp = timeStamp[read_idx];
  message = Message[read_idx];
  loglevel = log_level[read_idx];
  if (!isEmpty()) { 
    logLinesAvailable = true;
  }
  return true;
}

bool LogStruct::isEmpty() {
  return write_idx == read_idx;
}

bool LogStruct::logActiveRead() {
  clearExpiredEntries();
  return timePassedSince(lastReadTimeStamp) < LOG_BUFFER_EXPIRE;
}

String LogStruct::formatLine(int index, const String& lineEnd) {
  String output;

  output += timeStamp[index];
  output += F(" : ");
  output += Message[index];
  output += lineEnd;
  return output;
}

void LogStruct::clearExpiredEntries() {
  if (isEmpty()) {
    return;
  }

  if (timePassedSince(lastReadTimeStamp) > LOG_BUFFER_EXPIRE) {
    // Clear the entire log.
    // If web log is the only log active, it will not be checked again until it is read.
    for (read_idx = 0; read_idx < LOG_STRUCT_MESSAGE_LINES; ++read_idx) {
      Message[read_idx]   = String(); // Free also the reserved memory.
      timeStamp[read_idx] = 0;
      log_level[read_idx] = 0;
    }
    read_idx  = 0;
    write_idx = 0;
  }
}
