#include "../ControllerQueue/C017_queue_element.h"

#include "../DataStructs/ESPEasy_EventStruct.h"

C017_queue_element::C017_queue_element() {}

C017_queue_element::C017_queue_element(const struct EventStruct *event) :
  idx(event->idx),
  TaskIndex(event->TaskIndex),
  controller_idx(event->ControllerIndex),
  sensorType(event->sensorType) {}

size_t C017_queue_element::getSize() const {
  size_t total = sizeof(*this);

  for (int i = 0; i < VARS_PER_TASK; ++i) {
    total += txt[i].length();
  }
  return total;
}
