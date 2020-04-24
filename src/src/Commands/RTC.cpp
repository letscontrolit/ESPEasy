#include "../Commands/RTC.h"

#include "../Commands/Common.h"
#include "../../ESPEasy_fdwdecl.h"
#include "../../ESPEasy_common.h"
#include "../Globals/RTC.h"
#include "../DataStructs/RTCStruct.h"


String Command_RTC_Clear(struct EventStruct *event, const char* Line)
{
	initRTC();
	return return_command_success();
}

String Command_RTC_resetFlashWriteCounter(struct EventStruct *event, const char* Line)
{
	RTC.flashDayCounter = 0;
	return return_command_success();
}