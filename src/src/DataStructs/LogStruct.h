#ifndef DATASTRUCTS_LOGSTRUCT_H
#define DATASTRUCTS_LOGSTRUCT_H


#include "../../ESPEasy_common.h"

#include "../DataStructs/LogEntry.h"

/*********************************************************************************************\
 * LogStruct
\*********************************************************************************************/
#ifdef ESP32
  #define LOG_STRUCT_MESSAGE_LINES 120
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
#endif

#ifdef ESP32
  #define LOG_BUFFER_ACTIVE_READ_TIMEOUT 30000
#else
  #define LOG_BUFFER_ACTIVE_READ_TIMEOUT 5000
#endif


struct LogStruct {
    
    void add(const uint8_t loglevel, const String& line);
    void add(const uint8_t loglevel, String&& line);

    // Read the next item and append it to the given string.
    // Returns whether new lines are available.
//    bool get(String& output, const String& lineEnd);

    // Returns whether a line was retrieved.
    bool getNext(bool& logLinesAvailable, unsigned long& timestamp, String& message, uint8_t& loglevel);

    bool isEmpty() const {
      return !is_full && (write_idx == read_idx);
    }

    bool isFull() const { return is_full; }

    bool logActiveRead();

  private:

    void add_end();

    void clearExpiredEntries();

    void clearOldest();

    static int nextIndex(int idx) {
//      return ((++idx) == LOG_STRUCT_MESSAGE_LINES) ? 0 : idx;
      return (idx + 1) % LOG_STRUCT_MESSAGE_LINES;
    }

    LogEntry_t Message[LOG_STRUCT_MESSAGE_LINES];
    int write_idx = 0;
    int read_idx = 0;
    unsigned long lastReadTimeStamp = 0;
    bool is_full = false;
};



#endif // DATASTRUCTS_LOGSTRUCT_H