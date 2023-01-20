#ifndef DATASTRUCTS_LOGSTRUCT_H
#define DATASTRUCTS_LOGSTRUCT_H

#include <Arduino.h>

#include "../../ESPEasy_common.h"

/*********************************************************************************************\
 * LogStruct
\*********************************************************************************************/
#define LOG_STRUCT_MESSAGE_SIZE 128
#ifdef ESP32
  #define LOG_STRUCT_MESSAGE_LINES 60
  #define LOG_BUFFER_EXPIRE         30000  // Time after which a buffered log item is considered expired.
#else
  #ifdef USE_SECOND_HEAP
    #define LOG_STRUCT_MESSAGE_LINES 60
  #else
    #if defined(PLUGIN_BUILD_COLLECTION) || defined(PLUGIN_BUILD_DEV)
      #define LOG_STRUCT_MESSAGE_LINES 10
    #else
      #define LOG_STRUCT_MESSAGE_LINES 15
    #endif
  #endif
  #define LOG_BUFFER_EXPIRE         5000  // Time after which a buffered log item is considered expired.
#endif

struct LogStruct {
    
    void add(const uint8_t loglevel, const String& line);
    void add(const uint8_t loglevel, String&& line);

    // Read the next item and append it to the given string.
    // Returns whether new lines are available.
//    bool get(String& output, const String& lineEnd);

    // Returns whether a line was retrieved.
    bool getNext(bool& logLinesAvailable, unsigned long& timestamp, String& message, uint8_t& loglevel);

    bool isEmpty() const;

    bool isFull() const;

    bool logActiveRead();

  private:

    void add_end(const uint8_t loglevel);

    void clearExpiredEntries();

    void clearOldest();

    static int nextIndex(int idx) {
//      return ((++idx) == LOG_STRUCT_MESSAGE_LINES) ? 0 : idx;
      return (idx + 1) % LOG_STRUCT_MESSAGE_LINES;
    }

    String Message[LOG_STRUCT_MESSAGE_LINES] = {};
    unsigned long timeStamp[LOG_STRUCT_MESSAGE_LINES] = {0};
    int write_idx = 0;
    int read_idx = 0;
    unsigned long lastReadTimeStamp = 0;
    uint8_t log_level[LOG_STRUCT_MESSAGE_LINES] = {0};
    bool is_full = false;

};



#endif // DATASTRUCTS_LOGSTRUCT_H