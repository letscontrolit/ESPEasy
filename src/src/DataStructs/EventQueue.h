#ifndef DATASTRUCTS_EVENTQUEUE_H
#define DATASTRUCTS_EVENTQUEUE_H


#include <list>


#include "../Globals/Plugins.h"


struct EventQueueStruct {
  EventQueueStruct() = default;

  void        add(const String& event,
                  bool          deduplicate = false);

  void        add(const __FlashStringHelper *event,
                  bool                       deduplicate = false);

  void        addMove(String&& event,
                      bool     deduplicate = false);

  bool        getNext(String& event);

  void        clear();

  bool        isEmpty() const;

  std::size_t size() {
    return _eventQueue.size();
  }

private:

  bool isDuplicate(const String& event);

  std::list<String>_eventQueue;
};


#endif // DATASTRUCTS_EVENTQUEUE_H
