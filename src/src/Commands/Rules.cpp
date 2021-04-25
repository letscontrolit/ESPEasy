#include "../Commands/Rules.h"


#include "../../ESPEasy_common.h"


#include "../Commands/Common.h"

#include "../DataTypes/EventValueSource.h"

#include "../ESPEasyCore/Controller.h"
#include "../ESPEasyCore/ESPEasyRules.h"

#include "../Globals/EventQueue.h"
#include "../Globals/RuntimeData.h"
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
  if (Settings.UseRules) {
    String eventName = parseStringToEndKeepCase(Line, 2);

    eventName.replace('$', '#');
    eventQueue.addMove(std::move(eventName));
  }
  return return_command_success();
}

String Command_Rules_Events(struct EventStruct *event, const char *Line)
{
  if (Settings.UseRules) {
    const bool executeImmediately =
      SourceNeedsStatusUpdate(event->Source) ||
      event->Source == EventValueSource::Enum::VALUE_SOURCE_RULES;

    String eventName = parseStringToEndKeepCase(Line, 2);

    eventName.replace('$', '#');
    if (executeImmediately) {
      rulesProcessing(eventName); // TD-er: Process right now
    } else {
      eventQueue.addMove(std::move(eventName));
    }
  }
  return return_command_success();
}

String Command_Rules_Let(struct EventStruct *event, const char *Line)
{
  String TmpStr1;

  if (GetArgv(Line, TmpStr1, 3)) {
    if (event->Par1 >= 0) {
      double result = 0.0;

      if (!isError(Calculate(TmpStr1, result))) {
        setCustomFloatVar(event->Par1, result);
        return return_command_success();
      }
    }
  }
  return return_command_failed();
}
