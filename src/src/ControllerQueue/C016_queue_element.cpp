#include "../ControllerQueue/C016_queue_element.h"

#ifdef USES_C016

#include "../DataStructs/ESPEasy_EventStruct.h"
#include "../Globals/Plugins.h"
#include "../Globals/RuntimeData.h"
#include "../Helpers/ESPEasy_math.h"

C016_queue_element::C016_queue_element() :  sensorType(
    Sensor_VType::SENSOR_TYPE_NONE) {
  _timestamp      = 0;
  _controller_idx = 0;
  _taskIndex      = INVALID_TASK_INDEX;
}

C016_queue_element::C016_queue_element(C016_queue_element&& other)
  : sensorType(other.sensorType)
  , valueCount(other.valueCount)
{
  _timestamp      = other._timestamp;
  _controller_idx = other._controller_idx;
  _taskIndex      = other._taskIndex;

  for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
    values[i] = other.values[i];
  }
}

C016_queue_element::C016_queue_element(const struct EventStruct *event, uint8_t value_count) :
  unixTime(event->timestamp),
  sensorType(event->sensorType),
  valueCount(value_count)
{
  _controller_idx = event->ControllerIndex;
  _taskIndex      = event->TaskIndex;

  for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
    if ((i < value_count) && validTaskIndex(event->TaskIndex)) {
      values[i] = UserVar[event->BaseVarIndex + i];
    } else {
      values[i] = 0.0f;
    }
  }
}

C016_queue_element& C016_queue_element::operator=(C016_queue_element&& other) {
  _timestamp      = other._timestamp;
  _taskIndex      = other._taskIndex;
  _controller_idx = other._controller_idx;
  sensorType      = other.sensorType;
  valueCount      = other.valueCount;
  unixTime        = other.unixTime;

  for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
    values[i] = other.values[i];
  }
  return *this;
}

size_t C016_queue_element::getSize() const {
  return sizeof(*this);
}

bool C016_queue_element::isDuplicate(const Queue_element_base& other) const {
  const C016_queue_element& oth = static_cast<const C016_queue_element&>(other);

  if ((oth._controller_idx != _controller_idx) ||
      (oth._taskIndex != _taskIndex) ||
      (oth.sensorType != sensorType) ||
      (oth.valueCount != valueCount)) {
    return false;
  }

  for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
    if (!essentiallyEqual(oth.values[i], values[i])) {
      return false;
    }
  }
  return true;
}

C016_binary_element C016_queue_element::getBinary() const {
  C016_binary_element element;

  for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
    element.values[i] = values[i];
  }
  element.unixTime   = unixTime;
  element.TaskIndex  = _taskIndex;
  element.sensorType = sensorType;
  element.valueCount = valueCount;

  // It makes no sense to keep the controller index when storing it.
  // re-purpose it to store the pluginID
  element.pluginID = getPluginID_from_TaskIndex(_taskIndex);

  return element;
}

#endif // ifdef USES_C016
