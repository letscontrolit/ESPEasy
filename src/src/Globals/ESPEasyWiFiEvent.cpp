#include "../Globals/ESPEasyWiFiEvent.h"

#include "../../ESPEasy_common.h"


unsigned long connectionFailures = 0;

#ifdef ESP32
WiFiEventId_t wm_event_id;
#endif // ifdef ESP32

#ifdef ESP8266
WiFiEventHandler stationConnectedHandler;
WiFiEventHandler stationDisconnectedHandler;
WiFiEventHandler stationGotIpHandler;
WiFiEventHandler stationModeDHCPTimeoutHandler;
WiFiEventHandler APModeStationConnectedHandler;
WiFiEventHandler APModeStationDisconnectedHandler;
#endif // ifdef ESP8266


// WiFi related data
bool wifiSetup        = false;
bool wifiSetupConnect = false;
uint8_t wifiStatus    = ESPEASY_WIFI_DISCONNECTED;
LongTermTimer last_wifi_connect_attempt_moment;
unsigned int  wifi_connect_attempt = 0;
bool   wifi_considered_stable      = false;
int    wifi_reconnects             = -1; // First connection attempt is not a reconnect.
String last_ssid;
bool   bssid_changed   = false;
bool   channel_changed = false;

WiFiDisconnectReason lastDisconnectReason = WIFI_DISCONNECT_REASON_UNSPECIFIED;
LongTermTimer lastConnectMoment;
LongTermTimer lastDisconnectMoment;
LongTermTimer lastWiFiResetMoment;
LongTermTimer lastGetIPmoment;
LongTermTimer lastGetScanMoment;
LongTermTimer::Duration lastConnectedDuration_us = 0ll;
LongTermTimer timerAPoff;   // Timer to check whether the AP mode should be disabled (0 = disabled)
LongTermTimer timerAPstart; // Timer to start AP mode, started when no valid network is detected.
bool intent_to_reboot                = false;
uint8_t lastMacConnectedAPmode[6]    = { 0 };
uint8_t lastMacDisconnectedAPmode[6] = { 0 };


// Semaphore like bools for processing data gathered from WiFi events.
volatile bool processedConnect          = true;
volatile bool processedDisconnect       = true;
volatile bool processedGotIP            = true;
volatile bool processedDHCPTimeout      = true;
volatile bool processedConnectAPmode    = true;
volatile bool processedDisconnectAPmode = true;
volatile bool processedScanDone         = true;
bool wifiConnectAttemptNeeded           = true;
bool wifiConnectInProgress              = false;
