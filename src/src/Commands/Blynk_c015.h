#ifndef COMMAND_BLYNK_C015_H
#define COMMAND_BLYNK_C015_H

#include "../../define_plugin_sets.h"

#ifdef USES_C015

String Command_Blynk_Set(struct EventStruct *event, const char* Line)
{
  return Command_Blynk_Set_c015(event,Line);
}
#endif

#endif // COMMAND_BLYNK_C015_H
