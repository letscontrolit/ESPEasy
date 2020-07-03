#include "src/Globals/RTC.h"
#include "src/DataStructs/RTCStruct.h"
#include "src/DataStructs/ESPEasy_EventStruct.h"
#include "src/DataStructs/EventValueSource.h"
#include "src/Globals/Device.h"
#include "src/Globals/CPlugins.h"
#include "src/Globals/NPlugins.h"
#include "src/Globals/Plugins.h"
#include "src/Helpers/ESPEasy_time_calc.h"

#include "ESPEasy_plugindefs.h"
#include "ESPEasy-Globals.h"

#define TIMER_ID_SHIFT    28

#define SYSTEM_EVENT_QUEUE   0 // Not really a timer.
#define CONST_INTERVAL_TIMER 1
#define PLUGIN_TASK_TIMER    2
#define TASK_DEVICE_TIMER    3
#define GPIO_TIMER           4
#define PLUGIN_TIMER         5


#include <list>
struct EventStructCommandWrapper {
  EventStructCommandWrapper() : id(0) {}

  EventStructCommandWrapper(unsigned long i, const struct EventStruct& e) : id(i), event(e) {}

  unsigned long      id;
  String             cmd;
  String             line;
  struct EventStruct event;
};
std::list<EventStructCommandWrapper> EventQueue;


/*********************************************************************************************\
* Generic Timer functions.
\*********************************************************************************************/
void setTimer(unsigned long timerType, unsigned long id, unsigned long msecFromNow) {
  setNewTimerAt(getMixedId(timerType, id), millis() + msecFromNow);
}

void setNewTimerAt(unsigned long id, unsigned long timer) {
  START_TIMER;
  msecTimerHandler.registerAt(id, timer);
  STOP_TIMER(SET_NEW_TIMER);
}

// Mix timer type int with an ID describing the scheduled job.
unsigned long getMixedId(unsigned long timerType, unsigned long id) {
  return (timerType << TIMER_ID_SHIFT) + id;
}

unsigned long decodeSchedulerId(unsigned long mixed_id, unsigned long& timerType) {
  timerType = (mixed_id >> TIMER_ID_SHIFT);
  const unsigned long mask = (1 << TIMER_ID_SHIFT) - 1;
  return mixed_id & mask;
}

String decodeSchedulerId(unsigned long mixed_id) {
  if (mixed_id == 0) {
    return F("Background Task");
  }
  unsigned long timerType = 0;
  const unsigned long id  = decodeSchedulerId(mixed_id, timerType);
  String result;
  result.reserve(32);

  switch (timerType) {
    case CONST_INTERVAL_TIMER:
      result = F("Const Interval");
      break;
    case PLUGIN_TASK_TIMER:
      result = F("Plugin Task");
      break;
    case PLUGIN_TIMER:
      result = F("Plugin");
      break;
    case TASK_DEVICE_TIMER:
      result = F("Task Device");
      break;
    case GPIO_TIMER:
      result = F("GPIO");
      break;
  }
  result += F(" timer, id: ");
  result += String(id);
  return result;
}

/*********************************************************************************************\
* Handle scheduled timers.
\*********************************************************************************************/
void handle_schedule() {
  START_TIMER
  unsigned long timer = 0;
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
    backgroundtasks();
    process_system_event_queue();
    last_system_event_run = millis();
    STOP_TIMER(HANDLE_SCHEDULER_IDLE);
    return;
  }

  unsigned long timerType = 0;
  const unsigned long id  = decodeSchedulerId(mixed_id, timerType);

  delay(0); // See: https://github.com/letscontrolit/ESPEasy/issues/1818#issuecomment-425351328

  switch (timerType) {
    case CONST_INTERVAL_TIMER:
      process_interval_timer(id, timer);
      break;
    case PLUGIN_TASK_TIMER:
      process_plugin_task_timer(id);
      break;
    case PLUGIN_TIMER:
      process_plugin_timer(id);
      break;
    case TASK_DEVICE_TIMER:
      process_task_device_timer(id, timer);
      break;
    case GPIO_TIMER:
      process_gpio_timer(id);
      break;
  }
  STOP_TIMER(HANDLE_SCHEDULER_TASK);
}

