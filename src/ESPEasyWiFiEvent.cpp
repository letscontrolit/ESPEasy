#ifdef HAS_ETHERNET
#include "ETH.h"
#endif
#include "ESPEasyWiFiEvent.h"
#include "ESPEasyWifi_ProcessEvent.h"
#include "src/Globals/ESPEasyWiFiEvent.h"
#include "src/Globals/RTC.h"
#include "ESPEasyTimeTypes.h"
#include "ESPEasy_Log.h"
#include "ESPEasy_fdwdecl.h"

#include "src/DataStructs/RTCStruct.h"

#include "src/Helpers/ESPEasy_time_calc.h"

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

void markGotIP() {
  lastGetIPmoment.setNow();
  // Create the 'got IP event' so mark the wifiStatus to not have the got IP flag set
  // This also implies the services are not fully initialized.
  bitClear(wifiStatus, ESPEASY_WIFI_GOT_IP);
  bitClear(wifiStatus, ESPEASY_WIFI_SERVICES_INITIALIZED);
  processedGotIP = false;
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
      RTC.lastWiFiChannel = info.connected.channel;
      for (byte i = 0; i < 6; ++i) {
        if (RTC.lastBSSID[i] != info.connected.bssid[i]) {
          bssid_changed = true;
          RTC.lastBSSID[i]  = info.connected.bssid[i];
        }
      }
      char ssid_copy[33] = { 0 }; // Ensure space for maximum len SSID (32) plus trailing 0
      memcpy(ssid_copy, info.connected.ssid, info.connected.ssid_len);
      ssid_copy[32] = 0; // Potentially add 0-termination if none present earlier
      last_ssid = (const char*) ssid_copy;
      lastConnectMoment.setNow();
      wifi_considered_stable = false;
      processedConnect  = false;
      break;
    }
    case SYSTEM_EVENT_STA_DISCONNECTED:
      if (!ignoreDisconnectEvent) {
        ignoreDisconnectEvent = true;
        lastDisconnectMoment.setNow();
        WiFi.persistent(false);
        WiFi.disconnect(true);

        if (last_wifi_connect_attempt_moment.isSet() && (lastConnectMoment > last_wifi_connect_attempt_moment)) {
          // There was an unsuccessful connection attempt
          lastConnectedDuration_us = last_wifi_connect_attempt_moment.timeDiff(lastDisconnectMoment);
        } else {
          lastConnectedDuration_us = lastConnectMoment.timeDiff(lastDisconnectMoment);
        }
        processedDisconnect  = false;
        lastDisconnectReason = static_cast<WiFiDisconnectReason>(info.disconnected.reason);
      }
      break;
    case SYSTEM_EVENT_STA_GOT_IP:
      ignoreDisconnectEvent = false;
      markGotIP();
      break;
    case SYSTEM_EVENT_AP_STACONNECTED:

      for (byte i = 0; i < 6; ++i) {
        lastMacConnectedAPmode[i] = info.sta_connected.mac[i];
      }
      processedConnectAPmode = false;
      break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:

      for (byte i = 0; i < 6; ++i) {
        lastMacConnectedAPmode[i] = info.sta_disconnected.mac[i];
      }
      processedDisconnectAPmode = false;
      break;
    case SYSTEM_EVENT_SCAN_DONE:
      processedScanDone = false;
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
      eth_connected = true;
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
      break;
  }
}

#endif // ifdef ESP32

#ifdef ESP8266

void onConnected(const WiFiEventStationModeConnected& event) {
  lastConnectMoment.setNow();
  wifi_considered_stable = false;
  processedConnect  = false;
  channel_changed   = RTC.lastWiFiChannel != event.channel;
  RTC.lastWiFiChannel      = event.channel;
  last_ssid         = event.ssid;
  bssid_changed     = false;

  for (byte i = 0; i < 6; ++i) {
    if (RTC.lastBSSID[i] != event.bssid[i]) {
      bssid_changed = true;
      RTC.lastBSSID[i]  = event.bssid[i];
    }
  }
}

void onDisconnect(const WiFiEventStationModeDisconnected& event) {
  lastDisconnectMoment.setNow();

  if (lastConnectMoment > last_wifi_connect_attempt_moment) {
    // There was an unsuccessful connection attempt
    lastConnectedDuration_us = last_wifi_connect_attempt_moment.timeDiff(lastDisconnectMoment);
  } else {
    lastConnectedDuration_us = lastConnectMoment.timeDiff(lastDisconnectMoment);
  }
  lastDisconnectReason = event.reason;

  if (WiFi.status() == WL_CONNECTED) {
    // See https://github.com/esp8266/Arduino/issues/5912
    WiFi.persistent(false);
    WiFi.disconnect(true);
  }
  processedDisconnect = false;
}

void onGotIP(const WiFiEventStationModeGotIP& event) {
  markGotIP();
}

void ICACHE_RAM_ATTR onDHCPTimeout() {
  processedDHCPTimeout = false;
}

void onConnectedAPmode(const WiFiEventSoftAPModeStationConnected& event) {
  for (byte i = 0; i < 6; ++i) {
    lastMacConnectedAPmode[i] = event.mac[i];
  }
  processedConnectAPmode = false;
}

void onDisconnectedAPmode(const WiFiEventSoftAPModeStationDisconnected& event) {
  for (byte i = 0; i < 6; ++i) {
    lastMacDisconnectedAPmode[i] = event.mac[i];
  }
  processedDisconnectAPmode = false;
}

#endif // ifdef ESP8266
