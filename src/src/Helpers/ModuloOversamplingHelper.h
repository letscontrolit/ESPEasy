#ifndef HELPERS_MODULOOVERSAMPLINGHELPER_H
#define HELPERS_MODULOOVERSAMPLINGHELPER_H

#include "../../ESPEasy_common.h"
#include <limits>
#include "../Helpers/OversamplingHelper.h"

template<class T, class SUM_VALUE_TYPE = float>
class ModuloOversamplingHelper {
public:

  // When working with cyclical values which may 'overflow',
  // it is hard to compute an average.
  // For example with some angle in degrees, 359 and 1 degree are very close to eachother.
  // The expected average would be 0, however the numerical average is 180.
  //
  // Typical use cases:
  // - Compute direction in degrees (range 0 .. 359)
  // - Compute 'jitter' in some periodical signal
  // - Phase shift calculations
  ModuloOversamplingHelper() {
    // Just set some value for _modulo 
    // so there will be no overflow when applying the offset
    // And we have a default constructor.
    // However it is highly unlikely this is a practical value, 
    // so should not be run with this default value
    _modulo = std::numeric_limits<T>::max() / 2;
  }

  ModuloOversamplingHelper(T modulo) : _modulo(modulo) {}

  void setModulo(T modulo) {
    _modulo = modulo;
    reset();
  }

  // Add new sample
  void add(T currentValue) {
    // Make sure the value is in range 0 ... (_modulo - 1)
    currentValue %= _modulo;

    if (_oversampling.getCount() == 0) {
      // Work with values centered around the middle of the modulo range.
      // Since the template type T can be unsigned,
      // must make sure the offset is positive.
      // N.B. _offset is always less than or equal to (_modulo / 2)
      const T modulo_halve = (_modulo / 2);

      if (currentValue > modulo_halve) {
        _offset = currentValue - modulo_halve;
      } else {
        _offset = modulo_halve - currentValue;
      }
    }

    T shiftedValue = currentValue + _offset; // FIXME TD-er: Could overflow

    if (shiftedValue > _modulo) {
      shiftedValue -= _modulo;
    }

    _oversampling.add(shiftedValue);
  }

  // Get current oversampling value without reset.
  // @param value  Value will only be updated if there were samples available
  bool peek(SUM_VALUE_TYPE& value) const {
    if (!_oversampling.peek(value)) {
      return false;
    }

    if (value < _offset) {
      value += _modulo;
    }
    value -= _offset;
    return true;
  }

  // Get current oversampling value and reset.
  // @param value  Value will only be updated if there were samples available
  bool get(SUM_VALUE_TYPE& value) {
    // Must call functions of this class and not just return _oversampling.get()
    // This way we are sure the _offset is properly applied
    if (peek(value)) {
      reset();
      return true;
    }
    return false;
  }

  // Return number of used samples
  uint32_t getCount() const {
    return _oversampling.getCount();
  }

  // Clear all oversampling values
  void reset() {
    _oversampling.reset();
  }

  // Clear all oversampling values and add last average if there were samples available.
  SUM_VALUE_TYPE resetKeepLast() {
    SUM_VALUE_TYPE value{};

    // Must call functions of this class and not just return _oversampling.resetKeepLast()
    // This way we are sure the _offset is properly applied
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

    // Must call functions of this class and not just return _oversampling.resetKeepLast()
    // This way we are sure the _offset is properly applied
    if (get(value)) {
      if (count > countRatio) {
        const uint32_t weight = (count + (countRatio / 2)) / countRatio;

        for (uint32_t i = 0; i < weight; ++i) {
          add(static_cast<T>(value));
        }
      } else {
        add(static_cast<T>(value));
      }
    }
  }

  void setFilterPeaks(bool enable) {
    _oversampling.setFilterPeaks(enable);
  }

private:

  OversamplingHelper<T, SUM_VALUE_TYPE>_oversampling;
  T _modulo;
  T _offset{};
};

#endif // ifndef HELPERS_MODULOOVERSAMPLINGHELPER_H
