#include "../DataStructs/EventQueue.h"

#include "../../ESPEasy_common.h"

#include "../Globals/Settings.h"
#include "../Helpers/Misc.h"
#include "../Helpers/StringConverter.h"



void EventQueueStruct::add(const String& event, bool deduplicate)
{
  if (!deduplicate || !isDuplicate(event)) {
    // Do not add to the list while on 2nd heap
    #ifdef USE_SECOND_HEAP
    HeapSelectDram ephemeral;
    #endif // ifdef USE_SECOND_HEAP

    _eventQueue.push_back(event);
  }
}

void EventQueueStruct::add(const __FlashStringHelper *event, bool deduplicate)
{
  String str;
  {
    #ifdef USE_SECOND_HEAP
    HeapSelectIram ephemeral;
    #endif // ifdef USE_SECOND_HEAP
    str = event;
  }

  add(str, deduplicate);
}

void EventQueueStruct::addMove(String&& event, bool deduplicate)
{
  if (!event.length()) { return; }


  #ifdef USE_SECOND_HEAP
  String tmp;
  move_special(tmp, std::move(event));
  if (!deduplicate || !isDuplicate(event)) {
    // Do not add to the list while on 2nd heap
    #ifdef USE_SECOND_HEAP
    HeapSelectDram ephemeral;
    #endif // ifdef USE_SECOND_HEAP

    _eventQueue.emplace_back(std::move(tmp));
  }
  #else
  if (!deduplicate || !isDuplicate(event)) {
    _eventQueue.emplace_back(std::move(event));
  }

  #endif // ifdef USE_SECOND_HEAP
}

void EventQueueStruct::add(taskIndex_t TaskIndex, const String& varName, const String& eventValue)
{
  if (Settings.UseRules) {
    # ifdef USE_SECOND_HEAP
    HeapSelectIram ephemeral;
    # endif // ifdef USE_SECOND_HEAP

    if (eventValue.isEmpty()) {
      addMove(strformat(
        F("%s#%s"), 
        getTaskDeviceName(TaskIndex).c_str(), 
        varName.c_str()));
    } else {
      addMove(strformat(
        F("%s#%s=%s"), 
        getTaskDeviceName(TaskIndex).c_str(), 
        varName.c_str(), 
        eventValue.c_str()));
    }
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
