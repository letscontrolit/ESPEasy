#include "../Helpers/Scheduler.h"


#include "../../ESPEasy-Globals.h"

#include "../../_Plugin_Helper.h"

#include "../ControllerQueue/DelayQueueElements.h"
#include "../ESPEasyCore/ESPEasyGPIO.h"
#include "../ESPEasyCore/ESPEasyRules.h"
#include "../Globals/GlobalMapPortStatus.h"
#include "../Globals/RTC.h"
#include "../Globals/NPlugins.h"
#include "../Helpers/DeepSleep.h"
#include "../Helpers/ESPEasyRTC.h"
#include "../Helpers/Networking.h"
#include "../Helpers/PeriodicalActions.h"
#include "../Helpers/PortStatus.h"


#define TIMER_ID_SHIFT       28 // Must be decreased as soon as timers below reach 15


#ifndef BUILD_NO_DEBUG
const __FlashStringHelper * toString_f(ESPEasy_Scheduler::IntervalTimer_e timer) {
  switch (timer) {
    case ESPEasy_Scheduler::IntervalTimer_e::TIMER_20MSEC:           return F("TIMER_20MSEC");
    case ESPEasy_Scheduler::IntervalTimer_e::TIMER_100MSEC:          return F("TIMER_100MSEC");
    case ESPEasy_Scheduler::IntervalTimer_e::TIMER_1SEC:             return F("TIMER_1SEC");
    case ESPEasy_Scheduler::IntervalTimer_e::TIMER_30SEC:            return F("TIMER_30SEC");
    case ESPEasy_Scheduler::IntervalTimer_e::TIMER_MQTT:             return F("TIMER_MQTT");
    case ESPEasy_Scheduler::IntervalTimer_e::TIMER_STATISTICS:       return F("TIMER_STATISTICS");
    case ESPEasy_Scheduler::IntervalTimer_e::TIMER_GRATUITOUS_ARP:   return F("TIMER_GRATUITOUS_ARP");
    case ESPEasy_Scheduler::IntervalTimer_e::TIMER_MQTT_DELAY_QUEUE: return F("TIMER_MQTT_DELAY_QUEUE");
    default: 
      break;
  }
  return F("unknown");
}
#endif

String ESPEasy_Scheduler::toString(ESPEasy_Scheduler::IntervalTimer_e timer) {
#ifdef BUILD_NO_DEBUG
  return String(static_cast<int>(timer));
#else // ifdef BUILD_NO_DEBUG

  if (timer >= IntervalTimer_e::TIMER_C001_DELAY_QUEUE && 
      timer <= IntervalTimer_e::TIMER_C025_DELAY_QUEUE) 
  {
    String res;
    res.reserve(24);
    res = F("TIMER_C0");
    const int id = static_cast<int>(timer) - static_cast<int>(IntervalTimer_e::TIMER_C001_DELAY_QUEUE) + 1;

    if (id < 10) { res += '0'; }
    res += id;
    res += F("_DELAY_QUEUE");
    return res;
  }
  return toString_f(timer);
#endif // ifdef BUILD_NO_DEBUG
}

const __FlashStringHelper * ESPEasy_Scheduler::toString(ESPEasy_Scheduler::SchedulerTimerType_e timerType) {
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
  return F("unknown");
}

const __FlashStringHelper * ESPEasy_Scheduler::toString(ESPEasy_Scheduler::PluginPtrType pluginType) {
  switch (pluginType) {
    case PluginPtrType::TaskPlugin:         return F("Plugin");
    case PluginPtrType::ControllerPlugin:   return F("Controller");
#if FEATURE_NOTIFIER
    case PluginPtrType::NotificationPlugin: return F("Notification");
#endif
  }
  return F("unknown");
}

const __FlashStringHelper * toString_f(ESPEasy_Scheduler::IntendedRebootReason_e reason) {
  switch (reason) {
    case ESPEasy_Scheduler::IntendedRebootReason_e::DeepSleep:              return F("DeepSleep");
    case ESPEasy_Scheduler::IntendedRebootReason_e::DelayedReboot:          return F("DelayedReboot");
    case ESPEasy_Scheduler::IntendedRebootReason_e::ResetFactory:           return F("ResetFactory");
    case ESPEasy_Scheduler::IntendedRebootReason_e::ResetFactoryPinActive:  return F("ResetFactoryPinActive");
    case ESPEasy_Scheduler::IntendedRebootReason_e::ResetFactoryCommand:    return F("ResetFactoryCommand");
    case ESPEasy_Scheduler::IntendedRebootReason_e::CommandReboot:          return F("CommandReboot");
    case ESPEasy_Scheduler::IntendedRebootReason_e::RestoreSettings:        return F("RestoreSettings");
    case ESPEasy_Scheduler::IntendedRebootReason_e::OTA_error:              return F("OTA_error");
    case ESPEasy_Scheduler::IntendedRebootReason_e::ConnectionFailuresThreshold: return F("ConnectionFailuresThreshold");
  }
  return F("");
}

String ESPEasy_Scheduler::toString(ESPEasy_Scheduler::IntendedRebootReason_e reason) {
  String res = toString_f(reason);
  if (res.isEmpty()) return String(static_cast<int>(reason));
  return res;
}

