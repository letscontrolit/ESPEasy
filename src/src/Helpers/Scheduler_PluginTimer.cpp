#include "../Helpers/Scheduler.h"

#include "../DataStructs/Scheduler_PluginDeviceTimerID.h"
#include "../DataStructs/TimingStats.h"

#include "../DataTypes/ESPEasy_plugin_functions.h"

#include "../Helpers/_Plugin_init.h"

/*********************************************************************************************\
* Plugin Timer
* Essentially calling PLUGIN_DEVICETIMER_IN
* Similar to PLUGIN_TASKTIMER_IN, addressed to a plugin instead of a task.
\*********************************************************************************************/
void ESPEasy_Scheduler::setPluginTimer(unsigned long msecFromNow, pluginID_t pluginID, int Par1, int Par2, int Par3, int Par4, int Par5)
{
  // plugin number and par1 form a unique key that can be used to restart a timer
  // Use deviceIndex instead of pluginID, since the deviceIndex uses less bits.
  const deviceIndex_t deviceIndex = getDeviceIndex(pluginID);

  if (!validDeviceIndex(deviceIndex)) { return; }

  const PluginDeviceTimerID timerID(pluginID, Par1);

  systemTimerStruct timer_data;

  // PLUGIN_DEVICETIMER_IN does not address a task, so don't set TaskIndex
  timer_data.fromEvent(INVALID_TASK_INDEX, Par1, Par2, Par3, Par4, Par5);
  systemTimers[timerID.mixed_id] = timer_data;
  setNewTimerAt(timerID, millis() + msecFromNow);
}

void ESPEasy_Scheduler::process_plugin_timer(SchedulerTimerID id) {
  #ifdef USE_SECOND_HEAP
  HeapSelectDram ephemeral;
  #endif // ifdef USE_SECOND_HEAP

  START_TIMER;
  auto it = systemTimers.find(id.mixed_id);

  if (it == systemTimers.end()) { return; }

  struct EventStruct TempEvent(it->second.toEvent());

  // PLUGIN_DEVICETIMER_IN does not address a task, so don't set TaskIndex

  // extract deviceID from timer id:
  const PluginDeviceTimerID *tmp  = reinterpret_cast<const PluginDeviceTimerID *>(&id);
  const deviceIndex_t deviceIndex = tmp->get_deviceIndex();

  // TD-er: Not sure if we have to keep original source for notifications.
  TempEvent.Source = EventValueSource::Enum::VALUE_SOURCE_SYSTEM;

  //  const deviceIndex_t deviceIndex = getDeviceIndex_from_TaskIndex(it->second.TaskIndex);

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

  if (validDeviceIndex(deviceIndex)) {
    String dummy;
    PluginCall(deviceIndex, PLUGIN_DEVICETIMER_IN, &TempEvent, dummy);
  }
  STOP_TIMER(PLUGIN_CALL_DEVICETIMER_IN);
}
