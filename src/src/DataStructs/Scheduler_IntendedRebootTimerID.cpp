#include "../DataStructs/Scheduler_IntendedRebootTimerID.h"


IntendedRebootTimerID::IntendedRebootTimerID(IntendedRebootReason_e reason) :
  SchedulerTimerID(SchedulerTimerType_e::IntendedReboot)
{
  id = static_cast<uint32_t>(reason);
}

IntendedRebootReason_e IntendedRebootTimerID::getReason() const
{
  return static_cast<IntendedRebootReason_e>(id);
}

#ifndef BUILD_NO_DEBUG
String IntendedRebootTimerID::decode() const
{
  return toString(getReason());
}
#endif // ifndef BUILD_NO_DEBUG
