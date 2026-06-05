#ifndef COMMAND_NETWORKS_H
#define COMMAND_NETWORKS_H

#include "../../ESPEasy_common.h"

class String;


const __FlashStringHelper* Command_Network_Disable(struct EventStruct *event,
                                                const char         *Line);
const __FlashStringHelper* Command_Network_Enable(struct EventStruct *event,
                                               const char         *Line);

String Command_AccessInfo_Ls(struct EventStruct *event, const char* Line);
String Command_AccessInfo_Clear (struct EventStruct *event, const char* Line);
String Command_DNS (struct EventStruct *event, const char* Line);
String Command_Gateway (struct EventStruct *event, const char* Line);
String Command_IP (struct EventStruct *event, const char* Line);
#if FEATURE_USE_IPV6
String Command_show_all_IP6 (struct EventStruct *event, const char* Line);
#endif
String Command_Subnet (struct EventStruct *event, const char* Line);
#if FEATURE_ETHERNET
String Command_ETH_IP (struct EventStruct *event, const char* Line);
String Command_ETH_Gateway (struct EventStruct *event, const char* Line);
String Command_ETH_Subnet (struct EventStruct *event, const char* Line);
String Command_ETH_DNS (struct EventStruct *event, const char* Line);
String Command_ETH_Wifi_Mode (struct EventStruct *event, const char* Line);
String Command_ETH_Disconnect (struct EventStruct *event, const char* Line);
#endif
#if FEATURE_STORE_NETWORK_INTERFACE_SETTINGS
String Command_Network_ExportConfig (struct EventStruct *event, const char* Line);
String Command_Network_ImportConfig (struct EventStruct *event, const char* Line);
#endif

#endif // COMMAND_NETWORKS_H
