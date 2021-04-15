#ifndef COMMAND_NOTIFICATIONS_H
#define COMMAND_NOTIFICATIONS_H

#include "../../ESPEasy_common.h"

#ifdef USES_NOTIFIER

class String;

String Command_Notifications_Notify(struct EventStruct *event, const char* Line);

#endif

#endif // COMMAND_NOTIFICATIONS_H
