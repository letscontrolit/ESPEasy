#include "../DataTypes/NPluginID.h"

#include "../Helpers/StringConverter.h"

String npluginID_t::toDisplayString() const {
  if (value == 0) { return F("N---"); }
  return strformat(F("N%03d"), value);
}

const npluginID_t INVALID_N_PLUGIN_ID;
