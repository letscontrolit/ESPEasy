#ifndef DATASTRUCTS_EVENTSTRUCTCOMMANDWRAPPER_H
#define DATASTRUCTS_EVENTSTRUCTCOMMANDWRAPPER_H

#include "../../ESPEasy_common.h"

#include "../DataStructs/ESPEasy_EventStruct.h"

struct EventStructCommandWrapper {
  EventStructCommandWrapper() : id(0) {}

  EventStructCommandWrapper(uint32_t i, EventStruct&& e) : id(i), event(std::move(e)) {}

  uint32_t      id;
  String             cmd;
  String             line;
  EventStruct event;
};

#endif // DATASTRUCTS_EVENTSTRUCTCOMMANDWRAPPER_H