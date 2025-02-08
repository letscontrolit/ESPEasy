#include "../Helpers/Scheduler.h"


#include "../DataStructs/Scheduler_RulesTimerID.h"

#include "../ESPEasyCore/ESPEasyRules.h"

#include "../Globals/Settings.h"

#include "../Helpers/ESPEasy_time_calc.h"


/*********************************************************************************************\
* Rules Timer
\*********************************************************************************************/
static bool checkRulesTimerIndex(unsigned int timerIndex) {
  if ((timerIndex > RULES_TIMER_MAX) || (timerIndex == 0)) {
    if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
      String log = F("TIMER: invalid timer number ");
      log += timerIndex;
      addLogMove(LOG_LEVEL_ERROR, log);
    }
    return false;
  }
  return true;
}

bool ESPEasy_Scheduler::setRulesTimer(unsigned long msecFromNow, unsigned int timerIndex, int recurringCount, bool startImmediately) {
  if (!checkRulesTimerIndex(timerIndex)) { return false; }

  const RulesTimerID timerID(timerIndex);

  const systemTimerStruct timer_data(recurringCount, msecFromNow, timerIndex);

  systemTimers[timerID.mixed_id] = timer_data;
  setNewTimerAt(timerID, millis() + (startImmediately ? 10 : msecFromNow));
  return true;
}

void ESPEasy_Scheduler::process_rules_timer(SchedulerTimerID id, unsigned long lasttimer) {
  auto it = systemTimers.find(id.mixed_id);

  if (it == systemTimers.end()) { return; }

  if (it->second.isPaused()) {
    // Timer is paused.
    // Must keep this timer 'active' in the scheduler.
    // However it does not need to be looked at frequently as the resume function re-schedules it when needed.
    // Look for its state every second.
    setNewTimerAt(id, millis() + 1000);
    return;
  }

  // Create a deep copy of the timer data as we may delete it from the map before sending the event.
  const int loopCount  = it->second.getLoopCount();
  const int timerIndex = it->second.getTimerIndex();

  // Reschedule before sending the event, as it may get rescheduled in handling the timer event.
  if (it->second.isRecurring()) {
    // Recurring timer
    unsigned long newTimer = lasttimer;
    setNextTimeInterval(newTimer, it->second.getInterval());
    setNewTimerAt(id, newTimer);
    it->second.markNextRecurring();
  } else {
    systemTimers.erase(id.mixed_id);
  }

  if (loopCount > 0) {
    // Should be executed
    if (Settings.UseRules) {
      String event = F("Rules#Timer=");
      event += timerIndex;

      // Add count as 2nd eventvalue
      event += ',';
      event += loopCount;
      rulesProcessing(event); // TD-er: Do not add to the eventQueue, but execute right now.
    }
  }
}

bool ESPEasy_Scheduler::pause_rules_timer(unsigned long timerIndex) {
  if (!checkRulesTimerIndex(timerIndex)) { return false; }
  const RulesTimerID timerID(timerIndex);
  auto it = systemTimers.find(timerID.mixed_id);

  if (it == systemTimers.end()) {
    addLog(LOG_LEVEL_INFO, F("TIMER: no timer set"));
    return false;
  }

  unsigned long timer;

  if (msecTimerHandler.getTimerForId(timerID.mixed_id, timer)) {
    if (it->second.isPaused()) {
      addLog(LOG_LEVEL_INFO, F("TIMER: already paused"));
    } else {
      // Store remainder of interval
      const long timeLeft = timePassedSince(timer) * -1;

      if (timeLeft > 0) {
        it->second.setRemainder(timeLeft);
        return true;
      }
    }
  }
  return false;
}

bool ESPEasy_Scheduler::resume_rules_timer(unsigned long timerIndex) {
  if (!checkRulesTimerIndex(timerIndex)) { return false; }
  const RulesTimerID timerID(timerIndex);
  auto it = systemTimers.find(timerID.mixed_id);

  if (it == systemTimers.end()) { return false; }

  unsigned long timer;

  if (msecTimerHandler.getTimerForId(timerID.mixed_id, timer)) {
    if (it->second.isPaused()) {
      // Reschedule timer with remainder of interval
      setNewTimerAt(timerID, millis() + it->second.getRemainder());
      it->second.setRemainder(0);
      return true;
    }
  }
  return false;
}
