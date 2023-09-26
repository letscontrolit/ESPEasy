#ifndef DATASTRUCTS_TIMER_ID_COUPLE_H
#define DATASTRUCTS_TIMER_ID_COUPLE_H

#include "../../ESPEasy_common.h"


/*********************************************************************************************\
* TimerHandler Used by the Scheduler
\*********************************************************************************************/

struct timer_id_couple {
  timer_id_couple(unsigned long id, unsigned long newtimer) : _id(id), _timer(newtimer) {}

  timer_id_couple(unsigned long id) : _id(id) {
    _timer = millis();
  }

  bool operator<(const timer_id_couple& other) const;

  unsigned long _id;
  unsigned long _timer;
};


#endif // DATASTRUCTS_TIMER_ID_COUPLE_H
