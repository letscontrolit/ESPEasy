#ifndef NETWORK_H
#define NETWORK_H

#include "ESPEasy_common.h"

void NetworkConnectRelaxed();
bool NetworkConnected();
IPAddress NetworkLocalIP();
IPAddress NetworkSubnetMask();
IPAddress NetworkGatewayIP();
IPAddress NetworkDnsIP (uint8_t dns_no);
uint8_t * NetworkMacAddressAsBytes(uint8_t* mac);
String NetworkMacAddress();


#endif // NETWORK_H