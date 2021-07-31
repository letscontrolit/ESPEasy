#ifndef COMMAND_RULES_H
#define COMMAND_RULES_H

#include <Arduino.h>

const __FlashStringHelper * Command_Rules_Execute(struct EventStruct *event, const char *Line);
String Command_Rules_UseRules(struct EventStruct *event, const char *Line);
const __FlashStringHelper * Command_Rules_Async_Events(struct EventStruct *event, const char *Line);
const __FlashStringHelper * Command_Rules_Events(struct EventStruct *event, const char *Line);
const __FlashStringHelper * Command_Rules_Let(struct EventStruct *event, const char *Line);

#endif // COMMAND_RULES_H
