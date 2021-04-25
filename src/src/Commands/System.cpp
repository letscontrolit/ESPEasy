#include "../Commands/System.h"

#include "../../ESPEasy_common.h"


#include "../Commands/Common.h"

#include "../Globals/Settings.h"

#include "../Helpers/DeepSleep.h"
#include "../Helpers/Misc.h"
#include "../Helpers/Scheduler.h"

String Command_System_NoSleep(struct EventStruct *event, const char* Line)
{
	if (event->Par1 > 0)
		Settings.deepSleep_wakeTime = event->Par1; // set deep Sleep awake time
	else Settings.deepSleep_wakeTime = 0;
	return return_command_success();
}

String Command_System_deepSleep(struct EventStruct *event, const char* Line)
{
	if (event->Par1 >= 0) {
		deepSleepStart(event->Par1); // call the second part of the function to avoid check and enable one-shot operation
	}
	return return_command_success();
}

String Command_System_Reboot(struct EventStruct *event, const char* Line)
{
	pinMode(0, INPUT);
	pinMode(2, INPUT);
	pinMode(15, INPUT);
	reboot(ESPEasy_Scheduler::IntendedRebootReason_e::CommandReboot);
	return return_command_success();
}

