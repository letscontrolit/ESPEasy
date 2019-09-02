#ifndef ESPEASY_EVENTSTRUCT_H
#define ESPEASY_EVENTSTRUCT_H

/*********************************************************************************************\
 * EventStruct
\*********************************************************************************************/
struct EventStruct
{
  EventStruct() :
    Data(NULL), idx(0), Par1(0), Par2(0), Par3(0), Par4(0), Par5(0),
    Source(0), TaskIndex(TASKS_MAX), ControllerIndex(0), ProtocolIndex(0), NotificationIndex(0),
    BaseVarIndex(0), sensorType(0), OriginTaskIndex(0) {}
  EventStruct(const struct EventStruct& event):
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

  String String1;
  String String2;
  String String3;
  String String4;
  String String5;
  byte *Data;
  int idx;
  int Par1;
  int Par2;
  int Par3;
  int Par4;
  int Par5;
  byte Source;
  byte TaskIndex; // index position in TaskSettings array, 0-11
  byte ControllerIndex; // index position in Settings.Controller, 0-3
  byte ProtocolIndex; // index position in protocol array, depending on which controller plugins are loaded.
  byte NotificationIndex; // index position in Settings.Notification, 0-3
  //Edwin: Not needed, and wasnt used. We can determine the protocol index with getNotificationProtocolIndex(NotificationIndex)
  // byte NotificationProtocolIndex; // index position in notification array, depending on which controller plugins are loaded.
  byte BaseVarIndex;
  byte sensorType;
  byte OriginTaskIndex;
};

#endif // ESPEASY_EVENTSTRUCT_H