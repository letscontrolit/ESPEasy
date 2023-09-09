#ifndef DATASTRUCTS_SCHEDULER_CONSTINTERVALTIMERID_H
#define DATASTRUCTS_SCHEDULER_CONSTINTERVALTIMERID_H

#include "../DataStructs/SchedulerTimerID.h"

#include "../DataTypes/SchedulerIntervalTimer.h"

struct ConstIntervalTimerID : SchedulerTimerID {
  ConstIntervalTimerID(SchedulerIntervalTimer_e timer);

  SchedulerIntervalTimer_e getIntervalTimer() const;

#ifndef BUILD_NO_DEBUG
  String                   decode() const;
#endif // ifndef BUILD_NO_DEBUG
};


#endif // ifndef DATASTRUCTS_SCHEDULER_CONSTINTERVALTIMERID_H
