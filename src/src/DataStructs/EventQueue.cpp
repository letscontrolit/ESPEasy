#include "../DataStructs/EventQueue.h"

#include "../../ESPEasy_common.h"

#include "../Globals/Settings.h"
#include "../Helpers/Misc.h"


void EventQueueStruct::add(const String& event, bool deduplicate)
{
  #ifdef USE_SECOND_HEAP
  HeapSelectIram ephemeral;
  #endif // ifdef USE_SECOND_HEAP

  if (!deduplicate || !isDuplicate(event)) {
    _eventQueue.push_back(event);
  }
}

void EventQueueStruct::add(const __FlashStringHelper *event, bool deduplicate)
{
  #ifdef USE_SECOND_HEAP
  HeapSelectIram ephemeral;
  #endif // ifdef USE_SECOND_HEAP

  // Wrap in String() constructor to make sure it is using the 2nd heap allocator if present.
  if (!deduplicate || !isDuplicate(event)) {
    _eventQueue.push_back(String(event));
  }
}

void EventQueueStruct::addMove(String&& event, bool deduplicate)
{
  if (!event.length()) { return; }
  #ifdef USE_SECOND_HEAP
  HeapSelectIram ephemeral;

  if (!mmu_is_iram(&(event[0]))) {
    // Wrap in String constructor to make sure it is stored in the 2nd heap.
    if (!deduplicate || !isDuplicate(event)) {
      _eventQueue.push_back(String(event));
    }
    return;
  }
  #endif // ifdef USE_SECOND_HEAP

  if (!deduplicate || !isDuplicate(event)) {
    _eventQueue.emplace_back(std::move(event));
  }
}

void EventQueueStruct::add(taskIndex_t TaskIndex, const String& varName, const String& eventValue)
{
  if (Settings.UseRules) {
    String eventCommand = getTaskDeviceName(TaskIndex);
    eventCommand.reserve(eventCommand.length() + 2 + varName.length() + eventValue.length());
    eventCommand += '#';
    eventCommand += varName;

    if (!eventValue.isEmpty()) {
      eventCommand += '='; // Add arguments
      eventCommand += eventValue;
    }
    addMove(std::move(eventCommand));
  }
}

void EventQueueStruct::add(taskIndex_t TaskIndex, const String& varName, int eventValue)
{
  if (Settings.UseRules) {
    add(TaskIndex, varName, String(eventValue));
  }
}

void EventQueueStruct::add(taskIndex_t TaskIndex, const __FlashStringHelper *varName, const String& eventValue)
{
  if (Settings.UseRules) {
    add(TaskIndex, String(varName), eventValue);
  }
}

void EventQueueStruct::add(taskIndex_t TaskIndex, const __FlashStringHelper *varName, int eventValue)
{
  if (Settings.UseRules) {
    add(TaskIndex, String(varName), String(eventValue));
  }
}

bool EventQueueStruct::getNext(String& event)
{
  if (_eventQueue.empty()) {
    return false;
  }
  #ifdef USE_SECOND_HEAP
  {
    // Fetch the event and make sure it is allocated on the DRAM heap, not the 2nd heap
    // Otherwise checks like strnlen_P may crash on it.
    HeapSelectDram ephemeral;
    event = std::move(String(_eventQueue.front()));
  }
  #else // ifdef USE_SECOND_HEAP
  event = std::move(_eventQueue.front());
  #endif // ifdef USE_SECOND_HEAP
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

bool EventQueueStruct::isDuplicate(const String& event) {
  return std::find(_eventQueue.begin(), _eventQueue.end(), event) != _eventQueue.end();
}
