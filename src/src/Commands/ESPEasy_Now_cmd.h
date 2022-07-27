#ifndef COMMANDS_ESPEASY_NOW_CMD_H
#define COMMANDS_ESPEASY_NOW_CMD_H

#include "../../ESPEasy_common.h"

#ifdef USES_ESPEASY_NOW

#include "../Globals/ESPEasy_now_state.h"

class String;

String Command_ESPEasy_Now_Disable(struct EventStruct *event,
                                   const char         *Line);
String Command_ESPEasy_Now_Enable(struct EventStruct *event,
                                  const char         *Line);

#endif // ifdef USES_ESPEASY_NOW

#endif // COMMANDS_ESPEASY_NOW_CMD_H
