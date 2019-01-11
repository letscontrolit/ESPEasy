#ifndef COMMAND_RULES_H
#define COMMAND_RULES_H


#include "../ESPEasy-Globals.h"

String Command_Rules_Execute(struct EventStruct *event, const char *Line)
{
  String filename;

  if (GetArgv(Line, filename, 2)) {
    String event;
    rulesProcessingFile(filename, event);
  }
  return return_command_success();
}

String Command_Rules_UseRules(struct EventStruct *event, const char *Line)
{
  return Command_GetORSetBool(event, F("Rules:"),
                              Line,
                              (bool *)&Settings.UseRules,
                              1);
}

String Command_Rules_Events(struct EventStruct *event, const char *Line)
{
  String eventName = Line;

  eventName = eventName.substring(6);
  eventName.replace('$', '#');

  if (Settings.UseRules) {
    rulesProcessing(eventName);
  }
  return return_command_success();
}

String Command_Rules_Let(struct EventStruct *event, const char *Line)
{
  String TmpStr1;

  if (GetArgv(Line, TmpStr1, 3)) {
    float result = 0.0;
    Calculate(TmpStr1.c_str(), &result);
    customFloatVar[event->Par1 - 1] = result;
  }
  return return_command_success();
}

#endif // COMMAND_RULES_H
