#ifndef ESPEASY_EVENTSTRUCT_H
#define ESPEASY_EVENTSTRUCT_H

#include <Arduino.h>
#include "EventValueSource.h"
#include "../Globals/CPlugins.h"
#include "../Globals/NPlugins.h"
#include "../Globals/Plugins.h"

/*********************************************************************************************\
* EventStruct
\*********************************************************************************************/
struct EventStruct
{
  EventStruct();
  EventStruct(const struct EventStruct& event);
  EventStruct& operator=(const struct EventStruct& other);

  String                 String1;
  String                 String2;
  String                 String3;
  String                 String4;
  String                 String5;
  byte                  *Data;
  int                    idx;
  int                    Par1;
  int                    Par2;
  int                    Par3;
  int                    Par4;
  int                    Par5;
  EventValueSource::Enum Source;            // The origin of the values in the event. See EventValueSource.h
  taskIndex_t            TaskIndex;         // index position in TaskSettings array, 0-11
  controllerIndex_t      ControllerIndex;   // index position in Settings.Controller, 0-3
  notifierIndex_t        NotificationIndex; // index position in Settings.Notification, 0-3
  byte                   BaseVarIndex;
  byte                   sensorType;
  byte                   OriginTaskIndex;
};

#endif // ESPEASY_EVENTSTRUCT_H
