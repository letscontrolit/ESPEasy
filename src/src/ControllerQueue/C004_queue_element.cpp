#include "../ControllerQueue/C004_queue_element.h"

#include "../DataStructs/DeviceStruct.h"
#include "../DataStructs/ESPEasy_EventStruct.h"

C004_queue_element::C004_queue_element() {}

C004_queue_element::C004_queue_element(const struct EventStruct *event) :
  idx(event->idx),
  TaskIndex(event->TaskIndex),
  controller_idx(event->ControllerIndex),
  sensorType(event->sensorType) {
  if (sensorType == Sensor_VType::SENSOR_TYPE_STRING) {
    txt = event->String2;
  }
}

size_t C004_queue_element::getSize() const {
  return sizeof(*this) + txt.length();
}
