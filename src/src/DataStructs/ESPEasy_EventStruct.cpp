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

EventStruct::EventStruct(struct EventStruct&& event) :
  String1(std::move(event.String1))
  , String2(std::move(event.String2))
  , String3(std::move(event.String3))
  , String4(std::move(event.String4))
  , String5(std::move(event.String5))
  , Data(event.Data)
  , idx(event.idx)
  , Par1(event.Par1), Par2(event.Par2), Par3(event.Par3), Par4(event.Par4), Par5(event.Par5)
  , Source(event.Source), TaskIndex(event.TaskIndex), ControllerIndex(event.ControllerIndex)
  , NotificationIndex(event.NotificationIndex)
  , BaseVarIndex(event.BaseVarIndex), sensorType(event.sensorType)
  , OriginTaskIndex(event.OriginTaskIndex) {
    event.Data = nullptr;
    event.idx = 0;
    event.Par1 = 0;
    event.Par2 = 0;
    event.Par3 = 0;
    event.Par4 = 0;
    event.Par5 = 0;
    event.Source            = EventValueSource::Enum::VALUE_SOURCE_NOT_SET;
    event.TaskIndex         = INVALID_TASK_INDEX;       
    event.ControllerIndex   = INVALID_CONTROLLER_INDEX; 
    event.NotificationIndex = INVALID_NOTIFIER_INDEX;   // index position in Settings.Notification, 0-3
    event.BaseVarIndex      = 0;
    event.sensorType        = Sensor_VType::SENSOR_TYPE_NOT_SET;
    event.OriginTaskIndex   = 0;
  }

void EventStruct::deep_copy(const struct EventStruct& other) {
  // check for self-assignment
  if (&other == this) {
    return;
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
}

void EventStruct::deep_copy(const struct EventStruct* other) {
  if (other != nullptr) {
    deep_copy(*other);
  }
}


EventStruct& EventStruct::operator=(struct EventStruct&& other) {
  // check for self-assignment
  if (&other == this) {
    return *this;
  }
  String1           = std::move(other.String1);
  String2           = std::move(other.String2);
  String3           = std::move(other.String3);
  String4           = std::move(other.String4);
  String5           = std::move(other.String5);
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
