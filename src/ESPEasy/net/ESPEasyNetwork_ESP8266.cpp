#ifdef ESP8266

#include "../../ESPEasy/net/ESPEasyNetwork.h"

#include "../../src/ESPEasyCore/ESPEasy_Log.h"

#include "../../src/Globals/ESPEasy_time.h"
#include "../../src/Globals/Settings.h"

#include "../../src/Helpers/NetworkStatusLED.h"
#include "../../src/Helpers/Networking.h"
#include "../../src/Helpers/StringConverter.h"
#include "../../src/Helpers/MDNS_Helper.h"

#include "../net/Globals/ESPEasyWiFiEvent.h"
#include "../net/Globals/NetworkState.h"
#include "../net/wifi/ESPEasyWifi.h"


namespace ESPEasy {
namespace net {

void setNetworkMedium(NetworkMedium_t new_medium) {
  if (new_medium == NetworkMedium_t::Ethernet) {
    new_medium = NetworkMedium_t::WIFI;
  }

  if (new_medium == active_network_medium) {
    return;
  }

  switch (active_network_medium)
  {
    case NetworkMedium_t::Ethernet:
      break;
    case NetworkMedium_t::WIFI:
//      WiFiEventData.timerAPoff.setMillisFromNow(WIFI_AP_OFF_TIMER_DURATION);
//      WiFiEventData.timerAPstart.clear();
      break;
    case NetworkMedium_t::NotSet:
      break;
  }
  statusLED(true);
  active_network_medium = new_medium;
#ifndef BUILD_NO_DEBUG
  addLog(LOG_LEVEL_INFO, concat(F("Set Network mode: "), toString(active_network_medium)));
#endif
}

/*********************************************************************************************\
   Ethernet or Wifi Support for ESP32 Build flag FEATURE_ETHERNET
\*********************************************************************************************/

// void NetworkConnectRelaxed() {
//  if (!ESPEasy::net::NetworkConnected())
//  ESPEasy::net::wifi::WiFiConnectRelaxed();
// }

bool      NetworkConnected(bool force) { 
  static bool last_result = false;
  static uint32_t last_check_millis = 0;
  if (force || timePassedSince(last_check_millis) > 50 || last_check_millis == 0) {
    last_check_millis = millis();
    processNetworkEvents();
    last_result = ESPEasy::net::wifi::WiFiConnected();
  }
  return last_result;
}

IPAddress NetworkLocalIP()   { return WiFi.localIP(); }

IPAddress NetworkID()
{
  const IPAddress subnet = NetworkSubnetMask();
  IPAddress networkID    = NetworkLocalIP();

  for (uint8_t i = 0; i < 4; ++i) {
    networkID[i] &= subnet[i];
  }
  return networkID;
}

IPAddress NetworkBroadcast()
{
  const IPAddress subnet = NetworkSubnetMask();
  IPAddress broadcast    = NetworkLocalIP();

  for (uint8_t i = 0; i < 4; ++i) {
    broadcast[i] |= ~subnet[i];
  }
  return broadcast;
}

IPAddress   NetworkSubnetMask()          { return WiFi.subnetMask(); }

IPAddress   NetworkGatewayIP()           { return WiFi.gatewayIP(); }

IPAddress   NetworkDnsIP(uint8_t dns_no) { return WiFi.dnsIP(dns_no); }

MAC_address NetworkMacAddress() {
  MAC_address mac;

  WiFi.macAddress(mac.mac);
  return mac;
}

String NetworkGetHostname() { return String(WiFi.hostname()); }

#if FEATURE_WIFI

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

#endif // if FEATURE_WIFI

} // namespace net
} // namespace ESPEasy

#endif // ifdef ESP8266
