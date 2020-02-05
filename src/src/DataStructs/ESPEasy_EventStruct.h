#ifndef ESPEASY_EVENTSTRUCT_H
#define ESPEASY_EVENTSTRUCT_H

#include "../../ESPEasy_common.h"
#include "EventValueSource.h"
#include "../Globals/CPlugins.h"
#include "../Globals/Plugins.h"

/*********************************************************************************************\
* EventStruct
\*********************************************************************************************/
struct EventStruct
{
  EventStruct();
  EventStruct(const struct EventStruct& event);

  String            String1;
  String            String2;
  String            String3;
  String            String4;
  String            String5;
  byte             *Data;
  int               idx;
  int               Par1;
  int               Par2;
  int               Par3;
  int               Par4;
  int               Par5;
  byte              Source;            // The origin of the values in the event. See EventValueSource.h
  taskIndex_t       TaskIndex;         // index position in TaskSettings array, 0-11
  controllerIndex_t ControllerIndex;   // index position in Settings.Controller, 0-3
  protocolIndex_t   ProtocolIndex;     // index position in protocol array, depending on which controller plugins are loaded.
  byte              NotificationIndex; // index position in Settings.Notification, 0-3
  // Edwin: Not needed, and wasnt used. We can determine the protocol index with getNotificationProtocolIndex(NotificationIndex)
  // byte NotificationProtocolIndex; // index position in notification array, depending on which controller plugins are loaded.
  byte BaseVarIndex;
  byte sensorType;
  byte OriginTaskIndex;
};

#endif // ESPEASY_EVENTSTRUCT_H
