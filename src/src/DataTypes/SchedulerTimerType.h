#ifndef DATATYPES_SCHEDULERTIMERTYPE_H
#define DATATYPES_SCHEDULERTIMERTYPE_H

#include "../../ESPEasy_common.h"


  enum class SchedulerTimerType_e : uint8_t {
    SystemEventQueue       = 0u, // Not really a timer.
    ConstIntervalTimer     = 1u,
    PLUGIN_TASKTIMER_IN_e  = 2u, // Called with a previously defined event at a specific time, set via setPluginTaskTimer
    TaskDeviceTimer        = 3u, // Essentially calling PLUGIN_READ
    GPIO_timer             = 4u,
    PLUGIN_DEVICETIMER_IN_e = 5u, // Similar to PLUGIN_TASKTIMER_IN, addressed to a plugin instead of a task.
    RulesTimer             = 6u,
    IntendedReboot         = 15u // Used to show intended reboot
  };

  const __FlashStringHelper* toString(SchedulerTimerType_e timerType);


#endif