/*********************************************************************************************\
* Interval Timer
* These timers set a new scheduled timer, based on the old value.
* This will make their interval as constant as possible.
\*********************************************************************************************/
void setNextTimeInterval(unsigned long& timer, const unsigned long step) {
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

void setIntervalTimer(unsigned long id) {
  setIntervalTimer(id, millis());
}

void setIntervalTimerAt(unsigned long id, unsigned long newtimer) {
  setNewTimerAt(getMixedId(CONST_INTERVAL_TIMER, id), newtimer);
}

void setIntervalTimerOverride(unsigned long id, unsigned long msecFromNow) {
  unsigned long timer = millis();

  setNextTimeInterval(timer, msecFromNow);
  setNewTimerAt(getMixedId(CONST_INTERVAL_TIMER, id), timer);
}

void scheduleNextDelayQueue(unsigned long id, unsigned long nextTime) {
  if (nextTime != 0) {
    // Schedule for next process run.
    setIntervalTimerAt(id, nextTime);
  }
}

void setIntervalTimer(unsigned long id, unsigned long lasttimer) {
  // Set the initial timers for the regular runs
  unsigned long interval = 0;

  switch (id) {
    case TIMER_20MSEC:         interval = 20; break;
    case TIMER_100MSEC:        interval = 100; break;
    case TIMER_1SEC:           interval = 1000; break;
    case TIMER_30SEC:
    case TIMER_STATISTICS:     interval = 30000; break;
    case TIMER_MQTT:           interval = timermqtt_interval; break;
    case TIMER_GRATUITOUS_ARP: interval = timer_gratuitous_arp_interval; break;

    // Fall-through for all DelayQueue, which are just the fall-back timers.
    // The timers for all delay queues will be set according to their own settings as long as there is something to process.
    case TIMER_MQTT_DELAY_QUEUE:
    case TIMER_C001_DELAY_QUEUE:
    case TIMER_C003_DELAY_QUEUE:
    case TIMER_C004_DELAY_QUEUE:
    case TIMER_C007_DELAY_QUEUE:
    case TIMER_C008_DELAY_QUEUE:
    case TIMER_C009_DELAY_QUEUE:
    case TIMER_C010_DELAY_QUEUE:
    case TIMER_C011_DELAY_QUEUE:
    case TIMER_C012_DELAY_QUEUE:
    case TIMER_C013_DELAY_QUEUE:
    case TIMER_C014_DELAY_QUEUE:
    case TIMER_C015_DELAY_QUEUE:
    case TIMER_C016_DELAY_QUEUE:
    case TIMER_C017_DELAY_QUEUE:
    case TIMER_C018_DELAY_QUEUE:
    case TIMER_C019_DELAY_QUEUE:
    case TIMER_C020_DELAY_QUEUE:
      interval = 1000; break;
  }
  unsigned long timer = lasttimer;
  setNextTimeInterval(timer, interval);
  setNewTimerAt(getMixedId(CONST_INTERVAL_TIMER, id), timer);
}

void sendGratuitousARP_now() {
  sendGratuitousARP();

  if (Settings.gratuitousARP()) {
    timer_gratuitous_arp_interval = 100;
    setIntervalTimer(TIMER_GRATUITOUS_ARP);
  }
}

void process_interval_timer(unsigned long id, unsigned long lasttimer) {
  // Set the interval timer now, it may be altered by the commands below.
  // This is the default next-run-time.
  setIntervalTimer(id, lasttimer);

  switch (id) {
    case TIMER_20MSEC:         run50TimesPerSecond(); break;
    case TIMER_100MSEC:

      if (!UseRTOSMultitasking) {
        run10TimesPerSecond();
      }
      break;
    case TIMER_1SEC:             runOncePerSecond();      break;
    case TIMER_30SEC:            runEach30Seconds();      break;
#ifdef USES_MQTT
    case TIMER_MQTT:             runPeriodicalMQTT();     break;
#endif //USES_MQTT
    case TIMER_STATISTICS:       logTimerStatistics();    break;
    case TIMER_GRATUITOUS_ARP:

      // Slowly increase the interval timer.
      timer_gratuitous_arp_interval = 2 * timer_gratuitous_arp_interval;

      if (timer_gratuitous_arp_interval > TIMER_GRATUITOUS_ARP_MAX) {
        timer_gratuitous_arp_interval = TIMER_GRATUITOUS_ARP_MAX;
      }

      if (Settings.gratuitousARP()) {
        sendGratuitousARP();
      }
      break;
#ifdef USES_MQTT
    case TIMER_MQTT_DELAY_QUEUE: processMQTTdelayQueue(); break;
#endif //USES_MQTT
  #ifdef USES_C001
    case TIMER_C001_DELAY_QUEUE:
      process_c001_delay_queue();
      break;
  #endif // ifdef USES_C001
  #ifdef USES_C003
    case TIMER_C003_DELAY_QUEUE:
      process_c003_delay_queue();
      break;
  #endif // ifdef USES_C003
  #ifdef USES_C004
    case TIMER_C004_DELAY_QUEUE:
      process_c004_delay_queue();
      break;
  #endif // ifdef USES_C004
  #ifdef USES_C007
    case TIMER_C007_DELAY_QUEUE:
      process_c007_delay_queue();
      break;
  #endif // ifdef USES_C007
  #ifdef USES_C008
    case TIMER_C008_DELAY_QUEUE:
      process_c008_delay_queue();
      break;
  #endif // ifdef USES_C008
  #ifdef USES_C009
    case TIMER_C009_DELAY_QUEUE:
      process_c009_delay_queue();
      break;
  #endif // ifdef USES_C009
  #ifdef USES_C010
    case TIMER_C010_DELAY_QUEUE:
      process_c010_delay_queue();
      break;
  #endif // ifdef USES_C010
  #ifdef USES_C011
    case TIMER_C011_DELAY_QUEUE:
      process_c011_delay_queue();
      break;
  #endif // ifdef USES_C011
  #ifdef USES_C012
    case TIMER_C012_DELAY_QUEUE:
      process_c012_delay_queue();
      break;
  #endif // ifdef USES_C012

      /*
       #ifdef USES_C013
          case TIMER_C013_DELAY_QUEUE:
            process_c013_delay_queue();
            break;
       #endif
       */

      /*
       #ifdef USES_C014
          case TIMER_C014_DELAY_QUEUE:
            process_c014_delay_queue();
            break;
       #endif
       */
  #ifdef USES_C015
    case TIMER_C015_DELAY_QUEUE:
      process_c015_delay_queue();
      break;
  #endif // ifdef USES_C015
  #ifdef USES_C016
    case TIMER_C016_DELAY_QUEUE:
      process_c016_delay_queue();
      break;
  #endif // ifdef USES_C016

  #ifdef USES_C017
    case TIMER_C017_DELAY_QUEUE:
      process_c017_delay_queue();
      break;
  #endif // ifdef USES_C017

  #ifdef USES_C018
    case TIMER_C018_DELAY_QUEUE:
      process_c018_delay_queue();
      break;
  #endif

      /*
       #ifdef USES_C019
          case TIMER_C019_DELAY_QUEUE:
            process_c019_delay_queue();
            break;
       #endif
       */

      /*
       #ifdef USES_C020
          case TIMER_C020_DELAY_QUEUE:
            process_c020_delay_queue();
            break;
       #endif
       */

      // When extending this, also extend in DelayQueueElements.h
      // Also make sure to extend the "TIMER_C020_DELAY_QUEUE" list of defines.
  }
}

/*********************************************************************************************\
* Plugin Task Timer
\*********************************************************************************************/
unsigned long createPluginTaskTimerId(deviceIndex_t deviceIndex, int Par1) {
  const unsigned long mask  = (1 << TIMER_ID_SHIFT) - 1;
  const unsigned long mixed = (Par1 << 8) + deviceIndex;

  return mixed & mask;
}

/* // Not (yet) used
   void splitPluginTaskTimerId(const unsigned long mixed_id, byte& plugin, int& Par1) {
   const unsigned long mask = (1 << TIMER_ID_SHIFT) -1;
   plugin = mixed_id & 0xFF;
   Par1 = (mixed_id & mask) >> 8;
   }
 */
void setPluginTaskTimer(unsigned long msecFromNow, taskIndex_t taskIndex, int Par1, int Par2, int Par3, int Par4, int Par5)
{
  // plugin number and par1 form a unique key that can be used to restart a timer
  // Use deviceIndex instead of pluginID, since the deviceIndex uses less bits.
  const deviceIndex_t deviceIndex = getDeviceIndex_from_TaskIndex(taskIndex);
  if (!validDeviceIndex(deviceIndex)) return;

  const unsigned long systemTimerId = createPluginTaskTimerId(deviceIndex, Par1);
  systemTimerStruct   timer_data;

  timer_data.TaskIndex        = taskIndex;
  timer_data.Par1             = Par1;
  timer_data.Par2             = Par2;
  timer_data.Par3             = Par3;
  timer_data.Par4             = Par4;
  timer_data.Par5             = Par5;
  systemTimers[systemTimerId] = timer_data;
  setTimer(PLUGIN_TASK_TIMER, systemTimerId, msecFromNow);
}

void process_plugin_task_timer(unsigned long id) {
  START_TIMER;
  const systemTimerStruct timer_data = systemTimers[id];
  struct EventStruct TempEvent;
  TempEvent.TaskIndex = timer_data.TaskIndex;
  TempEvent.BaseVarIndex =  timer_data.TaskIndex * VARS_PER_TASK;
  TempEvent.Par1      = timer_data.Par1;
  TempEvent.Par2      = timer_data.Par2;
  TempEvent.Par3      = timer_data.Par3;
  TempEvent.Par4      = timer_data.Par4;
  TempEvent.Par5      = timer_data.Par5;

  // TD-er: Not sure if we have to keep original source for notifications.
  TempEvent.Source = EventValueSource::Enum::VALUE_SOURCE_SYSTEM;
  const deviceIndex_t deviceIndex = getDeviceIndex_from_TaskIndex(timer_data.TaskIndex);

  /*
     String log = F("proc_system_timer: Pluginid: ");
     log += deviceIndex;
     log += F(" taskIndex: ");
     log += timer_data.TaskIndex;
     log += F(" sysTimerID: ");
     log += id;
     addLog(LOG_LEVEL_INFO, log);
   */
  systemTimers.erase(id);

  if (validDeviceIndex(deviceIndex) && validUserVarIndex(TempEvent.BaseVarIndex)) {
    TempEvent.sensorType = Device[deviceIndex].VType;
    String dummy;
    Plugin_ptr[deviceIndex](PLUGIN_TIMER_IN, &TempEvent, dummy);
  }
  STOP_TIMER(PROC_SYS_TIMER);
}

/*********************************************************************************************\
* Plugin Timer
\*********************************************************************************************/
unsigned long createPluginTimerId(deviceIndex_t deviceIndex, int Par1) {
  const unsigned long mask  = (1 << TIMER_ID_SHIFT) - 1;
  const unsigned long mixed = (Par1 << 8) + deviceIndex;

  return mixed & mask;
}

/* // Not (yet) used
   void splitPluginTaskTimerId(const unsigned long mixed_id, byte& plugin, int& Par1) {
   const unsigned long mask = (1 << TIMER_ID_SHIFT) -1;
   plugin = mixed_id & 0xFF;
   Par1 = (mixed_id & mask) >> 8;
   }
 */
void setPluginTimer(unsigned long msecFromNow, pluginID_t pluginID, int Par1, int Par2, int Par3, int Par4, int Par5)
{
  // plugin number and par1 form a unique key that can be used to restart a timer
  // Use deviceIndex instead of pluginID, since the deviceIndex uses less bits.
  const deviceIndex_t deviceIndex = getDeviceIndex(pluginID);
  if (!validDeviceIndex(deviceIndex)) return;

  const unsigned long systemTimerId = createPluginTimerId(deviceIndex, Par1);
  systemTimerStruct   timer_data;

//timer_data.TaskIndex        = deviceIndex;
  timer_data.Par1             = Par1;
  timer_data.Par2             = Par2;
  timer_data.Par3             = Par3;
  timer_data.Par4             = Par4;
  timer_data.Par5             = Par5;
  systemTimers[systemTimerId] = timer_data;
  setTimer(PLUGIN_TIMER, systemTimerId, msecFromNow);
}

void process_plugin_timer(unsigned long id) {
  START_TIMER;
  const systemTimerStruct timer_data = systemTimers[id];
  struct EventStruct TempEvent;
//  TempEvent.TaskIndex = timer_data.TaskIndex;

// extract deviceID from timer id:
  const deviceIndex_t deviceIndex = ((1 << 8) -1) & id;

  TempEvent.Par1      = timer_data.Par1;
  TempEvent.Par2      = timer_data.Par2;
  TempEvent.Par3      = timer_data.Par3;
  TempEvent.Par4      = timer_data.Par4;
  TempEvent.Par5      = timer_data.Par5;

  // TD-er: Not sure if we have to keep original source for notifications.
  TempEvent.Source = EventValueSource::Enum::VALUE_SOURCE_SYSTEM;
//  const deviceIndex_t deviceIndex = getDeviceIndex_from_TaskIndex(timer_data.TaskIndex);

  /*
     String log = F("proc_system_timer: Pluginid: ");
     log += deviceIndex;
     log += F(" taskIndex: ");
     log += timer_data.TaskIndex;
     log += F(" sysTimerID: ");
     log += id;
     addLog(LOG_LEVEL_INFO, log);
   */
  systemTimers.erase(id);

  if (validDeviceIndex(deviceIndex)) {
    String dummy;
    Plugin_ptr[deviceIndex](PLUGIN_ONLY_TIMER_IN, &TempEvent, dummy);
  }
  STOP_TIMER(PROC_SYS_TIMER);
}

/*********************************************************************************************\
* GPIO Timer
* Special timer to handle timed GPIO actions
\*********************************************************************************************/
unsigned long createGPIOTimerId(byte pinNumber, int Par1) {
  const unsigned long mask  = (1 << TIMER_ID_SHIFT) - 1;
  const unsigned long mixed = (Par1 << 8) + pinNumber;

  return mixed & mask;
}

void setGPIOTimer(unsigned long msecFromNow, int Par1, int Par2, int Par3, int Par4, int Par5)
{
  // Par1 & Par2 form a unique key
  const unsigned long systemTimerId = createGPIOTimerId(Par1, Par2);

  setTimer(GPIO_TIMER, systemTimerId, msecFromNow);
}

void process_gpio_timer(unsigned long id) {
  // FIXME TD-er: Allow for all GPIO commands to be scheduled.
  byte pinNumber     = id & 0xFF;
  byte pinStateValue = (id >> 8);

  digitalWrite(pinNumber, pinStateValue);
}

/*********************************************************************************************\
* Task Device Timer
* This is the interval set in a plugin to get a new reading.
* These timers will re-schedule themselves as long as the plugin task is enabled.
* When the plugin task is initialized, a call to schedule_task_device_timer_at_init
* will bootstrap this sequence.
\*********************************************************************************************/
void schedule_task_device_timer_at_init(unsigned long task_index) {
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
void schedule_all_task_device_timers() {
  for (taskIndex_t task = 0; task < TASKS_MAX; task++) {
    schedule_task_device_timer_at_init(task);
  }
}

#ifdef USES_MQTT
void schedule_all_tasks_using_MQTT_controller() {
  controllerIndex_t ControllerIndex = firstEnabledMQTT_ControllerIndex();

  if (!validControllerIndex(ControllerIndex)) { return; }

  for (taskIndex_t task = 0; task < TASKS_MAX; task++) {
    if (Settings.TaskDeviceSendData[ControllerIndex][task] &&
        Settings.ControllerEnabled[ControllerIndex] &&
        Settings.Protocol[ControllerIndex])
    {
      schedule_task_device_timer_at_init(task);
    }
  }
}
#endif //USES_MQTT

void schedule_task_device_timer(unsigned long task_index, unsigned long runAt) {
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
    setNewTimerAt(getMixedId(TASK_DEVICE_TIMER, task_index), runAt);
  }
}

void process_task_device_timer(unsigned long task_index, unsigned long lasttimer) {
  if (!validTaskIndex(task_index)) { return; }
  unsigned long newtimer = Settings.TaskDeviceTimer[task_index];

  if (newtimer != 0) {
    newtimer = lasttimer + (newtimer * 1000);
    schedule_task_device_timer(task_index, newtimer);
  }
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
void schedule_plugin_task_event_timer(deviceIndex_t DeviceIndex, byte Function, struct EventStruct *event) {
  if (validDeviceIndex(DeviceIndex)) {
    schedule_event_timer(TaskPluginEnum, DeviceIndex, Function, event);
  }
}

void schedule_controller_event_timer(protocolIndex_t ProtocolIndex, byte Function, struct EventStruct *event) {
  if (validProtocolIndex(ProtocolIndex)) {
    schedule_event_timer(ControllerPluginEnum, ProtocolIndex, Function, event);
  }
}

void schedule_mqtt_controller_event_timer(protocolIndex_t ProtocolIndex, byte Function, char *c_topic, byte *b_payload, unsigned int length) {
  if (validProtocolIndex(ProtocolIndex)) {
    const unsigned long mixedId = createSystemEventMixedId(ControllerPluginEnum, ProtocolIndex, Function);
    EventQueue.emplace_back(mixedId, EventStruct());
    EventQueue.back().event.String1 = c_topic;

    String& payload = EventQueue.back().event.String2;
    payload.reserve(length);

    for (unsigned int i = 0; i < length; ++i) {
      char c = static_cast<char>(*(b_payload + i));
      payload += c;
    }
  }
}

void schedule_notification_event_timer(byte NotificationProtocolIndex, byte Function, struct EventStruct *event) {
  schedule_event_timer(NotificationPluginEnum, NotificationProtocolIndex, Function, event);
}

void schedule_event_timer(PluginPtrType ptr_type, byte Index, byte Function, struct EventStruct *event) {
  const unsigned long mixedId = createSystemEventMixedId(ptr_type, Index, Function);

  //  EventStructCommandWrapper eventWrapper(mixedId, *event);
  //  EventQueue.push_back(eventWrapper);
  EventQueue.emplace_back(mixedId, *event);
}

unsigned long createSystemEventMixedId(PluginPtrType ptr_type, uint16_t crc16) {
  unsigned long subId = ptr_type;

  subId = (subId << 16) + crc16;
  return getMixedId(SYSTEM_EVENT_QUEUE, subId);
}

unsigned long createSystemEventMixedId(PluginPtrType ptr_type, byte Index, byte Function) {
  unsigned long subId = ptr_type;

  subId = (subId << 8) + Index;
  subId = (subId << 8) + Function;
  return getMixedId(SYSTEM_EVENT_QUEUE, subId);
}

void process_system_event_queue() {
  if (EventQueue.size() == 0) { return; }
  unsigned long id       = EventQueue.front().id;
  byte Function          = id & 0xFF;
  byte Index             = (id >> 8) & 0xFF;
  PluginPtrType ptr_type = static_cast<PluginPtrType>((id >> 16) & 0xFF);

  // At this moment, the String is not being used in the plugin calls, so just supply a dummy String.
  // Also since these events will be processed asynchronous, the resulting
  //   output in the String is probably of no use elsewhere.
  // Else the line string could be used.
  String tmpString;

  switch (ptr_type) {
    case TaskPluginEnum:
      LoadTaskSettings(EventQueue.front().event.TaskIndex);
      Plugin_ptr[Index](Function, &EventQueue.front().event, tmpString);
      break;
    case ControllerPluginEnum:
      CPluginCall(Index, static_cast<CPlugin::Function>(Function), &EventQueue.front().event, tmpString);
      break;
    case NotificationPluginEnum:
      NPlugin_ptr[Index](static_cast<NPlugin::Function>(Function), &EventQueue.front().event, tmpString);
      break;
  }
  EventQueue.pop_front();
}
