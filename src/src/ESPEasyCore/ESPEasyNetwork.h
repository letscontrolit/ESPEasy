#ifndef ESPEASY_NETWORK_H
#define ESPEASY_NETWORK_H

#include "../../ESPEasy_common.h"

#include "../DataStructs/MAC_address.h"

#include <IPAddress.h>

#if FEATURE_USE_IPV6
#include <vector>
//typedef uint8_t ip6_addr_type_t;
//typedef std::vector<std::pair<IPAddress, ip6_addr_type_t>> IP6Addresses_t;
typedef std::vector<IPAddress> IP6Addresses_t;
#endif

void setNetworkMedium(NetworkMedium_t medium);

void NetworkConnectRelaxed();
bool NetworkConnected();
IPAddress NetworkLocalIP();
IPAddress NetworkSubnetMask();
IPAddress NetworkGatewayIP();
IPAddress NetworkDnsIP (uint8_t dns_no);
#if FEATURE_USE_IPV6

IPAddress NetworkLocalIP6();
IPAddress NetworkGlobalIP6();
IP6Addresses_t NetworkAllIPv6();

bool IPv6_link_local_from_MAC(const MAC_address& mac, IPAddress &ipv6);
bool is_IPv6_link_local_from_MAC(const MAC_address& mac);

// Assume we're in the same subnet, thus use our own IPv6 global address
bool IPv6_global_from_MAC(const MAC_address& mac, IPAddress &ipv6);
bool is_IPv6_global_from_MAC(const MAC_address& mac);

#endif
MAC_address NetworkMacAddress();
String NetworkGetHostNameFromSettings(bool force_add_unitnr = false);
String NetworkGetHostname();
String NetworkCreateRFCCompliantHostname(bool force_add_unitnr = false);
MAC_address WifiSoftAPmacAddress();
MAC_address WifiSTAmacAddress();

void CheckRunningServices();

#if FEATURE_ETHERNET
bool EthFullDuplex();
bool EthLinkUp();
uint8_t EthLinkSpeed();
#endif // if FEATURE_ETHERNET


#endif