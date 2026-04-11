#include "../DataStructs/SchedulerTimerID.h"

#include "../Helpers/Misc.h"

SchedulerTimerID::SchedulerTimerID(SchedulerTimerType_e timerType)
{
  set4BitToUL(mixed_id, 0, static_cast<uint32_t>(timerType));
}

void SchedulerTimerID::setTimerType(SchedulerTimerType_e timerType)
{
  set4BitToUL(mixed_id, 0, static_cast<uint32_t>(timerType));
}

SchedulerTimerType_e SchedulerTimerID::getTimerType() const
{
  return static_cast<SchedulerTimerType_e>(get4BitFromUL(mixed_id, 0));
}

uint32_t SchedulerTimerID::getId() const
{
  return mixed_id >> 4;
}
