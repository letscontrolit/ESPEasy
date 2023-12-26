#include "../ESPEasyCore/ESPEasyNetwork.h"

#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../ESPEasyCore/ESPEasyEth.h"
#include "../ESPEasyCore/ESPEasyWifi.h"
#include "../Globals/ESPEasy_time.h"
#include "../Globals/ESPEasyWiFiEvent.h"
#include "../Globals/NetworkState.h"
#include "../Globals/Settings.h"

#include "../Helpers/Network.h"
#include "../Helpers/Networking.h"
#include "../Helpers/StringConverter.h"
#include "../Helpers/MDNS_Helper.h"

#if FEATURE_ETHERNET
#include "../Globals/ESPEasyEthEvent.h"
#include <ETH.h>
#endif


#if FEATURE_USE_IPV6
#include <esp_netif.h>

// -----------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------- Private functions ------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------

esp_netif_t* get_esp_interface_netif(esp_interface_t interface);
#endif


void setNetworkMedium(NetworkMedium_t new_medium) {
#if !(FEATURE_ETHERNET)
  if (new_medium == NetworkMedium_t::Ethernet) {
    new_medium = NetworkMedium_t::WIFI;
  }
#endif
  if (active_network_medium == new_medium) {
    return;
  }
  switch (active_network_medium) {
    case NetworkMedium_t::Ethernet:
      #if FEATURE_ETHERNET
      // FIXME TD-er: How to 'end' ETH?
//      ETH.end();
      if (new_medium == NetworkMedium_t::WIFI) {
        WiFiEventData.clearAll();
      }
      #endif
      break;
    case NetworkMedium_t::WIFI:
      WiFiEventData.timerAPoff.setMillisFromNow(WIFI_AP_OFF_TIMER_DURATION);
      WiFiEventData.timerAPstart.clear();
      if (new_medium == NetworkMedium_t::Ethernet) {
        WifiDisconnect();
      }
      break;
    case NetworkMedium_t::NotSet:
      break;
  }
  statusLED(true);
  active_network_medium = new_medium;
  addLog(LOG_LEVEL_INFO, concat(F("Set Network mode: "), toString(active_network_medium)));
}


/*********************************************************************************************\
   Ethernet or Wifi Support for ESP32 Build flag FEATURE_ETHERNET
\*********************************************************************************************/
void NetworkConnectRelaxed() {
  if (NetworkConnected()) return;
#if FEATURE_ETHERNET
  if(active_network_medium == NetworkMedium_t::Ethernet) {
    if (ETHConnectRelaxed()) {
      return;
    }
    // Failed to start the Ethernet network, probably not present of wrong parameters.
    // So set the runtime active medium to WiFi to try connecting to WiFi or at least start the AP.
    setNetworkMedium(NetworkMedium_t::WIFI);
  }
#endif
  // Failed to start the Ethernet network, probably not present of wrong parameters.
  // So set the runtime active medium to WiFi to try connecting to WiFi or at least start the AP.
  WiFiConnectRelaxed();
}

bool NetworkConnected() {
  #if FEATURE_ETHERNET
  if(active_network_medium == NetworkMedium_t::Ethernet) {
    return ETHConnected();
  }
  #endif
  return WiFiConnected();
}

IPAddress NetworkLocalIP() {
  #if FEATURE_ETHERNET
  if(active_network_medium == NetworkMedium_t::Ethernet) {
    if(EthEventData.ethInitSuccess) {
      return ETH.localIP();
    } else {
      addLog(LOG_LEVEL_ERROR, F("Call NetworkLocalIP() only on connected Ethernet!"));
      return IPAddress();
    }
  }
  #endif
  return WiFi.localIP();
}

IPAddress NetworkSubnetMask() {
  #if FEATURE_ETHERNET
  if(active_network_medium == NetworkMedium_t::Ethernet) {
    if(EthEventData.ethInitSuccess) {
      return ETH.subnetMask();
    } else {
      addLog(LOG_LEVEL_ERROR, F("Call NetworkSubnetMask() only on connected Ethernet!"));
      return IPAddress();
    }
  }
  #endif
  return WiFi.subnetMask();
}

