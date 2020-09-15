#include "../ControllerQueue/C011_queue_element.h"

#include "../DataStructs/ESPEasy_EventStruct.h"

C011_queue_element::C011_queue_element() {}

C011_queue_element::C011_queue_element(const struct EventStruct *event) :
  idx(event->idx),
  TaskIndex(event->TaskIndex),
  controller_idx(event->ControllerIndex),
  sensorType(event->sensorType) {}

size_t C011_queue_element::getSize() const {
  size_t total = sizeof(*this);
  total += uri.length();
  total += HttpMethod.length();
  total += header.length();
  total += postStr.length();
  return total;
}
