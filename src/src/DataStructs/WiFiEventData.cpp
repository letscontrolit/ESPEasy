#include "WiFiEventData.h"

#include "../ESPEasyCore/ESPEasy_Log.h"

#include "../Globals/RTC.h"
#include "../Globals/SecuritySettings.h"
#include "../Globals/WiFi_AP_Candidates.h"

#include "../Helpers/ESPEasy_Storage.h"


// Bit numbers for WiFi status
#define ESPEASY_WIFI_CONNECTED               0
#define ESPEASY_WIFI_GOT_IP                  1
#define ESPEASY_WIFI_SERVICES_INITIALIZED    2

#define WIFI_RECONNECT_WAIT                  20000  // in milliSeconds

bool WiFiEventData_t::WiFiConnectAllowed() const {
  if (!wifiConnectAttemptNeeded) return false;
  if (wifiSetupConnect) return true;
  if (lastDisconnectMoment.isSet()) {
    // TODO TD-er: Make this time more dynamic.
    if (!lastDisconnectMoment.timeoutReached(1000)) {
      return false;
    }
  }
  return true;
}

bool WiFiEventData_t::unprocessedWifiEvents() const {
  if (processedConnect && processedDisconnect && processedGotIP && processedDHCPTimeout)
  {
    return false;
  }
  return true;
}

void WiFiEventData_t::clearAll() {
  lastDisconnectMoment.clear();
  lastConnectMoment.clear();
  lastGetIPmoment.clear();
  lastGetScanMoment.clear();
  last_wifi_connect_attempt_moment.clear();
  timerAPstart.clear();

  setWiFiDisconnected();
  lastWiFiResetMoment.setNow();
  wifi_considered_stable = false;

  // Mark all flags to default to prevent handling old events.
  processedConnect          = true;
  processedDisconnect       = true;
  processedGotIP            = true;
  processedDHCPTimeout      = true;
  processedConnectAPmode    = true;
  processedDisconnectAPmode = true;
  processedScanDone         = true;
  wifiConnectAttemptNeeded  = true;
  wifi_TX_pwr = 0;
  usedChannel = 0;
}

void WiFiEventData_t::markWiFiBegin() {
  setWiFiDisconnected();
  lastDisconnectMoment.clear();
  lastConnectMoment.clear();
  lastGetIPmoment.clear();
  last_wifi_connect_attempt_moment.setNow();
  wifi_considered_stable = false;
  wifiConnectInProgress  = true;
  usedChannel = 0;
  ++wifi_connect_attempt;
  if (!timerAPstart.isSet()) {
    timerAPstart.setMillisFromNow(WIFI_RECONNECT_WAIT);
  }
}

bool WiFiEventData_t::WiFiDisconnected() const {
  return wifiStatus == ESPEASY_WIFI_DISCONNECTED;
}

bool WiFiEventData_t::WiFiGotIP() const {
  return bitRead(wifiStatus, ESPEASY_WIFI_GOT_IP);
}

bool WiFiEventData_t::WiFiConnected() const {
  return bitRead(wifiStatus, ESPEASY_WIFI_CONNECTED);
}

bool WiFiEventData_t::WiFiServicesInitialized() const {
  return bitRead(wifiStatus, ESPEASY_WIFI_SERVICES_INITIALIZED);
}

void WiFiEventData_t::setWiFiDisconnected() {
  wifiStatus = ESPEASY_WIFI_DISCONNECTED;
}

void WiFiEventData_t::setWiFiGotIP() {
  bitSet(wifiStatus, ESPEASY_WIFI_GOT_IP);
}

void WiFiEventData_t::setWiFiConnected() {
  bitSet(wifiStatus, ESPEASY_WIFI_CONNECTED);
}

void WiFiEventData_t::setWiFiServicesInitialized() {
  if (!unprocessedWifiEvents()) {
    addLog(LOG_LEVEL_DEBUG, F("WiFi : WiFi services initialized"));
    bitSet(wifiStatus, ESPEASY_WIFI_SERVICES_INITIALIZED);
    wifiConnectInProgress = false;
  }
}

void WiFiEventData_t::markGotIP() {
  lastGetIPmoment.setNow();

  // Create the 'got IP event' so mark the wifiStatus to not have the got IP flag set
  // This also implies the services are not fully initialized.
  bitClear(wifiStatus, ESPEASY_WIFI_GOT_IP);
  bitClear(wifiStatus, ESPEASY_WIFI_SERVICES_INITIALIZED);
  processedGotIP = false;
}

void WiFiEventData_t::markLostIP() {
  bitClear(wifiStatus, ESPEASY_WIFI_GOT_IP);
  bitClear(wifiStatus, ESPEASY_WIFI_SERVICES_INITIALIZED);
}

void WiFiEventData_t::markDisconnect(WiFiDisconnectReason reason) {
  lastDisconnectMoment.setNow();
  usedChannel = 0;

  if (last_wifi_connect_attempt_moment.isSet() && !lastConnectMoment.isSet()) {
    // There was an unsuccessful connection attempt
    lastConnectedDuration_us = last_wifi_connect_attempt_moment.timeDiff(lastDisconnectMoment);
  } else {
    lastConnectedDuration_us = lastConnectMoment.timeDiff(lastDisconnectMoment);
  }
  lastDisconnectReason = reason;
  processedDisconnect  = false;
  wifiConnectInProgress = false;
}

void WiFiEventData_t::markConnected(const String& ssid, const uint8_t bssid[6], byte channel) {
  usedChannel = channel;
  lastConnectMoment.setNow();
  processedConnect    = false;
  channel_changed     = RTC.lastWiFiChannel != channel;
  last_ssid           = ssid;
  bssid_changed       = false;
  auth_mode           = WiFi_AP_Candidates.getCurrent().enc_type;

  RTC.lastWiFiChannel = channel;
  for (byte i = 0; i < 6; ++i) {
    if (RTC.lastBSSID[i] != bssid[i]) {
      bssid_changed    = true;
      RTC.lastBSSID[i] = bssid[i];
    }
  }
}

void WiFiEventData_t::markConnectedAPmode(const uint8_t mac[6]) {
  for (byte i = 0; i < 6; ++i) {
    lastMacConnectedAPmode[i] = mac[i];
  }
  processedConnectAPmode = false;
}

void WiFiEventData_t::markDisconnectedAPmode(const uint8_t mac[6]) {
  for (byte i = 0; i < 6; ++i) {
    lastMacDisconnectedAPmode[i] = mac[i];
  }
  processedDisconnectAPmode = false;
}

void WiFiEventData_t::setAuthMode(uint8_t newMode) {
  auth_mode = newMode;
}