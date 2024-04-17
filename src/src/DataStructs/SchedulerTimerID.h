#ifndef DATASTRUCTS_SCHEDULER_TIMER_ID_H
#define DATASTRUCTS_SCHEDULER_TIMER_ID_H

#include "../../ESPEasy_common.h"

#include "../DataTypes/SchedulerTimerType.h"


struct SchedulerTimerID {
  explicit SchedulerTimerID(SchedulerTimerType_e timerType);

  explicit SchedulerTimerID(uint32_t mixedID)  : mixed_id(mixedID) {}

  virtual ~SchedulerTimerID() {}

  void                 setTimerType(SchedulerTimerType_e timerType);
  SchedulerTimerType_e getTimerType() const;


  uint32_t             getId() const;

  // Have setId in the header file as it is used in the constructor of derived classes
  // Thus it should be inline as we otherwise cannot call member functions of base class in the constructor in a derived class
  void setId(uint32_t id)
  {
    mixed_id = (id << 4) | (mixed_id & 0x0f);
  }

  uint32_t mixed_id{};
};


#endif // ifndef DATASTRUCTS_SCHEDULER_TIMER_ID_H
