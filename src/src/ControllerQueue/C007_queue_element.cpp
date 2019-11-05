#include "../ControllerQueue/C007_queue_element.h"

#include "../DataStructs/ESPEasy_EventStruct.h"

C007_queue_element::C007_queue_element() : controller_idx(0), idx(0), TaskIndex(INVALID_TASK_INDEX), sensorType(0) {}

C007_queue_element::C007_queue_element(const struct EventStruct *event) :
  controller_idx(event->ControllerIndex),
  idx(event->idx),
  TaskIndex(event->TaskIndex),
  sensorType(event->sensorType) {}

size_t C007_queue_element::getSize() const {
  return sizeof(*this);
}
