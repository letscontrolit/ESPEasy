#ifndef DATASTRUCTS_EVENTQUEUE_H
#define DATASTRUCTS_EVENTQUEUE_H


#include <list>


#include "../Globals/Plugins.h"


struct EventQueueStruct {
  EventQueueStruct();

  void add(const String& event);

  void add(const __FlashStringHelper * event);

  void addMove(String&& event);

  bool getNext(String& event);

  void clear();

  bool isEmpty() const;

  std::size_t size() {
    return _eventQueue.size();
  }

  std::list<String>::iterator begin() {
    return _eventQueue.begin();
  }

  std::list<String>::iterator end() {
    return _eventQueue.end();
  }

private:

  std::list<String>_eventQueue;
};


#endif // DATASTRUCTS_EVENTQUEUE_H
