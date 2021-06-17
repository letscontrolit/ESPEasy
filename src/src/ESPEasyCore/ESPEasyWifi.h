#ifndef ESPEASY_WIFI_H
#define ESPEASY_WIFI_H

#include "../../ESPEasy_common.h"

#if defined(ESP8266)
  # include <ESP8266WiFi.h>
  # include <ESP8266WebServer.h>
#endif // if defined(ESP8266)
#if defined(ESP32)
  # include <WiFi.h>
  # include <WebServer.h>
#endif // if defined(ESP32)

#include "../DataTypes/WiFiConnectionProtocol.h"
#include "../DataStructs/WiFi_AP_Candidate.h"

#include "../Helpers/LongTermTimer.h"

#define WIFI_RECONNECT_WAIT                 20000 // in milliSeconds
#define WIFI_AP_OFF_TIMER_DURATION         300000 // in milliSeconds
#define WIFI_CONNECTION_CONSIDERED_STABLE  300000 // in milliSeconds
#define WIFI_ALLOW_AP_AFTERBOOT_PERIOD     5      // in minutes
#define WIFI_SCAN_INTERVAL_AP_USED         125000 // in milliSeconds
#define WIFI_SCAN_INTERVAL_MINIMAL          60000 // in milliSeconds


#ifdef ESPEASY_WIFI_CLEANUP_WORK_IN_PROGRESS

enum class WiFiState_e {
  // WiFi radio off
  OFF,
  // Only running in AP mode
  AP_only,
  // WiFi was in some kind of error state, waiting period
  ErrorRecovery,
  // STA mode + scanning
  STA_Scanning,
  // STA+AP mode + scanning, 
  // needs some careful handling to prevent disconnecting the connected stations
  STA_AP_Scanning,
  // Connecting to an AP
  STA_Connecting,
  // Reconnecting to an AP
  // May need to handle some specific disconnect reasons differently from connecting for the first time.
  STA_Reconnecting,
  // Connected to an AP
  STA_Connected
};


class ESPEasyWiFi_t {
public:

  // Start the process of connecting or start AP, depending on the existing configuration.
  bool begin();

  // Terminate WiFi activity
  void end();

  // Process the state machine for managing WiFi connection
  void loop();

  WiFiState_e getState() const { return _state; }

  // Get the IP-address in this order:
  // - STA interface if connected, 
  // - AP interface if active
  // - 0.0.0.0 if neither connected nor active.
  IPAddress getIP() const;

  void disconnect();




private:

  // Handle timeouts + start of AP mode
  void checkConnectProgress();

  // Check to see if we already have some AP to connect to.
  void checkScanningProgress();

  void startScanning();

  bool connectSTA();
  

  WiFi_AP_Candidate _active_sta;
  WiFi_AP_Candidate _AP_conf;

  String _last_ssid;
  MAC_address _last_bssid;
  uint8_t _last_channel = 0;
  WiFiState_e _state = WiFiState_e::OFF;

  LongTermTimer _last_state_change;
  LongTermTimer _last_seen_connected;
};


#endif // ESPEASY_WIFI_CLEANUP_WORK_IN_PROGRESS

bool WiFiConnected();
void WiFiConnectRelaxed();
void AttemptWiFiConnect();
bool prepareWiFi();
bool checkAndResetWiFi();
void resetWiFi();
void initWiFi();
void SetWiFiTXpower();
void SetWiFiTXpower(float dBm); // 0-20.5
void SetWiFiTXpower(float dBm, float rssi);
float GetRSSIthreshold(float& maxTXpwr);
WiFiConnectionProtocol getConnectionProtocol();
void WifiDisconnect();
void WiFiScanPeriodical();
bool WiFiScanAllowed();
void WifiScan(bool async, uint8_t channel = 0);
void WifiScan();
void setSTA(bool enable);
void setAP(bool enable);
const __FlashStringHelper * getWifiModeString(WiFiMode_t wifimode);
void setWifiMode(WiFiMode_t wifimode);
bool WifiIsAP(WiFiMode_t wifimode);
bool WifiIsSTA(WiFiMode_t wifimode);
bool useStaticIP();
bool wifiConnectTimeoutReached();
bool wifiAPmodeActivelyUsed();
void setConnectionSpeed();
void setupStaticIPconfig();
String formatScanResult(int i, const String& separator);
String formatScanResult(int i, const String& separator, int32_t& rssi);

String ESPeasyWifiStatusToString();
void logConnectionStatus();


#endif // ESPEASY_WIFI_H