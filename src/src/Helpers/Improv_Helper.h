#ifndef HELPERS_IMPROV_HELPER_H
#define HELPERS_IMPROV_HELPER_H

#include "../../ESPEasy_common.h"

#if FEATURE_IMPROV

#include <ImprovWiFiLibrary.h>

#include <deque>


class Improv_Helper_t {
public:

  Improv_Helper_t();

  void init();

  bool handle(uint8_t b, Stream *serialForWrite);

  bool getFromBuffer(uint8_t& b);

  size_t available() const;

private:
  ImprovWiFi _improv;

  std::deque<uint8_t> _tmpbuffer;
  bool _mustDumpBuffer = false;
};

#endif
#endif