IPAddress NetworkGatewayIP() {
  #if FEATURE_ETHERNET
  if(active_network_medium == NetworkMedium_t::Ethernet) {
    if(EthEventData.ethInitSuccess) {
      return ETH.gatewayIP();
    } else {
      addLog(LOG_LEVEL_ERROR, F("Call NetworkGatewayIP() only on connected Ethernet!"));
      return IPAddress();
    }
  }
  #endif
  return WiFi.gatewayIP();
}

IPAddress NetworkDnsIP(uint8_t dns_no) {
  scrubDNS();
  #if FEATURE_ETHERNET
  if(active_network_medium == NetworkMedium_t::Ethernet) {
    if(EthEventData.ethInitSuccess) {
      return ETH.dnsIP(dns_no);
    } else {
      addLog(LOG_LEVEL_ERROR, F("Call NetworkDnsIP(uint8_t dns_no) only on connected Ethernet!"));
      return IPAddress();
    }
  }
  #endif
  return WiFi.dnsIP(dns_no);
}

#if FEATURE_USE_IPV6
esp_netif_t * getActiveNetworkMediumInterface() {
  esp_interface_t iface = ESP_IF_MAX;
  #if FEATURE_ETHERNET
  if(active_network_medium == NetworkMedium_t::Ethernet) {
    if(EthEventData.ethInitSuccess) {
      esp_netif_t *res = ETH.netif();
      if (res == nullptr) {
        res = get_esp_interface_netif(ESP_IF_ETH);
      }
      if (res != nullptr)
        return res;
    }
  } else
  #endif
  {
    if (WifiIsSTA(WiFi.getMode())) {
      iface = ESP_IF_WIFI_STA;
    }
  }
  if (ESP_IF_MAX == iface) 
    return nullptr;
  return get_esp_interface_netif(iface);
}

IPAddress NetworkLocalIP6() {
  esp_netif_t * iface = getActiveNetworkMediumInterface();
  esp_ip6_addr_t addr;
  if (nullptr == iface ||
     esp_netif_get_ip6_linklocal(iface, &addr)) 
  {
    return IN6ADDR_ANY;
  }

  IPAddress res(IPv6, (const uint8_t*)addr.addr, addr.zone);
  return res;
}

IPAddress NetworkGlobalIP6() {
  esp_netif_t * iface = getActiveNetworkMediumInterface();
  esp_ip6_addr_t addr;
  if (nullptr == iface ||
     esp_netif_get_ip6_global(iface, &addr)) 
  {
    return IN6ADDR_ANY;
  }

  IPAddress res(IPv6, (const uint8_t*)addr.addr, addr.zone);
  return res;
}

IP6Addresses_t NetworkAllIPv6() {
  IP6Addresses_t addresses;
  esp_netif_t * iface = getActiveNetworkMediumInterface();
  if (nullptr != iface) {
    esp_ip6_addr_t esp_ip6_addr[LWIP_IPV6_NUM_ADDRESSES]{};

    int count = esp_netif_get_all_ip6(iface, esp_ip6_addr);
    for (int i = 0; i < count; ++i) {
      addresses.emplace_back(IPv6, (const uint8_t*)esp_ip6_addr[i].addr, esp_ip6_addr[i].zone);
    }
  }

  return addresses;
}

bool IPv6_from_MAC(const MAC_address& mac, IPAddress& ipv6)
{
  if (ipv6 == IN6ADDR_ANY) { return false; }
  int index_offset = 8;

  for (int i = 0; i < 6; ++i, ++index_offset) {
    ipv6[index_offset] = mac.mac[i];

    if (i == 0) {
      // invert bit 2
      bitToggle(ipv6[index_offset], 1);
    }

    if (i == 2) {
      ipv6[++index_offset] = 0xFF;
      ipv6[++index_offset] = 0xFE;
    }
  }
/*
  addLog(LOG_LEVEL_INFO, strformat(
     F("IPv6_from_MAC: Mac %s IP %s"),
     mac.toString().c_str(),
     ipv6.toString().c_str()
     ));
*/
  return true;
}

bool is_IPv6_based_on_MAC(const MAC_address& mac, const IPAddress& ipv6)
{
  IPAddress tmp = ipv6;

  if (IPv6_from_MAC(mac, tmp)) {
    return ipv6 == tmp;
  }
  return false;
}

bool IPv6_link_local_from_MAC(const MAC_address& mac, IPAddress& ipv6)
{
  ipv6 = NetworkLocalIP6();
  return IPv6_from_MAC(mac, ipv6);
}

