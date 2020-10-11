#include "../ControllerQueue/queue_element_single_value_base.h"

#include "../DataStructs/ESPEasy_EventStruct.h"

queue_element_single_value_base::queue_element_single_value_base() {}

queue_element_single_value_base::queue_element_single_value_base(const struct EventStruct *event, byte value_count) :
  idx(event->idx),
  TaskIndex(event->TaskIndex),
  controller_idx(event->ControllerIndex),
  valuesSent(0),
  valueCount(value_count) {}

/*
queue_element_single_value_base::queue_element_single_value_base(queue_element_single_value_base &&rval)
: idx(rval.idx), TaskIndex(rval.TaskIndex), 
  controller_idx(rval.controller_idx), 
  valuesSent(rval.valuesSent), valueCount(rval.valueCount)
{
  for (byte i = 0; i < VARS_PER_TASK; ++i) {
    String tmp(std::move(rval.txt[i]));
    txt[i] = tmp;
  }
}
*/

bool queue_element_single_value_base::checkDone(bool succesfull) const {
  if (succesfull) { ++valuesSent; }
  return valuesSent >= valueCount || valuesSent >= VARS_PER_TASK;
}

size_t queue_element_single_value_base::getSize() const {
  size_t total = sizeof(*this);

  for (int i = 0; i < VARS_PER_TASK; ++i) {
    total += txt[i].length();
  }
  return total;
}
