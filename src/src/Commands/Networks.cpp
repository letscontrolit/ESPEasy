#include "../Commands/Networks.h"

#include "../../ESPEasy/net/DataTypes/NetworkIndex.h"
#include "../../ESPEasy/net/ESPEasyNetwork.h"
#include "../../ESPEasy/net/Globals/NetworkState.h"
#include "../../ESPEasy/net/Helpers/NWAccessControl.h"
#include "../../ESPEasy/net/eth/ESPEasyEth.h"
#include "../../ESPEasy_common.h"
#include "../Commands/Common.h"
#include "../Globals/Settings.h"
#include "../Helpers/StringConverter.h"
#include "../WebServer/AccessControl.h"

#if FEATURE_ETHERNET
#include <ETH.h>
#endif

#if FEATURE_STORE_NETWORK_INTERFACE_SETTINGS
#include "../Helpers/KeyValueWriter_JSON.h"
#include "../../ESPEasy/net/Helpers/NWPlugin_import_export.h"
#endif

//      networkIndex = (event->Par1 - 1);   Par1 is here for 1 ... NETWORK_MAX
bool validNetworkVar(struct EventStruct *event, ESPEasy::net::networkIndex_t& networkIndex)
{
  if (event->Par1 <= 0) { return false; }
  networkIndex = static_cast<ESPEasy::net::networkIndex_t>(event->Par1 - 1);
  return validNetworkIndex(networkIndex);
}

const __FlashStringHelper * Command_Network_Disable(struct EventStruct *event, const char *Line)
{
  ESPEasy::net::networkIndex_t networkIndex;
  return return_command_boolean_result_flashstr(validNetworkVar(event, networkIndex) && setNetworkEnableStatus(networkIndex, false));
}

const __FlashStringHelper * Command_Network_Enable(struct EventStruct *event, const char *Line)
{
  ESPEasy::net::networkIndex_t networkIndex;
  return return_command_boolean_result_flashstr(validNetworkVar(event, networkIndex) && setNetworkEnableStatus(networkIndex, true));
}


String Command_AccessInfo_Ls(struct EventStruct *event, const char* Line)
{
  return return_result(event, concat(F("Allowed IP range : "), ESPEasy::net::describeAllowedIPrange()));
}

String Command_AccessInfo_Clear (struct EventStruct *event, const char* Line)
{
  clearAccessBlock();
  return Command_AccessInfo_Ls(event, Line);
}

String Command_DNS (struct EventStruct *event, const char* Line)
{
  return Command_GetORSetIP(event, F("DNS:"), Line, Settings.DNS, ESPEasy::net::NetworkDnsIP(0), 1);
}

String Command_Gateway (struct EventStruct *event, const char* Line)
{
  return Command_GetORSetIP(event, F("Gateway:"), Line, Settings.Gateway, ESPEasy::net::NetworkGatewayIP(),1);
}

String Command_IP (struct EventStruct *event, const char* Line)
{
  return Command_GetORSetIP(event, F("IP:"), Line, Settings.IP, ESPEasy::net::NetworkLocalIP(),1);
}

#if FEATURE_USE_IPV6
String Command_show_all_IP6 (struct EventStruct *event, const char* Line)
{
  // Only get all IPv6 addresses
  IP6Addresses_t addresses = ESPEasy::net::NetworkAllIPv6();
  String res;
  res += '[';
  bool first = true;
  for (auto it = addresses.begin(); it != addresses.end(); ++it)
  {
    if (first) {
      first = false;
    } else {
      res += ',';
    }
    res += wrap_String(it->toString(true), '"');
  }
  res += ']';
  return res;
}
#endif


String Command_Subnet (struct EventStruct *event, const char* Line)
{
  return Command_GetORSetIP(event, F("Subnet:"), Line, Settings.Subnet, ESPEasy::net::NetworkSubnetMask(), 1);
}

#if FEATURE_ETHERNET
String Command_ETH_IP (struct EventStruct *event, const char* Line)
{
  return Command_GetORSetIP(event, F("ETH_IP:"), Line, Settings.ETH_IP,ETH.localIP(),1);
}

String Command_ETH_Gateway (struct EventStruct *event, const char* Line)
{
  return Command_GetORSetIP(event, F("ETH_Gateway:"), Line, Settings.ETH_Gateway,ETH.gatewayIP(),1);
}

String Command_ETH_Subnet (struct EventStruct *event, const char* Line)
{
  return Command_GetORSetIP(event, F("ETH_Subnet:"), Line, Settings.ETH_Subnet,ETH.subnetMask(),1);
}

String Command_ETH_DNS (struct EventStruct *event, const char* Line)
{
  return Command_GetORSetIP(event, F("ETH_DNS:"), Line, Settings.ETH_DNS,ETH.dnsIP(),1);
}

String Command_ETH_Wifi_Mode (struct EventStruct *event, const char* Line)
{
  const ESPEasy::net::NetworkMedium_t orig_medium = Settings.NetworkMedium;
  const String result = Command_GetORSetETH(event, 
                             F("NetworkMedium:"), 
                             toString(active_network_medium),
                             Line, 
                             reinterpret_cast<uint8_t*>(&Settings.NetworkMedium), 
                             1);
  if (orig_medium != Settings.NetworkMedium) {
    if (!isValid(Settings.NetworkMedium)) {
      Settings.NetworkMedium = orig_medium;
      return return_command_failed();
    }
    ESPEasy::net::setNetworkMedium(Settings.NetworkMedium);
  }
  
  return result;
}

String Command_ETH_Disconnect (struct EventStruct *event, const char* Line)
{

  // FIXME TD-er: Must implement support for multiple and default eth interface
/*
  ESPEasy::net::eth::ethPower(0);
  delay(400);
//  ethPower(1);
  ESPEasy::net::setNetworkMedium(ESPEasy::net::NetworkMedium_t::Ethernet);
  ESPEasy::net::eth::ETHConnectRelaxed();
*/
  return return_command_success();
}

#endif // if FEATURE_ETHERNET


#if FEATURE_STORE_NETWORK_INTERFACE_SETTINGS

String Command_Network_ExportConfig(struct EventStruct *event, const char*Line)
{
  ESPEasy::net::networkIndex_t networkIndex;

  if (!validNetworkVar(event, networkIndex)) { return return_command_failed(); }

  PrintToString p2s;
  {
    KeyValueWriter_JSON writer(false, &p2s);

    String res = ESPEasy::net::NWPlugin_import_export::exportConfig(networkIndex, &writer);

    if (!res.isEmpty()) { return res; }
  }
  return p2s.getMove();
}

String Command_Network_ImportConfig(struct EventStruct *event, const char*Line)
{
  ESPEasy::net::networkIndex_t networkIndex;

  if (!validNetworkVar(event, networkIndex)) { return return_command_failed(); }
  const String json = parseStringToEndKeepCase(Line, 3);

  String res = ESPEasy::net::NWPlugin_import_export::importConfig(networkIndex, json);

  if (!res.isEmpty()) { return res; }

  return return_command_success();
}

#endif // if FEATURE_STORE_NETWORK_INTERFACE_SETTINGS
