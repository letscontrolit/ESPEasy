#include "../DataStructs/Scheduler_RulesTimerID.h"

#include "../Helpers/StringConverter.h"

RulesTimerID::RulesTimerID(unsigned int timerIndex) :
  SchedulerTimerID(SchedulerTimerType_e::RulesTimer)
{
  setId(timerIndex);
}

#ifndef BUILD_NO_DEBUG
String RulesTimerID::decode() const
{
  return concat(F("Rules#Timer="), getId());
}

#endif // ifndef BUILD_NO_DEBUG
