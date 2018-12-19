#define TIMER_ID_SHIFT    28

#define SYSTEM_EVENT_QUEUE   0  // Not really a timer.
#define CONST_INTERVAL_TIMER 1
#define PLUGIN_TASK_TIMER    2
#define TASK_DEVICE_TIMER    3
#define GPIO_TIMER           4

#include <list>
struct EventStructCommandWrapper {
  EventStructCommandWrapper() : id(0) {}
  EventStructCommandWrapper(unsigned long i, const struct EventStruct& e) : id(i), event(e) {}

  unsigned long id;
  String cmd;
  String line;
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

/*********************************************************************************************\
 * Handle scheduled timers.
\*********************************************************************************************/
void handle_schedule() {
  START_TIMER
  unsigned long timer;
  unsigned long mixed_id = 0;
  if (timePassedSince(last_system_event_run) < 500) {
    // Make sure system event queue will be looked at every now and then.
    mixed_id = msecTimerHandler.getNextId(timer);
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
  const unsigned long timerType = (mixed_id >> TIMER_ID_SHIFT);
  const unsigned long mask = (1 << TIMER_ID_SHIFT) -1;
  const unsigned long id = mixed_id & mask;

  delay(0); // See: https://github.com/letscontrolit/ESPEasy/issues/1818#issuecomment-425351328

  switch (timerType) {
    case CONST_INTERVAL_TIMER:
      process_interval_timer(id, timer);
      break;
    case PLUGIN_TASK_TIMER:
      process_plugin_task_timer(id);
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
    case TIMER_20MSEC:     interval = 20; break;
    case TIMER_100MSEC:    interval = 100; break;
    case TIMER_1SEC:       interval = 1000; break;
    case TIMER_30SEC:      interval = 30000; break;
    case TIMER_MQTT:       interval = timermqtt_interval; break;
    case TIMER_STATISTICS: interval = 30000; break;
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
      interval = 1000; break;
  }
  unsigned long timer = lasttimer;
  setNextTimeInterval(timer, interval);
  setNewTimerAt(getMixedId(CONST_INTERVAL_TIMER, id), timer);
}

void process_interval_timer(unsigned long id, unsigned long lasttimer) {
  // Set the interval timer now, it may be altered by the commands below.
  // This is the default next-run-time.
  setIntervalTimer(id, lasttimer);
  switch (id) {
    case TIMER_20MSEC:
      run50TimesPerSecond();
      break;
    case TIMER_100MSEC:
      if(!UseRTOSMultitasking)
        run10TimesPerSecond();
      break;
    case TIMER_1SEC:
      runOncePerSecond();
      break;
    case TIMER_30SEC:
      runEach30Seconds();
      break;
    case TIMER_MQTT:
      runPeriodicalMQTT();
      break;
    case TIMER_STATISTICS:
      logTimerStatistics();
      break;
    case TIMER_MQTT_DELAY_QUEUE:
      processMQTTdelayQueue();
      break;
  #ifdef USES_C001
    case TIMER_C001_DELAY_QUEUE:
      process_c001_delay_queue();
      break;
  #endif
  #ifdef USES_C003
    case TIMER_C003_DELAY_QUEUE:
      process_c003_delay_queue();
      break;
  #endif
  #ifdef USES_C004
    case TIMER_C004_DELAY_QUEUE:
      process_c004_delay_queue();
      break;
  #endif
  #ifdef USES_C007
    case TIMER_C007_DELAY_QUEUE:
      process_c007_delay_queue();
      break;
  #endif
  #ifdef USES_C008
    case TIMER_C008_DELAY_QUEUE:
      process_c008_delay_queue();
      break;
  #endif
  #ifdef USES_C009
    case TIMER_C009_DELAY_QUEUE:
      process_c009_delay_queue();
      break;
  #endif
  #ifdef USES_C010
    case TIMER_C010_DELAY_QUEUE:
      process_c010_delay_queue();
      break;
  #endif
  #ifdef USES_C011
    case TIMER_C011_DELAY_QUEUE:
      process_c011_delay_queue();
      break;
  #endif
  #ifdef USES_C012
    case TIMER_C012_DELAY_QUEUE:
      process_c012_delay_queue();
      break;
  #endif
/*
  #ifdef USES_C013
    case TIMER_C013_DELAY_QUEUE:
      process_c013_delay_queue();
      break;
  #endif
*/
// When extending this, also extend in _CPlugin_Helper.h
// Look for DEFINE_Cxxx_DELAY_QUEUE_MACRO
  }
}


/*********************************************************************************************\
 * Plugin Task Timer
\*********************************************************************************************/
unsigned long createPluginTaskTimerId(byte plugin, int Par1) {
  const unsigned long mask = (1 << TIMER_ID_SHIFT) -1;
  const unsigned long mixed = (Par1 << 8) + plugin;
  return (mixed & mask);
}

/* // Not (yet) used
void splitPluginTaskTimerId(const unsigned long mixed_id, byte& plugin, int& Par1) {
  const unsigned long mask = (1 << TIMER_ID_SHIFT) -1;
  plugin = mixed_id & 0xFF;
  Par1 = (mixed_id & mask) >> 8;
}
*/

void setPluginTaskTimer(unsigned long msecFromNow, byte plugin, short taskIndex, int Par1, int Par2, int Par3, int Par4, int Par5)
{
  // plugin number and par1 form a unique key that can be used to restart a timer
  const unsigned long systemTimerId = createPluginTaskTimerId(plugin, Par1);
  systemTimerStruct timer_data;
  timer_data.TaskIndex = taskIndex;
  timer_data.Par1 = Par1;
  timer_data.Par2 = Par2;
  timer_data.Par3 = Par3;
  timer_data.Par4 = Par4;
  timer_data.Par5 = Par5;
  systemTimers[systemTimerId] = timer_data;
  setTimer(PLUGIN_TASK_TIMER, systemTimerId, msecFromNow);
}

void process_plugin_task_timer(unsigned long id) {
  START_TIMER;
  const systemTimerStruct timer_data = systemTimers[id];
  struct EventStruct TempEvent;
  TempEvent.TaskIndex = timer_data.TaskIndex;
  TempEvent.Par1 = timer_data.Par1;
  TempEvent.Par2 = timer_data.Par2;
  TempEvent.Par3 = timer_data.Par3;
  TempEvent.Par4 = timer_data.Par4;
  TempEvent.Par5 = timer_data.Par5;
  // TD-er: Not sure if we have to keep original source for notifications.
  TempEvent.Source = VALUE_SOURCE_SYSTEM;
  const int y = getPluginId(timer_data.TaskIndex);
/*
  String log = F("proc_system_timer: Pluginid: ");
  log += y;
  log += F(" taskIndex: ");
  log += timer_data.TaskIndex;
  log += F(" sysTimerID: ");
  log += id;
  addLog(LOG_LEVEL_INFO, log);
*/
  systemTimers.erase(id);
  if (y >= 0) {
    String dummy;
    Plugin_ptr[y](PLUGIN_TIMER_IN, &TempEvent, dummy);
  }
  STOP_TIMER(PROC_SYS_TIMER);
}


/*********************************************************************************************\
 * GPIO Timer
 * Special timer to handle timed GPIO actions
\*********************************************************************************************/
unsigned long createGPIOTimerId(byte pinNumber, int Par1) {
  const unsigned long mask = (1 << TIMER_ID_SHIFT) -1;
  const unsigned long mixed = (Par1 << 8) + pinNumber;
  return (mixed & mask);
}

void setGPIOTimer(unsigned long msecFromNow, int Par1, int Par2, int Par3, int Par4, int Par5)
{
  // Par1 & Par2 form a unique key
  const unsigned long systemTimerId = createGPIOTimerId(Par1, Par2);
  setTimer(GPIO_TIMER, systemTimerId, msecFromNow);
}

void process_gpio_timer(unsigned long id) {
  // FIXME TD-er: Allow for all GPIO commands to be scheduled.
  byte pinNumber = id & 0xFF;
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
    runAt += (task_index * 37) + Settings.MessageDelay;
  } else {
    runAt += (task_index * 11) + 10;
  }
  schedule_task_device_timer(task_index, runAt);
}

// Typical use case is to run this when all needed connections are made.
void schedule_all_task_device_timers() {
  for (byte task = 0; task < TASKS_MAX; task++) {
    schedule_task_device_timer_at_init(task);
  }
}

void schedule_all_tasks_using_MQTT_controller() {
  int ControllerIndex = firstEnabledMQTTController();
  if (ControllerIndex < 0) return;
  for (byte task = 0; task < TASKS_MAX; task++) {
    if (Settings.TaskDeviceSendData[ControllerIndex][task] &&
        Settings.ControllerEnabled[ControllerIndex] &&
        Settings.Protocol[ControllerIndex])
    {
      schedule_task_device_timer_at_init(task);
    }
  }
}

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

