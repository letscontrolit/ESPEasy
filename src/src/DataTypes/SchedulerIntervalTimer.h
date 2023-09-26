#ifndef DATATYPES_SCHEDULERINTERVALTIMER_H
#define DATATYPES_SCHEDULERINTERVALTIMER_H

#include "../../ESPEasy_common.h"

  // ********************************************************************************
  //   Timers used in the scheduler
  // ********************************************************************************

  enum class SchedulerIntervalTimer_e : uint8_t {
    TIMER_20MSEC,
    TIMER_100MSEC,
    TIMER_1SEC,
    TIMER_30SEC,
    TIMER_MQTT,
    TIMER_STATISTICS,
    TIMER_GRATUITOUS_ARP,
    TIMER_MQTT_DELAY_QUEUE,
    TIMER_C001_DELAY_QUEUE,
    TIMER_C002_DELAY_QUEUE, // MQTT controller
    TIMER_C003_DELAY_QUEUE,
    TIMER_C004_DELAY_QUEUE,
    TIMER_C005_DELAY_QUEUE, // MQTT controller
    TIMER_C006_DELAY_QUEUE, // MQTT controller
    TIMER_C007_DELAY_QUEUE,
    TIMER_C008_DELAY_QUEUE,
    TIMER_C009_DELAY_QUEUE,
    TIMER_C010_DELAY_QUEUE,
    TIMER_C011_DELAY_QUEUE,
    TIMER_C012_DELAY_QUEUE,
    TIMER_C013_DELAY_QUEUE,
    TIMER_C014_DELAY_QUEUE,
    TIMER_C015_DELAY_QUEUE,
    TIMER_C016_DELAY_QUEUE,
    TIMER_C017_DELAY_QUEUE,
    TIMER_C018_DELAY_QUEUE,
    TIMER_C019_DELAY_QUEUE,
    TIMER_C020_DELAY_QUEUE,
    TIMER_C021_DELAY_QUEUE,
    TIMER_C022_DELAY_QUEUE,
    TIMER_C023_DELAY_QUEUE,
    TIMER_C024_DELAY_QUEUE,
    TIMER_C025_DELAY_QUEUE,

    // When extending this, search for EXTEND_CONTROLLER_IDS
    // in the code to find all places that need to be updated too.
  };

  String toString(SchedulerIntervalTimer_e timer);



#endif
