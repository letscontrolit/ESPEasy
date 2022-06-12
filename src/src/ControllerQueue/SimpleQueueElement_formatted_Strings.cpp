#include "../ControllerQueue/SimpleQueueElement_formatted_Strings.h"

#include "../DataStructs/ESPEasy_EventStruct.h"
#include "../Helpers/StringConverter.h"

#include "../../_Plugin_Helper.h"


SimpleQueueElement_formatted_Strings::SimpleQueueElement_formatted_Strings(struct EventStruct *event) :
  idx(event->idx),
  TaskIndex(event->TaskIndex),
  controller_idx(event->ControllerIndex),
  sensorType(event->sensorType),
  valuesSent(0) 
{
  #ifdef USE_SECOND_HEAP
  HeapSelectIram ephemeral;
  #endif

  valueCount = getValueCountForTask(TaskIndex);

  for (uint8_t i = 0; i < valueCount; ++i) {
    txt[i] = formatUserVarNoCheck(event, i);
  }
}

SimpleQueueElement_formatted_Strings::SimpleQueueElement_formatted_Strings(const struct EventStruct *event, uint8_t value_count) :
  idx(event->idx),
  TaskIndex(event->TaskIndex),
  controller_idx(event->ControllerIndex),
  sensorType(event->sensorType),
  valuesSent(0),
  valueCount(value_count) {}

SimpleQueueElement_formatted_Strings::SimpleQueueElement_formatted_Strings(SimpleQueueElement_formatted_Strings&& rval)
  : idx(rval.idx), _timestamp(rval._timestamp),  TaskIndex(rval.TaskIndex),
  controller_idx(rval.controller_idx), sensorType(rval.sensorType),
  valuesSent(rval.valuesSent), valueCount(rval.valueCount)
{
  #ifdef USE_SECOND_HEAP
  HeapSelectIram ephemeral;
  #endif

  for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
    txt[i] = std::move(rval.txt[i]);
  }
}

SimpleQueueElement_formatted_Strings& SimpleQueueElement_formatted_Strings::operator=(SimpleQueueElement_formatted_Strings&& rval) {
  idx            = rval.idx;
  _timestamp     = rval._timestamp;
  TaskIndex      = rval.TaskIndex;
  controller_idx = rval.controller_idx;
  sensorType     = rval.sensorType;
  valuesSent     = rval.valuesSent;
  valueCount     = rval.valueCount;

  for (size_t i = 0; i < VARS_PER_TASK; ++i) {
    #ifdef USE_SECOND_HEAP
    HeapSelectIram ephemeral;
    if (rval.txt[i].length() && !mmu_is_iram(&(rval.txt[i][0]))) {
      txt[i] = rval.txt[i];
    } else {
      txt[i] = std::move(rval.txt[i]);
    }
    #else
    txt[i] = std::move(rval.txt[i]);
    #endif // ifdef USE_SECOND_HEAP
  }
  return *this;
}

bool SimpleQueueElement_formatted_Strings::checkDone(bool succesfull) const {
  if (succesfull) { ++valuesSent; }
  return valuesSent >= valueCount || valuesSent >= VARS_PER_TASK;
}

size_t SimpleQueueElement_formatted_Strings::getSize() const {
  size_t total = sizeof(*this);

  for (int i = 0; i < VARS_PER_TASK; ++i) {
    total += txt[i].length();
  }
  return total;
}

bool SimpleQueueElement_formatted_Strings::isDuplicate(const SimpleQueueElement_formatted_Strings& rval) const {
  if ((rval.controller_idx != controller_idx) ||
      (rval.TaskIndex != TaskIndex) ||
      (rval.sensorType != sensorType) ||
      (rval.valueCount != valueCount) ||
      (rval.idx != idx)) {
    return false;
  }

  for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
    if (rval.txt[i] != txt[i]) {
      return false;
    }
  }
  return true;
}
