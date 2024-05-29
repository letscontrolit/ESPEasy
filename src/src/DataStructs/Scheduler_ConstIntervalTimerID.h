#ifndef DATASTRUCTS_SCHEDULER_CONSTINTERVALTIMERID_H
#define DATASTRUCTS_SCHEDULER_CONSTINTERVALTIMERID_H

#include "../DataStructs/SchedulerTimerID.h"

#include "../DataTypes/SchedulerIntervalTimer.h"

struct ConstIntervalTimerID : SchedulerTimerID {
  ConstIntervalTimerID(SchedulerIntervalTimer_e timer) :
    SchedulerTimerID(SchedulerTimerType_e::ConstIntervalTimer)
  {
    setId(static_cast<uint32_t>(timer));
  }

  SchedulerIntervalTimer_e getIntervalTimer() const
  {
    return static_cast<SchedulerIntervalTimer_e>(getId());
  }

#ifndef BUILD_NO_DEBUG
  String decode() const
  {
    return toString(getIntervalTimer());
  }

#endif // ifndef BUILD_NO_DEBUG
};


#endif // ifndef DATASTRUCTS_SCHEDULER_CONSTINTERVALTIMERID_H
