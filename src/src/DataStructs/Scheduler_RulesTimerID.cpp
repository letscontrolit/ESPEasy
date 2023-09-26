#include "../DataStructs/Scheduler_RulesTimerID.h"

#include "../Helpers/StringConverter.h"

RulesTimerID::RulesTimerID(unsigned int timerIndex) :
  SchedulerTimerID(SchedulerTimerType_e::RulesTimer)
{
  id = timerIndex;
}

#ifndef BUILD_NO_DEBUG
String RulesTimerID::decode() const
{
  return concat(F("Rules#Timer="), id);
}

#endif // ifndef BUILD_NO_DEBUG
