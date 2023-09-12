#ifndef DATATYPES_PLUGINID_H
#define DATATYPES_PLUGINID_H

#include "../../ESPEasy_common.h"

struct pluginID_t {
    pluginID_t() = default;

  static pluginID_t toPluginID(unsigned other);

  pluginID_t& operator=(const pluginID_t& other);

  bool operator==(const pluginID_t& other) const;
  bool operator!=(const pluginID_t& other) const;

  void setInvalid();

  String toDisplayString() const;

  uint8_t value{};
};



extern const pluginID_t INVALID_PLUGIN_ID;

#endif // ifndef DATATYPES_PLUGINID_H
