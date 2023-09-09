#include "../Helpers/Scheduler.h"


#include "../../ESPEasy-Globals.h"

#include "../ControllerQueue/DelayQueueElements.h"

#include "../DataStructs/Scheduler_ConstIntervalTimerID.h"

#include "../Globals/Settings.h"

#include "../Helpers/ESPEasy_time_calc.h"
#include "../Helpers/Networking.h"
#include "../Helpers/PeriodicalActions.h"


/*********************************************************************************************\
* Interval Timer
* These timers set a new scheduled timer, based on the old value.
* This will make their interval as constant as possible.
\*********************************************************************************************/

// Interval where it is more important to actually run the scheduled job, instead of keeping the time drift to a minimum.
// For example running the PLUGIN_FIFTY_PER_SECOND calls probably need to run as fast as possible as they need to fetch data before a buffer
// overflow happens.
// For those it is more important to actually run it than keeping pace.
void ESPEasy_Scheduler::setNextTimeInterval(unsigned long& timer, const unsigned long step) {
  timer += step;
  const long passed = timePassedSince(timer);

  if (passed < 0) {
    // Event has not yet happened, which is fine.
    return;
  }

  if (static_cast<unsigned long>(passed) > step) {
    // No need to keep running behind, start again.
    timer = millis() + step;
    return;
  }

  // Try to get in sync again.
  timer = millis() + (step - passed);
}

// More strict interval where no time drift is more important than missing a scheduled interval.
// For example timing for repeating longPulse where 2 scheduled intervals need to be at constant 'distance' from each other.
void ESPEasy_Scheduler::setNextStrictTimeInterval(unsigned long     & timer,
                                                  const unsigned long step) {
  timer += step;
  const long passed = timePassedSince(timer);

  if (passed <= 0) {
    // Event has not yet happened, which is fine.
    return;
  }

  // Try to get in sync again.
  const unsigned long stepsMissed = static_cast<unsigned long>(passed) / step;

  timer += (stepsMissed + 1) * step;
}

void ESPEasy_Scheduler::setIntervalTimer(SchedulerIntervalTimer_e intervalTimer) {
  setIntervalTimer(intervalTimer, millis());
}

void ESPEasy_Scheduler::setIntervalTimerAt(SchedulerIntervalTimer_e intervalTimer, unsigned long newtimer) {
  const ConstIntervalTimerID timerID(intervalTimer);

  setNewTimerAt(timerID, newtimer);
}

void ESPEasy_Scheduler::setIntervalTimerOverride(SchedulerIntervalTimer_e intervalTimer, unsigned long msecFromNow) {
  unsigned long timer = millis();

  setNextTimeInterval(timer, msecFromNow);
  const ConstIntervalTimerID timerID(intervalTimer);

  setNewTimerAt(timerID, timer);
}

void ESPEasy_Scheduler::scheduleNextDelayQueue(SchedulerIntervalTimer_e intervalTimer, unsigned long nextTime) {
  if (nextTime != 0) {
    // Schedule for next process run.
    setIntervalTimerAt(intervalTimer, nextTime);
  }
}

