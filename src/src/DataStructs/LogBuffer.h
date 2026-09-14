#ifndef DATASTRUCTS_LOGSTRUCT_H
#define DATASTRUCTS_LOGSTRUCT_H


#include "../../ESPEasy_common.h"

#include "../DataStructs/LogEntry.h"

#include "../DataTypes/LogLevels.h"

#include <list>

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
  # define LOG_BUFFER_ACTIVE_READ_TIMEOUT LOG_BUFFER_EXPIRE
#endif // ifdef ESP32

typedef std::list<LogEntry_t> LogEntry_queue;


struct LogBuffer {

  LogBuffer();

  void add(LogEntry_t&& logEntry);

  bool isEmpty() const {
    return LogEntries.empty();
  }

  // Returns whether a line was retrieved.
  bool getNext(LogDestination logDestination,
               uint32_t     & timestamp,
               String       & message,
               uint8_t      & loglevel);

  // Return true if messages available for given log destination.
  bool hasMessages(LogDestination logDestination);

  bool logActiveRead(LogDestination logDestination) const;

  void clearExpiredEntries();

private:

  LogEntry_queue           LogEntries{};
  uint32_t                 lastReadTimeStamp[NR_LOG_TO_DESTINATIONS]{};
  LogEntry_queue::iterator cache_iterator_pos[NR_LOG_TO_DESTINATIONS]{};

};


#endif // DATASTRUCTS_LOGSTRUCT_H
