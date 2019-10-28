#include "../ControllerQueue/C007_queue_element.h"

#include "../DataStructs/ESPEasy_EventStruct.h"

C007_queue_element::C007_queue_element() : controller_idx(0), TaskIndex(0), idx(0), sensorType(0) {}

C007_queue_element::C007_queue_element(const struct EventStruct *event) :
  controller_idx(event->ControllerIndex),
  TaskIndex(event->TaskIndex),
  idx(event->idx),
  sensorType(event->sensorType) {}

size_t C007_queue_element::getSize() const {
  return sizeof(this);
}
