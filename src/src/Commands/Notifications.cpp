#include "../Commands/Notifications.h"

#if FEATURE_NOTIFIER

#include "../Commands/Common.h"

#include "../../ESPEasy_common.h"
#include "../DataTypes/ESPEasy_plugin_functions.h"
#include "../Globals/ESPEasy_Scheduler.h"
#include "../Globals/Settings.h"
#include "../Globals/NPlugins.h"
#include "../Helpers/StringConverter.h"


const __FlashStringHelper * Command_Notifications_Notify(struct EventStruct *event, const char* Line)
{
	String message;
	String subject;
	GetArgv(Line, message, 3);
	GetArgv(Line, subject, 4);

	if (event->Par1 > 0) {
		int index = event->Par1 - 1;
		if (Settings.NotificationEnabled[index] && Settings.Notification[index] != INVALID_N_PLUGIN_ID.value) {
			nprotocolIndex_t NotificationProtocolIndex = 
			  getNProtocolIndex(npluginID_t::toPluginID(Settings.Notification[index]));
			if (validNProtocolIndex(NotificationProtocolIndex )) {
				struct EventStruct TempEvent(event->TaskIndex);
				// TempEvent.NotificationProtocolIndex = NotificationProtocolIndex;
				TempEvent.NotificationIndex = index;
				TempEvent.String1 = message;
				TempEvent.String2 = subject;
				Scheduler.schedule_notification_event_timer(NotificationProtocolIndex, NPlugin::Function::NPLUGIN_NOTIFY, std::move(TempEvent));
			}
		}
	}
	return return_command_success_flashstr();
}

#endif