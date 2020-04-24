#include "timer_id_couple.h"

#include "../Helpers/ESPEasy_time_calc.h"

  timer_id_couple::timer_id_couple(unsigned long id, unsigned long newtimer) : _id(id), _timer(newtimer) {}

  timer_id_couple::timer_id_couple(unsigned long id) : _id(id) {
    _timer = millis();
  }

  bool timer_id_couple::operator<(const timer_id_couple& other) {
    const unsigned long now(millis());

    // timediff > 0, means timer has already passed
    return timeDiff(_timer, now) > timeDiff(other._timer, now);
  }
