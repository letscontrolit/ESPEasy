#include "../DataStructs/Scheduler_TaskDeviceTimerID.h"

#include "../Helpers/Misc.h"
#include "../Helpers/StringConverter.h"

TaskDeviceTimerID::TaskDeviceTimerID(taskIndex_t taskIndex) :
  SchedulerTimerID(SchedulerTimerType_e::TaskDeviceTimer)
{
  setId(static_cast<uint32_t>(taskIndex));
}

#ifndef BUILD_NO_DEBUG
String TaskDeviceTimerID::decode() const
{
  const taskIndex_t taskIndex = getTaskIndex();

  return concat(F("Task "), validTaskIndex(taskIndex)
        ? getTaskDeviceName(taskIndex)
        : String(getId()));
}

#endif // ifndef BUILD_NO_DEBUG
