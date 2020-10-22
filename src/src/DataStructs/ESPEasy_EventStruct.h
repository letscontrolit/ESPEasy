#ifndef ESPEASY_EVENTSTRUCT_H
#define ESPEASY_EVENTSTRUCT_H

#include <Arduino.h>

#include "../DataTypes/ControllerIndex.h"
#include "../DataTypes/EventValueSource.h"
#include "../DataTypes/TaskIndex.h"
//#include "../Globals/CPlugins.h"
#include "../Globals/NPlugins.h"
//#include "../Globals/Plugins.h"
#include "DeviceStruct.h"


/*********************************************************************************************\
* EventStruct
\*********************************************************************************************/
struct EventStruct
{
  EventStruct();
  explicit EventStruct(taskIndex_t taskIndex);
  explicit EventStruct(const struct EventStruct& event);
  EventStruct& operator=(const struct EventStruct& other);

  void setTaskIndex(taskIndex_t taskIndex);

  // Check (and update) sensorType if not set, plus return (corrected) sensorType
  Sensor_VType getSensorType();

  String String1;
  String String2;
  String String3;
  String String4;
  String String5;
  byte  *Data = nullptr;
  int    idx  = 0;
  int    Par1 = 0;
  int    Par2 = 0;
  int    Par3 = 0;
  int    Par4 = 0;
  int    Par5 = 0;

  // The origin of the values in the event. See EventValueSource.h
  EventValueSource::Enum Source            = EventValueSource::Enum::VALUE_SOURCE_NOT_SET;
  taskIndex_t            TaskIndex         = INVALID_TASK_INDEX;       // index position in TaskSettings array, 0-11
  controllerIndex_t      ControllerIndex   = INVALID_CONTROLLER_INDEX; // index position in Settings.Controller, 0-3
  notifierIndex_t        NotificationIndex = INVALID_NOTIFIER_INDEX;   // index position in Settings.Notification, 0-3
  byte                   BaseVarIndex      = 0;
  Sensor_VType           sensorType        = Sensor_VType::SENSOR_TYPE_NOT_SET;
  byte                   OriginTaskIndex   = 0;
};

#endif // ESPEASY_EVENTSTRUCT_H
