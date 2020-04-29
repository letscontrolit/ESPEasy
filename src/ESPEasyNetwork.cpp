#include "ESPEasyNetwork.h"
#include "ESPEasy_fdwdecl.h"
#include "ESPEasy-Globals.h"
#include "ESPEasy_Log.h"
#include "ESPEasyEth.h"

#ifdef HAS_ETHERNET
#include "ETH.h"
#endif

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

String NetworkMacAddress() {
  #ifdef HAS_ETHERNET
  if(eth_wifi_mode == ETHERNET) {
    if(!eth_connected) {
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
  if(eth_wifi_mode == ETHERNET) {
    return ETHMacAddress(mac);
  } else {
    return WiFi.macAddress(mac);
  }
  #else
  return WiFi.macAddress(mac);
  #endif
}

String NetworkGetHostname() {
    #ifdef ESP32
      #ifdef HAS_ETHERNET 
      if(Settings.ETH_Wifi_Mode == ETHERNET) {
        return String(ETH.getHostname());
      }
        return String(WiFi.getHostname());
      #else
        return String(WiFi.getHostname());
      #endif
    #else
      return String(WiFi.hostname());
    #endif
}

// ********************************************************************************
// Determine Wifi AP name to set. (also used for mDNS)
// ********************************************************************************
String NetworkGetHostNameFromSettings()
{
  return Settings.getHostname();
}

String NetworkCreateRFCCompliantHostname() {
  return createRFCCompliantHostname(NetworkGetHostNameFromSettings());
}

// Create hostname with - instead of spaces
String createRFCCompliantHostname(String oldString) {
  String result(oldString);

  result.replace(" ", "-");
  result.replace("_", "-"); // See RFC952
  return result;
}

String WifiSoftAPmacAddress() {
    uint8_t  mac[]   = { 0, 0, 0, 0, 0, 0 };
    uint8_t *macread = WiFi.softAPmacAddress(mac);
    char     macaddress[20];
    formatMAC(macread, macaddress);
    return String(macaddress);
}