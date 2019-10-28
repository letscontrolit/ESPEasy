#include "../ControllerQueue/C015_queue_element.h"

#include "../DataStructs/ESPEasy_EventStruct.h"

C015_queue_element::C015_queue_element() : controller_idx(0), TaskIndex(0), idx(0), valuesSent(0) {}

C015_queue_element::C015_queue_element(const struct EventStruct *event, byte value_count) :
  controller_idx(event->ControllerIndex),
  TaskIndex(event->TaskIndex),
  idx(event->idx),
  valuesSent(0),
  valueCount(value_count) {}

bool C015_queue_element::checkDone(bool succesfull) const {
  if (succesfull) { ++valuesSent; }
  return valuesSent >= valueCount || valuesSent >= VARS_PER_TASK;
}

size_t C015_queue_element::getSize() const {
  size_t total = sizeof(this);

  for (int i = 0; i < VARS_PER_TASK; ++i) {
    total += txt[i].length();
  }
  return total;
}
