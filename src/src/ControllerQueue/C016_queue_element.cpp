#include "../ControllerQueue/C016_queue_element.h"

#include "../DataStructs/ESPEasy_EventStruct.h"
#include "../Globals/Plugins.h"
#include "../Globals/RuntimeData.h"
#include "../Helpers/ESPEasy_math.h"

#ifdef USES_C016

C016_queue_element::C016_queue_element() : _timestamp(0), TaskIndex(INVALID_TASK_INDEX), controller_idx(0), sensorType(
    Sensor_VType::SENSOR_TYPE_NONE) {}

C016_queue_element::C016_queue_element(C016_queue_element&& other)
  : _timestamp(other._timestamp)
  , TaskIndex(other.TaskIndex)
  , controller_idx(other.controller_idx)
  , sensorType(other.sensorType)
  , valueCount(other.valueCount)
{
  for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
    values[i] = other.values[i];
  }
}

C016_queue_element::C016_queue_element(const struct EventStruct *event, uint8_t value_count, unsigned long unixTime) :
  _timestamp(unixTime),
  TaskIndex(event->TaskIndex),
  controller_idx(event->ControllerIndex),
  sensorType(event->sensorType),
  valueCount(value_count)
{
  for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
    if (i < value_count && validTaskIndex(event->TaskIndex)) {
      values[i] = UserVar[event->BaseVarIndex + i];
    } else {
      values[i] = 0.0f;
    }
  }
}

C016_queue_element& C016_queue_element::operator=(C016_queue_element&& other) {
  _timestamp = other._timestamp;
  TaskIndex = other.TaskIndex;
  controller_idx = other.controller_idx;
  sensorType = other.sensorType;
  valueCount = other.valueCount;
  for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
    values[i] = other.values[i];
  }
  return *this;
}

size_t C016_queue_element::getSize() const {
  return sizeof(*this);
}

bool C016_queue_element::isDuplicate(const C016_queue_element& other) const {
  if ((other.controller_idx != controller_idx) ||
      (other.TaskIndex != TaskIndex) ||
      (other.sensorType != sensorType) ||
      (other.valueCount != valueCount)) {
    return false;
  }

  for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
    if (!essentiallyEqual(other.values[i] , values[i])) {
      return false;
    }
  }
  return true;
}

#endif // ifdef USES_C016
