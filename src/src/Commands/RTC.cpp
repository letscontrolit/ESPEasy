#include "../Commands/RTC.h"

#include "../../ESPEasy_common.h"


#include "../Commands/Common.h"

#include "../DataStructs/RTCStruct.h"

#include "../Globals/RTC.h"

#include "../Helpers/ESPEasyRTC.h"


const __FlashStringHelper * Command_RTC_Clear(struct EventStruct *event, const char* Line)
{
	initRTC();
	return return_command_success();
}

const __FlashStringHelper * Command_RTC_resetFlashWriteCounter(struct EventStruct *event, const char* Line)
{
	RTC.flashDayCounter = 0;
	return return_command_success();
}
