#include "../DataStructs/ESPEasy_EventStruct.h"

#include "../DataStructs/ESPEasyLimits.h"

EventStruct::EventStruct() :
  Data(nullptr), idx(0), Par1(0), Par2(0), Par3(0), Par4(0), Par5(0),
  Source(0), TaskIndex(TASKS_MAX), ControllerIndex(0), ProtocolIndex(0), NotificationIndex(0),
  BaseVarIndex(0), sensorType(0), OriginTaskIndex(0) {}

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
  , ProtocolIndex(event.ProtocolIndex), NotificationIndex(event.NotificationIndex)
  , BaseVarIndex(event.BaseVarIndex), sensorType(event.sensorType)
  , OriginTaskIndex(event.OriginTaskIndex)
{}
