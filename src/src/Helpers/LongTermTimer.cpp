#include "../Helpers/LongTermTimer.h"

LongTermTimer::LongTermTimer(const LongTermTimer& rhs) {
    _timer_usec = rhs.get();
}

LongTermTimer::LongTermTimer(bool usenow) : _timer_usec(0ull) {
    if (usenow) setNow();
}


  LongTermTimer& LongTermTimer::operator=(const LongTermTimer& rhs)
  {
    _timer_usec = rhs.get();
    return *this;
  }

  void LongTermTimer::setMillisFromNow(uint32_t millisFromNow) {
    _timer_usec = getMicros64() + (millisFromNow * 1000ull);
  }

  void LongTermTimer::setNow() {
    _timer_usec = getMicros64();
  }

  LongTermTimer::Duration LongTermTimer::millisPassedSince() const {
    return usecPassedSince() / 1000ll;
  }

  bool LongTermTimer::timeoutReached(uint32_t millisTimeout) const {
    return getMicros64() > (_timer_usec + (millisTimeout * 1000ull));
  }
