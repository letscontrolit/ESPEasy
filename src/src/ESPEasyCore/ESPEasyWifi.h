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


#define WIFI_RECONNECT_WAIT                20000  // in milliSeconds
#define WIFI_AP_OFF_TIMER_DURATION         60000  // in milliSeconds
#define WIFI_CONNECTION_CONSIDERED_STABLE  300000 // in milliSeconds
#define WIFI_ALLOW_AP_AFTERBOOT_PERIOD     5      // in minutes

bool WiFiConnected();
void WiFiConnectRelaxed();
bool prepareWiFi();
bool checkAndResetWiFi();
void resetWiFi();
void initWiFi();
void WifiDisconnect();
void WifiScan(bool async, bool quick);
void WifiScan();
void setSTA(bool enable);
void setAP(bool enable);
String getWifiModeString(WiFiMode_t wifimode);
void setWifiMode(WiFiMode_t wifimode);
bool WifiIsAP(WiFiMode_t wifimode);
bool useStaticIP();
bool wifiConnectTimeoutReached();
bool wifiAPmodeActivelyUsed();
void setConnectionSpeed();
void setupStaticIPconfig();
String formatScanResult(int i, const String& separator);
String formatScanResult(int i, const String& separator, int32_t& rssi);

#ifndef ESP32
String SDKwifiStatusToString(uint8_t sdk_wifistatus);
#endif

String ArduinoWifiStatusToString(uint8_t arduino_corelib_wifistatus);
String ESPeasyWifiStatusToString();
void logConnectionStatus();
String getLastDisconnectReason();

#endif // ESPEASY_WIFI_H