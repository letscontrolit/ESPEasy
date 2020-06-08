#include "../DataStructs/ESPEasy_EventStruct.h"

#include "../DataStructs/ESPEasyLimits.h"
#include "../DataStructs/EventValueSource.h"
#include "../Globals/Plugins.h"
#include "../Globals/CPlugins.h"
#include "../Globals/NPlugins.h"

EventStruct::EventStruct() :
  Data(nullptr), idx(0), Par1(0), Par2(0), Par3(0), Par4(0), Par5(0),
  Source(EventValueSource::Enum::VALUE_SOURCE_NOT_SET), 
  TaskIndex(INVALID_TASK_INDEX), ControllerIndex(INVALID_CONTROLLER_INDEX),
  NotificationIndex(INVALID_NOTIFIER_INDEX), BaseVarIndex(0),
  sensorType(0), OriginTaskIndex(0) {}

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
  if(&other == this)
      return *this;
  String1 = other.String1;
  String2 = other.String2;
  String3 = other.String3;
  String4 = other.String4;
  String5 = other.String5;
  Data = other.Data;
  idx = other.idx;
  Par1 = other.Par1;
  Par2 = other.Par2;
  Par3 = other.Par3;
  Par4 = other.Par4;
  Par5 = other.Par5;
  Source = other.Source;
  TaskIndex = other.TaskIndex;
  ControllerIndex = other.ControllerIndex;
  NotificationIndex = other.NotificationIndex;
  BaseVarIndex = other.BaseVarIndex;
  sensorType = other.sensorType;
  OriginTaskIndex = other.OriginTaskIndex;
  return *this;
}