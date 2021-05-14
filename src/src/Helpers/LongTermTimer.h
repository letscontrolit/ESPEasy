#ifndef HELPERS_LONGTERMTIMER_H
#define HELPERS_LONGTERMTIMER_H

#include <Arduino.h>

#include "ESPEasy_time_calc.h"

class LongTermTimer {
public:

  typedef int64_t Duration;

  LongTermTimer() {}

  explicit LongTermTimer(const LongTermTimer& rhs) {
    _timer_usec = rhs.get();
  }

  explicit LongTermTimer(bool usenow) : _timer_usec(0ull) {
    if (usenow) setNow();
  }

  //explicit LongTermTimer(uint64_t start_time) : _timer_usec(start_time) {}

  inline bool operator<(const LongTermTimer& rhs) const
  {
    return _timer_usec < rhs.get();
  }

  inline bool operator>(const LongTermTimer& rhs) const
  {
    return _timer_usec > rhs.get();
  }

  LongTermTimer& operator=(const LongTermTimer& rhs)
  {
    _timer_usec = rhs.get();
    return *this;
  }

  void clear() {
    _timer_usec = 0ull;
  }

  void set(uint64_t start_time) {
    _timer_usec = start_time;
  }

  void setMillisFromNow(uint32_t millisFromNow) {
    _timer_usec = getMicros64() + (millisFromNow * 1000ull);
  }

  bool isSet() const {
    return _timer_usec > 0ull;
  }

  uint64_t get() const {
    return _timer_usec;
  }

  void setNow() {
    _timer_usec = getMicros64();
  }

  // Positive when next is past this time value.
  Duration timeDiff(const LongTermTimer& next) const {
    return __timeDiff(_timer_usec, next.get());
  }

  Duration usecPassedSince() const {
    return __timeDiff(_timer_usec, getMicros64());
  }

  Duration millisPassedSince() const {
    return usecPassedSince() / 1000ll;
  }

  bool timeReached() const {
    return getMicros64() > _timer_usec;
  }

  bool timeoutReached(uint32_t millisTimeout) const {
    return getMicros64() > (_timer_usec + (millisTimeout * 1000ull));
  }

private:

  static Duration __timeDiff(uint64_t prev, uint64_t next) {
    return static_cast<int64_t>(next) - static_cast<int64_t>(prev);
  }

  uint64_t _timer_usec = 0ull;
};

#endif // HELPERS_LONGTERMTIMER_H
