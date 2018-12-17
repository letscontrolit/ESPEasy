#ifndef COMMAND_TIMER_H
#define COMMAND_TIMER_H


String Command_Timer_Set (struct EventStruct *event, const char* Line)
{
  if (event->Par1>=1 && event->Par1<=RULES_TIMER_MAX)
  {
      int timerId= event->Par1-1;
      if (event->Par2)
      {
        //start new timer
        RulesTimer[timerId].interval = event->Par2*1000;
        RulesTimer[timerId].paused = false;
        RulesTimer[timerId].timestamp = millis() + (1000 * event->Par2);
      }
      else
      {
        //disable existing timer
        RulesTimer[timerId].interval = 0;
        RulesTimer[timerId].paused = false;
        RulesTimer[timerId].timestamp = 0L;
      }
      return return_command_success();
  }
  else
  {
      addLog(LOG_LEVEL_ERROR, F("TIMER: invalid timer number"));
  }
  return return_command_failed();
}

String Command_Timer_Pause (struct EventStruct *event, const char* Line)
{
  if (event->Par1>=1 && event->Par1<=RULES_TIMER_MAX)
  {
      int timerId = event->Par1-1;
      if (RulesTimer[timerId].paused == false)
      {
        long delta = timePassedSince(RulesTimer[timerId].timestamp);
        if(RulesTimer[timerId].timestamp != 0L && delta < 0)
        {
          String eventName = F("Rules#TimerPause=");
          eventName += event->Par1;
          rulesProcessing(eventName);
          RulesTimer[timerId].paused = true;
          RulesTimer[timerId].interval = -delta; // set remaind time
        }
      }
      else
      {
        addLog(LOG_LEVEL_INFO, F("TIMER: already paused"));
      }
      return return_command_success();
  }
  else
  {
    addLog(LOG_LEVEL_ERROR, F("TIMER: invalid timer number"));
  }
  return return_command_failed();
}

String Command_Timer_Resume (struct EventStruct *event, const char* Line)
{
  if (event->Par1>=1 && event->Par1<=RULES_TIMER_MAX)
  {
      int timerId = event->Par1-1;
      if (RulesTimer[timerId].paused == true)
      {
        if(RulesTimer[timerId].interval > 0 && RulesTimer[timerId].timestamp != 0L)
        {
          String eventName = F("Rules#TimerResume=");
          eventName += event->Par1;
          rulesProcessing(eventName);
          RulesTimer[timerId].timestamp = millis() + (RulesTimer[timerId].interval);
          RulesTimer[timerId].paused = false;
        }
      }
      else
      {
        addLog(LOG_LEVEL_INFO, F("TIMER: already resumed"));
      }
      return return_command_success();
  }
  else
  {
    addLog(LOG_LEVEL_ERROR, F("TIMER: invalid timer number"));
  }
  return return_command_failed();
}

String Command_Delay (struct EventStruct *event, const char* Line)
{
  delayBackground(event->Par1);
  return return_command_success();
}

#endif // COMMAND_TIMER_H
