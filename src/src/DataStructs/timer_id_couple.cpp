#include "../DataStructs/timer_id_couple.h"

#include "../Helpers/ESPEasy_time_calc.h"

bool timer_id_couple::operator<(const timer_id_couple& other) const {
  // timeDiff is positive when _timer is before other._timer
  return timeDiff(_timer, other._timer) > 0;
}

bool timer_id_couple::operator()(const timer_id_couple& item) const {
  return _id == item._id;
}
