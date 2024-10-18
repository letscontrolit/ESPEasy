#ifndef DATATYPES_DEVICEINDEX_H
#define DATATYPES_DEVICEINDEX_H

#include "../../ESPEasy_common.h"

#include "../CustomBuild/ESPEasyLimits.h"

#if FEATURE_SUPPORT_OVER_255_PLUGINS
#define DEVICEINDEX_BASE_TYPE  uint16_t
#else
#define DEVICEINDEX_BASE_TYPE  uint8_t
#endif

struct deviceIndex_t {
  deviceIndex_t() = default;

  deviceIndex_t(const deviceIndex_t& other)
  {
    value = other.value;
  }

  static deviceIndex_t toDeviceIndex(unsigned other)
  {  
    deviceIndex_t res;
    res = other;
    return res;
  }

  // TD-er: Do not add constructor with int as argument,
  // as this makes it impossible for the compiler to typecheck its use.
  //  deviceIndex_t(int other);

  deviceIndex_t& operator=(unsigned other)  
  {
    value = (other < DEVICE_INDEX_MAX) ? other : DEVICE_INDEX_MAX;
    return *this;
  }


  deviceIndex_t& operator=(const deviceIndex_t& other)
  {
    value = other.value;
    return *this;
  }

  // TD-er: Using operator unsigned() makes it impossible for the compiler to check for types.
  // However, since the Device[] array is accessed using a deviceIndex_t, we need this operator unsigned()
  // on ESP8266.
  // On ESP32 we have a strongly typed DeviceVector class and thus we can properly check per operator.
  #ifndef ESP8266
  bool operator<(unsigned other) const { return value < other; }
  bool operator!=(unsigned other) const { return value != other; }
  bool operator!=(const deviceIndex_t& other) const { return value != other.value; }

  #else // ifndef ESP8266
  operator unsigned() const  { return value; }
  #endif // ifndef ESP8266

  deviceIndex_t& operator++()
  {
    // pre-increment, ++a
    ++value;
    return *this;
  }

  // FIXME TD-er: This should also be set to 16 bit if we support > 255 plugins, 
  // we should also support builds with > 255 plugins included
  DEVICEINDEX_BASE_TYPE value{}; // Init this to 0, so we can easily iterate over it.
};


extern deviceIndex_t INVALID_DEVICE_INDEX;

#endif // ifndef DATATYPES_DEVICEINDEX_H
