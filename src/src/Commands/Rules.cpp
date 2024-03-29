#include "../Commands/Rules.h"


#include "../../ESPEasy_common.h"


#include "../Commands/Common.h"

#include "../DataTypes/EventValueSource.h"

#include "../ESPEasyCore/Controller.h"
#include "../ESPEasyCore/ESPEasyRules.h"

#include "../Globals/EventQueue.h"
#include "../Globals/RulesCalculate.h"
#include "../Globals/RuntimeData.h"
#include "../Globals/Settings.h"

#include "../Helpers/Misc.h"
#include "../Helpers/StringConverter.h"

const __FlashStringHelper * Command_Rules_Execute(struct EventStruct *event, const char *Line)
{
  String filename;

  if (GetArgv(Line, filename, 2)) {
    String event;
    rulesProcessingFile(filename, event);
  }
  return return_command_success_flashstr();
}

String Command_Rules_UseRules(struct EventStruct *event, const char *Line)
{
  return Command_GetORSetBool(event, F("Rules:"),
                              Line,
                              (bool *)&Settings.UseRules,
                              1);
}

const __FlashStringHelper * Command_Rules_Async_Events(struct EventStruct *event, const char *Line)
{
  if (Settings.UseRules) {
    String eventName = parseStringToEndKeepCase(Line, 2);

    eventName.replace('$', '#');
    eventQueue.addMove(std::move(eventName));
  }
  return return_command_success_flashstr();
}

const __FlashStringHelper * Command_Rules_Events(struct EventStruct *event, const char *Line)
{
  if (Settings.UseRules) {
    const bool executeImmediately =
      SourceNeedsStatusUpdate(event->Source) ||
      event->Source == EventValueSource::Enum::VALUE_SOURCE_RULES ||
      event->Source == EventValueSource::Enum::VALUE_SOURCE_RULES_RESTRICTED;

    String eventName = parseStringToEndKeepCase(Line, 2);

    eventName.replace('$', '#');
    if (executeImmediately) {
      rulesProcessing(eventName); // TD-er: Process right now
    } else {
      eventQueue.addMove(std::move(eventName));
    }
  }
  return return_command_success_flashstr();
}

const __FlashStringHelper * Command_Rules_Let(struct EventStruct *event, const char *Line)
{
  String TmpStr1;

  if (GetArgv(Line, TmpStr1, 3)) {
    if (event->Par1 >= 0) {
      ESPEASY_RULES_FLOAT_TYPE result{};

      if (!isError(Calculate(TmpStr1, result))) {
        setCustomFloatVar(event->Par1, result);
        return return_command_success_flashstr();
      }
    }
  }
  return return_command_failed_flashstr();
}

const __FlashStringHelper * Command_Rules_IncDec(struct EventStruct *event, const char *Line, const ESPEASY_RULES_FLOAT_TYPE factor)
{
  String TmpStr1;
  ESPEASY_RULES_FLOAT_TYPE result = 1;

  if (GetArgv(Line, TmpStr1, 3) && isError(Calculate(TmpStr1, result))) {
    return return_command_failed_flashstr();
  }
  if (event->Par1 >= 0) {
    setCustomFloatVar(event->Par1, getCustomFloatVar(event->Par1) + (result * factor));
    return return_command_success_flashstr();
  }
  return return_command_failed_flashstr();
}

const __FlashStringHelper * Command_Rules_Inc(struct EventStruct *event, const char *Line)
{
  return Command_Rules_IncDec(event, Line, 1.0);
}

const __FlashStringHelper * Command_Rules_Dec(struct EventStruct *event, const char *Line)
{
  return Command_Rules_IncDec(event, Line, -1.0);
}
