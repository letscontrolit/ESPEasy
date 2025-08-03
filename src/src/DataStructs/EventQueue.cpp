#include "../DataStructs/EventQueue.h"

#include "../../ESPEasy_common.h"

#include "../Globals/Settings.h"
#include "../Helpers/Misc.h"
#include "../Helpers/StringConverter.h"



void EventQueueStruct::add(const String& event, bool deduplicate)
{
  if (!deduplicate || !isDuplicate(event)) {
#if defined(USE_SECOND_HEAP) || defined(ESP32)
    String tmp;
    reserve_special(tmp, event.length());
    tmp = event;

#ifdef USE_SECOND_HEAP
    // Do not add to the list while on 2nd heap
    HeapSelectDram ephemeral;
#endif


    _eventQueue.emplace_back(std::move(tmp));
#else
    _eventQueue.push_back(event);
#endif
  }
}

void EventQueueStruct::add(const __FlashStringHelper *event, bool deduplicate)
{
  String str;
  move_special(str, String(event));
  add(str, deduplicate);
}

void EventQueueStruct::addMove(String&& event, bool deduplicate)
{
  if (!event.length()) { return; }

  if (!deduplicate || !isDuplicate(event)) {
    #if defined(USE_SECOND_HEAP) || defined(ESP32)
    String tmp;
    move_special(tmp, std::move(event));

    #ifdef USE_SECOND_HEAP
    // Do not add to the list while on 2nd heap
    HeapSelectDram ephemeral;
    #endif
    _eventQueue.emplace_back(std::move(tmp));
    #else
    _eventQueue.emplace_back(std::move(event));
    #endif // ifdef USE_SECOND_HEAP
  }
}

void EventQueueStruct::add(taskIndex_t TaskIndex, const String& varName, const String& eventValue)
{
  if (Settings.UseRules) {
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
  event = std::move(_eventQueue.front());
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
