#ifndef DATASTRUCTS_SCHEDULER_TASKDEVICETIMERID_H
#define DATASTRUCTS_SCHEDULER_TASKDEVICETIMERID_H

#include "../DataStructs/SchedulerTimerID.h"

#include "../DataTypes/TaskIndex.h"

/*********************************************************************************************\
* Task Device Timer
* Essentially calling SensorSendTask, which calls PLUGIN_READ
\*********************************************************************************************/
struct TaskDeviceTimerID : SchedulerTimerID {
  TaskDeviceTimerID(taskIndex_t taskIndex);

  taskIndex_t getTaskIndex() const
  {
    return static_cast<taskIndex_t>(getId());
  }

#ifndef BUILD_NO_DEBUG
  String decode() const;
#endif // ifndef BUILD_NO_DEBUG
};

#endif // ifndef DATASTRUCTS_SCHEDULER_TASKDEVICETIMERID_H
