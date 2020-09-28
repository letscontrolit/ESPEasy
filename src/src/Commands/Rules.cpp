#include "../Commands/Rules.h"

#include "../../ESPEasy-Globals.h"
#include "../../ESPEasy_common.h"
#include "../../ESPEasy_fdwdecl.h"

#include "../Commands/Common.h"

#include "../DataStructs/EventValueSource.h"

#include "../Globals/EventQueue.h"
#include "../Globals/Settings.h"

#include "../Helpers/Misc.h"
#include "../Helpers/Rules_calculate.h"
#include "../Helpers/StringConverter.h"

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

String Command_Rules_Async_Events(struct EventStruct *event, const char *Line)
{
  String eventName = parseStringToEndKeepCase(Line, 2);
  eventName.replace('$', '#');

  if (Settings.UseRules) {
    eventQueue.add(eventName);
  }
  return return_command_success();
}


String Command_Rules_Events(struct EventStruct *event, const char *Line)
{
  String eventName = parseStringToEndKeepCase(Line, 2);
  eventName.replace('$', '#');

  if (Settings.UseRules) {
    const bool executeImmediately = 
        SourceNeedsStatusUpdate(event->Source) ||
        event->Source == EventValueSource::Enum::VALUE_SOURCE_RULES;
    if (executeImmediately) {
      rulesProcessing(eventName); // TD-er: Process right now 
    } else {
      eventQueue.add(eventName);
    }
  }
  return return_command_success();
}

String Command_Rules_Let(struct EventStruct *event, const char *Line)
{
  String TmpStr1;

  if (GetArgv(Line, TmpStr1, 3)) {
    float result = 0.0f;
    Calculate(TmpStr1.c_str(), &result);
    customFloatVar[event->Par1 - 1] = result;
  }
  return return_command_success();
}
