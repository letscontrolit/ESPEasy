#include "../DataStructs/Scheduler_PluginTaskTimerID.h"

#include "../Globals/Plugins.h"
#include "../Helpers/Misc.h"

PluginTaskTimerID::PluginTaskTimerID(taskIndex_t taskIndex, int Par1) :
  SchedulerTimerID(SchedulerTimerType_e::PLUGIN_TASKTIMER_IN_e)
{
  constexpr unsigned nrBitsTaskIndex = NR_BITS(TASKS_MAX);
  constexpr unsigned mask            = MASK_BITS(nrBitsTaskIndex);

  if (validTaskIndex(taskIndex)) {
    id = (taskIndex & mask) | (Par1 << nrBitsTaskIndex);
  }
}

taskIndex_t PluginTaskTimerID::getTaskIndex() const
{
  constexpr unsigned nrBitsTaskIndex = NR_BITS(TASKS_MAX);
  constexpr unsigned mask            = MASK_BITS(nrBitsTaskIndex);

  return static_cast<taskIndex_t>(id & mask);
}

#ifndef BUILD_NO_DEBUG
String PluginTaskTimerID::decode() const
{
  const taskIndex_t taskIndex = getTaskIndex();

  if (validTaskIndex(taskIndex)) {
    return getTaskDeviceName(taskIndex);
  }
  return String(id);
}

#endif // ifndef BUILD_NO_DEBUG
