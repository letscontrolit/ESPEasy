#include "../Helpers/Scheduler.h"


#include "../../ESPEasy-Globals.h"

#include "../../_Plugin_Helper.h"


#include "../DataStructs/Scheduler_GPIOTimerID.h"
#include "../DataStructs/Scheduler_IntendedRebootTimerID.h"
#include "../DataStructs/Scheduler_PluginDeviceTimerID.h"
#include "../DataStructs/Scheduler_RulesTimerID.h"
#include "../DataStructs/Scheduler_SystemEventQueueTimerID.h"
#include "../DataStructs/Scheduler_TaskDeviceTimerID.h"

#include "../DataStructs/TimingStats.h"

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

String ESPEasy_Scheduler::decodeSchedulerId(SchedulerTimerID timerID) {
  if (timerID.mixed_id == 0) {
    return F("Background Task");
  }
  String idStr                   = String(timerID.id);
  String result                  = toString(timerID.getTimerType());

  result += F(": ");

#ifndef BUILD_NO_DEBUG
  result.reserve(64);

  String decoded = timerID.decode();




  if (!decoded.isEmpty()) {
    result += decoded;
    return result;
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
    case SchedulerTimerType_e::PluginCallForTask_e:
//      process_plugin_call_for_task(timerID);
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

bool ESPEasy_Scheduler::setRulesTimer(unsigned long msecFromNow, unsigned int timerIndex, int recurringCount) {
  if (!checkRulesTimerIndex(timerIndex)) { return false; }

  const RulesTimerID timerID(timerIndex);

  const systemTimerStruct timer_data(recurringCount, msecFromNow, timerIndex);

  systemTimers[timerID.mixed_id] = timer_data;
  setNewTimerAt(timerID, millis() + msecFromNow);
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

  systemTimerStruct   timer_data;

  // PLUGIN_DEVICETIMER_IN does not address a task, so don't set TaskIndex
  timer_data.fromEvent(INVALID_TASK_INDEX, Par1, Par2, Par3, Par4, Par5);
  systemTimers[timerID.mixed_id] = timer_data;
  setNewTimerAt(timerID, millis() + msecFromNow);
}

void ESPEasy_Scheduler::process_plugin_timer(SchedulerTimerID id) {
  #ifdef USE_SECOND_HEAP
  HeapSelectDram ephemeral;
  #endif

  START_TIMER;
  auto it                          = systemTimers.find(id.mixed_id);

  if (it == systemTimers.end()) { return; }

  struct EventStruct TempEvent(it->second.toEvent());

  // PLUGIN_DEVICETIMER_IN does not address a task, so don't set TaskIndex

  // extract deviceID from timer id:
  const PluginDeviceTimerID* tmp = reinterpret_cast<const PluginDeviceTimerID*>(&id);  
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

/*********************************************************************************************\
* GPIO Timer
* Special timer to handle timed GPIO actions
\*********************************************************************************************/
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
    const GPIOTimerID timerID(GPIOType, pinnr, state);
    const systemTimerStruct timer_data(
      recurringCount, 
      repeatInterval, 
      state,
      alternateInterval);
    systemTimers[timerID.mixed_id] = timer_data;
    setNewTimerAt(timerID, millis() + msecFromNow);
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
      const GPIOTimerID timerID(GPIOType, pinnr, state);
      auto it = systemTimers.find(timerID.mixed_id);
      if (it != systemTimers.end()) {
        systemTimers.erase(it);
      }
      msecTimerHandler.remove(timerID.mixed_id);
    }
  }
}

