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
String NetworkGetHostNameFromSettings();
String NetworkGetHostname();
String NetworkCreateRFCCompliantHostname();
String createRFCCompliantHostname(String oldString);
String WifiSoftAPmacAddress();


#endif // NETWORK_H