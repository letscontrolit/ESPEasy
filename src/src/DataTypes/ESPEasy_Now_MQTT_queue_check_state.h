#ifndef DATATYPES_ESPEASY_NOW_MQTT_QUEUE_CHECK_STATE_H
#define DATATYPES_ESPEASY_NOW_MQTT_QUEUE_CHECK_STATE_H


#include <stdint.h>
#include <Arduino.h>

#include "../../ESPEasy_common.h"
#include "../Globals/ESPEasy_now_state.h"

#ifdef USES_ESPEASY_NOW
struct ESPEasy_Now_MQTT_QueueCheckState {
  enum Enum : uint8_t {
    Unset = 0,
    Empty = 1,
    Full  = 2
  };
};

#endif // ifdef USES_ESPEASY_NOW

#endif // ifndef DATATYPES_ESPEASY_NOW_MQTT_QUEUE_CHECK_STATE_H
