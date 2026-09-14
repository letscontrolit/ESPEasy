#ifndef DATASTRUCTS_TIMER_ID_COUPLE_H
#define DATASTRUCTS_TIMER_ID_COUPLE_H

#include "../../ESPEasy_common.h"


/*********************************************************************************************\
* TimerHandler Used by the Scheduler
\*********************************************************************************************/

struct timer_id_couple {
  timer_id_couple(uint32_t id, uint32_t newtimer) : _id(id), _timer(newtimer) {}

  timer_id_couple(uint32_t id) : _id(id) {
    _timer = millis();
  }

  bool operator<(const timer_id_couple& other) const;

  // Returns true when _id matches.
  bool operator()(const timer_id_couple& item) const;


  uint32_t _id{};
  uint32_t _timer{};
};


#endif // DATASTRUCTS_TIMER_ID_COUPLE_H
