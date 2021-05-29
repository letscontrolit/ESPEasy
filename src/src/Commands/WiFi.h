#ifndef COMMAND_WIFI_H
#define COMMAND_WIFI_H

#include "../Globals/SecuritySettings.h"

class String;

String Command_Wifi_SSID(struct EventStruct *event, const char* Line);
String Command_Wifi_Key(struct EventStruct *event, const char* Line);
String Command_Wifi_SSID2(struct EventStruct *event, const char* Line);
String Command_Wifi_Key2(struct EventStruct *event, const char* Line);
const __FlashStringHelper * Command_Wifi_Scan(struct EventStruct *event, const char* Line);
const __FlashStringHelper * Command_Wifi_Connect(struct EventStruct *event, const char* Line);
const __FlashStringHelper * Command_Wifi_Disconnect(struct EventStruct *event, const char* Line);
const __FlashStringHelper * Command_Wifi_APMode(struct EventStruct *event, const char* Line);
const __FlashStringHelper * Command_Wifi_STAMode(struct EventStruct *event, const char* Line);
String Command_Wifi_Mode(struct EventStruct *event, const char* Line);
const __FlashStringHelper * Command_Wifi_AllowAP(struct EventStruct *event, const char* Line);

// FIXME: TD-er This is not an erase, but actually storing the current settings
// in the wifi settings of the core library
const __FlashStringHelper * Command_WiFi_Erase(struct EventStruct *event, const char* Line);

#endif // COMMAND_WIFI_H
