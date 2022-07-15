#ifndef COMMAND_UDP_H
#define COMMAND_UDP_H

#include <Arduino.h>

#ifdef FEATURE_ESP_P2P
const __FlashStringHelper * Command_UDP_Test (struct EventStruct *event, const char* Line);
String Command_UDP_Port(struct EventStruct *event, const char* Line);
const __FlashStringHelper * Command_UPD_SendTo(struct EventStruct *event, const char* Line);
const __FlashStringHelper * Command_UDP_SendToUPD(struct EventStruct *event, const char* Line);

#endif
#endif // COMMAND_UDP_H
