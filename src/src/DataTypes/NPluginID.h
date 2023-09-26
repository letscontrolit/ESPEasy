#ifndef DATATYPES_NPLUGINID_H
#define DATATYPES_NPLUGINID_H

#include "../../ESPEasy_common.h"

struct npluginID_t {
  constexpr npluginID_t() = default;

  npluginID_t(const npluginID_t& other)
  {
    value = other.value;
  }

  constexpr explicit npluginID_t(uint8_t id) : value(id) {}

  static npluginID_t toPluginID(unsigned other)
  {
    npluginID_t res;

    if (other <= 255) { res.value = other; }
    return res;
  }

  npluginID_t& operator=(const npluginID_t& other)
  {
    value = other.value;
    return *this;
  }

  bool operator==(const npluginID_t& other) const
  {
    return this->value == other.value;
  }

  bool operator!=(const npluginID_t& other) const
  {
    return this->value != other.value;
  }

  void setInvalid()
  {
    value = 0;
  }

  String toDisplayString() const;

  uint8_t value{};
};


extern const npluginID_t INVALID_N_PLUGIN_ID;


#endif // ifndef DATATYPES_NPLUGINID_H
