#pragma once

#include "../../ESPEasy_common.h"

#include "../DataTypes/LogLevels.h"


#ifdef ESP32
  # define LOG_BUFFER_EXPIRE         30000 // Time after which a buffered log item is considered expired.
#else
  # define LOG_BUFFER_EXPIRE         5000  // Time after which a buffered log item is considered expired.
#endif // ifdef ESP32


struct LogEntry_t {

  LogEntry_t() = delete;

  LogEntry_t(const LogEntry_t& rhs) = delete;

  LogEntry_t(const uint8_t              logLevel,
             const __FlashStringHelper *message);

  LogEntry_t(const uint8_t logLevel,
             const char   *message);

  LogEntry_t(const uint8_t logLevel,
             const String& message);

  LogEntry_t(const uint8_t logLevel,
             String     && message);

  ~LogEntry_t();

  LogEntry_t(LogEntry_t&& rhs);
  LogEntry_t& operator=(LogEntry_t&& rhs);

  void        clear();

  LogEntry_t& operator=(const LogEntry_t& rhs) = delete;

  operator bool() const {
    return isValid();
  }

  void   setSubscribers();

  // This may clear pending reads when a log subscriber is no longer actively reading
  void   updateSubscribers();

  void   markReadBySubscriber(uint8_t subscriber);

  bool   validForSubscriber(uint8_t subscriber) const;

  size_t print(Print& out,
               size_t offset = 0,
               size_t length = std::numeric_limits<size_t>::max()) const;

  String   getMessage() const;

  bool     isExpired() const;

  bool     isValid() const;

  uint8_t  getLogLevel() const;

  uint32_t getTimestamp() const { return _timestamp; }

private:

  void    *_message{};
  uint32_t _timestamp;
  union {
    uint32_t _flags;

    struct {
      uint32_t _strLength             : 19;
      uint32_t _isFlashString         : 1;
      uint32_t _logLevel              : 4;
      uint32_t _subscriberPendingRead : 8; // See NR_LOG_TO_DESTINATIONS

    };

  };

};
