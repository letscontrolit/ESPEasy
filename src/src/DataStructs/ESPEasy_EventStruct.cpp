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

EventStruct::EventStruct(const struct EventStruct& event) :
  String1(event.String1)
  , String2(event.String2)
  , String3(event.String3)
  , String4(event.String4)
  , String5(event.String5)
  , Data(event.Data)
  , idx(event.idx)
  , Par1(event.Par1), Par2(event.Par2), Par3(event.Par3), Par4(event.Par4), Par5(event.Par5)
  , Source(event.Source), TaskIndex(event.TaskIndex), ControllerIndex(event.ControllerIndex)
  , NotificationIndex(event.NotificationIndex)
  , BaseVarIndex(event.BaseVarIndex), sensorType(event.sensorType)
  , OriginTaskIndex(event.OriginTaskIndex)
{}

EventStruct& EventStruct::operator=(const struct EventStruct& other) {
  // check for self-assignment
  if (&other == this) {
    return *this;
  }
  String1           = other.String1;
  String2           = other.String2;
  String3           = other.String3;
  String4           = other.String4;
  String5           = other.String5;
  Data              = other.Data;
  idx               = other.idx;
  Par1              = other.Par1;
  Par2              = other.Par2;
  Par3              = other.Par3;
  Par4              = other.Par4;
  Par5              = other.Par5;
  Source            = other.Source;
  TaskIndex         = other.TaskIndex;
  ControllerIndex   = other.ControllerIndex;
  NotificationIndex = other.NotificationIndex;
  BaseVarIndex      = other.BaseVarIndex;
  sensorType        = other.sensorType;
  OriginTaskIndex   = other.OriginTaskIndex;
  return *this;
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
