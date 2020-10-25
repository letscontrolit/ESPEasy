#ifndef DATASTRUCTS_WIFIEVENTDATA_H
#define DATASTRUCTS_WIFIEVENTDATA_H

#include "../DataTypes/WiFiDisconnectReason.h"
#include "../Helpers/LongTermTimer.h"

// WifiStatus
#define ESPEASY_WIFI_DISCONNECTED            0

// Bit numbers for WiFi status
#define ESPEASY_WIFI_CONNECTED               0
#define ESPEASY_WIFI_GOT_IP                  1
#define ESPEASY_WIFI_SERVICES_INITIALIZED    2

#ifdef ESP32
# include <esp_event.h>
# include <WiFiGeneric.h>
# include <WiFiType.h>

#endif // ifdef ESP32

#include <IPAddress.h>

#ifdef ESP8266
# include <ESP8266WiFiGeneric.h>
# include <ESP8266WiFiType.h>
#endif


struct WiFiEventData_t {

  bool unprocessedWifiEvents() const;


  // WiFi related data
  bool          wifiSetup        = false;
  bool          wifiSetupConnect = false;
  uint8_t       wifiStatus       = ESPEASY_WIFI_DISCONNECTED;
  LongTermTimer last_wifi_connect_attempt_moment;
  unsigned int  wifi_connect_attempt   = 0;
  bool          wifi_considered_stable = false;
  int           wifi_reconnects        = -1; // First connection attempt is not a reconnect.
  String        last_ssid;
  bool          bssid_changed   = false;
  bool          channel_changed = false;

  WiFiDisconnectReason    lastDisconnectReason = WIFI_DISCONNECT_REASON_UNSPECIFIED;
  LongTermTimer           lastConnectMoment;
  LongTermTimer           lastDisconnectMoment;
  LongTermTimer           lastWiFiResetMoment;
  LongTermTimer           lastGetIPmoment;
  LongTermTimer           lastGetScanMoment;
  LongTermTimer::Duration lastConnectedDuration_us = 0ll;
  LongTermTimer           timerAPoff;   // Timer to check whether the AP mode should be disabled (0 = disabled)
  LongTermTimer           timerAPstart; // Timer to start AP mode, started when no valid network is detected.
  bool                    intent_to_reboot             = false;
  uint8_t                 lastMacConnectedAPmode[6]    = { 0 };
  uint8_t                 lastMacDisconnectedAPmode[6] = { 0 };


  // Semaphore like bools for processing data gathered from WiFi events.
  bool processedConnect          = true;
  bool processedDisconnect       = true;
  bool processedGotIP            = true;
  bool processedDHCPTimeout      = true;
  bool processedConnectAPmode    = true;
  bool processedDisconnectAPmode = true;
  bool processedScanDone         = true;
  bool wifiConnectAttemptNeeded  = true;
  bool wifiConnectInProgress     = false;

  unsigned long connectionFailures = 0;

  #ifdef ESP32
  WiFiEventId_t wm_event_id;
  #endif

};

#endif // ifndef DATASTRUCTS_WIFIEVENTDATA_H