bool is_IPv6_link_local_from_MAC(const MAC_address& mac)
{
  return is_IPv6_based_on_MAC(mac, NetworkLocalIP6());
}

// Assume we're in the same subnet, thus use our own IPv6 global address
bool IPv6_global_from_MAC(const MAC_address& mac, IPAddress& ipv6)
{
  ipv6 = NetworkGlobalIP6();
  return IPv6_from_MAC(mac, ipv6);
}

bool is_IPv6_global_from_MAC(const MAC_address& mac)
{
  return is_IPv6_based_on_MAC(mac, NetworkGlobalIP6());
}

#endif // if FEATURE_USE_IPV6



MAC_address NetworkMacAddress() {
  #if FEATURE_ETHERNET
  if(active_network_medium == NetworkMedium_t::Ethernet) {
    return ETHMacAddress();
  }
  #endif
  MAC_address mac;
  WiFi.macAddress(mac.mac);
  return mac;
}

String NetworkGetHostname() {
    #ifdef ESP32
      #if FEATURE_ETHERNET 
      if(Settings.NetworkMedium == NetworkMedium_t::Ethernet && EthEventData.ethInitSuccess) {
        return String(ETH.getHostname());
      }
      #endif
      return String(WiFi.getHostname());
    #else
      return String(WiFi.hostname());
    #endif
}

// ********************************************************************************
// Determine Wifi AP name to set. (also used for mDNS)
// ********************************************************************************
String NetworkGetHostNameFromSettings(bool force_add_unitnr)
{
  if (force_add_unitnr) return Settings.getHostname(true);
  return Settings.getHostname();
}

String NetworkCreateRFCCompliantHostname(bool force_add_unitnr) {
  String hostname(NetworkGetHostNameFromSettings(force_add_unitnr));
  // Create hostname with - instead of spaces

  // See RFC952.
  // Allowed chars:
  // * letters (a-z, A-Z)
  // * numerals (0-9)
  // * Hyphen (-)
  replaceUnicodeByChar(hostname, '-');
  for (size_t i = 0; i < hostname.length(); ++i) {
    const char c = hostname[i];
    if (!isAlphaNumeric(c)) {
      hostname[i] = '-';
    }
  }

  // May not start or end with a hyphen
  const String dash('-');
  while (hostname.startsWith(dash)) {
    hostname = hostname.substring(1);
  }
  while (hostname.endsWith(dash)) {
    hostname = hostname.substring(0, hostname.length() - 1);
  }

  // May not contain only numerals
  bool onlyNumerals = true;
  for (size_t i = 0; onlyNumerals && i < hostname.length(); ++i) {
    const char c = hostname[i];
    if (!isdigit(c)) {
      onlyNumerals = false;
    }
  }
  if (onlyNumerals) {
    hostname = concat(F("ESPEasy-"), hostname);
  }

  if (hostname.length() > 24) {
    hostname = hostname.substring(0, 24);
  }

  return hostname;
}

MAC_address WifiSoftAPmacAddress() {
  MAC_address mac;
  WiFi.softAPmacAddress(mac.mac);
  return mac;
}

MAC_address WifiSTAmacAddress() {
  MAC_address mac;
  WiFi.macAddress(mac.mac);
  return mac;
}

void CheckRunningServices() {
  // First try to get the time, since that may be used in logs
  if (Settings.UseNTP() && node_time.timeSource > timeSource_t::NTP_time_source) {
    node_time.lastNTPSyncTime_ms = 0;
    node_time.initTime();
  }
#if FEATURE_SET_WIFI_TX_PWR
  if (active_network_medium == NetworkMedium_t::WIFI) 
  {
    SetWiFiTXpower();
  }
#endif
  set_mDNS();
}

#if FEATURE_ETHERNET
bool EthFullDuplex()
{
  if (EthEventData.ethInitSuccess)
    return ETH.fullDuplex();
  return false;
}

bool EthLinkUp()
{
  if (EthEventData.ethInitSuccess) {
    #ifdef ESP_IDF_VERSION_MAJOR
    // FIXME TD-er: See: https://github.com/espressif/arduino-esp32/issues/6105
    return EthEventData.EthConnected();
    #else
    return ETH.linkUp();
    #endif
  }
  return false;
}

uint8_t EthLinkSpeed()
{
  if (EthEventData.ethInitSuccess) {
    return ETH.linkSpeed();
  }
  return 0;
}
#endif
