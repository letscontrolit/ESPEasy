#include "../Globals/ESPEasyWiFiEvent.h"

#include "../../ESPEasy_common.h"


unsigned long connectionFailures = 0;

#ifdef ESP8266
WiFiEventHandler stationConnectedHandler;
WiFiEventHandler stationDisconnectedHandler;
WiFiEventHandler stationGotIpHandler;
WiFiEventHandler stationModeDHCPTimeoutHandler;
WiFiEventHandler APModeStationConnectedHandler;
WiFiEventHandler APModeStationDisconnectedHandler;
#endif // ifdef ESP8266


// WiFi related data
bool wifiSetup                                 = false;
bool wifiSetupConnect                          = false;
uint8_t wifiStatus                             = ESPEASY_WIFI_DISCONNECTED;
unsigned long last_wifi_connect_attempt_moment = 0;
unsigned int  wifi_connect_attempt             = 0;
int wifi_reconnects                            = -1; // First connection attempt is not a reconnect.
String  last_ssid;
bool    bssid_changed                     = false;
bool    channel_changed                   = false;

WiFiDisconnectReason lastDisconnectReason = WIFI_DISCONNECT_REASON_UNSPECIFIED;
unsigned long lastConnectMoment           = 0;
unsigned long lastDisconnectMoment        = 0;
unsigned long lastGetIPmoment             = 0;
unsigned long lastGetScanMoment           = 0;
unsigned long lastConnectedDuration       = 0;
bool intent_to_reboot                     = false;
uint8_t lastMacConnectedAPmode[6]         = { 0 };
uint8_t lastMacDisconnectedAPmode[6]      = { 0 };


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
