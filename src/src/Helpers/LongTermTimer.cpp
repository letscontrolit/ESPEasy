#include "LongTermTimer.h"

#include "ESPEasy_time_calc.h"


uint64_t LongTermTimer::get() const {
  return _timer;
}

void LongTermTimer::setNow() {
  _timer = getMicros64();
}

LongTermTimer::Duration LongTermTimer::timeDiff(const LongTermTimer& next) const {
  return __timeDiff(_timer, next.get());
}

LongTermTimer::Duration LongTermTimer::usecPassedSince() const {
  return __timeDiff(_timer, getMicros64());
}

LongTermTimer::Duration LongTermTimer::millisPassedSince() const {
  return usecPassedSince() / 1000ll;
}

bool LongTermTimer::timeReached() const {
  return getMicros64() > _timer;
}

bool LongTermTimer::timeoutReached(uint32_t millisTimeout) const {
  return getMicros64() > (_timer + (millisTimeout * 1000ull));
}

LongTermTimer::Duration LongTermTimer::__timeDiff(uint64_t prev, uint64_t next) {
  return static_cast<int64_t>(next) - static_cast<int64_t>(prev);
}
