#include "../DataStructs/Scheduler_ConstIntervalTimerID.h"

/*

ConstIntervalTimerID::ConstIntervalTimerID(SchedulerIntervalTimer_e timer) :
  SchedulerTimerID(SchedulerTimerType_e::ConstIntervalTimer)
{
  id = static_cast<uint32_t>(timer);
}

SchedulerIntervalTimer_e ConstIntervalTimerID::getIntervalTimer() const
{
  return static_cast<SchedulerIntervalTimer_e>(id);
}

#ifndef BUILD_NO_DEBUG
String ConstIntervalTimerID::decode() const
{
  return toString(getIntervalTimer());
}

#endif // ifndef BUILD_NO_DEBUG
*/