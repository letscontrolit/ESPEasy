#include "../Helpers/Scheduler.h"

#include "../DataStructs/Scheduler_PluginTaskTimerID.h"

#include "../Globals/Settings.h"

/*********************************************************************************************\
* Plugin Task Timer
\*********************************************************************************************/
void ESPEasy_Scheduler::setPluginTaskTimer(
  unsigned long msecFromNow,
  taskIndex_t taskIndex,
  int Par1, int Par2, int Par3, int Par4, int Par5)
{
  setPluginTaskTimer(msecFromNow,
                     taskIndex,
                     PLUGIN_TASKTIMER_IN,
                     Par1, Par2, Par3, Par4, Par5);
}

void ESPEasy_Scheduler::setPluginTaskTimer(
  unsigned long msecFromNow,
  taskIndex_t taskIndex,
  PluginFunctions_e function,
  int Par1, int Par2, int Par3, int Par4, int Par5)
{
  // taskIndex and par1 form a unique key that can be used to restart a timer
  if (!validTaskIndex(taskIndex)) { return; }

  if (!Settings.TaskDeviceEnabled[taskIndex]) { return; }

  const PluginTaskTimerID timerID(taskIndex, Par1);

  systemTimerStruct timer_data;

  timer_data.fromEvent(taskIndex, Par1, Par2, Par3, Par4, Par5);
  systemTimers[timerID.mixed_id] = timer_data;
  setNewTimerAt(timerID, millis() + msecFromNow);
}

void ESPEasy_Scheduler::process_plugin_task_timer(SchedulerTimerID id) {
  #ifdef USE_SECOND_HEAP
  HeapSelectDram ephemeral;
  #endif // ifdef USE_SECOND_HEAP

  auto it = systemTimers.find(id.mixed_id);

  if (it == systemTimers.end()) { return; }

  struct EventStruct TempEvent(it->second.toEvent());

  // TD-er: Not sure if we have to keep original source for notifications.
  TempEvent.Source = EventValueSource::Enum::VALUE_SOURCE_SYSTEM;


  /*
     String log = F("proc_system_timer: Pluginid: ");
     log += deviceIndex;
     log += F(" taskIndex: ");
     log += it->second.TaskIndex;
     log += F(" sysTimerID: ");
     log += id;
     addLog(LOG_LEVEL_INFO, log);
   */
  systemTimers.erase(id.mixed_id);

  {
    String dummy;
    const PluginTaskTimerID *tmp      = reinterpret_cast<const PluginTaskTimerID *>(&id);
    const PluginFunctions_e  function = tmp->getFunction();
    PluginCall(function, &TempEvent, dummy);
  }
}
