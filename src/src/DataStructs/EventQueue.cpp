#include "EventQueue.h"

EventQueueStruct::EventQueueStruct() {}

void EventQueueStruct::add(const String& event)
{
  #ifdef CORE_POST_3_0_0
  HeapSelectIram ephemeral;
  #endif

  _eventQueue.push_back(event);
}

void EventQueueStruct::add(const __FlashStringHelper * event)
{
  #ifdef CORE_POST_3_0_0
  HeapSelectIram ephemeral;
  #endif
  _eventQueue.push_back(event);
}

void EventQueueStruct::addMove(String&& event)
{
  #ifdef CORE_POST_3_0_0
  HeapSelectIram ephemeral;
  _eventQueue.push_back(event);
  #else
  _eventQueue.emplace_back(std::move(event));
  #endif
}

bool EventQueueStruct::getNext(String& event)
{
  if (_eventQueue.empty()) {
    return false;
  }
  #ifdef CORE_POST_3_0_0
  event = _eventQueue.front();
  #else
  event = std::move(_eventQueue.front());
  #endif
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