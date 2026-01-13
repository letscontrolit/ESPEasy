#include "../DataTypes/ESPEasy_plugin_functions.h"

#ifdef ESP32

# include "../Helpers/StringConverter.h"

# include <esp_netif.h>

bool NWPlugin::canQueryViaNetworkInterface(NWPlugin::Function function)
{
  switch (function)
  {
    // TD-er: Do not try to fetch hostname via base class. No idea why, but it doesn't work well
    case NWPlugin::Function::NWPLUGIN_WEBFORM_SHOW_ACTIVE:
    case NWPlugin::Function::NWPLUGIN_WEBFORM_SHOW_ROUTE_PRIO:
    case NWPlugin::Function::NWPLUGIN_WEBFORM_SHOW_HOSTNAME:
    case NWPlugin::Function::NWPLUGIN_WEBFORM_SHOW_NAME:
    case NWPlugin::Function::NWPLUGIN_WEBFORM_SHOW_HW_ADDRESS:
    case NWPlugin::Function::NWPLUGIN_WEBFORM_SHOW_IP:
    case NWPlugin::Function::NWPLUGIN_CLIENT_IP_WEB_ACCESS_ALLOWED:

      return true;
    default: break;
  }
  return false;
}

const __FlashStringHelper * NWPlugin::toString(ConnectionState connectionState, bool asUnicodeSymbol)
{
  if (asUnicodeSymbol) {
    switch (connectionState)
    {
      case NWPlugin::ConnectionState::Disabled:     return F("&#10060;");         // Same icon as used everywhere else in ESPEasy to
      // indicate disabled
      case NWPlugin::ConnectionState::Error:        return F("&#x26A0;");         // Warning Emoji
      case NWPlugin::ConnectionState::Initializing: return F("&#x23F3;");         // Hourglass Not Done Emoji
      case NWPlugin::ConnectionState::Connecting:   return F("&#x23F6;&#x23F3;"); // LinkUp & Initializing
      case NWPlugin::ConnectionState::LinkDown:     return F("&#x1F53B;");        // Red Triangle Pointed Down Emoji
      case NWPlugin::ConnectionState::LinkUp:       return F("&#x23F6;");         // Black Medium Up-Pointing Triangle
      case NWPlugin::ConnectionState::Connected:    return F("&#x1F517;");        // Link Emoji
    }
  }
  else {
    switch (connectionState)
    {
      case NWPlugin::ConnectionState::Disabled:     return F("Disabled");
      case NWPlugin::ConnectionState::Error:        return F("Error");
      case NWPlugin::ConnectionState::Initializing: return F("Initializing");
      case NWPlugin::ConnectionState::Connecting:   return F("Connecting");
      case NWPlugin::ConnectionState::LinkDown:     return F("LinkDown");
      case NWPlugin::ConnectionState::LinkUp:       return F("LinkUp");
      case NWPlugin::ConnectionState::Connected:    return F("Connected");
    }
  }
  return F("");
}

const __FlashStringHelper * NWPlugin::toString(NWPlugin::IP_type ip_type)
{
  switch (ip_type)
  {
    case NWPlugin::IP_type::inet:              return F("Inet");
    case NWPlugin::IP_type::network_id_cdr:    return F("Network/CDR");
    case NWPlugin::IP_type::netmask:           return F("Netmask");
    case NWPlugin::IP_type::broadcast:         return F("Broadcast");
    case NWPlugin::IP_type::gateway:           return F("Gateway");
    case NWPlugin::IP_type::dns1:              return F("Dns1");
    case NWPlugin::IP_type::dns2:              return F("Dns2");
# if CONFIG_LWIP_IPV6
    case NWPlugin::IP_type::ipv6_unknown:      return F("IPv6 unknown");
    case NWPlugin::IP_type::ipv6_global:       return F("IPv6 global");
    case NWPlugin::IP_type::ipv6_link_local:   return F("IPv6 link local");
    case NWPlugin::IP_type::ipv6_site_local:   return F("IPv6 site local");
    case NWPlugin::IP_type::ipv6_unique_local: return F("IPv6 unique local");
    case NWPlugin::IP_type::ipv4_mapped_ipv6:  return F("IPv4 mapped IPv6");
# endif // if CONFIG_LWIP_IPV6
  }
  return F("unknown");
}

bool NWPlugin::print_IP_address(NWPlugin::IP_type ip_type, NetworkInterface*networkInterface, Print& out)
{
  const IPAddress ip(get_IP_address(ip_type, networkInterface));
  const size_t    nrBytes = ip.printTo(out, true) > 0;

  if (ip.type() == IPv4) {
    const uint32_t val(ip);

    if (val == 0) { return false; }
  }

  if (ip_type == NWPlugin::IP_type::network_id_cdr) {
    out.print('/');
    out.print(networkInterface->subnetCIDR());
  }
  return nrBytes > 0;
}