void ESPEasy_Scheduler::markIntendedReboot(ESPEasy_Scheduler::IntendedRebootReason_e reason) {
  const unsigned long mixed_id = getMixedId(SchedulerTimerType_e::IntendedReboot, static_cast<unsigned long>(reason));

  RTC.lastMixedSchedulerId = mixed_id;
  saveToRTC();
}

/*********************************************************************************************\
* Generic Timer functions.
\*********************************************************************************************/
void ESPEasy_Scheduler::setNewTimerAt(unsigned long id, unsigned long timer) {
  START_TIMER;
  msecTimerHandler.registerAt(id, timer);
  STOP_TIMER(SET_NEW_TIMER);
}

// Mix timer type int with an ID describing the scheduled job.
unsigned long ESPEasy_Scheduler::getMixedId(SchedulerTimerType_e timerType, unsigned long id) {
  return (static_cast<unsigned long>(timerType) << TIMER_ID_SHIFT) + id;
}

unsigned long ESPEasy_Scheduler::decodeSchedulerId(unsigned long mixed_id, SchedulerTimerType_e& timerType) {
  timerType = static_cast<SchedulerTimerType_e>(mixed_id >> TIMER_ID_SHIFT);
  const unsigned long mask = (1 << TIMER_ID_SHIFT) - 1;

  return mixed_id & mask;
}

