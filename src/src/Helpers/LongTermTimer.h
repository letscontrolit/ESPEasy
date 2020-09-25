#ifndef HELPERS_LONGTERMTIMER_H
#define HELPERS_LONGTERMTIMER_H

#include <Arduino.h>

class LongTermTimer {
public:

  typedef int64_t Duration;

  LongTermTimer() {}

  explicit LongTermTimer(uint64_t start_time) : _timer(start_time) {}

  inline bool operator<(const LongTermTimer& rhs) const
  {
    return _timer < rhs.get();
  }

  inline bool operator>(const LongTermTimer& rhs) const
  {
    return _timer > rhs.get();
  }

  LongTermTimer& operator=(const LongTermTimer& rhs)
  {
    _timer = rhs.get();
    return *this;
  }

  void clear() {
    _timer = 0ull;
  }

  void set(uint64_t start_time) {
    _timer = start_time;
  }

  bool isSet() const {
    return _timer > 0ull;
  }

  uint64_t ICACHE_RAM_ATTR get() const;

  void ICACHE_RAM_ATTR     setNow();

  // Positive when next is past this time value.
  Duration ICACHE_RAM_ATTR timeDiff(const LongTermTimer& next) const;

  Duration ICACHE_RAM_ATTR usecPassedSince() const;
  Duration ICACHE_RAM_ATTR millisPassedSince() const;

  bool ICACHE_RAM_ATTR     timeReached() const;

  bool ICACHE_RAM_ATTR     timeoutReached(uint32_t millisTimeout) const;

private:

  static Duration ICACHE_RAM_ATTR __timeDiff(uint64_t prev,
                                             uint64_t next);

  uint64_t _timer = 0ull;
};

#endif // HELPERS_LONGTERMTIMER_H
