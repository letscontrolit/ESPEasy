#ifndef HELPERS_LONGTERMTIMER_H
#define HELPERS_LONGTERMTIMER_H

#include <Arduino.h>

#include "../Helpers/ESPEasy_time_calc.h"

class LongTermTimer {
public:

  typedef int64_t Duration;

  LongTermTimer() = default;

  explicit LongTermTimer(const LongTermTimer& rhs);

  explicit LongTermTimer(bool usenow);

  //explicit LongTermTimer(uint64_t start_time) : _timer_usec(start_time) {}

  inline bool operator<(const LongTermTimer& rhs) const
  {
    return _timer_usec < rhs.get();
  }

  inline bool operator>(const LongTermTimer& rhs) const
  {
    return _timer_usec > rhs.get();
  }

  LongTermTimer& operator=(const LongTermTimer& rhs);

  void clear() {
    _timer_usec = 0ull;
  }

  void set(uint64_t start_time) {
    _timer_usec = start_time;
  }

  void setMillisFromNow(uint32_t millisFromNow);

  bool isSet() const {
    return _timer_usec > 0ull;
  }

  uint64_t get() const {
    return _timer_usec;
  }

  void setNow();

  // Positive when next is past this time value.
  Duration timeDiff(const LongTermTimer& next) const {
    return __timeDiff(_timer_usec, next.get());
  }

  Duration usecPassedSince() const {
    return __timeDiff(_timer_usec, getMicros64());
  }

  Duration millisPassedSince() const;

  bool timeReached() const {
    return getMicros64() > _timer_usec;
  }

  bool timeoutReached(uint32_t millisTimeout) const;

private:

  static Duration __timeDiff(uint64_t prev, uint64_t next) {
    return static_cast<int64_t>(next) - static_cast<int64_t>(prev);
  }

  uint64_t _timer_usec{};
};

#endif // HELPERS_LONGTERMTIMER_H
