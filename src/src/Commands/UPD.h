#ifndef COMMAND_UDP_H
#define COMMAND_UDP_H

#include <Arduino.h>

const __FlashStringHelper * Command_UDP_Test (struct EventStruct *event, const char* Line);
String Command_UDP_Port(struct EventStruct *event, const char* Line);
const __FlashStringHelper * Command_UPD_SendTo(struct EventStruct *event, const char* Line);
const __FlashStringHelper * Command_UDP_SendToUPD(struct EventStruct *event, const char* Line);

#endif // COMMAND_UDP_H
