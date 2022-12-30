#include "../ControllerQueue/C011_queue_element.h"

#include "../DataStructs/ESPEasy_EventStruct.h"

#ifdef USES_C011


C011_queue_element::C011_queue_element(const struct EventStruct *event) :
  idx(event->idx),
  TaskIndex(event->TaskIndex),
  sensorType(event->sensorType)
{
  controller_idx = event->ControllerIndex;
}

size_t C011_queue_element::getSize() const {
  size_t total = sizeof(*this);

  total += uri.length();
  total += HttpMethod.length();
  total += header.length();
  total += postStr.length();
  return total;
}

bool C011_queue_element::isDuplicate(const Queue_element_base& other) const {
  const C011_queue_element& oth = static_cast<const C011_queue_element&>(other);

  if ((oth.controller_idx != controller_idx) ||
      (oth.TaskIndex != TaskIndex) ||
      (oth.sensorType != sensorType) ||
      (oth.idx != idx)) {
    return false;
  }
  return oth.uri.equals(uri) &&
         oth.HttpMethod.equals(HttpMethod) &&
         oth.header.equals(header) &&
         oth.postStr.equals(postStr);
}

#endif // ifdef USES_C011
