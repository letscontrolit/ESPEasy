#ifndef COMMAND_RULES_H
#define COMMAND_RULES_H

#include "../../ESPEasy_common.h"

const __FlashStringHelper * Command_Rules_Execute(struct EventStruct *event, const char *Line);
String Command_Rules_UseRules(struct EventStruct *event, const char *Line);
const __FlashStringHelper * Command_Rules_Async_Events(struct EventStruct *event, const char *Line);
const __FlashStringHelper * Command_Rules_Events(struct EventStruct *event, const char *Line);
const __FlashStringHelper * Command_Rules_Let(struct EventStruct *event, const char *Line);
#if FEATURE_STRING_VARIABLES
const __FlashStringHelper * Command_Rules_LetStr(struct EventStruct *event, const char *Line);
#endif // if FEATURE_STRING_VARIABLES
const __FlashStringHelper * Command_Rules_Inc(struct EventStruct *event, const char *Line);
const __FlashStringHelper * Command_Rules_Dec(struct EventStruct *event, const char *Line);

#endif // COMMAND_RULES_H
