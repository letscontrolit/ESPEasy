#ifndef COMMAND_NOTIFICATIONS_H
#define COMMAND_NOTIFICATIONS_H


bool Command_Notifications_Notify(struct EventStruct *event, const char* Line)
{
  char TmpStr1[INPUT_COMMAND_SIZE];
  String message = "";
  if (GetArgv(Line, TmpStr1, 3))
    message = TmpStr1;

  if (event->Par1 > 0)
  {
    int index = event->Par1 -1;
    if (Settings.NotificationEnabled[index] && Settings.Notification[index] != 0)
    {
      byte NotificationProtocolIndex = getNotificationProtocolIndex(Settings.Notification[index]);
      if (NotificationProtocolIndex!=NPLUGIN_NOT_FOUND)
      {
        struct EventStruct TempEvent;
        // TempEvent.NotificationProtocolIndex = NotificationProtocolIndex;
        TempEvent.NotificationIndex = index;
        NPlugin_ptr[NotificationProtocolIndex](NPLUGIN_NOTIFY, &TempEvent, message);
      }
    }
  }
  return true;
}

#endif // COMMAND_NOTIFICATIONS_H