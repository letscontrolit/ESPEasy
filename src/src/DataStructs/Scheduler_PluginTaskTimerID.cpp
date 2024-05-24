#include "../DataStructs/Scheduler_PluginTaskTimerID.h"

#include "../Globals/Plugins.h"
#include "../Helpers/Misc.h"

PluginTaskTimerID::PluginTaskTimerID(taskIndex_t       taskIndex,
                                     int               Par1,
                                     PluginFunctions_e function) :
  SchedulerTimerID(SchedulerTimerType_e::PLUGIN_TASKTIMER_IN_e)
{
  constexpr unsigned nrBitsTaskIndex      = NR_BITS(TASKS_MAX);
  constexpr unsigned mask_taskIndex       = MASK_BITS(nrBitsTaskIndex);
  constexpr unsigned nrBitsPluginFunction = NrBitsPluginFunctions;
  constexpr unsigned mask_function        = MASK_BITS(nrBitsPluginFunction);

  if (validTaskIndex(taskIndex)) {
    setId((taskIndex & mask_taskIndex) |
         ((function & mask_function) << nrBitsTaskIndex) |
         (Par1 << (nrBitsTaskIndex + nrBitsPluginFunction)));
  }
}

taskIndex_t PluginTaskTimerID::getTaskIndex() const
{
  constexpr unsigned nrBitsTaskIndex = NR_BITS(TASKS_MAX);
  constexpr unsigned mask_taskIndex  = MASK_BITS(nrBitsTaskIndex);

  return static_cast<taskIndex_t>(getId() & mask_taskIndex);
}

PluginFunctions_e PluginTaskTimerID::getFunction() const
{
  constexpr unsigned nrBitsTaskIndex      = NR_BITS(TASKS_MAX);
  constexpr unsigned nrBitsPluginFunction = NrBitsPluginFunctions;
  constexpr unsigned mask_function        = MASK_BITS(nrBitsPluginFunction);

  return static_cast<PluginFunctions_e>((getId() >> nrBitsTaskIndex) & mask_function);
}

#ifndef BUILD_NO_DEBUG
String PluginTaskTimerID::decode() const
{
  const taskIndex_t taskIndex = getTaskIndex();

  if (validTaskIndex(taskIndex)) {
    return getTaskDeviceName(taskIndex);
  }
  return String(getId());
}

#endif // ifndef BUILD_NO_DEBUG
