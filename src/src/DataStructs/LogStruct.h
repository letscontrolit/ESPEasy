#ifndef DATASTRUCTS_LOGSTRUCT_H
#define DATASTRUCTS_LOGSTRUCT_H

#include <Arduino.h>

#include "../../ESPEasy_common.h"

/*********************************************************************************************\
 * LogStruct
\*********************************************************************************************/
#define LOG_STRUCT_MESSAGE_SIZE 128
#ifdef ESP32
  #define LOG_STRUCT_MESSAGE_LINES 30
  #define LOG_BUFFER_EXPIRE         30000  // Time after which a buffered log item is considered expired.
#else
  #if defined(PLUGIN_BUILD_TESTING) || defined(PLUGIN_BUILD_DEV)
    #define LOG_STRUCT_MESSAGE_LINES 10
  #else
    #define LOG_STRUCT_MESSAGE_LINES 15
  #endif
  #define LOG_BUFFER_EXPIRE         5000  // Time after which a buffered log item is considered expired.
#endif

struct LogStruct {
    
    void add(const byte loglevel, const char *line);

    // Read the next item and append it to the given string.
    // Returns whether new lines are available.
    bool get(String& output, const String& lineEnd);

    String get_logjson_formatted(bool& logLinesAvailable, unsigned long& timestamp);

    bool isEmpty();

    bool logActiveRead();

  private:
    String formatLine(int index, const String& lineEnd);

    String logjson_formatLine(int index);

    void clearExpiredEntries();

    String Message[LOG_STRUCT_MESSAGE_LINES];
    unsigned long timeStamp[LOG_STRUCT_MESSAGE_LINES] = {0};
    int write_idx = 0;
    int read_idx = 0;
    unsigned long lastReadTimeStamp = 0;
    byte log_level[LOG_STRUCT_MESSAGE_LINES] = {0};

};



#endif // DATASTRUCTS_LOGSTRUCT_H