String ESPEasy_Scheduler::decodeSchedulerId(unsigned long mixed_id) {
  if (mixed_id == 0) {
    return F("Background Task");
  }
  SchedulerTimerType_e timerType = SchedulerTimerType_e::SystemEventQueue;
  const unsigned long  id        = decodeSchedulerId(mixed_id, timerType);
  String idStr                   = String(id);
  String result                  = toString(timerType);

  result += F(": ");

#ifndef BUILD_NO_DEBUG
  result.reserve(64);

  switch (timerType) {
    case SchedulerTimerType_e::SystemEventQueue:
    {
      const PluginPtrType ptr_type = static_cast<PluginPtrType>((id >> 16) & 0xFF);
      const uint8_t index          = (id >> 8) & 0xFF;
      const uint8_t function       = id & 0xFF;
      result += toString(ptr_type);
      result += ',';
      result += (index + 1); // TaskIndex / ControllerIndex / NotificationIndex
      result += ',';
      result += function;

      return result;
    }
    case SchedulerTimerType_e::ConstIntervalTimer:
    {
      result +=  toString(static_cast<ESPEasy_Scheduler::IntervalTimer_e>(id));
      return result;
    }
    case SchedulerTimerType_e::PLUGIN_TASKTIMER_IN_e:
    {
      const taskIndex_t taskIndex = ((1 << 8) - 1) & id;

      if (validTaskIndex(taskIndex)) {
        idStr = getTaskDeviceName(taskIndex);
      }
      result += idStr;
      return result;
    }
    case SchedulerTimerType_e::PLUGIN_DEVICETIMER_IN_e:
    {
      const deviceIndex_t deviceIndex = ((1 << 8) - 1) & id;

      if (validDeviceIndex(deviceIndex)) {
        idStr = getPluginNameFromDeviceIndex(deviceIndex);
      }
      result += idStr;
      return result;
    }
    case SchedulerTimerType_e::TaskDeviceTimer:
    {
      result = F("Task ");

      // Id is taskIndex
      result += (id + 1);
      return result;
    }
    case SchedulerTimerType_e::GPIO_timer:
    {
      uint8_t GPIOType      = static_cast<uint8_t>((id) & 0xFF);
      uint8_t pinNumber     = static_cast<uint8_t>((id >> 8) & 0xFF);
      uint8_t pinStateValue = static_cast<uint8_t>((id >> 16) & 0xFF);

      switch (GPIOType)
      {
        case GPIO_TYPE_INTERNAL:
          result += F("int");
          break;
        case GPIO_TYPE_MCP:
          result += F("MCP");
          break;
        case GPIO_TYPE_PCF:
          result += F("PCF");
          break;
        default:
          result += F("?");
          break;
      }
      result += F(" pin: ");
      result += pinNumber;
      result += F(" state: ");
      result += pinStateValue;
      return result;
    }
    case SchedulerTimerType_e::RulesTimer:
    {
      result = F("Rules#Timer=");
      const unsigned long mask    = (1 << TIMER_ID_SHIFT) - 1;
      const unsigned long timerID = id & mask;
      result += timerID;
      return result;
    }
    case SchedulerTimerType_e::IntendedReboot:
    {
      result += toString(static_cast<ESPEasy_Scheduler::IntendedRebootReason_e>(id));
      return result;
    }
  }
#endif // ifndef BUILD_NO_DEBUG
  result += F(" timer, id: ");
  result += idStr;
  return result;
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

  SchedulerTimerType_e timerType = SchedulerTimerType_e::SystemEventQueue;
  const unsigned long  id        = decodeSchedulerId(mixed_id, timerType);

  delay(0); // See: https://github.com/letscontrolit/ESPEasy/issues/1818#issuecomment-425351328

  switch (timerType) {
    case SchedulerTimerType_e::ConstIntervalTimer:
      process_interval_timer(static_cast<ESPEasy_Scheduler::IntervalTimer_e>(id), timer);
      break;
    case SchedulerTimerType_e::PLUGIN_TASKTIMER_IN_e:
      process_plugin_task_timer(id);
      break;
    case SchedulerTimerType_e::PLUGIN_DEVICETIMER_IN_e:
      process_plugin_timer(id);
      break;
    case SchedulerTimerType_e::RulesTimer:
      process_rules_timer(id, timer);
      break;
    case SchedulerTimerType_e::TaskDeviceTimer:
      process_task_device_timer(id, timer);
      break;
    case SchedulerTimerType_e::GPIO_timer:
      process_gpio_timer(id, timer);
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

/*********************************************************************************************\
* Interval Timer
* These timers set a new scheduled timer, based on the old value.
* This will make their interval as constant as possible.
\*********************************************************************************************/

// Interval where it is more important to actually run the scheduled job, instead of keeping the time drift to a minimum.
// For example running the PLUGIN_FIFTY_PER_SECOND calls probably need to run as fast as possible as they need to fetch data before a buffer overflow happens.
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

void ESPEasy_Scheduler::setIntervalTimer(IntervalTimer_e id) {
  setIntervalTimer(id, millis());
}

void ESPEasy_Scheduler::setIntervalTimerAt(IntervalTimer_e id, unsigned long newtimer) {
  setNewTimerAt(getMixedId(SchedulerTimerType_e::ConstIntervalTimer, static_cast<unsigned long>(id)), newtimer);
}

void ESPEasy_Scheduler::setIntervalTimerOverride(IntervalTimer_e id, unsigned long msecFromNow) {
  unsigned long timer = millis();

  setNextTimeInterval(timer, msecFromNow);
  setNewTimerAt(getMixedId(SchedulerTimerType_e::ConstIntervalTimer, static_cast<unsigned long>(id)), timer);
}

void ESPEasy_Scheduler::scheduleNextDelayQueue(IntervalTimer_e id, unsigned long nextTime) {
  if (nextTime != 0) {
    // Schedule for next process run.
    setIntervalTimerAt(id, nextTime);
  }
}

void ESPEasy_Scheduler::setIntervalTimer(IntervalTimer_e id, unsigned long lasttimer) {
  // Set the initial timers for the regular runs
  unsigned long interval = 0;

  switch (id) {
    case IntervalTimer_e::TIMER_20MSEC:         interval = 20; break;
    case IntervalTimer_e::TIMER_100MSEC:        interval = 100; break;
    case IntervalTimer_e::TIMER_1SEC:           interval = 1000; break;
    case IntervalTimer_e::TIMER_30SEC:
    case IntervalTimer_e::TIMER_STATISTICS:     interval = 30000; break;
    case IntervalTimer_e::TIMER_MQTT:           interval = timermqtt_interval; break;
    case IntervalTimer_e::TIMER_GRATUITOUS_ARP: interval = timer_gratuitous_arp_interval; break;

    // Fall-through for all DelayQueue, which are just the fall-back timers.
    // The timers for all delay queues will be set according to their own settings as long as there is something to process.
    case IntervalTimer_e::TIMER_MQTT_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C001_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C003_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C004_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C007_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C008_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C009_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C010_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C011_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C012_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C013_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C014_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C015_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C016_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C017_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C018_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C019_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C020_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C021_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C022_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C023_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C024_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C025_DELAY_QUEUE:
      // When extending this, search for EXTEND_CONTROLLER_IDS
      // in the code to find all places that need to be updated too.
      interval = 1000; break;
  }
  unsigned long timer = lasttimer;

  setNextTimeInterval(timer, interval);
  setNewTimerAt(getMixedId(SchedulerTimerType_e::ConstIntervalTimer, static_cast<unsigned long>(id)), timer);
}

void ESPEasy_Scheduler::sendGratuitousARP_now() {
  sendGratuitousARP();

  if (Settings.gratuitousARP()) {
    timer_gratuitous_arp_interval = 100;
    setIntervalTimer(ESPEasy_Scheduler::IntervalTimer_e::TIMER_GRATUITOUS_ARP);
  }
}

void ESPEasy_Scheduler::process_interval_timer(IntervalTimer_e id, unsigned long lasttimer) {
  // Set the interval timer now, it may be altered by the commands below.
  // This is the default next-run-time.
  setIntervalTimer(id, lasttimer);

  switch (id) {
    case IntervalTimer_e::TIMER_20MSEC:         run50TimesPerSecond(); break;
    case IntervalTimer_e::TIMER_100MSEC:

      if (!UseRTOSMultitasking) {
        run10TimesPerSecond();
      }
      break;
    case IntervalTimer_e::TIMER_1SEC:             runOncePerSecond();      break;
    case IntervalTimer_e::TIMER_30SEC:            runEach30Seconds();      break;
    case IntervalTimer_e::TIMER_MQTT:
#if FEATURE_MQTT
      runPeriodicalMQTT();
#endif // if FEATURE_MQTT
      break;
    case IntervalTimer_e::TIMER_STATISTICS:       logTimerStatistics();    break;
    case IntervalTimer_e::TIMER_GRATUITOUS_ARP:

      // Slowly increase the interval timer.
      timer_gratuitous_arp_interval = 2 * timer_gratuitous_arp_interval;

      if (timer_gratuitous_arp_interval > TIMER_GRATUITOUS_ARP_MAX) {
        timer_gratuitous_arp_interval = TIMER_GRATUITOUS_ARP_MAX;
      }

      if (Settings.gratuitousARP()) {
        sendGratuitousARP();
      }
      break;
    case IntervalTimer_e::TIMER_MQTT_DELAY_QUEUE:
#if FEATURE_MQTT
      processMQTTdelayQueue();
#endif // if FEATURE_MQTT
      break;
    case IntervalTimer_e::TIMER_C001_DELAY_QUEUE:
  #ifdef USES_C001
      process_c001_delay_queue();
  #endif // ifdef USES_C001
      break;
    case IntervalTimer_e::TIMER_C003_DELAY_QUEUE:
  #ifdef USES_C003
      process_c003_delay_queue();
  #endif // ifdef USES_C003
      break;
    case IntervalTimer_e::TIMER_C004_DELAY_QUEUE:
  #ifdef USES_C004
      process_c004_delay_queue();
  #endif // ifdef USES_C004
      break;
    case IntervalTimer_e::TIMER_C007_DELAY_QUEUE:
  #ifdef USES_C007
      process_c007_delay_queue();
  #endif // ifdef USES_C007
      break;
    case IntervalTimer_e::TIMER_C008_DELAY_QUEUE:
  #ifdef USES_C008
      process_c008_delay_queue();
  #endif // ifdef USES_C008
      break;
    case IntervalTimer_e::TIMER_C009_DELAY_QUEUE:
  #ifdef USES_C009
      process_c009_delay_queue();
  #endif // ifdef USES_C009
      break;
    case IntervalTimer_e::TIMER_C010_DELAY_QUEUE:
  #ifdef USES_C010
      process_c010_delay_queue();
  #endif // ifdef USES_C010
      break;
    case IntervalTimer_e::TIMER_C011_DELAY_QUEUE:
  #ifdef USES_C011
      process_c011_delay_queue();
  #endif // ifdef USES_C011
      break;
    case IntervalTimer_e::TIMER_C012_DELAY_QUEUE:
  #ifdef USES_C012
      process_c012_delay_queue();
  #endif // ifdef USES_C012
      break;

    case IntervalTimer_e::TIMER_C013_DELAY_QUEUE:
      /*
       #ifdef USES_C013
            process_c013_delay_queue();
       #endif
       */
      break;

    case IntervalTimer_e::TIMER_C014_DELAY_QUEUE:
      /*
       #ifdef USES_C014
            process_c014_delay_queue();
       #endif
       */
      break;

    case IntervalTimer_e::TIMER_C015_DELAY_QUEUE:
  #ifdef USES_C015
      process_c015_delay_queue();
  #endif // ifdef USES_C015
      break;
    case IntervalTimer_e::TIMER_C016_DELAY_QUEUE:
  #ifdef USES_C016
      process_c016_delay_queue();
  #endif // ifdef USES_C016
      break;

    case IntervalTimer_e::TIMER_C017_DELAY_QUEUE:
  #ifdef USES_C017
      process_c017_delay_queue();
  #endif // ifdef USES_C017
      break;

    case IntervalTimer_e::TIMER_C018_DELAY_QUEUE:
  #ifdef USES_C018
      process_c018_delay_queue();
  #endif // ifdef USES_C018
      break;

    case IntervalTimer_e::TIMER_C019_DELAY_QUEUE:
      /*
       #ifdef USES_C019
            process_c019_delay_queue();
       #endif
       */
      break;

    case IntervalTimer_e::TIMER_C020_DELAY_QUEUE:
      /*
       #ifdef USES_C020
            process_c020_delay_queue();
       #endif
       */
      break;

    case IntervalTimer_e::TIMER_C021_DELAY_QUEUE:
      /*
       #ifdef USES_C021
            process_c021_delay_queue();
       #endif
       */
      break;

    case IntervalTimer_e::TIMER_C022_DELAY_QUEUE:
      /*
       #ifdef USES_C022
            process_c022_delay_queue();
       #endif
       */
      break;

    case IntervalTimer_e::TIMER_C023_DELAY_QUEUE:
      /*
       #ifdef USES_C023
            process_c023_delay_queue();
       #endif
       */
      break;

    case IntervalTimer_e::TIMER_C024_DELAY_QUEUE:
      /*
       #ifdef USES_C024
            process_c024_delay_queue();
       #endif
       */
      break;

    case IntervalTimer_e::TIMER_C025_DELAY_QUEUE:
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

/*********************************************************************************************\
* Plugin Task Timer
\*********************************************************************************************/
unsigned long ESPEasy_Scheduler::createPluginTaskTimerId(taskIndex_t taskIndex, int Par1) {
  const unsigned long mask  = (1 << TIMER_ID_SHIFT) - 1;
  const unsigned long mixed = (Par1 << 8) + taskIndex;

  return mixed & mask;
}

void ESPEasy_Scheduler::setPluginTaskTimer(unsigned long msecFromNow, taskIndex_t taskIndex, int Par1, int Par2, int Par3, int Par4, int Par5)
{
  // taskIndex and par1 form a unique key that can be used to restart a timer
  if (!validTaskIndex(taskIndex)) return;
  if (!Settings.TaskDeviceEnabled[taskIndex]) return;
  const unsigned long mixedTimerId = getMixedId(
    SchedulerTimerType_e::PLUGIN_TASKTIMER_IN_e, 
    createPluginTaskTimerId(taskIndex, Par1));

  systemTimerStruct timer_data;

  timer_data.fromEvent(taskIndex, Par1, Par2, Par3, Par4, Par5);
  systemTimers[mixedTimerId] = timer_data;
  setNewTimerAt(mixedTimerId, millis() + msecFromNow);
}

void ESPEasy_Scheduler::process_plugin_task_timer(unsigned long id) {
  #ifdef USE_SECOND_HEAP
  HeapSelectDram ephemeral;
  #endif

  const unsigned long mixedTimerId = getMixedId(SchedulerTimerType_e::PLUGIN_TASKTIMER_IN_e, id);
  auto it                          = systemTimers.find(mixedTimerId);

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
  systemTimers.erase(mixedTimerId);

  {
    String dummy;
    PluginCall(PLUGIN_TASKTIMER_IN, &TempEvent, dummy);
  }
}

/*********************************************************************************************\
* Rules Timer
\*********************************************************************************************/
unsigned long ESPEasy_Scheduler::createRulesTimerId(unsigned int timerIndex) {
  const unsigned long mask  = (1 << TIMER_ID_SHIFT) - 1;
  const unsigned long mixed = timerIndex;

  return mixed & mask;
}

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

bool ESPEasy_Scheduler::setRulesTimer(unsigned long msecFromNow, unsigned int timerIndex, int recurringCount) {
  if (!checkRulesTimerIndex(timerIndex)) { return false; }

  const unsigned long mixedTimerId = getMixedId(SchedulerTimerType_e::RulesTimer, createRulesTimerId(timerIndex));
  const systemTimerStruct timer_data(recurringCount, msecFromNow, timerIndex);

  systemTimers[mixedTimerId] = timer_data;
  setNewTimerAt(mixedTimerId, millis() + msecFromNow);
  return true;
}

void ESPEasy_Scheduler::process_rules_timer(unsigned long id, unsigned long lasttimer) {
  const unsigned long mixedTimerId = getMixedId(SchedulerTimerType_e::RulesTimer, id);

  auto it = systemTimers.find(mixedTimerId);

  if (it == systemTimers.end()) { return; }

  if (it->second.isPaused()) {
    // Timer is paused.
    // Must keep this timer 'active' in the scheduler.
    // However it does not need to be looked at frequently as the resume function re-schedules it when needed.
    // Look for its state every second.
    setNewTimerAt(mixedTimerId, millis() + 1000);
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
    setNewTimerAt(mixedTimerId, newTimer);
    it->second.markNextRecurring();
  } else {
    systemTimers.erase(mixedTimerId);
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
  const unsigned long mixedTimerId = getMixedId(SchedulerTimerType_e::RulesTimer, createRulesTimerId(timerIndex));
  auto it                          = systemTimers.find(mixedTimerId);

  if (it == systemTimers.end()) {
    addLog(LOG_LEVEL_INFO, F("TIMER: no timer set"));
    return false;
  }

  unsigned long timer;

  if (msecTimerHandler.getTimerForId(mixedTimerId, timer)) {
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
  const unsigned long mixedTimerId = getMixedId(SchedulerTimerType_e::RulesTimer, createRulesTimerId(timerIndex));
  auto it                          = systemTimers.find(mixedTimerId);

  if (it == systemTimers.end()) { return false; }

  unsigned long timer;

  if (msecTimerHandler.getTimerForId(mixedTimerId, timer)) {
    if (it->second.isPaused()) {
      // Reschedule timer with remainder of interval
      setNewTimerAt(mixedTimerId, millis() + it->second.getRemainder());
      it->second.setRemainder(0);
      return true;
    }
  }
  return false;
}

/*********************************************************************************************\
* Plugin Timer
* Essentially calling PLUGIN_DEVICETIMER_IN
* Similar to PLUGIN_TASKTIMER_IN, addressed to a plugin instead of a task.
\*********************************************************************************************/
unsigned long ESPEasy_Scheduler::createPluginTimerId(deviceIndex_t deviceIndex, int Par1) {
  const unsigned long mask  = (1 << TIMER_ID_SHIFT) - 1;
  const unsigned long mixed = (Par1 << 8) + deviceIndex;

  return mixed & mask;
}

void ESPEasy_Scheduler::setPluginTimer(unsigned long msecFromNow, pluginID_t pluginID, int Par1, int Par2, int Par3, int Par4, int Par5)
{
  // plugin number and par1 form a unique key that can be used to restart a timer
  // Use deviceIndex instead of pluginID, since the deviceIndex uses less bits.
  const deviceIndex_t deviceIndex = getDeviceIndex(pluginID);

  if (!validDeviceIndex(deviceIndex)) { return; }

  const unsigned long mixedTimerId = getMixedId(SchedulerTimerType_e::PLUGIN_DEVICETIMER_IN_e, createPluginTimerId(deviceIndex, Par1));
  systemTimerStruct   timer_data;

  // PLUGIN_DEVICETIMER_IN does not address a task, so don't set TaskIndex
  timer_data.fromEvent(INVALID_TASK_INDEX, Par1, Par2, Par3, Par4, Par5);
  systemTimers[mixedTimerId] = timer_data;
  setNewTimerAt(mixedTimerId, millis() + msecFromNow);
}

void ESPEasy_Scheduler::process_plugin_timer(unsigned long id) {
  #ifdef USE_SECOND_HEAP
  HeapSelectDram ephemeral;
  #endif

  START_TIMER;
  const unsigned long mixedTimerId = getMixedId(SchedulerTimerType_e::PLUGIN_DEVICETIMER_IN_e, id);
  auto it                          = systemTimers.find(mixedTimerId);

  if (it == systemTimers.end()) { return; }

  struct EventStruct TempEvent(it->second.toEvent());

  // PLUGIN_DEVICETIMER_IN does not address a task, so don't set TaskIndex

  // extract deviceID from timer id:
  const deviceIndex_t deviceIndex = ((1 << 8) - 1) & id;

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
  systemTimers.erase(mixedTimerId);

  if (validDeviceIndex(deviceIndex)) {
    String dummy;
    Plugin_ptr[deviceIndex](PLUGIN_DEVICETIMER_IN, &TempEvent, dummy);
  }
  STOP_TIMER(PLUGIN_CALL_DEVICETIMER_IN);
}

/*********************************************************************************************\
* GPIO Timer
* Special timer to handle timed GPIO actions
\*********************************************************************************************/
unsigned long ESPEasy_Scheduler::createGPIOTimerId(uint8_t GPIOType, uint8_t pinNumber, int Par1) {
  const unsigned long mask = (1 << TIMER_ID_SHIFT) - 1;

  //  const unsigned long mixed = (Par1 << 8) + pinNumber;
  const unsigned long mixed = (Par1 << 16) + (pinNumber << 8) + GPIOType;

  return mixed & mask;
}

void ESPEasy_Scheduler::setGPIOTimer(
  unsigned long msecFromNow, 
  pluginID_t pluginID, 
  int        pinnr,
  int        state,
  int        repeatInterval,
  int        recurringCount,
  int        alternateInterval)
{
  uint8_t GPIOType = GPIO_TYPE_INVALID;

  switch (pluginID) {
    case PLUGIN_GPIO:
      GPIOType = GPIO_TYPE_INTERNAL;
      break;
    case PLUGIN_PCF:
      GPIOType = GPIO_TYPE_PCF;
      break;
    case PLUGIN_MCP:
      GPIOType = GPIO_TYPE_MCP;
      break;
  }

  if (GPIOType != GPIO_TYPE_INVALID) {
    // Par1 & Par2 & GPIOType form a unique key
    const unsigned long mixedTimerId = getMixedId(
      SchedulerTimerType_e::GPIO_timer, 
      createGPIOTimerId(GPIOType, pinnr, state));

    const systemTimerStruct timer_data(
      recurringCount, 
      repeatInterval, 
      state,
      alternateInterval);
    systemTimers[mixedTimerId] = timer_data;
    setNewTimerAt(mixedTimerId, millis() + msecFromNow);
  }
}

void ESPEasy_Scheduler::clearGPIOTimer(pluginID_t pluginID, int pinnr)
{
  uint8_t GPIOType = GPIO_TYPE_INVALID;

  switch (pluginID) {
    case PLUGIN_GPIO:
      GPIOType = GPIO_TYPE_INTERNAL;
      break;
    case PLUGIN_PCF:
      GPIOType = GPIO_TYPE_PCF;
      break;
    case PLUGIN_MCP:
      GPIOType = GPIO_TYPE_MCP;
      break;
  }

  if (GPIOType != GPIO_TYPE_INVALID) {
    // Par1 & Par2 & GPIOType form a unique key
    for (int state = 0; state <= 1; ++state) {
      const unsigned long mixedTimerId = getMixedId(
        SchedulerTimerType_e::GPIO_timer, 
        createGPIOTimerId(GPIOType, pinnr, state));
      auto it = systemTimers.find(mixedTimerId);
      if (it != systemTimers.end()) {
        systemTimers.erase(it);
      }
      msecTimerHandler.remove(mixedTimerId);
    }
  }
}

void ESPEasy_Scheduler::process_gpio_timer(unsigned long id, unsigned long lasttimer) {
 const unsigned long mixedTimerId = getMixedId(
    SchedulerTimerType_e::GPIO_timer, id);

  auto it = systemTimers.find(mixedTimerId);
  if (it == systemTimers.end()) {
    return;
  }

  // Reschedule before sending the event, as it may get rescheduled in handling the timer event.
  if (it->second.isRecurring()) {
    // Recurring timer
    it->second.markNextRecurring();
    
    unsigned long newTimer = lasttimer;
    setNextTimeInterval(newTimer, it->second.getInterval());
    setNewTimerAt(mixedTimerId, newTimer);
  }

  uint8_t GPIOType      = static_cast<uint8_t>((id) & 0xFF);
  uint8_t pinNumber     = static_cast<uint8_t>((id >> 8) & 0xFF);
  uint8_t pinStateValue = static_cast<uint8_t>((id >> 16) & 0xFF);
  if (it->second.isAlternateState()) {
    pinStateValue = (pinStateValue > 0) ? 0 : 1;
  }

  uint8_t pluginID = PLUGIN_GPIO;

  switch (GPIOType)
  {
    case GPIO_TYPE_INTERNAL:
      GPIO_Internal_Write(pinNumber, pinStateValue);
      pluginID = PLUGIN_GPIO;
      break;
#ifdef USES_P009
    case GPIO_TYPE_MCP:
      GPIO_MCP_Write(pinNumber, pinStateValue);
      pluginID = PLUGIN_MCP;
      break;
#endif
#ifdef USES_P019
    case GPIO_TYPE_PCF:
      GPIO_PCF_Write(pinNumber, pinStateValue);
      pluginID = PLUGIN_PCF;
      break;
#endif
    default:
      return;
  }


  if (!it->second.isRecurring()) {
    Scheduler.clearGPIOTimer(pluginID, pinNumber);
  }

  const uint32_t key = createKey(pluginID, pinNumber);

  // WARNING: operator [] creates an entry in the map if key does not exist
  portStatusStruct tempStatus = globalMapPortStatus[key];

  tempStatus.mode    = PIN_MODE_OUTPUT;
  tempStatus.command = 1; // set to 1 in order to display the status in the PinStatus page

  if (tempStatus.state != pinStateValue) {
    tempStatus.state        = pinStateValue;
    tempStatus.output       = pinStateValue;
    tempStatus.forceEvent   = 1;
    tempStatus.forceMonitor = 1;
  }
  savePortStatus(key, tempStatus);
}

/*********************************************************************************************\
* Task Device Timer
* This is the interval set in a plugin to get a new reading.
* These timers will re-schedule themselves as long as the plugin task is enabled.
* When the plugin task is initialized, a call to schedule_task_device_timer_at_init
* will bootstrap this sequence.
\*********************************************************************************************/
void ESPEasy_Scheduler::schedule_task_device_timer_at_init(unsigned long task_index) {
  unsigned long runAt = millis();

  if (!isDeepSleepEnabled()) {
    // Deepsleep is not enabled, add some offset based on the task index
    // to make sure not all are run at the same time.
    // This scheduled time may be overriden by the plugin's own init.
    runAt += (task_index * 37) + 100;
  } else {
    runAt += (task_index * 11) + 10;
  }
  schedule_task_device_timer(task_index, runAt);
}

// Typical use case is to run this when all needed connections are made.
void ESPEasy_Scheduler::schedule_all_task_device_timers() {
  for (taskIndex_t task = 0; task < TASKS_MAX; task++) {
    schedule_task_device_timer_at_init(task);
  }
}

void ESPEasy_Scheduler::schedule_task_device_timer(unsigned long task_index, unsigned long runAt) {
  /*
     String log = F("schedule_task_device_timer: task: ");
     log += task_index;
     log += F(" @ ");
     log += runAt;
     if (Settings.TaskDeviceEnabled[task_index]) {
      log += F(" (enabled)");
     }
     addLog(LOG_LEVEL_INFO, log);
   */

  if (!validTaskIndex(task_index)) { return; }

  const deviceIndex_t DeviceIndex = getDeviceIndex_from_TaskIndex(task_index);

  if (!validDeviceIndex(DeviceIndex)) { return; }

  // TD-er: Tasks without a timer or optional timer set to 0 should still be able to call PLUGIN_READ
  // For example to schedule a read from the PLUGIN_TEN_PER_SECOND when a new value is ready.

  /*
     if (!Device[DeviceIndex].TimerOption) { return; }

     if (Device[DeviceIndex].TimerOptional && (Settings.TaskDeviceTimer[task_index] == 0)) {
      return;
     }
   */

  if (Settings.TaskDeviceEnabled[task_index]) {
    setNewTimerAt(getMixedId(SchedulerTimerType_e::TaskDeviceTimer, task_index), runAt);
  }
}

void ESPEasy_Scheduler::reschedule_task_device_timer(unsigned long task_index, unsigned long lasttimer) {
  if (!validTaskIndex(task_index)) { return; }
  unsigned long newtimer = Settings.TaskDeviceTimer[task_index];

  if (newtimer != 0) {
    newtimer = lasttimer + (newtimer * 1000);
    schedule_task_device_timer(task_index, newtimer);
  }
}

void ESPEasy_Scheduler::process_task_device_timer(unsigned long task_index, unsigned long lasttimer) {
  if (!validTaskIndex(task_index)) { return; }
  reschedule_task_device_timer(task_index, lasttimer);
  START_TIMER;
  SensorSendTask(task_index);
  STOP_TIMER(SENSOR_SEND_TASK);
}

/*********************************************************************************************\
* System Event Timer
* Handling of these events will be asynchronous and being called from the loop().
* Thus only use these when the result is not needed immediately.
* Proper use case is calling from a callback function, since those cannot use yield() or delay()
\*********************************************************************************************/
void ESPEasy_Scheduler::schedule_plugin_task_event_timer(deviceIndex_t DeviceIndex, uint8_t Function, struct EventStruct&& event) {
  if (validDeviceIndex(DeviceIndex)) {
    schedule_event_timer(PluginPtrType::TaskPlugin, DeviceIndex, Function, std::move(event));
  }
}

#if FEATURE_MQTT
void ESPEasy_Scheduler::schedule_mqtt_plugin_import_event_timer(deviceIndex_t DeviceIndex,
                                                                taskIndex_t   TaskIndex,
                                                                uint8_t       Function,
                                                                char         *c_topic,
                                                                uint8_t      *b_payload,
                                                                unsigned int  length) {
  if (validDeviceIndex(DeviceIndex)) {
    const unsigned long mixedId = createSystemEventMixedId(PluginPtrType::TaskPlugin, DeviceIndex, static_cast<uint8_t>(Function));
    EventStruct  event(TaskIndex);
    const size_t topic_length = strlen_P(c_topic);

    if (!(event.String1.reserve(topic_length) && event.String2.reserve(length))) {
      addLog(LOG_LEVEL_ERROR, F("MQTT : Out of Memory! Cannot process MQTT message"));
      return;
    }

    for (size_t i = 0; i < topic_length; ++i) {
      event.String1 += c_topic[i];
    }

    for (unsigned int i = 0; i < length; ++i) {
      const char c = static_cast<char>(*(b_payload + i));
      event.String2 += c;
    }

    // Emplace using move.
    // This makes sure the relatively large event will not be in memory twice.
    ScheduledEventQueue.emplace_back(mixedId, std::move(event));
  }
}
#endif

void ESPEasy_Scheduler::schedule_controller_event_timer(protocolIndex_t ProtocolIndex, uint8_t Function, struct EventStruct&& event) {
  if (validProtocolIndex(ProtocolIndex)) {
    schedule_event_timer(PluginPtrType::ControllerPlugin, ProtocolIndex, Function, std::move(event));
  }
}

unsigned long ESPEasy_Scheduler::createSystemEventMixedId(PluginPtrType ptr_type, uint8_t Index, uint8_t Function) {
  unsigned long subId = static_cast<unsigned long>(ptr_type);

  subId = (subId << 8) + Index;
  subId = (subId << 8) + Function;
  return getMixedId(SchedulerTimerType_e::SystemEventQueue, subId);
}

#if FEATURE_MQTT
void ESPEasy_Scheduler::schedule_mqtt_controller_event_timer(protocolIndex_t   ProtocolIndex,
                                                             CPlugin::Function Function,
                                                             char             *c_topic,
                                                             uint8_t          *b_payload,
                                                             unsigned int      length) {
  if (validProtocolIndex(ProtocolIndex)) {
    // Emplace empty event in the queue first and the fill it.
    // This makes sure the relatively large event will not be in memory twice.
    const unsigned long mixedId = createSystemEventMixedId(PluginPtrType::ControllerPlugin, ProtocolIndex, static_cast<uint8_t>(Function));
    ScheduledEventQueue.emplace_back(mixedId, EventStruct());
    ScheduledEventQueue.back().event.String1 = c_topic;

    String& payload = ScheduledEventQueue.back().event.String2;

    if (!payload.reserve(length)) {
      addLog(LOG_LEVEL_ERROR, F("MQTT : Out of Memory! Cannot process MQTT message"));
    }

    for (unsigned int i = 0; i < length; ++i) {
      char c = static_cast<char>(*(b_payload + i));
      payload += c;
    }
  }
}
#endif

#if FEATURE_NOTIFIER
void ESPEasy_Scheduler::schedule_notification_event_timer(uint8_t              NotificationProtocolIndex,
                                                          NPlugin::Function    Function,
                                                          struct EventStruct&& event) {
  schedule_event_timer(PluginPtrType::NotificationPlugin, NotificationProtocolIndex, static_cast<uint8_t>(Function), std::move(event));
}
#endif

void ESPEasy_Scheduler::schedule_event_timer(PluginPtrType ptr_type, uint8_t Index, uint8_t Function, struct EventStruct&& event) {
  const unsigned long mixedId = createSystemEventMixedId(ptr_type, Index, Function);

  //  EventStructCommandWrapper eventWrapper(mixedId, *event);
  //  ScheduledEventQueue.push_back(eventWrapper);
  ScheduledEventQueue.emplace_back(mixedId, std::move(event));
}

void ESPEasy_Scheduler::process_system_event_queue() {
  #ifdef USE_SECOND_HEAP
  HeapSelectDram ephemeral;
  #endif

  if (ScheduledEventQueue.size() == 0) { return; }

  START_TIMER

  const unsigned long id = ScheduledEventQueue.front().id;

  if (RTC.lastMixedSchedulerId != id) {
    RTC.lastMixedSchedulerId = id;
    saveToRTC();
  }

  uint8_t Function       = id & 0xFF;
  uint8_t Index          = (id >> 8) & 0xFF;
  PluginPtrType ptr_type = static_cast<PluginPtrType>((id >> 16) & 0xFF);

  // At this moment, the String is not being used in the plugin calls, so just supply a dummy String.
  // Also since these events will be processed asynchronous, the resulting
  //   output in the String is probably of no use elsewhere.
  // Else the line string could be used.
  String tmpString;

  switch (ptr_type) {
    case PluginPtrType::TaskPlugin:

      if (validDeviceIndex(Index)) {
        if ((Function != PLUGIN_READ && 
             Function != PLUGIN_MQTT_CONNECTION_STATE && 
             Function != PLUGIN_MQTT_IMPORT)
           || Device[Index].ErrorStateValues) {
          // FIXME TD-er: LoadTaskSettings should only be called when needed, not pre-emptive.
          LoadTaskSettings(ScheduledEventQueue.front().event.TaskIndex);
        }
        Plugin_ptr[Index](Function, &ScheduledEventQueue.front().event, tmpString);
      }
      break;
    case PluginPtrType::ControllerPlugin:
      CPluginCall(Index, static_cast<CPlugin::Function>(Function), &ScheduledEventQueue.front().event, tmpString);
      break;
#if FEATURE_NOTIFIER
    case PluginPtrType::NotificationPlugin:
      NPlugin_ptr[Index](static_cast<NPlugin::Function>(Function), &ScheduledEventQueue.front().event, tmpString);
      break;
#endif
  }
  ScheduledEventQueue.pop_front();
  STOP_TIMER(PROCESS_SYSTEM_EVENT_QUEUE);
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
