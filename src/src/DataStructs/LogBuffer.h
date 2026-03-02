#ifndef DATASTRUCTS_LOGSTRUCT_H
#define DATASTRUCTS_LOGSTRUCT_H


#include "../../ESPEasy_common.h"

#include "../DataStructs/LogEntry.h"

#include "../DataTypes/LogLevels.h"

#include <deque>

/*********************************************************************************************\
* LogBuffer
\*********************************************************************************************/
#ifdef ESP32
  # define LOG_STRUCT_MESSAGE_LINES 120
#else
  # ifdef USE_SECOND_HEAP
    #  define LOG_STRUCT_MESSAGE_LINES 60
  # else
    #  if defined(PLUGIN_BUILD_COLLECTION) || defined(PLUGIN_BUILD_DEV)
      #   define LOG_STRUCT_MESSAGE_LINES 10
    #  else
      #   define LOG_STRUCT_MESSAGE_LINES 15
    #  endif // if defined(PLUGIN_BUILD_COLLECTION) || defined(PLUGIN_BUILD_DEV)
  # endif // ifdef USE_SECOND_HEAP
#endif // ifdef ESP32

#ifdef ESP32
  # define LOG_BUFFER_ACTIVE_READ_TIMEOUT 30000
#else
  # define LOG_BUFFER_ACTIVE_READ_TIMEOUT 5000
#endif // ifdef ESP32

typedef std::deque<LogEntry_t> LogEntry_queue;


struct LogBuffer {

  LogBuffer() = default;

  void add(LogEntry_t&& logEntry);

  bool isEmpty() const {
    return LogEntries.empty();
  }

  // Returns whether a line was retrieved.
  bool getNext(LogDestination   logDestination,
               uint32_t& timestamp,
               String  & message,
               uint8_t & loglevel);

  // Return the number of messages left for given log destination.
  uint32_t getNrMessages(LogDestination logDestination) const;

  bool logActiveRead(LogDestination logDestination);

  void clearExpiredEntries();

private:

  LogEntry_queue LogEntries;
  uint32_t       lastReadTimeStamp[NR_LOG_TO_DESTINATIONS]{};

  uint32_t cache_iterator_pos[NR_LOG_TO_DESTINATIONS]{};

};


#endif // DATASTRUCTS_LOGSTRUCT_H
