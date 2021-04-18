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

#define WIFI_RECONNECT_WAIT                 20000 // in milliSeconds
#define WIFI_AP_OFF_TIMER_DURATION         300000 // in milliSeconds
#define WIFI_CONNECTION_CONSIDERED_STABLE  300000 // in milliSeconds
#define WIFI_ALLOW_AP_AFTERBOOT_PERIOD     5      // in minutes
#define WIFI_SCAN_INTERVAL_AP_USED         125000 // in milliSeconds
#define WIFI_SCAN_INTERVAL_MINIMAL          60000 // in milliSeconds

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
String getWifiModeString(WiFiMode_t wifimode);
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