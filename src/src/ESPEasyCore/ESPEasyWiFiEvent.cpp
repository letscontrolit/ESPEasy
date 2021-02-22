#include "ESPEasyWiFiEvent.h"

#ifdef HAS_ETHERNET
#include "ETH.h"
#endif

#include "../DataStructs/RTCStruct.h"

#include "../DataTypes/ESPEasyTimeSource.h"

#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../ESPEasyCore/ESPEasyNetwork.h"
#include "../ESPEasyCore/ESPEasyWifi.h"
#include "../ESPEasyCore/ESPEasyWifi_ProcessEvent.h"

#include "../Globals/ESPEasyWiFiEvent.h"
#include "../Globals/RTC.h"

#include "../Helpers/ESPEasy_time_calc.h"


#ifdef HAS_ETHERNET
extern bool eth_connected;
#endif

#ifdef ESP32
void WiFi_Access_Static_IP::set_use_static_ip(bool enabled) {
  _useStaticIp = enabled;
}

#endif // ifdef ESP32
#ifdef ESP8266
void WiFi_Access_Static_IP::set_use_static_ip(bool enabled) {
  _useStaticIp = enabled;
}

#endif // ifdef ESP8266


void setUseStaticIP(bool enabled) {
  WiFi_Access_Static_IP tmp_wifi;

  tmp_wifi.set_use_static_ip(enabled);
}


// ********************************************************************************
// Functions called on events.
// Make sure not to call anything in these functions that result in delay() or yield()
// ********************************************************************************
#ifdef ESP32
#include <WiFi.h>

static bool ignoreDisconnectEvent = false;

void WiFiEvent(system_event_id_t event, system_event_info_t info) {
  switch (event) {
    case SYSTEM_EVENT_STA_CONNECTED:
    {
      char ssid_copy[33] = { 0 }; // Ensure space for maximum len SSID (32) plus trailing 0
      memcpy(ssid_copy, info.connected.ssid, info.connected.ssid_len);
      ssid_copy[32] = 0; // Potentially add 0-termination if none present earlier
      WiFiEventData.markConnected((const char*) ssid_copy, info.connected.bssid, info.connected.channel);
      break;
    }
    case SYSTEM_EVENT_STA_DISCONNECTED:
      if (!ignoreDisconnectEvent) {
        ignoreDisconnectEvent = true;
        WiFiEventData.markDisconnect(static_cast<WiFiDisconnectReason>(info.disconnected.reason));
        WiFi.persistent(false);
        WiFi.disconnect(true);
      }
      break;
    case SYSTEM_EVENT_STA_GOT_IP:
      ignoreDisconnectEvent = false;
      WiFiEventData.markGotIP();
      break;
    case SYSTEM_EVENT_AP_STACONNECTED:
      WiFiEventData.markConnectedAPmode(info.sta_connected.mac);
      break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:
      WiFiEventData.markDisconnectedAPmode(info.sta_disconnected.mac);
      break;
    case SYSTEM_EVENT_SCAN_DONE:
      WiFiEventData.processedScanDone = false;
      break;
#ifdef HAS_ETHERNET
    case SYSTEM_EVENT_ETH_START:
      addLog(LOG_LEVEL_INFO, F("ETH Started"));
      break;
    case SYSTEM_EVENT_ETH_CONNECTED:
      addLog(LOG_LEVEL_INFO, F("ETH Connected"));
      eth_connected = true;
      processEthernetConnected();
      break;
    case SYSTEM_EVENT_ETH_GOT_IP:
      eth_connected = true;
      if (loglevelActiveFor(LOG_LEVEL_INFO))
      {
        String log = F("ETH MAC: ");
        log += NetworkMacAddress();
        log += F(" IPv4: ");
        log += NetworkLocalIP().toString();
        log += " (";
        log += NetworkGetHostname();
        log += F(") GW: ");
        log += NetworkGatewayIP().toString();
        log += F(" SN: ");
        log += NetworkSubnetMask().toString();
        if (ETH.fullDuplex()) {
          log += F(" FULL_DUPLEX");
        }
        log += F(" ");
        log += ETH.linkSpeed();
        log += F("Mbps");
        addLog(LOG_LEVEL_INFO, log);
      }
      break;
    case SYSTEM_EVENT_ETH_DISCONNECTED:
      addLog(LOG_LEVEL_ERROR, F("ETH Disconnected"));
      eth_connected = false;
      processEthernetDisconnected();
      break;
    case SYSTEM_EVENT_ETH_STOP:
      addLog(LOG_LEVEL_INFO, F("ETH Stopped"));
      eth_connected = false;
      break;
    case SYSTEM_EVENT_GOT_IP6:
      addLog(LOG_LEVEL_INFO, F("ETH Got IP6"));
      break;
#endif //HAS_ETHERNET
    default:
      {
        String log = F("UNKNOWN EVENT: ");
        log += event;
        addLog(LOG_LEVEL_ERROR, log);
      }
      break;
  }
}

#endif // ifdef ESP32

#ifdef ESP8266

void onConnected(const WiFiEventStationModeConnected& event) {
  WiFiEventData.markConnected(event.ssid, event.bssid, event.channel);
}

void onDisconnect(const WiFiEventStationModeDisconnected& event) {
  WiFiEventData.markDisconnect(event.reason);
  if (WiFi.status() == WL_CONNECTED) {
    // See https://github.com/esp8266/Arduino/issues/5912
    WiFi.persistent(false);
    WiFi.disconnect(true);
  }
}

void onGotIP(const WiFiEventStationModeGotIP& event) {
  WiFiEventData.markGotIP();
}

void ICACHE_RAM_ATTR onDHCPTimeout() {
  WiFiEventData.processedDHCPTimeout = false;
}

void onConnectedAPmode(const WiFiEventSoftAPModeStationConnected& event) {
  WiFiEventData.markConnectedAPmode(event.mac);
}

void onDisconnectedAPmode(const WiFiEventSoftAPModeStationDisconnected& event) {
  WiFiEventData.markDisconnectedAPmode(event.mac);
}

#endif // ifdef ESP8266
