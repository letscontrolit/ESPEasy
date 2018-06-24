#ifndef COMMAND_NETWORKS_H
#define COMMAND_NETWORKS_H


#include "../ESPEasy-Globals.h"

bool Command_AccessInfo_Ls(struct EventStruct *event, const char* Line)
{
  bool success = true;
  Serial.print(F("Allowed IP range : "));
  Serial.println(describeAllowedIPrange());
  return success;
}

bool Command_AccessInfo_Clear (struct EventStruct *event, const char* Line)
{
  bool success = true;
  clearAccessBlock();
  Serial.print(F("Allowed IP range : "));
  Serial.println(describeAllowedIPrange());
  return success;
}

bool Command_DNS (struct EventStruct *event, const char* Line)
{
  return Command_GetORSetIP(F("DNS:"), Line, Settings.DNS,WiFi.dnsIP(0),1);
}

bool Command_Gateway (struct EventStruct *event, const char* Line)
{
  return Command_GetORSetIP(F("Gateway:"), Line, Settings.Gateway,WiFi.gatewayIP(),1);
}

bool Command_IP (struct EventStruct *event, const char* Line)
{
  return Command_GetORSetIP(F("IP:"), Line, Settings.IP,WiFi.localIP(),1);
}

bool Command_Subnet (struct EventStruct *event, const char* Line)
{
  return Command_GetORSetIP(F("Subnet:"), Line, Settings.Subnet,WiFi.subnetMask(),1);
}

#endif // COMMAND_NETWORKS_H