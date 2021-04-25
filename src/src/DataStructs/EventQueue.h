#ifndef DATASTRUCTS_EVENTQUEUE_H
#define DATASTRUCTS_EVENTQUEUE_H


#include <list>
#include "../../ESPEasy_common.h"

#include "../Globals/Plugins.h"


struct EventQueueStruct {
  EventQueueStruct();

  void add(const String& event);

  void addMove(String&& event);

  bool getNext(String& event);

  void clear();

  bool isEmpty() const;

private:

  std::list<String>_eventQueue;
};


#endif // DATASTRUCTS_EVENTQUEUE_H
