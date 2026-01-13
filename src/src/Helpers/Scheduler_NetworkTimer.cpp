#include "../Helpers/Scheduler.h"

#include "../DataStructs/Scheduler_NWPluginTimerID.h"

#include "../Globals/Settings.h"

/*********************************************************************************************\
* Network Adapter Timer  (NWPLUGIN_TASKTIMER_IN)
* Can be scheduled per combo networkIndex & Par1 (20 least significant bits)
\*********************************************************************************************/
void ESPEasy_Scheduler::setNetworkInitTimer(unsigned long                msecFromNow,
                                            ESPEasy::net::networkIndex_t networkIndex)
{
  setNetworkTimer(msecFromNow, networkIndex, NWPlugin::Function::NWPLUGIN_INIT);
}

void ESPEasy_Scheduler::setNetworkExitTimer(unsigned long                msecFromNow,
                                            ESPEasy::net::networkIndex_t networkIndex)
{
  setNetworkTimer(msecFromNow, networkIndex, NWPlugin::Function::NWPLUGIN_EXIT);
}

void ESPEasy_Scheduler::setNetworkTimer(unsigned long                msecFromNow,
                                        ESPEasy::net::networkIndex_t networkIndex,
                                        NWPlugin::Function           function,
                                        int                          Par1,
                                        int                          Par2,
                                        int                          Par3,
                                        int                          Par4,
                                        int                          Par5)
{
  if (!validNetworkIndex(networkIndex)) { return; }

  if (!Settings.getNetworkEnabled(networkIndex)) { return; }

  const NWPluginTimerID timerID(networkIndex, Par1, function);

  systemTimerStruct timer_data;

  timer_data.fromEvent(networkIndex, Par1, Par2, Par3, Par4, Par5);
  systemTimers[timerID.mixed_id] = timer_data;
  setNewTimerAt(timerID, millis() + msecFromNow);
}

void ESPEasy_Scheduler::process_network_timer(SchedulerTimerID id)
{
  auto it = systemTimers.find(id.mixed_id);

  if (it == systemTimers.end()) { return; }

  struct EventStruct TempEvent(it->second.toNetworkEvent());

  // TD-er: Not sure if we have to keep original source for notifications.
  TempEvent.Source = EventValueSource::Enum::VALUE_SOURCE_SYSTEM;
  systemTimers.erase(id.mixed_id);

  {
    String dummy;
    const NWPluginTimerID   *tmp      = reinterpret_cast<const NWPluginTimerID *>(&id);
    const NWPlugin::Function function = tmp->getFunction();
    ESPEasy::net::NWPluginCall(function, &TempEvent, dummy);
  }

}
