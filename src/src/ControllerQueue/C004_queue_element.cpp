#include "../ControllerQueue/C004_queue_element.h"

#include "../DataStructs/DeviceStruct.h"
#include "../DataStructs/ESPEasy_EventStruct.h"

C004_queue_element::C004_queue_element() : controller_idx(0), idx(0), TaskIndex(INVALID_TASK_INDEX), sensorType(0) {}

C004_queue_element::C004_queue_element(const struct EventStruct *event) :
  controller_idx(event->ControllerIndex),
  idx(event->idx),
  TaskIndex(event->TaskIndex),
  sensorType(event->sensorType) {
  if (sensorType == SENSOR_TYPE_STRING) {
    txt = event->String2;
  }
}

size_t C004_queue_element::getSize() const {
  return sizeof(*this) + txt.length();
}
