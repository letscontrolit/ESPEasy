#ifndef DATASTRUCTS_SCHEDULER_INTENDEDREBOOTTIMERID_H
#define DATASTRUCTS_SCHEDULER_INTENDEDREBOOTTIMERID_H

#include "../DataStructs/SchedulerTimerID.h"

#include "../DataTypes/IntendedRebootReason.h"

struct IntendedRebootTimerID : SchedulerTimerID {
  IntendedRebootTimerID(IntendedRebootReason_e reason);

  IntendedRebootReason_e getReason() const;

#ifndef BUILD_NO_DEBUG
  String                 decode() const override;
#endif // ifndef BUILD_NO_DEBUG
};


#endif // ifndef DATASTRUCTS_SCHEDULER_INTENDEDREBOOTTIMERID_H
