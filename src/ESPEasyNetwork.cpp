#include "ESPEasyNetwork.h"
#include "ESPEasy_fdwdecl.h"
#include "ESPEasy-Globals.h"
#include "ESPEasy_Log.h"
#include "ESPEasyEth.h"
#include "ETH.h"

/*********************************************************************************************\
   Ethernet or Wifi Support for ESP32 Build flag HAS_ETHERNET
\*********************************************************************************************/
void NetworkConnectRelaxed() {
#ifdef HAS_ETHERNET
  addLog(LOG_LEVEL_INFO, F("Connect to: "));
  addLog(LOG_LEVEL_INFO, ethGetDebugEthWifiModeStr());
  if(eth_wifi_mode == ETHERNET) {
    ETHConnectRelaxed();
  } else {
    WiFiConnectRelaxed();
  }
#else
  WiFiConnectRelaxed();
#endif
}

bool NetworkConnected() {
  #ifdef HAS_ETHERNET
  if(eth_wifi_mode == ETHERNET) {
    return ETHConnected();
  } else {
    return WiFiConnected();
  }
  #else
  return WiFiConnected();
  #endif
}

IPAddress NetworkLocalIP() {
  #ifdef HAS_ETHERNET
  if(eth_wifi_mode == ETHERNET) {
    if(eth_connected) {
      return ETH.localIP();
    } else {
      addLog(LOG_LEVEL_ERROR, F("Call NetworkLocalIP() only on connected Ethernet!"));
      return IPAddress();
    }
  } else {
    return WiFi.localIP();
  }
  #else
  return WiFi.localIP();
  #endif
}

IPAddress NetworkSubnetMask() {
  #ifdef HAS_ETHERNET
  if(eth_wifi_mode == ETHERNET) {
    if(eth_connected) {
      return ETH.subnetMask();
    } else {
      addLog(LOG_LEVEL_ERROR, F("Call NetworkSubnetMask() only on connected Ethernet!"));
      return IPAddress();
    }
  } else {
    return WiFi.subnetMask();
  }
  #else
  return WiFi.subnetMask();
  #endif
}

IPAddress NetworkGatewayIP() {
  #ifdef HAS_ETHERNET
  if(eth_wifi_mode == ETHERNET) {
    if(eth_connected) {
      return ETH.gatewayIP();
    } else {
      addLog(LOG_LEVEL_ERROR, F("Call NetworkGatewayIP() only on connected Ethernet!"));
      return IPAddress();
    }
  } else {
    return WiFi.gatewayIP();
  }
  #else
  return WiFi.gatewayIP();
  #endif
}

IPAddress NetworkDnsIP (uint8_t dns_no) {
  #ifdef HAS_ETHERNET
  if(eth_wifi_mode == ETHERNET) {
    if(eth_connected) {
      return ETH.dnsIP();
    } else {
      addLog(LOG_LEVEL_ERROR, F("Call NetworkDnsIP(uint8_t dns_no) only on connected Ethernet!"));
      return IPAddress();
    }
  } else {
    return WiFi.dnsIP(dns_no);
  }
  #else
  return WiFi.dnsIP(dns_no);
  #endif
}

uint8_t * NetworkMacAddressAsBytes(uint8_t* mac) {
  #ifdef HAS_ETHERNET
  if(eth_wifi_mode == ETHERNET) {
    if(eth_connected) {
      return WiFi.macAddress(mac);
    } else {
      addLog(LOG_LEVEL_ERROR, F("Call NetworkMacAddressAsBytes(uint8_t* mac) only on connected Ethernet!"));
      return mac;
    }
  } else {
    return WiFi.macAddress(mac);
  }
  #else
  return WiFi.macAddress(mac);
  #endif
  return WiFi.macAddress(mac);
}

String NetworkMacAddress() {
  #ifdef HAS_ETHERNET
  if(eth_wifi_mode == ETHERNET) {
    if(!eth_connected) {
      addLog(LOG_LEVEL_ERROR, F("Call NetworkMacAddress() only on connected Ethernet!"));
    }
  }
  #endif
  
  uint8_t  mac[]   = { 0, 0, 0, 0, 0, 0 };
  uint8_t *macread = NetworkMacAddressAsBytes(mac);
  char     macaddress[20];
  formatMAC(macread, macaddress);
  
  return String(macaddress);
}