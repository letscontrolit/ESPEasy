#include "../DataStructs/WiFiEventData.h"

#include "../ESPEasyCore/ESPEasy_Log.h"

#include "../Globals/RTC.h"
#include "../Globals/SecuritySettings.h"
#include "../Globals/WiFi_AP_Candidates.h"

#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/Networking.h"

// Bit numbers for WiFi status
#define ESPEASY_WIFI_CONNECTED               0
#define ESPEASY_WIFI_GOT_IP                  1
#define ESPEASY_WIFI_SERVICES_INITIALIZED    2

#define WIFI_RECONNECT_WAIT                  30000  // in milliSeconds

#define CONNECT_TIMEOUT_MAX                  4000   // in milliSeconds

bool WiFiEventData_t::WiFiConnectAllowed() const {
  if (WiFi.status() == WL_IDLE_STATUS) {
    // FIXME TD-er: What to do now? Set a timer?
    //return false;
    if (last_wifi_connect_attempt_moment.isSet() && 
       !last_wifi_connect_attempt_moment.timeoutReached(WIFI_PROCESS_EVENTS_TIMEOUT)) {
      return false;
    }
  }
  if (!wifiConnectAttemptNeeded) return false;
  if (intent_to_reboot) return false;
  if (wifiSetupConnect) return true;
  if (wifiConnectInProgress) {
    if (last_wifi_connect_attempt_moment.isSet() && 
       !last_wifi_connect_attempt_moment.timeoutReached(WIFI_PROCESS_EVENTS_TIMEOUT)) {
      return false;
    }
  } 
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
  if (!processedConnect) {
    if (lastConnectMoment.isSet() && lastConnectMoment.timeoutReached(WIFI_PROCESS_EVENTS_TIMEOUT)) {
      return false;
    }
  }
  if (!processedGotIP) {
    if (lastGetIPmoment.isSet() && lastGetIPmoment.timeoutReached(WIFI_PROCESS_EVENTS_TIMEOUT)) {
      return false;
    }
  }
  if (!processedDisconnect) {
    if (lastDisconnectMoment.isSet() && lastDisconnectMoment.timeoutReached(WIFI_PROCESS_EVENTS_TIMEOUT)) {
      return false;
    }
  }
  if (!processedDHCPTimeout) {
    return false;
  }
  return true;
}

void WiFiEventData_t::clearAll() {
  markWiFiTurnOn();
  lastGetScanMoment.clear();
  last_wifi_connect_attempt_moment.clear();
  timerAPstart.clear();

  lastWiFiResetMoment.setNow();
  wifi_TX_pwr = 0;
  usedChannel = 0;
}

void WiFiEventData_t::markWiFiTurnOn() {
  setWiFiDisconnected();
//  lastDisconnectMoment.clear();
  lastConnectMoment.clear();
  lastGetIPmoment.clear();
  wifi_considered_stable    = false;
  
  clear_processed_flags();
}

void WiFiEventData_t::clear_processed_flags() {
  // Mark all flags to default to prevent handling old events.
  processedConnect          = true;
  processedDisconnect       = true;
  processedGotIP            = true;
  processedDHCPTimeout      = true;
  processedConnectAPmode    = true;
  processedDisconnectAPmode = true;
  processedScanDone         = true;
  wifiConnectAttemptNeeded  = true;
  wifiConnectInProgress     = false;
  processingDisconnect.clear();
  dns0_cache = IPAddress();
  dns1_cache = IPAddress();
}

