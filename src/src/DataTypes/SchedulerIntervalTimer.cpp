#include "../DataTypes/SchedulerIntervalTimer.h"

#include "../Helpers/StringConverter.h"

#ifndef BUILD_NO_DEBUG
const __FlashStringHelper * toString_f(SchedulerIntervalTimer_e timer) {
  switch (timer) {
    case SchedulerIntervalTimer_e::TIMER_20MSEC:           return F("TIMER_20MSEC");
    case SchedulerIntervalTimer_e::TIMER_100MSEC:          return F("TIMER_100MSEC");
    case SchedulerIntervalTimer_e::TIMER_1SEC:             return F("TIMER_1SEC");
    case SchedulerIntervalTimer_e::TIMER_30SEC:            return F("TIMER_30SEC");
    case SchedulerIntervalTimer_e::TIMER_MQTT:             return F("TIMER_MQTT");
    case SchedulerIntervalTimer_e::TIMER_STATISTICS:       return F("TIMER_STATISTICS");
    case SchedulerIntervalTimer_e::TIMER_GRATUITOUS_ARP:   return F("TIMER_GRATUITOUS_ARP");
    case SchedulerIntervalTimer_e::TIMER_MQTT_DELAY_QUEUE: return F("TIMER_MQTT_DELAY_QUEUE");
    default: 
      break;
  }
  return F("unknown");
}
#endif

String toString(SchedulerIntervalTimer_e timer) {
#ifdef BUILD_NO_DEBUG
  return String(static_cast<int>(timer));
#else // ifdef BUILD_NO_DEBUG

  if (timer >= SchedulerIntervalTimer_e::TIMER_C001_DELAY_QUEUE && 
      timer <= SchedulerIntervalTimer_e::TIMER_C025_DELAY_QUEUE) 
  {
    const int id = static_cast<int>(timer) - static_cast<int>(SchedulerIntervalTimer_e::TIMER_C001_DELAY_QUEUE) + 1;
    String res;
    res.reserve(24);
    res = F("TIMER_");
    res += get_formatted_Controller_number(id);
    res += F("_DELAY_QUEUE");
    return res;
  }
  return toString_f(timer);
#endif // ifdef BUILD_NO_DEBUG
}