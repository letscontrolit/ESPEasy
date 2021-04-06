#include "../ESPEasyCore/ESPEasyNetwork.h"

#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../ESPEasyCore/ESPEasyEth.h"
#include "../ESPEasyCore/ESPEasyWifi.h"
#include "../Globals/ESPEasyWiFiEvent.h"
#include "../Globals/NetworkState.h"
#include "../Globals/Settings.h"

#include "../Helpers/Network.h"
#include "../Helpers/StringConverter.h"
#include "../Helpers/MDNS_Helper.h"

#ifdef HAS_ETHERNET
#include <ETH.h>
#endif

void setNetworkMedium(NetworkMedium_t medium) {
  if (active_network_medium == medium) {
    return;
  }
  switch (active_network_medium) {
    case NetworkMedium_t::Ethernet:
      #ifdef HAS_ETHERNET
      // FIXME TD-er: How to 'end' ETH?
//      ETH.end();
      #endif
      break;
    case NetworkMedium_t::WIFI:
      WiFiEventData.timerAPoff.setNow();
      WiFiEventData.timerAPstart.clear();
      WifiDisconnect();
      break;
  }
  statusLED(true);
  active_network_medium = medium;
  addLog(LOG_LEVEL_INFO, String(F("Set Network mode: ")) + toString(active_network_medium));
}


/*********************************************************************************************\
   Ethernet or Wifi Support for ESP32 Build flag HAS_ETHERNET
\*********************************************************************************************/
void NetworkConnectRelaxed() {
  if (NetworkConnected()) return;
#ifdef HAS_ETHERNET
  if(active_network_medium == NetworkMedium_t::Ethernet) {
    if (ETHConnectRelaxed()) {
      return;
    }
    // Failed to start the Ethernet network, probably not present of wrong parameters.
    // So set the runtime active medium to WiFi to try connecting to WiFi or at least start the AP.
    setNetworkMedium(NetworkMedium_t::WIFI);
  }
#endif
  WiFiConnectRelaxed();
}

bool NetworkConnected() {
  #ifdef HAS_ETHERNET
  if(active_network_medium == NetworkMedium_t::Ethernet) {
    return ETHConnected();
  }
  #endif
  return WiFiConnected();
}

IPAddress NetworkLocalIP() {
  #ifdef HAS_ETHERNET
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
  #ifdef HAS_ETHERNET
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
  #ifdef HAS_ETHERNET
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

IPAddress NetworkDnsIP (uint8_t dns_no) {
  #ifdef HAS_ETHERNET
  if(active_network_medium == NetworkMedium_t::Ethernet) {
    if(EthEventData.ethInitSuccess) {
      return ETH.dnsIP();
    } else {
      addLog(LOG_LEVEL_ERROR, F("Call NetworkDnsIP(uint8_t dns_no) only on connected Ethernet!"));
      return IPAddress();
    }
  }
  #endif
  return WiFi.dnsIP(dns_no);
}

String NetworkMacAddress() {
  #ifdef HAS_ETHERNET
  if(active_network_medium == NetworkMedium_t::Ethernet) {
    if(!EthEventData.ethInitSuccess) {
      addLog(LOG_LEVEL_ERROR, F("Call NetworkMacAddress() only on connected Ethernet!"));
    } else {
      return ETH.macAddress();
    }
  }
  #endif
  
  uint8_t  mac[]   = { 0, 0, 0, 0, 0, 0 };
  uint8_t *macread = NetworkMacAddressAsBytes(mac);
  char     macaddress[20];
  formatMAC(macread, macaddress);
  
  return String(macaddress);
}

uint8_t * NetworkMacAddressAsBytes(uint8_t* mac) {
  #ifdef HAS_ETHERNET
  if(active_network_medium == NetworkMedium_t::Ethernet) {
    return ETHMacAddress(mac);
  }
  #endif
  return WiFi.macAddress(mac);
}

String NetworkGetHostname() {
    #ifdef ESP32
      #ifdef HAS_ETHERNET 
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
  return createRFCCompliantHostname(NetworkGetHostNameFromSettings(force_add_unitnr));
}

// Create hostname with - instead of spaces
String createRFCCompliantHostname(const String& oldString) {
  String result(oldString);

  result.replace(' ', '-');
  result.replace('_', '-'); // See RFC952
  return result;
}

String WifiSoftAPmacAddress() {
    uint8_t  mac[]   = { 0, 0, 0, 0, 0, 0 };
    uint8_t *macread = WiFi.softAPmacAddress(mac);
    char     macaddress[20];
    formatMAC(macread, macaddress);
    return String(macaddress);
}

void CheckRunningServices() {
  set_mDNS();
  SetWiFiTXpower();
}

#ifdef HAS_ETHERNET
bool EthFullDuplex()
{
  if (EthEventData.ethInitSuccess)
    return ETH.fullDuplex();
  return false;
}

bool EthLinkUp()
{
  if (EthEventData.ethInitSuccess)
    return ETH.linkUp();
  return false;
}

uint8_t EthLinkSpeed()
{
  if (EthEventData.ethInitSuccess)
    return ETH.linkSpeed();
  return 0;
}
#endif
