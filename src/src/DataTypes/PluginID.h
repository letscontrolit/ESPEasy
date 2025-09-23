#ifndef DATATYPES_PLUGINID_H
#define DATATYPES_PLUGINID_H

#include "../../ESPEasy_common.h"

#if FEATURE_SUPPORT_OVER_255_PLUGINS
#define PLUGINID_BASE_TYPE  uint16_t
#else
#define PLUGINID_BASE_TYPE  uint8_t
#endif



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

  // Max value = 4095, due to how the bits are stored in C016_binary_element
  PLUGINID_BASE_TYPE value{};
};

// Invalid PluginID value = 0
extern const pluginID_t INVALID_PLUGIN_ID;

#endif // ifndef DATATYPES_PLUGINID_H
