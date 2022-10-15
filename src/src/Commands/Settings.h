#ifndef COMMAND_SETTINGS_H
#define COMMAND_SETTINGS_H

#include <Arduino.h>

String Command_Settings_Build(struct EventStruct *event, const char* Line);
String Command_Settings_Unit(struct EventStruct *event, const char* Line);
String Command_Settings_Name(struct EventStruct *event, const char* Line);
String Command_Settings_Password(struct EventStruct *event, const char* Line);
const __FlashStringHelper * Command_Settings_Password_Clear(struct EventStruct *event, const char* Line);
const __FlashStringHelper * Command_Settings_Save(struct EventStruct *event, const char* Line);
const __FlashStringHelper * Command_Settings_Load(struct EventStruct *event, const char* Line);
const __FlashStringHelper * Command_Settings_Print(struct EventStruct *event, const char* Line);
const __FlashStringHelper * Command_Settings_Reset(struct EventStruct *event, const char* Line);

#endif // COMMAND_SETTINGS_H
