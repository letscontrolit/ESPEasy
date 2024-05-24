#include "../DataStructs/Scheduler_IntendedRebootTimerID.h"


IntendedRebootTimerID::IntendedRebootTimerID(IntendedRebootReason_e reason) :
  SchedulerTimerID(SchedulerTimerType_e::IntendedReboot)
{
  setId(static_cast<uint32_t>(reason));
}