void ESPEasy_Scheduler::setIntervalTimer(SchedulerIntervalTimer_e intervalTimer, unsigned long lasttimer) {
  // Set the initial timers for the regular runs
  unsigned long interval = 0;

  switch (intervalTimer) {
    case SchedulerIntervalTimer_e::TIMER_20MSEC:         interval = 20; break;
    case SchedulerIntervalTimer_e::TIMER_100MSEC:        interval = 100; break;
    case SchedulerIntervalTimer_e::TIMER_1SEC:           interval = 1000; break;
    case SchedulerIntervalTimer_e::TIMER_30SEC:
    case SchedulerIntervalTimer_e::TIMER_STATISTICS:     interval = 30000; break;
    case SchedulerIntervalTimer_e::TIMER_MQTT:           interval = timermqtt_interval; break;
    case SchedulerIntervalTimer_e::TIMER_GRATUITOUS_ARP: interval = timer_gratuitous_arp_interval; break;

    // Fall-through for all DelayQueue, which are just the fall-back timers.
    // The timers for all delay queues will be set according to their own settings as long as there is something to process.
    case SchedulerIntervalTimer_e::TIMER_MQTT_DELAY_QUEUE:
    case SchedulerIntervalTimer_e::TIMER_C001_DELAY_QUEUE:
    case SchedulerIntervalTimer_e::TIMER_C002_DELAY_QUEUE:
    case SchedulerIntervalTimer_e::TIMER_C003_DELAY_QUEUE:
    case SchedulerIntervalTimer_e::TIMER_C004_DELAY_QUEUE:
    case SchedulerIntervalTimer_e::TIMER_C005_DELAY_QUEUE:
    case SchedulerIntervalTimer_e::TIMER_C006_DELAY_QUEUE:
    case SchedulerIntervalTimer_e::TIMER_C007_DELAY_QUEUE:
    case SchedulerIntervalTimer_e::TIMER_C008_DELAY_QUEUE:
    case SchedulerIntervalTimer_e::TIMER_C009_DELAY_QUEUE:
    case SchedulerIntervalTimer_e::TIMER_C010_DELAY_QUEUE:
    case SchedulerIntervalTimer_e::TIMER_C011_DELAY_QUEUE:
    case SchedulerIntervalTimer_e::TIMER_C012_DELAY_QUEUE:
    case SchedulerIntervalTimer_e::TIMER_C013_DELAY_QUEUE:
    case SchedulerIntervalTimer_e::TIMER_C014_DELAY_QUEUE:
    case SchedulerIntervalTimer_e::TIMER_C015_DELAY_QUEUE:
    case SchedulerIntervalTimer_e::TIMER_C016_DELAY_QUEUE:
    case SchedulerIntervalTimer_e::TIMER_C017_DELAY_QUEUE:
    case SchedulerIntervalTimer_e::TIMER_C018_DELAY_QUEUE:
    case SchedulerIntervalTimer_e::TIMER_C019_DELAY_QUEUE:
    case SchedulerIntervalTimer_e::TIMER_C020_DELAY_QUEUE:
    case SchedulerIntervalTimer_e::TIMER_C021_DELAY_QUEUE:
    case SchedulerIntervalTimer_e::TIMER_C022_DELAY_QUEUE:
    case SchedulerIntervalTimer_e::TIMER_C023_DELAY_QUEUE:
    case SchedulerIntervalTimer_e::TIMER_C024_DELAY_QUEUE:
    case SchedulerIntervalTimer_e::TIMER_C025_DELAY_QUEUE:
      // When extending this, search for EXTEND_CONTROLLER_IDS
      // in the code to find all places that need to be updated too.
      interval = 1000; break;
  }
  unsigned long timer = lasttimer;

  setNextTimeInterval(timer, interval);
  const ConstIntervalTimerID timerID(intervalTimer);

  setNewTimerAt(timerID, timer);
}

void ESPEasy_Scheduler::sendGratuitousARP_now() {
  sendGratuitousARP();

  if (Settings.gratuitousARP()) {
    timer_gratuitous_arp_interval = 100;
    setIntervalTimer(SchedulerIntervalTimer_e::TIMER_GRATUITOUS_ARP);
  }
}

