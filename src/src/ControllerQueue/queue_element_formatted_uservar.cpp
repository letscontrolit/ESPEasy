#include "../ControllerQueue/queue_element_formatted_uservar.h"

#include "../DataStructs/ESPEasy_EventStruct.h"
#include "../Helpers/StringConverter.h"
#include "../../_Plugin_Helper.h"


queue_element_formatted_uservar::queue_element_formatted_uservar() {}

queue_element_formatted_uservar::queue_element_formatted_uservar(EventStruct *event) :
  idx(event->idx),
  TaskIndex(event->TaskIndex),
  controller_idx(event->ControllerIndex),
  sensorType(event->sensorType)
{
  valueCount = getValueCountForTask(TaskIndex);
  for (byte i = 0; i < valueCount; ++i) {
    txt[i] = formatUserVarNoCheck(event, i);
  }
}

size_t queue_element_formatted_uservar::getSize() const {
  size_t total = sizeof(*this);

  for (int i = 0; i < VARS_PER_TASK; ++i) {
    total += txt[i].length();
  }
  return total;
}

bool queue_element_formatted_uservar::isDuplicate(const queue_element_formatted_uservar& other) const {
  if (other.controller_idx != controller_idx || 
      other.TaskIndex != TaskIndex ||
      other.sensorType != sensorType ||
      other.valueCount != valueCount) {
    return false;
  }
  for (byte i = 0; i < VARS_PER_TASK; ++i) {
    if (other.txt[i] != txt[i]) {
      return false;
    }
  }
  return true;
}
