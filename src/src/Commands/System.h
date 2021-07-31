#ifndef COMMAND_SYSTEM_H
#define COMMAND_SYSTEM_H

#include <Arduino.h>

const __FlashStringHelper * Command_System_NoSleep(struct EventStruct *event, const char* Line);
const __FlashStringHelper * Command_System_deepSleep(struct EventStruct *event, const char* Line);
const __FlashStringHelper * Command_System_Reboot(struct EventStruct *event, const char* Line);

#endif // COMMAND_SYSTEM_H
