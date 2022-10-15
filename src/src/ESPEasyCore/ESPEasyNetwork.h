#ifndef ESPEASY_NETWORK_H
#define ESPEASY_NETWORK_H

#include "../../ESPEasy_common.h"

#include "../DataStructs/MAC_address.h"

void setNetworkMedium(NetworkMedium_t medium);

void NetworkConnectRelaxed();
bool NetworkConnected();
IPAddress NetworkLocalIP();
IPAddress NetworkSubnetMask();
IPAddress NetworkGatewayIP();
IPAddress NetworkDnsIP (uint8_t dns_no);
MAC_address NetworkMacAddress();
String NetworkGetHostNameFromSettings(bool force_add_unitnr = false);
String NetworkGetHostname();
String NetworkCreateRFCCompliantHostname(bool force_add_unitnr = false);
String createRFCCompliantHostname(const String& oldString);
MAC_address WifiSoftAPmacAddress();
MAC_address WifiSTAmacAddress();

void CheckRunningServices();

#if FEATURE_ETHERNET
bool EthFullDuplex();
bool EthLinkUp();
uint8_t EthLinkSpeed();
#endif // if FEATURE_ETHERNET


#endif