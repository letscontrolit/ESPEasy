#pragma once

#include "../../ESPEasy_common.h"

#include "../net/DataStructs/MAC_address.h"

#include <IPAddress.h>

#if FEATURE_USE_IPV6
# include <vector>

// typedef uint8_t ip6_addr_type_t;
// typedef std::vector<std::pair<IPAddress, ip6_addr_type_t>> IP6Addresses_t;
typedef std::vector<IPAddress> IP6Addresses_t;
#endif // if FEATURE_USE_IPV6


#define NETWORK_INDEX_WIFI_STA  0 // Always the first network index
#define NETWORK_INDEX_WIFI_AP   1 // Always the 2nd network index

namespace ESPEasy {
namespace net {

void           setNetworkMedium(NetworkMedium_t medium);

// void           NetworkConnectRelaxed();
bool           NetworkConnected(bool force = false);
IPAddress      NetworkLocalIP();
IPAddress      NetworkID();
IPAddress      NetworkBroadcast();
IPAddress      NetworkSubnetMask();
IPAddress      NetworkGatewayIP();
IPAddress      NetworkDnsIP(uint8_t dns_no);
uint64_t       NetworkConnectDuration_ms();
uint32_t       NetworkConnectCount();

#if FEATURE_USE_IPV6

IPAddress      NetworkLocalIP6();
IPAddress      NetworkGlobalIP6();
IP6Addresses_t NetworkAllIPv6();

bool           IPv6_link_local_from_MAC(const MAC_address& mac,
                                        IPAddress        & ipv6);
bool           is_IPv6_link_local_from_MAC(const MAC_address& mac);

// Assume we're in the same subnet, thus use our own IPv6 global address
bool           IPv6_global_from_MAC(const MAC_address& mac,
                                    IPAddress        & ipv6);
bool           is_IPv6_global_from_MAC(const MAC_address& mac);

#endif // if FEATURE_USE_IPV6
MAC_address    NetworkMacAddress();
String         NetworkGetHostNameFromSettings(bool force_add_unitnr = false);
String         NetworkGetHostname();
String         NetworkCreateRFCCompliantHostname(bool force_add_unitnr = false);
String         makeRFCCompliantName(const String& name,
                                    const char    replaceChar = '-',
                                    const char    allowedChar = '-',
                                    const size_t  maxlength   = 24);
#if FEATURE_WIFI
MAC_address WifiSoftAPmacAddress();
MAC_address WifiSTAmacAddress();
#endif // if FEATURE_WIFI

void        CheckRunningServices();

#if FEATURE_ETHERNET
bool        EthFullDuplex();
bool        EthLinkUp();
uint8_t     EthLinkSpeed();
#endif // if FEATURE_ETHERNET


} // namespace net
} // namespace ESPEasy
