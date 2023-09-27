#include "../DataTypes/PluginID.h"

#include "../Helpers/StringConverter.h"


String pluginID_t::toDisplayString() const {
  if (value == 0) { return F("P---"); }
  return strformat(F("P%03d"), value);
}

const pluginID_t INVALID_PLUGIN_ID;
