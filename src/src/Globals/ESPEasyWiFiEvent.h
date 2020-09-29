#ifndef GLOBALS_ESPEASYWIFIEVENT_H
#define GLOBALS_ESPEASYWIFIEVENT_H

#include <Arduino.h>
#include <IPAddress.h>
#include <stdint.h>

#include "../Helpers/LongTermTimer.h"

// WifiStatus
#define ESPEASY_WIFI_DISCONNECTED            0

// Bit numbers for WiFi status
#define ESPEASY_WIFI_CONNECTED               0
#define ESPEASY_WIFI_GOT_IP                  1
#define ESPEASY_WIFI_SERVICES_INITIALIZED    2


extern unsigned long connectionFailures;


#ifdef ESP32
# include <esp_event.h>
# include <WiFiGeneric.h>
# include <WiFiType.h>


enum WiFiDisconnectReason
{
  WIFI_DISCONNECT_REASON_UNSPECIFIED              = 1,
  WIFI_DISCONNECT_REASON_AUTH_EXPIRE              = 2,
  WIFI_DISCONNECT_REASON_AUTH_LEAVE               = 3,
  WIFI_DISCONNECT_REASON_ASSOC_EXPIRE             = 4,
  WIFI_DISCONNECT_REASON_ASSOC_TOOMANY            = 5,
  WIFI_DISCONNECT_REASON_NOT_AUTHED               = 6,
  WIFI_DISCONNECT_REASON_NOT_ASSOCED              = 7,
  WIFI_DISCONNECT_REASON_ASSOC_LEAVE              = 8,
  WIFI_DISCONNECT_REASON_ASSOC_NOT_AUTHED         = 9,
  WIFI_DISCONNECT_REASON_DISASSOC_PWRCAP_BAD      = 10, /* 11h */
  WIFI_DISCONNECT_REASON_DISASSOC_SUPCHAN_BAD     = 11, /* 11h */
  WIFI_DISCONNECT_REASON_IE_INVALID               = 13, /* 11i */
  WIFI_DISCONNECT_REASON_MIC_FAILURE              = 14, /* 11i */
  WIFI_DISCONNECT_REASON_4WAY_HANDSHAKE_TIMEOUT   = 15, /* 11i */
  WIFI_DISCONNECT_REASON_GROUP_KEY_UPDATE_TIMEOUT = 16, /* 11i */
  WIFI_DISCONNECT_REASON_IE_IN_4WAY_DIFFERS       = 17, /* 11i */
  WIFI_DISCONNECT_REASON_GROUP_CIPHER_INVALID     = 18, /* 11i */
  WIFI_DISCONNECT_REASON_PAIRWISE_CIPHER_INVALID  = 19, /* 11i */
  WIFI_DISCONNECT_REASON_AKMP_INVALID             = 20, /* 11i */
  WIFI_DISCONNECT_REASON_UNSUPP_RSN_IE_VERSION    = 21, /* 11i */
  WIFI_DISCONNECT_REASON_INVALID_RSN_IE_CAP       = 22, /* 11i */
  WIFI_DISCONNECT_REASON_802_1X_AUTH_FAILED       = 23, /* 11i */
  WIFI_DISCONNECT_REASON_CIPHER_SUITE_REJECTED    = 24, /* 11i */

  WIFI_DISCONNECT_REASON_BEACON_TIMEOUT    = 200,
  WIFI_DISCONNECT_REASON_NO_AP_FOUND       = 201,
  WIFI_DISCONNECT_REASON_AUTH_FAIL         = 202,
  WIFI_DISCONNECT_REASON_ASSOC_FAIL        = 203,
  WIFI_DISCONNECT_REASON_HANDSHAKE_TIMEOUT = 204
};

void WiFiEvent(system_event_id_t   event,
               system_event_info_t info);
extern WiFiEventId_t wm_event_id;
#endif // ifdef ESP32

#ifdef ESP8266
# include <ESP8266WiFiGeneric.h>
# include <ESP8266WiFiType.h>
class IPAddress;

extern WiFiEventHandler stationConnectedHandler;
extern WiFiEventHandler stationDisconnectedHandler;
extern WiFiEventHandler stationGotIpHandler;
extern WiFiEventHandler stationModeDHCPTimeoutHandler;
extern WiFiEventHandler APModeStationConnectedHandler;
extern WiFiEventHandler APModeStationDisconnectedHandler;
#endif // ifdef ESP8266


// WiFi related data
extern bool wifiSetup;
extern bool wifiSetupConnect;
extern uint8_t wifiStatus;
extern LongTermTimer last_wifi_connect_attempt_moment;
extern unsigned int  wifi_connect_attempt;
extern bool   wifi_considered_stable;
extern int    wifi_reconnects; // First connection attempt is not a reconnect.
extern String last_ssid;
extern bool   bssid_changed;
extern bool   channel_changed;

extern WiFiDisconnectReason lastDisconnectReason;
extern LongTermTimer lastConnectMoment;
extern LongTermTimer lastDisconnectMoment;
extern LongTermTimer lastWiFiResetMoment;
extern LongTermTimer lastGetIPmoment;
extern LongTermTimer lastGetScanMoment;
extern LongTermTimer::Duration lastConnectedDuration_us;
extern LongTermTimer timerAPoff;   // Timer to check whether the AP mode should be disabled (0 = disabled)
extern LongTermTimer timerAPstart; // Timer to start AP mode, started when no valid network is detected.

extern bool intent_to_reboot;
extern uint8_t lastMacConnectedAPmode[6];
extern uint8_t lastMacDisconnectedAPmode[6];


// Semaphore like bools for processing data gathered from WiFi events.
extern volatile bool processedConnect;
extern volatile bool processedDisconnect;
extern volatile bool processedGotIP;
extern volatile bool processedDHCPTimeout;
extern volatile bool processedConnectAPmode;
extern volatile bool processedDisconnectAPmode;
extern volatile bool processedScanDone;
extern bool wifiConnectAttemptNeeded;
extern bool wifiConnectInProgress;

#endif // GLOBALS_ESPEASYWIFIEVENT_H
