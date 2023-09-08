#ifndef DATASTRUCTS_SCHEDULER_GPIOTIMERID_H
#define DATASTRUCTS_SCHEDULER_GPIOTIMERID_H

#include "../DataStructs/SchedulerTimerID.h"

/*********************************************************************************************\
* GPIO Timer
* Special timer to handle timed GPIO actions
\*********************************************************************************************/
struct GPIOTimerID : SchedulerTimerID {
  GPIOTimerID(uint8_t GPIOType,
              uint8_t pinNumber,
              int     Par1);

  uint8_t getGPIO_type() const;

  uint8_t getPinNumber() const;

  uint8_t getPinStateValue() const;

#ifndef BUILD_NO_DEBUG
  String  decode() const override;
#endif // ifndef BUILD_NO_DEBUG
};

#endif // ifndef DATASTRUCTS_SCHEDULER_GPIOTIMERID_H
