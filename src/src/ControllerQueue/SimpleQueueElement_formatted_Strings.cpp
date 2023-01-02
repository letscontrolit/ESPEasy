#include "../ControllerQueue/SimpleQueueElement_formatted_Strings.h"

#include "../DataStructs/ESPEasy_EventStruct.h"
#include "../Helpers/StringConverter.h"

#include "../../_Plugin_Helper.h"


SimpleQueueElement_formatted_Strings::SimpleQueueElement_formatted_Strings(struct EventStruct *event) :
  idx(event->idx),
  sensorType(event->sensorType),
  valuesSent(0)
{
  controller_idx = event->ControllerIndex;
  TaskIndex = event->TaskIndex;
  #ifdef USE_SECOND_HEAP
  HeapSelectIram ephemeral;
  #endif // ifdef USE_SECOND_HEAP

  valueCount = getValueCountForTask(TaskIndex);

  for (uint8_t i = 0; i < valueCount; ++i) {
    txt[i] = formatUserVarNoCheck(event, i);
  }
}

SimpleQueueElement_formatted_Strings::SimpleQueueElement_formatted_Strings(const struct EventStruct *event, uint8_t value_count) :
  idx(event->idx),
  sensorType(event->sensorType),
  valuesSent(0),
  valueCount(value_count) {
  controller_idx = event->ControllerIndex;
  TaskIndex      = event->TaskIndex;
}

SimpleQueueElement_formatted_Strings::SimpleQueueElement_formatted_Strings(SimpleQueueElement_formatted_Strings&& rval)
  : idx(rval.idx),
  sensorType(rval.sensorType),
  valuesSent(rval.valuesSent), valueCount(rval.valueCount)
{
  _timestamp     = rval._timestamp;
  controller_idx = rval.controller_idx;
  TaskIndex      = rval.TaskIndex;
  #ifdef USE_SECOND_HEAP
  HeapSelectIram ephemeral;
  #endif // ifdef USE_SECOND_HEAP

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
    #else // ifdef USE_SECOND_HEAP
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

bool SimpleQueueElement_formatted_Strings::isDuplicate(const Queue_element_base& rval) const {
  const SimpleQueueElement_formatted_Strings& oth = static_cast<const SimpleQueueElement_formatted_Strings&>(rval);

  if ((oth.controller_idx != controller_idx) ||
      (oth.TaskIndex != TaskIndex) ||
      (oth.sensorType != sensorType) ||
      (oth.valueCount != valueCount) ||
      (oth.idx != idx)) {
    return false;
  }

  for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
    if (oth.txt[i] != txt[i]) {
      return false;
    }
  }
  return true;
}
