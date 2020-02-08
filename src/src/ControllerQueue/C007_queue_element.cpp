#include "../ControllerQueue/C007_queue_element.h"

#include "../DataStructs/ESPEasy_EventStruct.h"

C007_queue_element::C007_queue_element() {}

C007_queue_element::C007_queue_element(const struct EventStruct *event) :
  idx(event->idx),
  TaskIndex(event->TaskIndex),
  controller_idx(event->ControllerIndex),
  sensorType(event->sensorType) {}

size_t C007_queue_element::getSize() const {
  return sizeof(*this);
}
