#include "EventQueue.h"

EventQueueStruct::EventQueueStruct() {}

void EventQueueStruct::add(const String& event)
{
  _eventQueue.push_back(event);
}

bool EventQueueStruct::getNext(String& event)
{
  if (_eventQueue.empty()) {
    return false;
  }
  event = _eventQueue.front();
  _eventQueue.pop_front();
  return true;
}

void EventQueueStruct::clear()
{
  _eventQueue.clear();
}

bool EventQueueStruct::isEmpty() const
{
  return _eventQueue.empty();
}