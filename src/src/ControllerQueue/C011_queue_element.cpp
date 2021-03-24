#include "../ControllerQueue/C011_queue_element.h"

#include "../DataStructs/ESPEasy_EventStruct.h"

#ifdef USES_C011

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

bool C011_queue_element::isDuplicate(const C011_queue_element& other) const {
  if (other.controller_idx != controller_idx || 
      other.TaskIndex != TaskIndex ||
      other.sensorType != sensorType ||
      other.idx != idx) {
    return false;
  }
  return (other.uri.equals(uri) && 
          other.HttpMethod.equals(HttpMethod) && 
          other.header.equals(header) && 
          other.postStr.equals(postStr));
}


#endif
