#define TIMER_ID_SHIFT    16

#define CONST_INTERVAL_TIMER 1
#define GENERIC_TIMER        2

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
