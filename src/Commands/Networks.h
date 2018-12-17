#ifndef COMMAND_NETWORKS_H
#define COMMAND_NETWORKS_H


#include "../ESPEasy-Globals.h"

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

#endif // COMMAND_NETWORKS_H
