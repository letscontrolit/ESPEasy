#ifndef HELPERS_IMPROV_HELPER_H
#define HELPERS_IMPROV_HELPER_H

#include "../../ESPEasy_common.h"

#if FEATURE_IMPROV

#include <ImprovWiFiLibrary.h>


class Improv_Helper_t {
public:

  Improv_Helper_t();

  void init();

  bool handle(uint8_t b, Stream *serialForWrite);

private:
  ImprovWiFi _improv;
};

#endif
#endif