void WiFiEventData_t::markWiFiBegin() {
  markWiFiTurnOn();
  last_wifi_connect_attempt_moment.setNow();
  wifiConnectInProgress  = true;
  usedChannel = 0;
  ++wifi_connect_attempt;
  if (!timerAPstart.isSet()) {
    timerAPstart.setMillisFromNow(3 * WIFI_RECONNECT_WAIT);
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
  wifiConnectInProgress = false;
  wifiStatus            = ESPEASY_WIFI_DISCONNECTED;
  last_wifi_connect_attempt_moment.clear();
  wifiConnectInProgress = false;
}

void WiFiEventData_t::setWiFiGotIP() {
  bitSet(wifiStatus, ESPEASY_WIFI_GOT_IP);
  processedGotIP = true;
  if (valid_DNS_address(WiFi.dnsIP(0))) {
    dns0_cache = WiFi.dnsIP(0);
  }
  if (valid_DNS_address(WiFi.dnsIP(1))) {
    dns1_cache = WiFi.dnsIP(1);
  }
}

void WiFiEventData_t::setWiFiConnected() {
  bitSet(wifiStatus, ESPEASY_WIFI_CONNECTED);
  processedConnect = true;
}

void WiFiEventData_t::setWiFiServicesInitialized() {
  if (!unprocessedWifiEvents() && WiFiConnected() && WiFiGotIP()) {
    # ifndef BUILD_NO_DEBUG
    addLog(LOG_LEVEL_DEBUG, F("WiFi : WiFi services initialized"));
    #endif
    bitSet(wifiStatus, ESPEASY_WIFI_SERVICES_INITIALIZED);
    wifiConnectInProgress = false;
    wifiConnectAttemptNeeded = false;
    dns0_cache = WiFi.dnsIP(0);
    dns1_cache = WiFi.dnsIP(1);
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
/*
  #if defined(ESP32)
  if ((WiFi.getMode() & WIFI_MODE_STA) == 0) return;
  #else // if defined(ESP32)
  if ((WiFi.getMode() & WIFI_STA) == 0) return;
  #endif // if defined(ESP32)
*/
  lastDisconnectMoment.setNow();
  usedChannel = 0;

  if (last_wifi_connect_attempt_moment.isSet() && !lastConnectMoment.isSet()) {
    // There was an unsuccessful connection attempt
    lastConnectedDuration_us = last_wifi_connect_attempt_moment.timeDiff(lastDisconnectMoment);
  } else {
    if (last_wifi_connect_attempt_moment.isSet())
      lastConnectedDuration_us = lastConnectMoment.timeDiff(lastDisconnectMoment);
    else 
      lastConnectedDuration_us = 0;
  }
  lastDisconnectReason = reason;
  processedDisconnect  = false;
  wifiConnectInProgress = false;
}

void WiFiEventData_t::markConnected(const String& ssid, const uint8_t bssid[6], uint8_t channel) {
  usedChannel = channel;
  lastConnectMoment.setNow();
  processedConnect    = false;
  channel_changed     = RTC.lastWiFiChannel != channel;
  last_ssid           = ssid;
  bssid_changed       = false;
  auth_mode           = WiFi_AP_Candidates.getCurrent().enc_type;

  RTC.lastWiFiChannel = channel;
  for (uint8_t i = 0; i < 6; ++i) {
    if (RTC.lastBSSID[i] != bssid[i]) {
      bssid_changed    = true;
      RTC.lastBSSID[i] = bssid[i];
    }
  }
}

void WiFiEventData_t::markConnectedAPmode(const uint8_t mac[6]) {
  lastMacConnectedAPmode = mac;
  processedConnectAPmode = false;
}

void WiFiEventData_t::markDisconnectedAPmode(const uint8_t mac[6]) {
  lastMacDisconnectedAPmode = mac;
  processedDisconnectAPmode = false;
}

void WiFiEventData_t::setAuthMode(uint8_t newMode) {
  auth_mode = newMode;
}


String WiFiEventData_t::ESPeasyWifiStatusToString() const {
  String log;
  if (WiFiDisconnected()) {
    log = F("DISCONNECTED");
  } else {
    if (WiFiConnected()) {
      log += F("Conn. ");
    }
    if (WiFiGotIP()) {
      log += F("IP ");
    }
    if (WiFiServicesInitialized()) {
      log += F("Init");
    }
  }
  return log;
}


uint32_t WiFiEventData_t::getSuggestedTimeout(int index, uint32_t minimum_timeout) const {
  auto it = connectDurations.find(index);
  if (it == connectDurations.end()) {
    return 3 * minimum_timeout;
  }
  const uint32_t res = 3 * it->second;
  return constrain(res, minimum_timeout, CONNECT_TIMEOUT_MAX);
}