IPAddress NWPlugin::get_IP_address(NWPlugin::IP_type ip_type, NetworkInterface*networkInterface)
{
  IPAddress ip;

  if (networkInterface) {
# if CONFIG_LWIP_IPV6
    esp_ip6_addr_type_t ip6_addr_type = ESP_IP6_ADDR_IS_UNKNOWN;
# endif

    switch (ip_type)
    {
      case NWPlugin::IP_type::inet:           return networkInterface->localIP();
      case NWPlugin::IP_type::network_id_cdr: return networkInterface->networkID();
      case NWPlugin::IP_type::netmask:        return networkInterface->subnetMask();
      case NWPlugin::IP_type::broadcast:      return networkInterface->broadcastIP();
      case NWPlugin::IP_type::gateway:        return networkInterface->gatewayIP();
      case NWPlugin::IP_type::dns1:           return networkInterface->dnsIP(0);
      case NWPlugin::IP_type::dns2:           return networkInterface->dnsIP(1);
# if CONFIG_LWIP_IPV6
      case NWPlugin::IP_type::ipv6_unknown:      ip6_addr_type = ESP_IP6_ADDR_IS_UNKNOWN;
        break;
      case NWPlugin::IP_type::ipv6_global:       ip6_addr_type = ESP_IP6_ADDR_IS_GLOBAL;
        break;
      case NWPlugin::IP_type::ipv6_link_local:   ip6_addr_type = ESP_IP6_ADDR_IS_LINK_LOCAL;
        break;
      case NWPlugin::IP_type::ipv6_site_local:   ip6_addr_type = ESP_IP6_ADDR_IS_SITE_LOCAL;
        break;
      case NWPlugin::IP_type::ipv6_unique_local: ip6_addr_type = ESP_IP6_ADDR_IS_UNIQUE_LOCAL;
        break;
      case NWPlugin::IP_type::ipv4_mapped_ipv6:  ip6_addr_type = ESP_IP6_ADDR_IS_IPV4_MAPPED_IPV6;
        break;
# endif // if CONFIG_LWIP_IPV6
      default:
        return ip;
    }
# if CONFIG_LWIP_IPV6

    if (networkInterface->netif()) {
      esp_ip6_addr_t if_ip6[CONFIG_LWIP_IPV6_NUM_ADDRESSES];
      int v6addrs = esp_netif_get_all_ip6(networkInterface->netif(), if_ip6);

      for (int i = 0; i < v6addrs; ++i) {
        if (esp_netif_ip6_get_addr_type(&if_ip6[i]) == ip6_addr_type) {
          return IPAddress(IPv6, (const uint8_t *)if_ip6[i].addr, if_ip6[i].zone);
        }
      }
    }
# endif // if CONFIG_LWIP_IPV6
  }

  return ip;
}

bool NWPlugin::get_subnet(NWPlugin::IP_type ip_type, NetworkInterface*networkInterface, IPAddress& networkID, IPAddress& broadcast)
{
  if (!networkInterface) { return false; }
  IPAddress ip = get_IP_address(ip_type, networkInterface);

  if (ip == INADDR_NONE) { return false; }

# if CONFIG_LWIP_IPV6

  if (ip.type() == IPv6) {
    // FIXME TD-er: For now, just assume /64 subnets until we have actually access to some subnet info.
    networkID = ip;
    broadcast = ip;

    uint8_t i = 8;

    if ((ip[0] == 0xFD) || (ip[0] == 0xFE)) {
      // /8 subnet for local IPv6
      // 0xFD: Unique Local
      // 0xFE: Link Local
      i = 1;
    }

    for (; i < 16; ++i) {
      networkID[i] = 0;
      broadcast[i] = 0xFF;
    }
    return true;
  }
# endif // if CONFIG_LWIP_IPV6

  networkID = networkInterface->networkID();
  broadcast = networkInterface->broadcastIP();
  return true;
}
#endif

bool NWPlugin::ipLessEqual(const IPAddress& ip, const IPAddress& high)
{
  // FIXME TD-er: Must check whether both are of same type and check full range IPv6
  int nrOctets = 4;

  # if FEATURE_USE_IPV6

  if (ip.type() != high.type()) { return false; }

  if (ip.type() == IPv6) {
    nrOctets = 16;
  }
  # endif // if FEATURE_USE_IPV6

  for (int i = 0; i < nrOctets; ++i) {
    if (ip[i] != high[i]) {
      return ip[i] < high[i];
    }
  }

  // Is equal
  return true;
}

bool NWPlugin::ipInRange(const IPAddress& ip, const IPAddress& low, const IPAddress& high) { 
  return ipLessEqual(low, ip) && ipLessEqual(ip, high); 
}

