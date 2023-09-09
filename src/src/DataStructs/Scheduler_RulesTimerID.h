#ifndef DATASTRUCTS_SCHEDULER_RULESTIMERID_H
#define DATASTRUCTS_SCHEDULER_RULESTIMERID_H

#include "../DataStructs/SchedulerTimerID.h"

struct RulesTimerID : SchedulerTimerID {
  RulesTimerID(unsigned int timerIndex);

#ifndef BUILD_NO_DEBUG
  String decode() const;
#endif // ifndef BUILD_NO_DEBUG
};


#endif // ifndef DATASTRUCTS_SCHEDULER_RULESTIMERID_H
