#include "../Commands/Networks.h"

#include "../../ESPEasy_common.h"
#include "../Commands/Common.h"
#include "../ESPEasyCore/ESPEasyNetwork.h"
#include "../ESPEasyCore/ESPEasyEth.h"
#include "../Globals/NetworkState.h"
#include "../Globals/Settings.h"
#include "../Helpers/StringConverter.h"
#include "../WebServer/AccessControl.h"


#if FEATURE_ETHERNET
#include <ETH.h>
#endif

String Command_AccessInfo_Ls(struct EventStruct *event, const char* Line)
{
  return return_result(event, concat(F("Allowed IP range : "), describeAllowedIPrange()));
}

String Command_AccessInfo_Clear (struct EventStruct *event, const char* Line)
{
  clearAccessBlock();
  return Command_AccessInfo_Ls(event, Line);
}

String Command_DNS (struct EventStruct *event, const char* Line)
{
  return Command_GetORSetIP(event, F("DNS:"), Line, Settings.DNS, NetworkDnsIP(0), 1);
}

String Command_Gateway (struct EventStruct *event, const char* Line)
{
  return Command_GetORSetIP(event, F("Gateway:"), Line, Settings.Gateway, NetworkGatewayIP(),1);
}

String Command_IP (struct EventStruct *event, const char* Line)
{
  return Command_GetORSetIP(event, F("IP:"), Line, Settings.IP, NetworkLocalIP(),1);
}

String Command_Subnet (struct EventStruct *event, const char* Line)
{
  return Command_GetORSetIP(event, F("Subnet:"), Line, Settings.Subnet, NetworkSubnetMask(), 1);
}

#if FEATURE_ETHERNET
String Command_ETH_Phy_Addr (struct EventStruct *event, const char* Line)
{
  return Command_GetORSetInt8_t(event, F("ETH_Phy_Addr:"), Line, reinterpret_cast<int8_t*>(&Settings.ETH_Phy_Addr),1);
}

String Command_ETH_Pin_mdc (struct EventStruct *event, const char* Line)
{
  return Command_GetORSetInt8_t(event, F("ETH_Pin_mdc:"), Line, reinterpret_cast<int8_t*>(&Settings.ETH_Pin_mdc),1);
}

String Command_ETH_Pin_mdio (struct EventStruct *event, const char* Line)
{
  return Command_GetORSetInt8_t(event, F("ETH_Pin_mdio:"), Line, reinterpret_cast<int8_t*>(&Settings.ETH_Pin_mdio),1);
}

String Command_ETH_Pin_power (struct EventStruct *event, const char* Line)
{
  return Command_GetORSetInt8_t(event, F("ETH_Pin_power:"), Line, reinterpret_cast<int8_t*>(&Settings.ETH_Pin_power),1);
}

String Command_ETH_Phy_Type (struct EventStruct *event, const char* Line)
{
  return Command_GetORSetInt8_t(event, F("ETH_Phy_Type:"), Line, reinterpret_cast<int8_t*>(&Settings.ETH_Phy_Type),1);
}

String Command_ETH_Clock_Mode (struct EventStruct *event, const char* Line)
{
  return Command_GetORSetETH(event, 
                             F("ETH_Clock_Mode:"), 
                             toString(Settings.ETH_Clock_Mode),
                             Line, 
                             reinterpret_cast<uint8_t*>(&Settings.ETH_Clock_Mode),
                             1);
}

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
  const NetworkMedium_t orig_medium = Settings.NetworkMedium;
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
    setNetworkMedium(Settings.NetworkMedium);
  }
  
  return result;
}

String Command_ETH_Disconnect (struct EventStruct *event, const char* Line)
{

  ethPower(0);
  delay(400);
//  ethPower(1);
  setNetworkMedium(NetworkMedium_t::Ethernet);
  ETHConnectRelaxed();

  return return_command_success_str();
}

#endif // if FEATURE_ETHERNET
