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
  String reply;

  switch (SecuritySettings.IPblockLevel)
  {
    case ALL_ALLOWED:
      reply +=  F("All Allowed");
      break;
    default:
    {
      IPAddress low, high;
      getIPallowedRange(low, high);
      reply +=  formatIP(low);
      reply +=  F(" - ");
      reply +=  formatIP(high);
    }
  }
  return reply;
}

bool getIPallowedRange(IPAddress& low, IPAddress& high)
{
  switch (SecuritySettings.IPblockLevel)
  {
    case LOCAL_SUBNET_ALLOWED:
      low  = NetworkID();
      high = NetworkBroadcast();
      return true;
    case ONLY_IP_RANGE_ALLOWED:
      low  = IPAddress(SecuritySettings.AllowedIPrangeLow);
      high = IPAddress(SecuritySettings.AllowedIPrangeHigh);
      break;
    default:
      low  = IPAddress(0, 0, 0, 0);
      high = IPAddress(255, 255, 255, 255);
      return false;
  }
  return true;
}

} // namespace net
} // namespace ESPEasy
