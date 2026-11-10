#ifndef DATASTRUCTS_EVENTQUEUE_H
#define DATASTRUCTS_EVENTQUEUE_H


#include <list>


#include "../Globals/Plugins.h"


struct EventQueueStruct {
  EventQueueStruct() = default;

  void add(const String& event,
           bool          deduplicate = false, 
           bool          toFront = false);

  void add(const __FlashStringHelper *event,
           bool                       deduplicate = false, 
           bool                       toFront = false);

  void addDeDup(String&& event, bool toFront = false);

  void addDeDup(const String& event, bool toFront = false);

  void addDeDup(const __FlashStringHelper *event, bool toFront = false);


  void addMove(String&& event,
               bool     deduplicate = false, 
               bool     toFront = false);

  // Add event formatted as Taskname#varName=eventvalue
  void add(taskIndex_t   TaskIndex,
           const String& varName,
           const String& eventValue);
  void add(taskIndex_t   TaskIndex,
           const String& varName,
           int           eventValue);
  void add(taskIndex_t                TaskIndex,
           const __FlashStringHelper *varName,
           const String             & eventValue);
  void add(taskIndex_t                TaskIndex,
           const __FlashStringHelper *varName,
           int                        eventValue);

  bool        getNext(String& event);

  void        clear();

  bool        isEmpty() const;

  std::size_t size() { return _eventQueue.size(); }

private:

  bool isDuplicate(const String& event);

  std::list<String>_eventQueue;

};


#endif // DATASTRUCTS_EVENTQUEUE_H
