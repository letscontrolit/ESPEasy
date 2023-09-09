#include "../DataTypes/SchedulerTimerType.h"



const __FlashStringHelper * toString(SchedulerTimerType_e timerType) {
  switch (timerType) {
    case SchedulerTimerType_e::SystemEventQueue:        return F("SystemEventQueue");
    case SchedulerTimerType_e::ConstIntervalTimer:      return F("Const Interval");
    case SchedulerTimerType_e::PLUGIN_TASKTIMER_IN_e:   return F("PLUGIN_TASKTIMER_IN");
    case SchedulerTimerType_e::TaskDeviceTimer:         return F("PLUGIN_READ");
    case SchedulerTimerType_e::GPIO_timer:              return F("GPIO_timer");
    case SchedulerTimerType_e::PLUGIN_DEVICETIMER_IN_e: return F("PLUGIN_DEVICETIMER_IN");
    case SchedulerTimerType_e::RulesTimer:              return F("Rules#Timer");
    case SchedulerTimerType_e::IntendedReboot:          return F("Intended Reboot");
  }
  return F("Unknown");
}
