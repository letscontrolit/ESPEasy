#ifndef COMMAND_SYSTEM_H
#define COMMAND_SYSTEM_H

#include <Arduino.h>

const __FlashStringHelper * Command_System_NoSleep(struct EventStruct *event, const char* Line);
const __FlashStringHelper * Command_System_deepSleep(struct EventStruct *event, const char* Line);
const __FlashStringHelper * Command_System_Reboot(struct EventStruct *event, const char* Line);

#ifdef ESP8266
// Erase the RF calibration partition (4k)
const __FlashStringHelper * Command_System_Erase_RFcal(struct EventStruct *event, const char* Line);
// Erase the SDK WiFI partition (12k)
const __FlashStringHelper * Command_System_Erase_SDK_WiFiconfig(struct EventStruct *event, const char* Line);
#endif

#endif // COMMAND_SYSTEM_H