void ESPEasy_Scheduler::process_gpio_timer(SchedulerTimerID timerID, unsigned long lasttimer) {
  auto it = systemTimers.find(timerID.mixed_id);
  if (it == systemTimers.end()) {
    return;
  }

  // Reschedule before sending the event, as it may get rescheduled in handling the timer event.
  if (it->second.isRecurring()) {
    // Recurring timer
    it->second.markNextRecurring();
    
    unsigned long newTimer = lasttimer;
    setNextTimeInterval(newTimer, it->second.getInterval());
    setNewTimerAt(timerID, newTimer);
  }

  const GPIOTimerID* tmp = reinterpret_cast<const GPIOTimerID*>(&timerID);

  uint8_t GPIOType      = tmp->getGPIO_type();
  uint8_t pinNumber     = tmp->getPinNumber();
  uint8_t pinStateValue = tmp->getPinStateValue();
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
    const TaskDeviceTimerID timerID(task_index);
    setNewTimerAt(timerID, runAt);
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

void ESPEasy_Scheduler::process_task_device_timer(SchedulerTimerID timerID, unsigned long lasttimer) {
  const TaskDeviceTimerID* tmp = reinterpret_cast<const TaskDeviceTimerID*>(&timerID);
  const taskIndex_t task_index = tmp->getTaskIndex();
  if (!validTaskIndex(task_index)) { return; }
  START_TIMER;
  struct EventStruct TempEvent(task_index);
  SensorSendTask(&TempEvent, 0, lasttimer);
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
    schedule_event_timer(SchedulerPluginPtrType_e::TaskPlugin, DeviceIndex, Function, std::move(event));
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
    const SystemEventQueueTimerID timerID(SchedulerPluginPtrType_e::TaskPlugin, DeviceIndex, static_cast<uint8_t>(Function));
    ScheduledEventQueue.emplace_back(timerID.mixed_id, std::move(event));
  }
}
#endif

void ESPEasy_Scheduler::schedule_controller_event_timer(protocolIndex_t ProtocolIndex, uint8_t Function, struct EventStruct&& event) {
  if (validProtocolIndex(ProtocolIndex)) {
    schedule_event_timer(SchedulerPluginPtrType_e::ControllerPlugin, ProtocolIndex, Function, std::move(event));
  }
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
    const SystemEventQueueTimerID timerID(SchedulerPluginPtrType_e::ControllerPlugin, ProtocolIndex, static_cast<uint8_t>(Function));

    ScheduledEventQueue.emplace_back(timerID.mixed_id, EventStruct());
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
  schedule_event_timer(SchedulerPluginPtrType_e::NotificationPlugin, NotificationProtocolIndex, static_cast<uint8_t>(Function), std::move(event));
}
#endif

void ESPEasy_Scheduler::schedule_event_timer(SchedulerPluginPtrType_e ptr_type, uint8_t Index, uint8_t Function, struct EventStruct&& event) {
  const SystemEventQueueTimerID timerID(ptr_type, Index, Function);

  //  EventStructCommandWrapper eventWrapper(mixedId, *event);
  //  ScheduledEventQueue.push_back(eventWrapper);
  ScheduledEventQueue.emplace_back(timerID.mixed_id, std::move(event));
}

void ESPEasy_Scheduler::process_system_event_queue() {
  #ifdef USE_SECOND_HEAP
  HeapSelectDram ephemeral;
  #endif

  if (ScheduledEventQueue.size() == 0) { return; }

  START_TIMER

  const SchedulerTimerID timerID(ScheduledEventQueue.front().id);
  const SystemEventQueueTimerID* sysEventTimerID = reinterpret_cast<const SystemEventQueueTimerID*>(&timerID);



  if (RTC.lastMixedSchedulerId != timerID.mixed_id) {
    RTC.lastMixedSchedulerId = timerID.mixed_id;
    saveToRTC();
  }

  uint8_t Function       = sysEventTimerID->getFunction();
  uint8_t Index          = sysEventTimerID->getIndex();
  SchedulerPluginPtrType_e ptr_type = sysEventTimerID->getPtrType();

  // At this moment, the String is not being used in the plugin calls, so just supply a dummy String.
  // Also since these events will be processed asynchronous, the resulting
  //   output in the String is probably of no use elsewhere.
  // Else the line string could be used.
  String tmpString;

  switch (ptr_type) {
    case SchedulerPluginPtrType_e::TaskPlugin:

      if (validDeviceIndex(Index)) {
        if ((Function != PLUGIN_READ && 
             Function != PLUGIN_MQTT_CONNECTION_STATE && 
             Function != PLUGIN_MQTT_IMPORT)
           || Device[Index].ErrorStateValues) {
          // FIXME TD-er: LoadTaskSettings should only be called when needed, not pre-emptive.
          LoadTaskSettings(ScheduledEventQueue.front().event.TaskIndex);
        }
        PluginCall(Index, Function, &ScheduledEventQueue.front().event, tmpString);
      }
      break;
    case SchedulerPluginPtrType_e::ControllerPlugin:
      CPluginCall(Index, static_cast<CPlugin::Function>(Function), &ScheduledEventQueue.front().event, tmpString);
      break;
#if FEATURE_NOTIFIER
    case SchedulerPluginPtrType_e::NotificationPlugin:
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
