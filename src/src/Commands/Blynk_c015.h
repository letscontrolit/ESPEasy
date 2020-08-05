#ifndef COMMAND_BLYNK_C015_H
#define COMMAND_BLYNK_C015_H

#include "../../ESPEasy_common.h"

#ifdef USES_C015

String Command_Blynk_Set_c015(struct EventStruct *event, const char* Line);

String Command_Blynk_Set(struct EventStruct *event, const char* Line)
{
  return Command_Blynk_Set_c015(event,Line);
}
#endif

#endif // COMMAND_BLYNK_C015_H
