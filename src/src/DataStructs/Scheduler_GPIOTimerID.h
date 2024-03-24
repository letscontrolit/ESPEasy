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

  uint8_t getGPIO_type() const {
    return static_cast<uint8_t>((getId()) & 0xFF);
  }

  uint8_t getPinNumber() const {
    return static_cast<uint8_t>((getId() >> 8) & 0xFF);
  }

  uint8_t getPinStateValue() const {
    return static_cast<uint8_t>((getId() >> 16) & 0xFF);
  }

#ifndef BUILD_NO_DEBUG
  String decode() const;
#endif // ifndef BUILD_NO_DEBUG
};

#endif // ifndef DATASTRUCTS_SCHEDULER_GPIOTIMERID_H
