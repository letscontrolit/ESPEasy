#include "../Commands/Timer.h"


#include "../Commands/Common.h"
#include "../../ESPEasy_fdwdecl.h"
#include "../../ESPEasy_common.h"

#include "../../ESPEasy-Globals.h"
#include "../Helpers/ESPEasy_time_calc.h"
#include "../Helpers/Scheduler.h"


String Command_Timer_Set(struct EventStruct *event, const char *Line)
{
  if (event->Par2 >= 0)
  {
    // start new timer when interval > 0
    // Clear timer when interval == 0
    if (setRulesTimer(
          event->Par2 * 1000, // interval
          event->Par1,        // timer index
          false               // isRecurring
          ))
    { 
      return return_command_success();
    }
  } else {
    addLog(LOG_LEVEL_ERROR, F("TIMER: time must be positive"));
  }
  return return_command_failed();
}

String Command_Timer_Pause(struct EventStruct *event, const char *Line)
{
  if (pause_rules_timer(event->Par1)) {
    String eventName = F("Rules#TimerPause=");
    eventName += event->Par1;
    rulesProcessing(eventName); // TD-er: Process right now
    return return_command_success();
  }
  return return_command_failed();
}

String Command_Timer_Resume(struct EventStruct *event, const char *Line)
{
  if (resume_rules_timer(event->Par1)) {
    String eventName = F("Rules#TimerResume=");
    eventName += event->Par1;
    rulesProcessing(eventName); // TD-er: Process right now
    return return_command_success();
  }
  return return_command_failed();
}

String Command_Delay(struct EventStruct *event, const char *Line)
{
  delayBackground(event->Par1);
  return return_command_success();
}
