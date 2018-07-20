#define TIMER_ID_SHIFT    28

#define CONST_INTERVAL_TIMER 1
#define GENERIC_TIMER        2
#define SYSTEM_TIMER         3

void setTimer(unsigned long id) {
  setTimer(GENERIC_TIMER, id, 0);
}

void setIntervalTimer(unsigned long id) {
  setIntervalTimer(id, millis());
}

void setIntervalTimerOverride(unsigned long id, unsigned long msecFromNow) {
  unsigned long timer = millis();
  setNextTimeInterval(timer, msecFromNow);
  msecTimerHandler.registerAt(getMixedId(CONST_INTERVAL_TIMER, id), timer);
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
  }
  unsigned long timer = lasttimer;
  setNextTimeInterval(timer, interval);
  msecTimerHandler.registerAt(getMixedId(CONST_INTERVAL_TIMER, id), timer);
}

void setTimer(unsigned long timerType, unsigned long id, unsigned long msecFromNow) {
  msecTimerHandler.registerFromNow(getMixedId(timerType, id), msecFromNow);
}

void setSystemTimer(unsigned long timer, byte plugin, short taskIndex, int Par1, int Par2, int Par3, int Par4, int Par5)
{
  // plugin number and par1 form a unique key that can be used to restart a timer
  // first check if a timer is not already running for this request
  const unsigned long systemTimerId = createSystemTimerId(plugin, Par1);
  systemTimerStruct timer_data;
  timer_data.TaskIndex = taskIndex;
  timer_data.Par1 = Par1;
  timer_data.Par2 = Par2;
  timer_data.Par3 = Par3;
  timer_data.Par4 = Par4;
  timer_data.Par5 = Par5;
  systemTimers[systemTimerId] = timer_data;
  setTimer(SYSTEM_TIMER, systemTimerId, timer);
}

unsigned long createSystemTimerId(byte plugin, int Par1) {
  const unsigned long mask = (1 << TIMER_ID_SHIFT) -1;
  const unsigned long mixed = (Par1 << 8) + plugin;
/*
  if (mixed & !mask) {
    String error = F("createSystemTimerId: Information lost for Par1: ");
    error += Par1;
    error += F(" with plugin: ");
    error += plugin;
    addLog(LOG_LEVEL_ERROR, error);
  }
*/
  return (mixed & mask);
}

void splitSystemTimerId(const unsigned long mixed_id, byte& plugin, int& Par1) {
  const unsigned long mask = (1 << TIMER_ID_SHIFT) -1;
  plugin = mixed_id & 0xFF;
  Par1 = (mixed_id & mask) >> 8;
}

unsigned long getMixedId(unsigned long timerType, unsigned long id) {
  return (timerType << TIMER_ID_SHIFT) + id;
}

void handle_schedule() {
  unsigned long timer;
  const unsigned long mixed_id = msecTimerHandler.getNextId(timer);
  if (mixed_id == 0) return;
  const unsigned long timerType = (mixed_id >> TIMER_ID_SHIFT);
  const unsigned long mask = (1 << TIMER_ID_SHIFT) -1;
  const unsigned long id = mixed_id & mask;

  switch (timerType) {
    case CONST_INTERVAL_TIMER:
      setIntervalTimer(id, timer);
      process_interval_timer(id);
      break;
    case SYSTEM_TIMER:
      process_system_timer(id);
      break;
  }
}

void process_interval_timer(unsigned long id) {
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
  }
}

void process_system_timer(unsigned long id) {
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
  String log = F("proc_system_timer: Pluginid: ");
  log += y;
  log += F(" taskIndex: ");
  log += timer_data.TaskIndex;
  log += F(" sysTimerID: ");
  log += id;
  addLog(LOG_LEVEL_INFO, log);
  if (y >= 0) {
    String dummy;
    Plugin_ptr[y](PLUGIN_TIMER_IN, &TempEvent, dummy);
  }
  systemTimers.erase(id);
  STOP_TIMER(PROC_SYS_TIMER);
}
