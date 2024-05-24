#ifndef DATATYPES_PLUGINID_H
#define DATATYPES_PLUGINID_H

#include "../../ESPEasy_common.h"

struct __attribute__((__packed__)) pluginID_t {
  pluginID_t() = default;

  pluginID_t(const pluginID_t& other)
  {
    value = other.value;
  }

  constexpr explicit pluginID_t(uint8_t id) : value(id) {}


  static pluginID_t toPluginID(unsigned other)
  {
    pluginID_t res;

    if (other <= 255) { res.value = other; }

    return res;
  }

  pluginID_t& operator=(const pluginID_t& other)
  {
    value = other.value;
    return *this;
  }

  bool operator==(const pluginID_t& other) const
  {
    return this->value == other.value;
  }

  bool operator!=(const pluginID_t& other) const
  {
    return this->value != other.value;
  }

  void setInvalid() {
    value = 0;
  }

  String toDisplayString() const;

  uint8_t value{};
};


extern const pluginID_t INVALID_PLUGIN_ID;

#endif // ifndef DATATYPES_PLUGINID_H
