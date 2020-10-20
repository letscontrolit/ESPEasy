#ifndef NETWORK_H
#define NETWORK_H

#include "ESPEasy-Globals.h"

void NetworkConnectRelaxed();
bool NetworkConnected();
IPAddress NetworkLocalIP();
IPAddress NetworkSubnetMask();
IPAddress NetworkGatewayIP();
IPAddress NetworkDnsIP (uint8_t dns_no);
uint8_t * NetworkMacAddressAsBytes(uint8_t* mac);
String NetworkMacAddress();
String NetworkGetHostNameFromSettings(bool force_add_unitnr = false);
String NetworkGetHostname();
String NetworkCreateRFCCompliantHostname(bool force_add_unitnr = false);
String createRFCCompliantHostname(const String& oldString);
String WifiSoftAPmacAddress();


#endif // NETWORK_H