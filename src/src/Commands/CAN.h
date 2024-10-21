#ifndef COMMAND_CAN_H
#define COMMAND_CAN_H

#include "../../ESPEasy_common.h"

#if FEATURE_CAN

const __FlashStringHelper* Command_CAN_SendToCAN(struct EventStruct *event, const char * line);

const __FlashStringHelper* Command_CAN_SendCAN(struct EventStruct *event, const char * line);

#endif
#endif