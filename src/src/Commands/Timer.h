#ifndef COMMAND_TIMER_H
#define COMMAND_TIMER_H

#include "../../ESPEasy_common.h"

const __FlashStringHelper * Command_Timer_Set (struct EventStruct *event, const char* Line);
const __FlashStringHelper * Command_Timer_Set_ms (struct EventStruct *event, const char* Line);
const __FlashStringHelper * Command_Loop_Timer_Set (struct EventStruct *event, const char* Line);
const __FlashStringHelper * Command_Loop_Timer_Set_ms (struct EventStruct *event, const char* Line);
const __FlashStringHelper * Command_Loop_Timer_SetAndRun (struct EventStruct *event, const char* Line);
const __FlashStringHelper * Command_Loop_Timer_SetAndRun_ms (struct EventStruct *event, const char* Line);
const __FlashStringHelper * Command_Timer_Pause (struct EventStruct *event, const char* Line);
const __FlashStringHelper * Command_Timer_Resume (struct EventStruct *event, const char* Line);
const __FlashStringHelper * Command_Delay (struct EventStruct *event, const char* Line);

#endif // COMMAND_TIMER_H
