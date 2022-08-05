#ifndef COMMAND_NOTIFICATIONS_H
#define COMMAND_NOTIFICATIONS_H

#include "../../ESPEasy_common.h"

#if FEATURE_NOTIFIER


const __FlashStringHelper * Command_Notifications_Notify(struct EventStruct *event, const char* Line);

#endif

#endif // COMMAND_NOTIFICATIONS_H
