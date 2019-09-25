#include "../ControllerQueue/queue_element_single_value_base.h"

#include "../DataStructs/ESPEasy_EventStruct.h"

queue_element_single_value_base::queue_element_single_value_base() : controller_idx(0), TaskIndex(0), idx(0), valuesSent(0) {}

queue_element_single_value_base::queue_element_single_value_base(const struct EventStruct *event, byte value_count) :
  controller_idx(event->ControllerIndex),
  TaskIndex(event->TaskIndex),
  idx(event->idx),
  valuesSent(0),
  valueCount(value_count) {}

bool queue_element_single_value_base::checkDone(bool succesfull) const {
  if (succesfull) { ++valuesSent; }
  return valuesSent >= valueCount || valuesSent >= VARS_PER_TASK;
}

size_t queue_element_single_value_base::getSize() const {
  size_t total = sizeof(this);

  for (int i = 0; i < VARS_PER_TASK; ++i) {
    total += txt[i].length();
  }
  return total;
}
