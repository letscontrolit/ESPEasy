#ifndef DATASTRUCTS_SCHEDULER_TIMER_ID_H
#define DATASTRUCTS_SCHEDULER_TIMER_ID_H

#include "../../ESPEasy_common.h"

#include "../DataTypes/SchedulerTimerType.h"


struct SchedulerTimerID {
  explicit SchedulerTimerID(SchedulerTimerType_e timerType);

  explicit SchedulerTimerID(uint32_t mixedID)  : mixed_id(mixedID) {}

  union {
    struct {
      uint32_t id         : 28;
      uint32_t timer_type : 4; // Change this when SchedulerTimerType_e needs more bits
    };

    uint32_t mixed_id{};
  };

  void setTimerType(SchedulerTimerType_e timerType)
  {
    timer_type = static_cast<uint32_t>(timerType);
  }

  SchedulerTimerType_e getTimerType() const
  {
    return static_cast<SchedulerTimerType_e>(timer_type);
  }
};


#endif // ifndef DATASTRUCTS_SCHEDULER_TIMER_ID_H
