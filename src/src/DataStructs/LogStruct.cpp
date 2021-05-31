#include "../DataStructs/LogStruct.h"

#include "../Helpers/ESPEasy_time_calc.h"
#include "../Helpers/StringConverter.h"



void LogStruct::add(const byte loglevel, const char *line) {
  write_idx = (write_idx + 1) % LOG_STRUCT_MESSAGE_LINES;

  if (write_idx == read_idx) {
    // Buffer full, move read_idx to overwrite oldest entry.
    read_idx = (read_idx + 1) % LOG_STRUCT_MESSAGE_LINES;
  }
  timeStamp[write_idx] = millis();
  log_level[write_idx] = loglevel;

  // Must use PROGMEM aware functions here to process line
  unsigned int linelength = strlen_P(line);

  if (linelength > LOG_STRUCT_MESSAGE_SIZE - 1) {
    linelength = LOG_STRUCT_MESSAGE_SIZE - 1;
  }
  Message[write_idx] = "";
  if (!Message[write_idx].reserve(linelength)) {
    return;
  }

  const char* c = line;
  for (unsigned i = 0; i < linelength; ++i) {
    Message[write_idx] += static_cast<char>(pgm_read_byte(c++));
  }
}

// Read the next item and append it to the given string.
// Returns whether new lines are available.
bool LogStruct::get(String& output, const String& lineEnd) {
  lastReadTimeStamp = millis();

  if (!isEmpty()) {
    read_idx = (read_idx + 1) % LOG_STRUCT_MESSAGE_LINES;
    output  += formatLine(read_idx, lineEnd);
  }
  return !isEmpty();
}

bool LogStruct::getNext(bool& logLinesAvailable, unsigned long& timestamp, String& message, byte& loglevel) {
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
  output += " : ";
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
