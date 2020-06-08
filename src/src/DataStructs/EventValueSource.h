#ifndef DATASTRUCTS_EVENT_VALUE_SOURCE_H
#define DATASTRUCTS_EVENT_VALUE_SOURCE_H

class EventValueSource {
public:

  // Keep the values as they can be used by other/older builds to communicate with ESPEasy
  enum class Enum : byte {
    VALUE_SOURCE_NOT_SET      = 0,
    VALUE_SOURCE_SYSTEM       = 1,
    VALUE_SOURCE_SERIAL       = 2,
    VALUE_SOURCE_HTTP         = 3,
    VALUE_SOURCE_MQTT         = 4,
    VALUE_SOURCE_UDP          = 5,
    VALUE_SOURCE_WEB_FRONTEND = 6,
    VALUE_SOURCE_RULES        = 7,

    VALUE_SOURCE_NR_VALUES
  };
};

#endif // DATASTRUCTS_EVENT_VALUE_SOURCE_H