#ifdef ESP32
bool NWPlugin::IP_in_subnet(const IPAddress & ip,
                            NetworkInterface *networkInterface)
{
  const NWPlugin::IP_type ip_type = get_IP_type(ip);
  IPAddress networkID;
  IPAddress broadcast;

  if (!get_subnet(ip_type, networkInterface, networkID, broadcast)) { 
    return false; 
  }
#if FEATURE_USE_IPV6
  if (ip_type == NWPlugin::IP_type::ipv6_link_local) {
    // Must match zone, or else it will always match.
    return  (ip.zone() == networkID.zone());
  }
#endif

  return ipLessEqual(networkID, ip) && ipLessEqual(ip, broadcast);
}

NWPlugin::IP_type NWPlugin::get_IP_type(const IPAddress& ip)
{
# if FEATURE_USE_IPV6

  if (ip.type() == IPv6) {
    switch (ip.addr_type())
    {
      case ESP_IP6_ADDR_IS_UNKNOWN:
      {
        return NWPlugin::IP_type::ipv6_unknown;
      }
      case ESP_IP6_ADDR_IS_GLOBAL:
      {
        return NWPlugin::IP_type::ipv6_global;
      }
      case ESP_IP6_ADDR_IS_LINK_LOCAL:
      {
        return NWPlugin::IP_type::ipv6_link_local;
      }
      case ESP_IP6_ADDR_IS_SITE_LOCAL:
      {
        return NWPlugin::IP_type::ipv6_site_local;
      }
      case ESP_IP6_ADDR_IS_UNIQUE_LOCAL:
      {
        return NWPlugin::IP_type::ipv6_unique_local;
      }
      case ESP_IP6_ADDR_IS_IPV4_MAPPED_IPV6:
      {
        return NWPlugin::IP_type::ipv4_mapped_ipv6;
      }
    }
  }
# endif // if FEATURE_USE_IPV6

  // FIXME TD-er: Maybe also try to determine whether we have a netmask/broadcast/etc?
  return NWPlugin::IP_type::inet;
}

const __FlashStringHelper * NWPlugin::toString(NWPlugin::NetforkFlags flag)
{
  switch (flag)
  {
    case NWPlugin::NetforkFlags::DHCP_client:           return F("DHCP Client");
    case NWPlugin::NetforkFlags::DHCP_server:           return F("DHCP Server");
    case NWPlugin::NetforkFlags::AutoUp:                return F("Auto Up");
    case NWPlugin::NetforkFlags::GratuituousArp:        return F("Gratuituous Arp");
    case NWPlugin::NetforkFlags::EventIPmodified:       return F("Send Event when IP Modified");
    case NWPlugin::NetforkFlags::isPPP:                 return F("PPP");
    case NWPlugin::NetforkFlags::isBridge:              return F("Bridge");
    case NWPlugin::NetforkFlags::MLD_v6_report:         return F("MLD IPv6 Report");
    case NWPlugin::NetforkFlags::IPv6_autoconf_enabled: return F("IPv6 Autoconf");

  }
  return F("unknown");
}

bool NWPlugin::isFlagSet(NWPlugin::NetforkFlags flag, NetworkInterface*networkInterface)
{
  if (networkInterface == nullptr) { return false; }
  auto netif = networkInterface->netif();

  if (netif == nullptr) { return false; }
  const auto flags = esp_netif_get_flags(netif);

  const uint32_t mask = static_cast<int>(flag);
  return flags & mask;
}

bool NWPlugin::forceDHCP_request(NetworkInterface*networkInterface)
{
  if (!NWPlugin::isFlagSet(NWPlugin::NetforkFlags::DHCP_client, networkInterface)) {
    return false;
  }

  auto netif = networkInterface->netif();

  if (netif == nullptr) { return false; }

  const char*desc = esp_netif_get_desc(netif);
  esp_err_t  err  = esp_netif_dhcpc_stop(netif);

  if (err != 0) {
    addLog(LOG_LEVEL_ERROR, strformat(F("%s: DHCPc could not be stopped! Error: 0x%04x: %s"), desc, err, esp_err_to_name(err)));
    return false;
  }
  addLog(LOG_LEVEL_INFO, strformat(F("%s: DHCPc stopped"), desc));
  err = esp_netif_dhcpc_start(netif);

  if ((err != ESP_OK) && (err != ESP_ERR_ESP_NETIF_DHCP_ALREADY_STARTED)) {
    addLog(LOG_LEVEL_ERROR, strformat(F("%s: DHCPc could not be started! Error: 0x%04x: %s"), desc, err, esp_err_to_name(err)));
    return false;
  }
  addLog(LOG_LEVEL_INFO, strformat(F("%s: DHCPc Client started"), desc));
  return true;
}

#endif // ifdef ESP32
