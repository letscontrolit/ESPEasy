#include "LongTermTimer.h"

#include "ESPEasy_time_calc.h"


uint64_t LongTermTimer::get() const {
  return _timer_usec;
}

void LongTermTimer::setNow() {
  _timer_usec = getMicros64();
}

LongTermTimer::Duration LongTermTimer::timeDiff(const LongTermTimer& next) const {
  return __timeDiff(_timer_usec, next.get());
}

LongTermTimer::Duration LongTermTimer::usecPassedSince() const {
  return __timeDiff(_timer_usec, getMicros64());
}

LongTermTimer::Duration LongTermTimer::millisPassedSince() const {
  return usecPassedSince() / 1000ll;
}

bool LongTermTimer::timeReached() const {
  return getMicros64() > _timer_usec;
}

bool LongTermTimer::timeoutReached(uint32_t millisTimeout) const {
  return getMicros64() > (_timer_usec + (millisTimeout * 1000ull));
}

LongTermTimer::Duration LongTermTimer::__timeDiff(uint64_t prev, uint64_t next) {
  return static_cast<int64_t>(next) - static_cast<int64_t>(prev);
}
