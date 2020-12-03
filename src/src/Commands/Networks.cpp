#include "../Commands/Networks.h"

#include "../../ESPEasy_common.h"
#include "../Commands/Common.h"
#include "../Globals/Settings.h"
#include "../WebServer/AccessControl.h"


#ifdef HAS_ETHERNET
#include "ETH.h"
#endif

String Command_AccessInfo_Ls(struct EventStruct *event, const char* Line)
{
  String result = F("Allowed IP range : ");
  result += describeAllowedIPrange();
  return return_result(event, result);
}

String Command_AccessInfo_Clear (struct EventStruct *event, const char* Line)
{
  clearAccessBlock();
  return Command_AccessInfo_Ls(event, Line);
}

String Command_DNS (struct EventStruct *event, const char* Line)
{
  return Command_GetORSetIP(event, F("DNS:"), Line, Settings.DNS,WiFi.dnsIP(0),1);
}

String Command_Gateway (struct EventStruct *event, const char* Line)
{
  return Command_GetORSetIP(event, F("Gateway:"), Line, Settings.Gateway,WiFi.gatewayIP(),1);
}

String Command_IP (struct EventStruct *event, const char* Line)
{
  return Command_GetORSetIP(event, F("IP:"), Line, Settings.IP,WiFi.localIP(),1);
}

String Command_Subnet (struct EventStruct *event, const char* Line)
{
  return Command_GetORSetIP(event, F("Subnet:"), Line, Settings.Subnet,WiFi.subnetMask(),1);
}

#ifdef HAS_ETHERNET
String Command_ETH_Phy_Addr (struct EventStruct *event, const char* Line)
{
  return Command_GetORSetUint8_t(event, F("ETH_Phy_Addr:"), Line, (uint8_t*)&Settings.ETH_Phy_Addr,1);
}

String Command_ETH_Pin_mdc (struct EventStruct *event, const char* Line)
{
  return Command_GetORSetInt8_t(event, F("ETH_Pin_mdc:"), Line, (int8_t*)&Settings.ETH_Pin_mdc,1);
}

String Command_ETH_Pin_mdio (struct EventStruct *event, const char* Line)
{
  return Command_GetORSetInt8_t(event, F("ETH_Pin_mdio:"), Line, (int8_t*)&Settings.ETH_Pin_mdio,1);
}

String Command_ETH_Pin_power (struct EventStruct *event, const char* Line)
{
  return Command_GetORSetInt8_t(event, F("ETH_Pin_power:"), Line, (int8_t*)&Settings.ETH_Pin_power,1);
}

String Command_ETH_Phy_Type (struct EventStruct *event, const char* Line)
{
  return Command_GetORSetInt8_t(event, F("ETH_Phy_Type:"), Line, (int8_t*)&Settings.ETH_Phy_Type,1);
}

String Command_ETH_Clock_Mode (struct EventStruct *event, const char* Line)
{
  return Command_GetORSetUint8_t(event, F("ETH_Clock_Mode:"), Line, (uint8_t*)&Settings.ETH_Clock_Mode,1);
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
  return Command_GetORSetUint8_t(event, F("NetworkMedium:"), Line, (uint8_t*)&Settings.NetworkMedium,1);
}

#endif
