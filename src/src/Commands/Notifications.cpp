#include "../Commands/Notifications.h"

#include "../Commands/Common.h"
#include "../../ESPEasy_fdwdecl.h"
#include "../../ESPEasy_common.h"
#include "../DataStructs/ESPEasy_plugin_functions.h"
#include "../Globals/ESPEasy_Scheduler.h"
#include "../Globals/Settings.h"
#include "../Globals/NPlugins.h"
#include "../Helpers/StringConverter.h"


String Command_Notifications_Notify(struct EventStruct *event, const char* Line)
{
	String message = "";
	GetArgv(Line, message, 3);

	if (event->Par1 > 0) {
		int index = event->Par1 - 1;
		if (Settings.NotificationEnabled[index] && Settings.Notification[index] != 0) {
			nprotocolIndex_t NotificationProtocolIndex = getNProtocolIndex(Settings.Notification[index]);
			if (validNProtocolIndex(NotificationProtocolIndex )) {
				struct EventStruct TempEvent(event->TaskIndex);
				// TempEvent.NotificationProtocolIndex = NotificationProtocolIndex;
				TempEvent.NotificationIndex = index;
				TempEvent.String1 = message;
				Scheduler.schedule_notification_event_timer(NotificationProtocolIndex, NPlugin::Function::NPLUGIN_NOTIFY, &TempEvent);
			}
		}
	}
	return return_command_success();
}