#ifndef COMMAND_NOTIFICATIONS_H
#define COMMAND_NOTIFICATIONS_H


String Command_Notifications_Notify(struct EventStruct *event, const char* Line)
{
	String message = "";
	GetArgv(Line, message, 3);

	if (event->Par1 > 0) {
		int index = event->Par1 - 1;
		if (Settings.NotificationEnabled[index] && Settings.Notification[index] != 0) {
			byte NotificationProtocolIndex = getNotificationProtocolIndex(Settings.Notification[index]);
			if (NotificationProtocolIndex != NPLUGIN_NOT_FOUND) {
				struct EventStruct TempEvent;
				// TempEvent.NotificationProtocolIndex = NotificationProtocolIndex;
				TempEvent.NotificationIndex = index;
				TempEvent.TaskIndex = event->TaskIndex;
				TempEvent.String1 = message;
				schedule_notification_event_timer(NotificationProtocolIndex, NPLUGIN_NOTIFY, &TempEvent);
			}
		}
	}
	return return_command_success();
}

#endif // COMMAND_NOTIFICATIONS_H
