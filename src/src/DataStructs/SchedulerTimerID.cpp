#include "../DataStructs/SchedulerTimerID.h"

SchedulerTimerID::SchedulerTimerID(SchedulerTimerType_e timerType)
{
  timer_type = static_cast<uint32_t>(timerType);
}
