#include "../DataStructs/SchedulerTimerID.h"

SchedulerTimerID::SchedulerTimerID(SchedulerTimerType_e timerType)
{
  timer_type = static_cast<uint32_t>(timerType);
}

SchedulerTimerID::SchedulerTimerID(uint32_t mixedID) : mixed_id(mixedID) {}


void SchedulerTimerID::setTimerType(SchedulerTimerType_e timerType)
{
  timer_type = static_cast<uint32_t>(timerType);
}

SchedulerTimerType_e SchedulerTimerID::getTimerType() const
{
  return static_cast<SchedulerTimerType_e>(timer_type);
}

#ifndef BUILD_NO_DEBUG
/*
String SchedulerTimerID::decode() const
{
  return String(id);
}
*/
#endif // ifndef BUILD_NO_DEBUG
