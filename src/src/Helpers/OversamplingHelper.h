#ifndef HELPERS_OVERSAMPLINGHELPER_H
#define HELPERS_OVERSAMPLINGHELPER_H

#include "../../ESPEasy_common.h"
#include <limits>

template<class T, class SUM_VALUE_TYPE=float>
class OversamplingHelper {
public:

  // Oversampling by taking the average over N samples.
  // Will filter out peak values when filterPeaksEnabled is set.
  OversamplingHelper() = default;

  // Add new sample
  void add(T currentValue) {
    _sum += currentValue;
    ++_count;

    if (currentValue > _maxval) {
      _maxval = currentValue;
    }

    if (currentValue < _minval) {
      _minval = currentValue;
    }
  }

  // Get current oversampling value without reset.
  // @param value  Value will only be updated if there were samples available
  bool peek(SUM_VALUE_TYPE& value) const {
    if (_count == 0) { return false; }
    SUM_VALUE_TYPE sum      = _sum;
    uint32_t count = _count;

    if ((count >= 3) && _filterPeaks) {
      // Remove peak values from the average
      sum   -= _maxval;
      sum   -= _minval;
      count -= 2;
    }
    value  = sum / count;
    return true;
  }

  // Get current oversampling value and reset.
  // @param value  Value will only be updated if there were samples available
  bool get(SUM_VALUE_TYPE& value) {
    if (peek(value)) {
      reset();
      return true;
    }
    return false;
  }

  // Return number of used samples
  uint32_t getCount() const {
    return _count;
  }

  // Clear all oversampling values
  void reset() {
    _count  = 0;
    _sum    = 0.0f;
    _minval = std::numeric_limits<T>::max();
    _maxval = std::numeric_limits<T>::min();
  }

  // Clear all oversampling values and add last average if there were samples available.
  SUM_VALUE_TYPE resetKeepLast() {
    SUM_VALUE_TYPE value{};

    if (get(value)) {
      add(static_cast<T>(value));
    }
    return value;
  }

  // Clear all oversampling values and add last average if there were samples available.
  // Last value will be added with a weight according to the given ratio of the last count.
  // @param countRatio  New weight will be previous nr of samples / countRatio
  void resetKeepLastWeighted(int countRatio) {
    SUM_VALUE_TYPE value{};
    const uint32_t count = getCount();

    if (get(value)) {
      add(static_cast<T>(value));
      if (count > countRatio) {
        const uint32_t weight = (count + (countRatio / 2)) / countRatio;
        _count *= weight;
        _sum = value * _count;
      }
    }
  }

  void setFilterPeaks(bool enable) {
    _filterPeaks = enable;
  }

private:

  uint32_t _count{};
  SUM_VALUE_TYPE _sum{};
  T _minval         = std::numeric_limits<T>::max();
  T _maxval         = std::numeric_limits<T>::min();
  bool _filterPeaks = true;
};

#endif // ifndef HELPERS_OVERSAMPLINGHELPER_H
