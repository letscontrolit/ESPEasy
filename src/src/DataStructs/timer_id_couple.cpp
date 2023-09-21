#include "../DataStructs/timer_id_couple.h"

#include "../Helpers/ESPEasy_time_calc.h"

bool timer_id_couple::operator<(const timer_id_couple& other) const {
  const unsigned long now(millis());

  // timediff > 0, means timer has already passed
  return timeDiff(_timer, now) > timeDiff(other._timer, now);
}
