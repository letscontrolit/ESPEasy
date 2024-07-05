#include "../DataStructs/ESPEasy_EventStruct.h"

#include "../../ESPEasy_common.h"

#include "../CustomBuild/ESPEasyLimits.h"
#include "../DataTypes/EventValueSource.h"
#include "../Globals/Plugins.h"
#include "../Globals/CPlugins.h"
#include "../Globals/NPlugins.h"

#include "../../_Plugin_Helper.h"

EventStruct::EventStruct(taskIndex_t taskIndex) :
  TaskIndex(taskIndex), BaseVarIndex(taskIndex * VARS_PER_TASK)
{
  if (taskIndex >= INVALID_TASK_INDEX) {
    BaseVarIndex = 0;
  }
}

void EventStruct::deep_copy(const struct EventStruct& other) {
  this->operator=(other);
}

void EventStruct::deep_copy(const struct EventStruct *other) {
  if (other != nullptr) {
    deep_copy(*other);
  }
}

void EventStruct::setTaskIndex(taskIndex_t taskIndex) {
  TaskIndex = taskIndex;

  if (TaskIndex < INVALID_TASK_INDEX) {
    BaseVarIndex = taskIndex * VARS_PER_TASK;
  }
  sensorType = Sensor_VType::SENSOR_TYPE_NOT_SET;
}

void EventStruct::clear() {
  *this = EventStruct();
}

Sensor_VType EventStruct::getSensorType() {
  const int tmp_idx = idx;

  checkDeviceVTypeForTask(this);
  idx = tmp_idx;
  return sensorType;
}

int64_t EventStruct::getTimestamp_as_systemMicros() const
{
  if (timestamp_sec == 0) 
  return  getMicros64();

  // FIXME TD-er: What to do when system time has not been set?
  int64_t res = node_time.Unixtime_to_systemMicros(timestamp_sec, timestamp_frac);
  if (res < 0) {
    // Unix time was from before we booted
    // FIXME TD-er: What to do now?
    return  getMicros64();
  }
  return res;
}

void EventStruct::setUnixTimeTimestamp()
{
  timestamp_sec = node_time.getUnixTime(timestamp_frac);
}

void EventStruct::setLocalTimeTimestamp()
{
  timestamp_sec = node_time.getLocalUnixTime(timestamp_frac);
}