#ifndef COMMAND_NETWORKS_H
#define COMMAND_NETWORKS_H

class String;


String Command_AccessInfo_Ls(struct EventStruct *event, const char* Line);
String Command_AccessInfo_Clear (struct EventStruct *event, const char* Line);
String Command_DNS (struct EventStruct *event, const char* Line);
String Command_Gateway (struct EventStruct *event, const char* Line);
String Command_IP (struct EventStruct *event, const char* Line);
String Command_Subnet (struct EventStruct *event, const char* Line);
String Command_ETH_Phy_Addr (struct EventStruct *event, const char* Line);
String Command_ETH_Pin_mdc (struct EventStruct *event, const char* Line);
String Command_ETH_Pin_mdio (struct EventStruct *event, const char* Line);
String Command_ETH_Pin_power (struct EventStruct *event, const char* Line);
String Command_ETH_Phy_Type (struct EventStruct *event, const char* Line);
String Command_ETH_Clock_Mode (struct EventStruct *event, const char* Line);
String Command_ETH_IP (struct EventStruct *event, const char* Line);
String Command_ETH_Gateway (struct EventStruct *event, const char* Line);
String Command_ETH_Subnet (struct EventStruct *event, const char* Line);
String Command_ETH_DNS (struct EventStruct *event, const char* Line);
String Command_ETH_Wifi_Mode (struct EventStruct *event, const char* Line);

#endif // COMMAND_NETWORKS_H
