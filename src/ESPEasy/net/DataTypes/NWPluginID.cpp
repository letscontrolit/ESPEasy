#include "../DataTypes/NWPluginID.h"

#include "../../../src/Helpers/StringConverter.h"

namespace ESPEasy {
namespace net {

String nwpluginID_t::toDisplayString() const {
  if (value == 0) { return F("NW---"); }
  return strformat(F("NW%03d"), value);
}

const nwpluginID_t INVALID_NW_PLUGIN_ID{};

} // namespace net
} // namespace ESPEasy
