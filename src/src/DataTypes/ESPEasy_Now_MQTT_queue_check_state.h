#ifndef DATATYPES_ESPEASY_NOW_MQTT_QUEUE_CHECK_STATE_H
#define DATATYPES_ESPEASY_NOW_MQTT_QUEUE_CHECK_STATE_H


#include <stdint.h>
#include <Arduino.h>

struct ESPEasy_Now_MQTT_QueueCheckState {
  enum Enum : uint8_t {
    Unset = 0,
    Empty = 1,
    Full = 2
  };
};

#endif