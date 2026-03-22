#pragma once

#include "../../../ESPEasy_common.h"

namespace ESPEasy {
namespace net {

struct __attribute__((__packed__)) nwpluginID_t {
  nwpluginID_t() = default;

  nwpluginID_t(const nwpluginID_t& other)
  {
    value = other.value;
  }

  constexpr explicit nwpluginID_t(uint8_t id) : value(id) {}

  static nwpluginID_t toPluginID(unsigned other)
  {
    nwpluginID_t res;

    if (other <= 255) { res.value = other; }

    return res;
  }

  bool isValid() const
  {
    return value != 0;
  }

  operator int() const
  {
    return value;
  }

  operator bool() const
  {
    return isValid();
  }

  nwpluginID_t& operator=(const nwpluginID_t& other)
  {
    value = other.value;
    return *this;
  }

  bool operator==(const nwpluginID_t& other) const
  {
    return this->value == other.value;
  }

  bool operator!=(const nwpluginID_t& other) const
  {
    return this->value != other.value;
  }

  void   setInvalid() { value = 0; }

  String toDisplayString() const;

  uint8_t value{};

};


extern const nwpluginID_t INVALID_NW_PLUGIN_ID;

} // namespace net
} // namespace ESPEasy
