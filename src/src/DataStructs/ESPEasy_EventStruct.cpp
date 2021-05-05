#include "../DataStructs/ESPEasy_EventStruct.h"

#include "../CustomBuild/ESPEasyLimits.h"
#include "../DataTypes/EventValueSource.h"
#include "../Globals/Plugins.h"
#include "../Globals/CPlugins.h"
#include "../Globals/NPlugins.h"

#include "../../_Plugin_Helper.h"

EventStruct::EventStruct() {}

EventStruct::EventStruct(taskIndex_t taskIndex) :
  TaskIndex(taskIndex), BaseVarIndex(taskIndex * VARS_PER_TASK) 
{}

void EventStruct::deep_copy(const struct EventStruct& other) {
  this->operator=(other);
}

void EventStruct::deep_copy(const struct EventStruct* other) {
  if (other != nullptr) {
    deep_copy(*other);
  }
}

void EventStruct::setTaskIndex(taskIndex_t taskIndex) {
  TaskIndex    = taskIndex;
  BaseVarIndex = taskIndex * VARS_PER_TASK;
  sensorType   = Sensor_VType::SENSOR_TYPE_NOT_SET;
}

Sensor_VType EventStruct::getSensorType() {
  const int tmp_idx = idx;
  checkDeviceVTypeForTask(this);
  idx = tmp_idx;
  return sensorType;
}
