#ifndef DATASTRUCT_LOGENTRY_H
#define DATASTRUCT_LOGENTRY_H

#include "../../ESPEasy_common.h"

class LogEntry_t {
public:

  LogEntry_t() = default;

  bool add(const uint8_t loglevel,
           const String& line);
  bool add(const uint8_t loglevel,
           String     && line);

  void clear();

  bool isExpired() const;


  String _message;
  unsigned long _timestamp{};
  uint8_t _loglevel{};
};


#endif // ifndef DATASTRUCT_LOGENTRY_H
