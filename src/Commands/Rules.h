#ifndef COMMAND_RULES_H
#define COMMAND_RTC_H


#include "../ESPEasy-Globals.h"

bool Command_Rules_Execute (struct EventStruct *event, const char* Line)
{
   bool success = true;
   char TmpStr1[INPUT_COMMAND_SIZE];
   if (GetArgv(Line, TmpStr1, 2))
   {
    String fileName = TmpStr1;
    String event = "";
    rulesProcessingFile(fileName, event);
   }
   return success;
}


bool Command_Rules_UseRules(struct EventStruct *event, const char* Line)
{
  return Command_GetORSetBool(F("Rules:"),
   Line,
   (bool *)&Settings.UseRules,
   1);
}

bool Command_Rules_Events(struct EventStruct *event, const char* Line)
{
  String eventName = Line;
  eventName = eventName.substring(6);
  eventName.replace('$', '#');
  if (Settings.UseRules)
    rulesProcessing(eventName);
  return true;
}

#endif // COMMAND_RULES_H