void ESPEasy_Scheduler::process_interval_timer(SchedulerTimerID timerID, unsigned long lasttimer) {
  // Set the interval timer now, it may be altered by the commands below.
  // This is the default next-run-time.

  const ConstIntervalTimerID *tmp              = reinterpret_cast<const ConstIntervalTimerID *>(&timerID);
  const SchedulerIntervalTimer_e intervalTimer = tmp->getIntervalTimer();

  setIntervalTimer(intervalTimer, lasttimer);

  switch (intervalTimer) {
    case SchedulerIntervalTimer_e::TIMER_20MSEC:         run50TimesPerSecond(); break;
    case SchedulerIntervalTimer_e::TIMER_100MSEC:

      if (!UseRTOSMultitasking) {
        run10TimesPerSecond();
      }
      break;
    case SchedulerIntervalTimer_e::TIMER_1SEC:             runOncePerSecond();      break;
    case SchedulerIntervalTimer_e::TIMER_30SEC:            runEach30Seconds();      break;
    case SchedulerIntervalTimer_e::TIMER_MQTT:
#if FEATURE_MQTT
      runPeriodicalMQTT();
#endif // if FEATURE_MQTT
      break;
    case SchedulerIntervalTimer_e::TIMER_STATISTICS:       logTimerStatistics();    break;
    case SchedulerIntervalTimer_e::TIMER_GRATUITOUS_ARP:

      // Slowly increase the interval timer.
      timer_gratuitous_arp_interval = 2 * timer_gratuitous_arp_interval;

      if (timer_gratuitous_arp_interval > TIMER_GRATUITOUS_ARP_MAX) {
        timer_gratuitous_arp_interval = TIMER_GRATUITOUS_ARP_MAX;
      }

      if (Settings.gratuitousARP()) {
        sendGratuitousARP();
      }
      break;
    case SchedulerIntervalTimer_e::TIMER_MQTT_DELAY_QUEUE:
    case SchedulerIntervalTimer_e::TIMER_C002_DELAY_QUEUE:
    case SchedulerIntervalTimer_e::TIMER_C005_DELAY_QUEUE:
    case SchedulerIntervalTimer_e::TIMER_C006_DELAY_QUEUE:
#if FEATURE_MQTT
      processMQTTdelayQueue();
#endif // if FEATURE_MQTT
      break;
    case SchedulerIntervalTimer_e::TIMER_C001_DELAY_QUEUE:
  #ifdef USES_C001
      process_c001_delay_queue();
  #endif // ifdef USES_C001
      break;
    case SchedulerIntervalTimer_e::TIMER_C003_DELAY_QUEUE:
  #ifdef USES_C003
      process_c003_delay_queue();
  #endif // ifdef USES_C003
      break;
    case SchedulerIntervalTimer_e::TIMER_C004_DELAY_QUEUE:
  #ifdef USES_C004
      process_c004_delay_queue();
  #endif // ifdef USES_C004
      break;
    case SchedulerIntervalTimer_e::TIMER_C007_DELAY_QUEUE:
  #ifdef USES_C007
      process_c007_delay_queue();
  #endif // ifdef USES_C007
      break;
    case SchedulerIntervalTimer_e::TIMER_C008_DELAY_QUEUE:
  #ifdef USES_C008
      process_c008_delay_queue();
  #endif // ifdef USES_C008
      break;
    case SchedulerIntervalTimer_e::TIMER_C009_DELAY_QUEUE:
  #ifdef USES_C009
      process_c009_delay_queue();
  #endif // ifdef USES_C009
      break;
    case SchedulerIntervalTimer_e::TIMER_C010_DELAY_QUEUE:
  #ifdef USES_C010
      process_c010_delay_queue();
  #endif // ifdef USES_C010
      break;
    case SchedulerIntervalTimer_e::TIMER_C011_DELAY_QUEUE:
  #ifdef USES_C011
      process_c011_delay_queue();
  #endif // ifdef USES_C011
      break;
    case SchedulerIntervalTimer_e::TIMER_C012_DELAY_QUEUE:
  #ifdef USES_C012
      process_c012_delay_queue();
  #endif // ifdef USES_C012
      break;

    case SchedulerIntervalTimer_e::TIMER_C013_DELAY_QUEUE:
      /*
       #ifdef USES_C013
            process_c013_delay_queue();
       #endif
       */
      break;

    case SchedulerIntervalTimer_e::TIMER_C014_DELAY_QUEUE:
      /*
       #ifdef USES_C014
            process_c014_delay_queue();
       #endif
       */
      break;

    case SchedulerIntervalTimer_e::TIMER_C015_DELAY_QUEUE:
  #ifdef USES_C015
      process_c015_delay_queue();
  #endif // ifdef USES_C015
      break;
    case SchedulerIntervalTimer_e::TIMER_C016_DELAY_QUEUE:
  #ifdef USES_C016
      process_c016_delay_queue();
  #endif // ifdef USES_C016
      break;

    case SchedulerIntervalTimer_e::TIMER_C017_DELAY_QUEUE:
  #ifdef USES_C017
      process_c017_delay_queue();
  #endif // ifdef USES_C017
      break;

    case SchedulerIntervalTimer_e::TIMER_C018_DELAY_QUEUE:
  #ifdef USES_C018
      process_c018_delay_queue();
  #endif // ifdef USES_C018
      break;

    case SchedulerIntervalTimer_e::TIMER_C019_DELAY_QUEUE:
      /*
       #ifdef USES_C019
            process_c019_delay_queue();
       #endif
       */
      break;

    case SchedulerIntervalTimer_e::TIMER_C020_DELAY_QUEUE:
      /*
       #ifdef USES_C020
            process_c020_delay_queue();
       #endif
       */
      break;

    case SchedulerIntervalTimer_e::TIMER_C021_DELAY_QUEUE:
      /*
       #ifdef USES_C021
            process_c021_delay_queue();
       #endif
       */
      break;

    case SchedulerIntervalTimer_e::TIMER_C022_DELAY_QUEUE:
      /*
       #ifdef USES_C022
            process_c022_delay_queue();
       #endif
       */
      break;

    case SchedulerIntervalTimer_e::TIMER_C023_DELAY_QUEUE:
      /*
       #ifdef USES_C023
            process_c023_delay_queue();
       #endif
       */
      break;

    case SchedulerIntervalTimer_e::TIMER_C024_DELAY_QUEUE:
      /*
       #ifdef USES_C024
            process_c024_delay_queue();
       #endif
       */
      break;

    case SchedulerIntervalTimer_e::TIMER_C025_DELAY_QUEUE:
      /*
       #ifdef USES_C025
            process_c025_delay_queue();
       #endif
       */
      break;

      // When extending this, search for EXTEND_CONTROLLER_IDS
      // in the code to find all places that need to be updated too.
  }
}
