#pragma once

#include "../../../ESPEasy_common.h"

#include "../../../src/CustomBuild/ESPEasyLimits.h"

namespace ESPEasy {
namespace net {

struct networkDriverIndex_t {
  networkDriverIndex_t() = default;

  networkDriverIndex_t(const networkDriverIndex_t& other)
  {
    value = other.value;
  }

  static networkDriverIndex_t toNetworkDriverIndex(unsigned other)
  {
    networkDriverIndex_t res;
    res = other;
    return res;
  }

  // TD-er: Do not add constructor with int as argument,
  // as this makes it impossible for the compiler to typecheck its use.
  //  networkDriverIndex_t(int other);

  networkDriverIndex_t& operator=(unsigned other)
  {
    value = (other < NETWORKDRIVER_INDEX_MAX) ? other : NETWORKDRIVER_INDEX_MAX;
    return *this;
  }

  networkDriverIndex_t& operator=(const networkDriverIndex_t& other)
  {
    value = other.value;
    return *this;
  }

  // TD-er: Using operator unsigned() makes it impossible for the compiler to check for types.
  // However, since the NetworkDriver[] array is accessed using a networkDriverIndex_t, we need this operator unsigned()
  // on ESP8266.
  // On ESP32 we have a strongly typed NetworkDriverVector class and thus we can properly check per operator.
  #ifndef ESP8266
  bool operator<(unsigned other) const                     { return value < other; }

  bool operator!=(unsigned other) const                    { return value != other; }

  bool operator!=(const networkDriverIndex_t& other) const { return value != other.value; }

  #else // ifndef ESP8266
  operator unsigned() const  {
    return value;
  }
  #endif // ifndef ESP8266

  networkDriverIndex_t& operator++()
  {
    // pre-increment, ++a
    ++value;
    return *this;
  }

  uint8_t value{}; // Init this to 0, so we can easily iterate over it.

};


extern networkDriverIndex_t INVALID_NETWORKDRIVER_INDEX;

} // namespace net
} // namespace ESPEasy
