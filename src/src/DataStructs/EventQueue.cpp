#include "EventQueue.h"

EventQueueStruct::EventQueueStruct() {}

void EventQueueStruct::add(const String& event)
{
  #ifdef USE_SECOND_HEAP
  HeapSelectIram ephemeral;
  #endif

  _eventQueue.push_back(event);
}

void EventQueueStruct::add(const __FlashStringHelper * event)
{
  #ifdef USE_SECOND_HEAP
  HeapSelectIram ephemeral;
  #endif
  _eventQueue.push_back(event);
}

void EventQueueStruct::addMove(String&& event)
{
  #ifdef USE_SECOND_HEAP
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
  #ifdef USE_SECOND_HEAP
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