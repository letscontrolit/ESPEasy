#ifndef DATASTRUCTS_WIFIEVENTDATA_H
#define DATASTRUCTS_WIFIEVENTDATA_H

#include "../DataTypes/WiFiDisconnectReason.h"
#include "../Helpers/LongTermTimer.h"


#ifdef ESP32
# include <esp_event.h>
# include <WiFiGeneric.h>
# include <WiFiType.h>

#endif // ifdef ESP32

#include <IPAddress.h>

#ifdef ESP8266
# include <ESP8266WiFiGeneric.h>
# include <ESP8266WiFiType.h>
#endif // ifdef ESP8266


struct WiFiEventData_t {
  WiFiEventData_t();

  bool WiFiConnectAllowed() const;

  bool unprocessedWifiEvents() const;

  void clearAll();
  void markWiFiBegin();

  bool WiFiDisconnected() const;
  bool WiFiGotIP() const;
  bool WiFiConnected() const;
  bool WiFiServicesInitialized() const;

  void setWiFiDisconnected();
  void setWiFiGotIP();
  void setWiFiConnected();
  void setWiFiServicesInitialized();


  void markGotIP();
  void markDisconnect(WiFiDisconnectReason reason);
  void markConnected(const String& ssid,
                     const uint8_t bssid[6],
                     byte          channel);
  void markConnectedAPmode(const uint8_t mac[6]);
  void markDisconnectedAPmode(const uint8_t mac[6]);


  // WiFi related data
  bool          wifiSetup        = false;
  bool          wifiSetupConnect = false;
  uint8_t       wifiStatus;
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
  #endif // ifdef ESP32
};

#endif   // ifndef DATASTRUCTS_WIFIEVENTDATA_H
