#ifndef DATASTRUCTS_EVENTSTRUCTCOMMANDWRAPPER_H
#define DATASTRUCTS_EVENTSTRUCTCOMMANDWRAPPER_H

#include <Arduino.h>
#include "ESPEasy_EventStruct.h"

struct EventStructCommandWrapper {
  EventStructCommandWrapper() : id(0) {}

  EventStructCommandWrapper(unsigned long i, const EventStruct& e) : id(i), event(e) {}

  unsigned long      id;
  String             cmd;
  String             line;
  EventStruct event;
};

#endif // DATASTRUCTS_EVENTSTRUCTCOMMANDWRAPPER_H