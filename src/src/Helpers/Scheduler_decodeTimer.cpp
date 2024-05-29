#include "../Helpers/Scheduler.h"

#include "../DataTypes/SchedulerTimerType.h"

#ifndef BUILD_NO_DEBUG
# include "../DataStructs/Scheduler_ConstIntervalTimerID.h"
# include "../DataStructs/Scheduler_GPIOTimerID.h"
# include "../DataStructs/Scheduler_IntendedRebootTimerID.h"
# include "../DataStructs/Scheduler_PluginDeviceTimerID.h"
# include "../DataStructs/Scheduler_PluginTaskTimerID.h"
# include "../DataStructs/Scheduler_RulesTimerID.h"
# include "../DataStructs/Scheduler_SystemEventQueueTimerID.h"
# include "../DataStructs/Scheduler_TaskDeviceTimerID.h"
#endif // ifndef BUILD_NO_DEBUG

String ESPEasy_Scheduler::decodeSchedulerId(SchedulerTimerID timerID) {
  if (timerID.mixed_id == 0) {
    return F("Background Task");
  }
  String result = toString(timerID.getTimerType());

  result += F(": ");

#ifndef BUILD_NO_DEBUG

  String decoded;

  // Must call decode on specific class instance.
  switch (timerID.getTimerType()) {
    case SchedulerTimerType_e::ConstIntervalTimer:
      decoded = reinterpret_cast<const ConstIntervalTimerID *>(&timerID)->decode();
      break;
    case SchedulerTimerType_e::GPIO_timer:
      decoded = reinterpret_cast<const GPIOTimerID *>(&timerID)->decode();
      break;
    case SchedulerTimerType_e::IntendedReboot:
      decoded = reinterpret_cast<const IntendedRebootTimerID *>(&timerID)->decode();
      break;
    case SchedulerTimerType_e::PLUGIN_DEVICETIMER_IN_e:
      decoded = reinterpret_cast<const PluginDeviceTimerID *>(&timerID)->decode();
      break;
    case SchedulerTimerType_e::PLUGIN_TASKTIMER_IN_e:
      decoded = reinterpret_cast<const PluginTaskTimerID *>(&timerID)->decode();
      break;
    case SchedulerTimerType_e::RulesTimer:
      decoded = reinterpret_cast<const RulesTimerID *>(&timerID)->decode();
      break;
    case SchedulerTimerType_e::SystemEventQueue:
      decoded = reinterpret_cast<const SystemEventQueueTimerID *>(&timerID)->decode();
      break;
    case SchedulerTimerType_e::TaskDeviceTimer:
      decoded = reinterpret_cast<const TaskDeviceTimerID *>(&timerID)->decode();
      break;

      // TD-er: Do not add a default: case here, so the compiler will warn us when we're missing one later.
  }

  if (!decoded.isEmpty()) {
    result += decoded;
    return result;
  }

#endif // ifndef BUILD_NO_DEBUG
  result += F(" timer, id: ");
  result += timerID.getId();
  return result;
}
