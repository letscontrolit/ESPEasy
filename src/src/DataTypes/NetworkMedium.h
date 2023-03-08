#ifndef DATATYPES_NETWORKMEDIUM_H
#define DATATYPES_NETWORKMEDIUM_H

#include <Arduino.h>

#include "../../ESPEasy_common.h"

// Is stored in settings
enum class NetworkMedium_t : uint8_t {
  WIFI            = 0,
  Ethernet        = 1,
#ifdef USES_ESPEASY_NOW
  ESPEasyNOW_only = 2,
#endif
  NotSet          = 255
};

bool   isValid(NetworkMedium_t medium);

const __FlashStringHelper * toString(NetworkMedium_t medium);


#endif // DATATYPES_NETWORKMEDIUM_H