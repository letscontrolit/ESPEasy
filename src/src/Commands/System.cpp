#include "../Commands/System.h"

#include "../../ESPEasy_common.h"


#include "../Commands/Common.h"

#include "../Globals/Settings.h"

#include "../Helpers/DeepSleep.h"
#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/Misc.h"
#include "../Helpers/Scheduler.h"

const __FlashStringHelper * Command_System_NoSleep(struct EventStruct *event, const char* Line)
{
	if (event->Par1 > 0)
		Settings.deepSleep_wakeTime = event->Par1; // set deep Sleep awake time
	else Settings.deepSleep_wakeTime = 0;
	return return_command_success_flashstr();
}

const __FlashStringHelper * Command_System_deepSleep(struct EventStruct *event, const char* Line)
{
	if (event->Par1 >= 0) {
		deepSleepStart(event->Par1); // call the second part of the function to avoid check and enable one-shot operation
	}
	return return_command_success_flashstr();
}

const __FlashStringHelper * Command_System_Reboot(struct EventStruct *event, const char* Line)
{
	pinMode(0, INPUT);
	pinMode(2, INPUT);
	pinMode(15, INPUT);
	reboot(IntendedRebootReason_e::CommandReboot);
	return return_command_success_flashstr();
}

#ifdef ESP8266
// Erase the RF calibration partition (4k)
const __FlashStringHelper * Command_System_Erase_RFcal(struct EventStruct *event, const char* Line)
{
	if (clearRFcalPartition()) {
		return F("Cleared RFcal partition. Please reboot!");
	}
	return return_command_failed_flashstr();
}

// Erase the SDK WiFI partition (12k)
const __FlashStringHelper * Command_System_Erase_SDK_WiFiconfig(struct EventStruct *event, const char* Line)
{
	if (clearWiFiSDKpartition()) {
		return F("Cleared SDK WiFi partition. Please reboot!");
	}
	return return_command_failed_flashstr();
}

#endif
