#ifndef COMMAND_UDP_H
#define COMMAND_UDP_H

#include "../../ESPEasy_common.h"

#include <Arduino.h>
String Command_UDP_Port(struct EventStruct *event, const char* Line);

#if FEATURE_ESPEASY_P2P
const __FlashStringHelper * Command_UDP_Test (struct EventStruct *event, const char* Line);
const __FlashStringHelper * Command_UPD_SendTo(struct EventStruct *event, const char* Line);
#endif
const __FlashStringHelper * Command_UDP_SendToUPD(struct EventStruct *event, const char* Line);

#endif // COMMAND_UDP_H
