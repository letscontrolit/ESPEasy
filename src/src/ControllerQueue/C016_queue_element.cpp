#include "../ControllerQueue/C016_queue_element.h"

#include "../DataStructs/ESPEasy_EventStruct.h"
#include "../Globals/Plugins.h"
#include "../Globals/RuntimeData.h"
#include "../Helpers/ESPEasy_math.h"

#ifdef USES_C016

C016_queue_element::C016_queue_element() : TaskIndex(INVALID_TASK_INDEX), sensorType(
    Sensor_VType::SENSOR_TYPE_NONE) {
  _timestamp     = 0;
  controller_idx = 0;
}

C016_queue_element::C016_queue_element(C016_queue_element&& other)
  : TaskIndex(other.TaskIndex)
  , sensorType(other.sensorType)
  , valueCount(other.valueCount)
{
  _timestamp     = other._timestamp;
  controller_idx = other.controller_idx;

  for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
    values[i] = other.values[i];
  }
}

C016_queue_element::C016_queue_element(const struct EventStruct *event, uint8_t value_count, unsigned long unixTime) :
  TaskIndex(event->TaskIndex),
  sensorType(event->sensorType),
  valueCount(value_count)
{
  _timestamp     = unixTime;
  controller_idx = event->ControllerIndex;

  for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
    if ((i < value_count) && validTaskIndex(event->TaskIndex)) {
      values[i] = UserVar[event->BaseVarIndex + i];
    } else {
      values[i] = 0.0f;
    }
  }
}

C016_queue_element& C016_queue_element::operator=(C016_queue_element&& other) {
  _timestamp     = other._timestamp;
  TaskIndex      = other.TaskIndex;
  controller_idx = other.controller_idx;
  sensorType     = other.sensorType;
  valueCount     = other.valueCount;

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

  if ((oth.controller_idx != controller_idx) ||
      (oth.TaskIndex != TaskIndex) ||
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
  element._timestamp     = _timestamp;
  element.TaskIndex      = TaskIndex;
  element.controller_idx = controller_idx;
  element.sensorType     = sensorType;
  element.valueCount     = valueCount;

  return element;
}

void C016_binary_element::setPluginID_insteadOf_controller_idx() {
  controller_idx = getPluginID_from_TaskIndex(TaskIndex);
}

#endif // ifdef USES_C016