  if (task_index >= TASKS_MAX) return;
  byte DeviceIndex = getDeviceIndex(Settings.TaskDeviceNumber[task_index]);
  if (!Device[DeviceIndex].TimerOption) return;
  if (Device[DeviceIndex].TimerOptional && Settings.TaskDeviceTimer[task_index] == 0) {
    return;
  }
  if (Settings.TaskDeviceEnabled[task_index]) {
    setNewTimerAt(getMixedId(TASK_DEVICE_TIMER, task_index), runAt);
  }
}

void process_task_device_timer(unsigned long task_index, unsigned long lasttimer) {
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
void schedule_plugin_task_event_timer(byte DeviceIndex, byte Function, struct EventStruct* event) {
  schedule_event_timer(TaskPluginEnum, DeviceIndex, Function, event);
}

void schedule_controller_event_timer(byte ProtocolIndex, byte Function, struct EventStruct* event) {
  schedule_event_timer(ControllerPluginEnum, ProtocolIndex, Function, event);
}

void schedule_notification_event_timer(byte NotificationProtocolIndex, byte Function, struct EventStruct* event) {
  schedule_event_timer(NotificationPluginEnum, NotificationProtocolIndex, Function, event);
}

void schedule_command_timer(const char * cmd, struct EventStruct *event, const char* line) {
  String cmdStr;
  cmdStr += cmd;
  String lineStr;
  lineStr += line;
  // Using CRC here based on the cmd AND line, to make sure other commands are
  // not removed from the queue,  since the ID used in the queue must be unique.
  const int crc = calc_CRC16(cmdStr) ^ calc_CRC16(lineStr);
  const unsigned long mixedId = createSystemEventMixedId(CommandTimerEnum, static_cast<uint16_t>(crc));
  EventStructCommandWrapper eventWrapper(mixedId, *event);
  eventWrapper.cmd = cmdStr;
  eventWrapper.line = lineStr;
  EventQueue.push_back(eventWrapper);
}

void schedule_event_timer(PluginPtrType ptr_type, byte Index, byte Function, struct EventStruct* event) {
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
  if (EventQueue.size() == 0) return;
  unsigned long id = EventQueue.front().id;
  byte Function = id & 0xFF;
  byte Index = (id >> 8) & 0xFF;
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
      CPluginCall(Index, Function, &EventQueue.front().event, tmpString);
      break;
    case NotificationPluginEnum:
      NPlugin_ptr[Index](Function, &EventQueue.front().event, tmpString);
      break;
    case CommandTimerEnum:
      {
        String status = doExecuteCommand(
            EventQueue.front().cmd.c_str(),
            &EventQueue.front().event,
            EventQueue.front().line.c_str());
        delay(0);
        SendStatus(EventQueue.front().event.Source, status);
        delay(0);
        break;
      }
  }
  EventQueue.pop_front();
}
