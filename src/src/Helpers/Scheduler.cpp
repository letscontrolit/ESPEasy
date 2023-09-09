#include "../Helpers/Scheduler.h"

#include "../../ESPEasy-Globals.h"
#include "../../_Plugin_Helper.h"

#include "../DataStructs/Scheduler_IntendedRebootTimerID.h"
#include "../DataStructs/TimingStats.h"

#include "../ESPEasyCore/ESPEasyRules.h"

#include "../Globals/RTC.h"

#include "../Helpers/ESPEasyRTC.h"


void ESPEasy_Scheduler::markIntendedReboot(IntendedRebootReason_e reason) {
  const IntendedRebootTimerID id(reason);

  RTC.lastMixedSchedulerId = id.mixed_id;
  saveToRTC();
}

/*********************************************************************************************\
* Generic Timer functions.
\*********************************************************************************************/
void ESPEasy_Scheduler::setNewTimerAt(SchedulerTimerID id, unsigned long timer) {
  START_TIMER;
  msecTimerHandler.registerAt(id.mixed_id, timer);
  STOP_TIMER(SET_NEW_TIMER);
}

/*********************************************************************************************\
* Handle scheduled timers.
\*********************************************************************************************/
void ESPEasy_Scheduler::handle_schedule() {
  START_TIMER
  unsigned long timer    = 0;
  unsigned long mixed_id = 0;

  if (timePassedSince(last_system_event_run) < 500) {
    // Make sure system event queue will be looked at every now and then.
    mixed_id = msecTimerHandler.getNextId(timer);
  }

  if (RTC.lastMixedSchedulerId != mixed_id) {
    RTC.lastMixedSchedulerId = mixed_id;
    saveToRTC();
  }

  if (mixed_id == 0) {
    // No id ready to run right now.
    // Events are not that important to run immediately.
    // Make sure normal scheduled jobs run at higher priority.
    // backgroundtasks();
    process_system_event_queue();

    // System events may have added one or more rule events, try to process those
    processNextEvent();
    last_system_event_run = millis();
    STOP_TIMER(HANDLE_SCHEDULER_IDLE);
    return;
  }

  const SchedulerTimerID timerID(mixed_id);

  delay(0); // See: https://github.com/letscontrolit/ESPEasy/issues/1818#issuecomment-425351328

  switch (timerID.getTimerType()) {
    case SchedulerTimerType_e::ConstIntervalTimer:
      process_interval_timer(timerID, timer);
      break;
    case SchedulerTimerType_e::PLUGIN_TASKTIMER_IN_e:
      process_plugin_task_timer(timerID);
      break;
    case SchedulerTimerType_e::PLUGIN_DEVICETIMER_IN_e:
      process_plugin_timer(timerID);
      break;
    case SchedulerTimerType_e::RulesTimer:
      process_rules_timer(timerID, timer);
      break;
    case SchedulerTimerType_e::TaskDeviceTimer:
      process_task_device_timer(timerID, timer);
      break;
    case SchedulerTimerType_e::GPIO_timer:
      process_gpio_timer(timerID, timer);
      break;

    case SchedulerTimerType_e::SystemEventQueue:
    case SchedulerTimerType_e::IntendedReboot:
      // TD-er: Not really something that needs to be processed here.
      // - SystemEventQueue has its own ScheduledEventQueue which isn't time based.
      // - IntendedReboot is just used to mark the intended reboot reason in RTC.
      break;
  }
  STOP_TIMER(HANDLE_SCHEDULER_TASK);
}

String ESPEasy_Scheduler::getQueueStats() {
  return msecTimerHandler.getQueueStats();
}

void ESPEasy_Scheduler::updateIdleTimeStats() {
  msecTimerHandler.updateIdleTimeStats();
}

float ESPEasy_Scheduler::getIdleTimePct() const {
  return msecTimerHandler.getIdleTimePct();
}

void ESPEasy_Scheduler::setEcoMode(bool enabled) {
  msecTimerHandler.setEcoMode(enabled);
}
