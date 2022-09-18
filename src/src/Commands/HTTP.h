#ifndef COMMAND_HTTP_H
#define COMMAND_HTTP_H

#include <Arduino.h>

#if FEATURE_SEND_TO_HTTP
const __FlashStringHelper * Command_HTTP_SendToHTTP(struct EventStruct *event, const char* Line);
#endif // FEATURE_SEND_TO_HTTP

#endif // COMMAND_HTTP_H
