#include "../ControllerQueue/C016_queue_element.h"

#include "../DataStructs/ESPEasy_EventStruct.h"
#include "../Globals/Plugins.h"
#include "../Globals/RuntimeData.h"

#ifdef USES_C016

C016_queue_element::C016_queue_element() : _timestamp(0), TaskIndex(INVALID_TASK_INDEX), controller_idx(0), sensorType(
    Sensor_VType::SENSOR_TYPE_NONE) {}

C016_queue_element::C016_queue_element(const struct EventStruct *event, byte value_count, unsigned long unixTime) :
  _timestamp(unixTime),
  TaskIndex(event->TaskIndex),
  controller_idx(event->ControllerIndex),
  sensorType(event->sensorType),
  valueCount(value_count)
{
  for (byte i = 0; i < VARS_PER_TASK; ++i) {
    if (i < value_count) {
      values[i] = UserVar[event->BaseVarIndex + i];
    } else {
      values[i] = 0.0f;
    }
  }
}

size_t C016_queue_element::getSize() const {
  return sizeof(*this);
}

bool C016_queue_element::isDuplicate(const C016_queue_element& other) const {
  if (other.controller_idx != controller_idx || 
      other.TaskIndex != TaskIndex ||
      other.sensorType != sensorType ||
      other.valueCount != valueCount) {
    return false;
  }
  for (byte i = 0; i < VARS_PER_TASK; ++i) {
    if (other.values[i] != values[i]) {
      return false;
    }
  }
  return true;
}

#endif
