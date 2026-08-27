#include "../Helpers/NWAccessControl.h"

#include "../ESPEasyNetwork.h"
#include "../Globals/NWPlugins.h"

#include "../../../src/Globals/SecuritySettings.h"
#include "../../../src/Helpers/StringConverter.h"

namespace ESPEasy {
namespace net {


// ********************************************************************************
// Allowed IP range check
// ********************************************************************************

bool ipInAllowedSubnet(const IPAddress& ip)
{
  String ip_str = ip.toString();

  for (networkIndex_t x = 0; x < NETWORK_MAX; x++) {
    EventStruct tempEvent;
    tempEvent.NetworkIndex = x;

    if (NWPluginCall(NWPlugin::Function::NWPLUGIN_CLIENT_IP_WEB_ACCESS_ALLOWED, &tempEvent, ip_str)) {
      return true;
    }
  }
  return false;
}

String describeAllowedIPrange() {
  return describeAllowedIPrange(ESPEasy::net::NetworkLocalIP());
}

String describeAllowedIPrange(const IPAddress& ip) {
  if (SecuritySettings.IPblockLevel == ALL_ALLOWED) {
    return F("All Allowed");
  }
  String allowedRange;
  String ip_str = ip.toString();

  for (networkIndex_t x = 0; x < NETWORK_MAX; x++) {
    EventStruct tempEvent;
    tempEvent.NetworkIndex = x;

    if (NWPluginCall(NWPlugin::Function::NWPLUGIN_CLIENT_IP_WEB_ACCESS_ALLOWED, &tempEvent, ip_str)) {
      if (!tempEvent.String1.isEmpty() && !tempEvent.String2.isEmpty()) {

        if (!allowedRange.isEmpty()) { allowedRange += F(", "); }
        allowedRange +=  tempEvent.String1;
        allowedRange +=  F(" - ");
        allowedRange +=  tempEvent.String2;
      }
    }
  }
  return allowedRange;
}

} // namespace net
} // namespace ESPEasy
