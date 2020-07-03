#include "ESPEasyWifi_ProcessEvent.h"
#include "ESPEasy-Globals.h"
#include "ESPEasyNetwork.h"
#include "ESPEasyWifi.h"
#include "ESPEasyWiFi_credentials.h"
#include "ESPEasy_fdwdecl.h"
#include "src/Globals/ESPEasyWiFiEvent.h"
#include "src/Globals/RTC.h"
#include "src/Globals/MQTT.h"
#include "src/Helpers/ESPEasy_time_calc.h"
#include "src/DataStructs/SchedulerTimers.h"

bool unprocessedWifiEvents() {
  if (processedConnect && processedDisconnect && processedGotIP && processedDHCPTimeout)
  {
    return false;
  }
  return true;
}

// ********************************************************************************
// Called from the loop() to make sure events are processed as soon as possible.
// These functions are called from Setup() or Loop() and thus may call delay() or yield()
// ********************************************************************************
void handle_unprocessedWiFiEvents()
{
  if (WiFi.status() == WL_DISCONNECTED) {
    delay(100);
  }

  if ((wifiStatus != ESPEASY_WIFI_SERVICES_INITIALIZED) || unprocessedWifiEvents()) {
    // WiFi connection is not yet available, so introduce some extra delays to
    // help the background tasks managing wifi connections
    delay(1);

    if (wifiConnectAttemptNeeded) {
      NetworkConnectRelaxed();
    }

    // Process disconnect events before connect events.
    if (!processedDisconnect) {
      #ifndef BUILD_NO_DEBUG
      addLog(LOG_LEVEL_DEBUG, F("WIFI : Entering processDisconnect()"));
      #endif // ifndef BUILD_NO_DEBUG
      processDisconnect();
    }

    if (!processedConnect) {
      #ifndef BUILD_NO_DEBUG
      addLog(LOG_LEVEL_DEBUG, F("WIFI : Entering processConnect()"));
      #endif // ifndef BUILD_NO_DEBUG
      processConnect();
    }

    if (!processedGotIP) {
      #ifndef BUILD_NO_DEBUG
      addLog(LOG_LEVEL_DEBUG, F("WIFI : Entering processGotIP()"));
      #endif // ifndef BUILD_NO_DEBUG
      processGotIP();
    }

    if (!processedDHCPTimeout) {
      #ifndef BUILD_NO_DEBUG
      addLog(LOG_LEVEL_DEBUG, F("WIFI : DHCP timeout, Calling disconnect()"));
      #endif // ifndef BUILD_NO_DEBUG
      processedDHCPTimeout = true;
      processDisconnect();
    }

    if (wifiStatus & ESPEASY_WIFI_CONNECTED) {
      // The actual connection has been made, no need to wait for IP to release this semaphore.
      wifiConnectInProgress = false;
    }
    if (wifiStatus != ESPEASY_WIFI_SERVICES_INITIALIZED) {
      if (WiFi.isConnected()) {
        // Apparently we did miss some WiFi events.
        if ((wifiStatus & ESPEASY_WIFI_CONNECTED) == 0) {
          addLog(LOG_LEVEL_DEBUG, F("WiFi : Force 'WiFi Connected' event"));
          processedConnect = false;
        }

        if ((wifiStatus & ESPEASY_WIFI_GOT_IP) == 0) {
          addLog(LOG_LEVEL_DEBUG, F("WiFi : Force 'WiFi Got IP' event"));
          processedGotIP = false;
        }
      }
    }


    if (wifiStatus != ESPEASY_WIFI_SERVICES_INITIALIZED) {
      if ((wifiStatus & ESPEASY_WIFI_GOT_IP) && (wifiStatus & ESPEASY_WIFI_CONNECTED)) {
        markWiFi_services_initialized();
      }
    }
  } else if (!NetworkConnected()) {
    // Somehow the WiFi has entered a limbo state.
    // FIXME TD-er: This may happen on WiFi config with AP_STA mode active.
    //    addLog(LOG_LEVEL_ERROR, F("Wifi status out sync"));
    //    resetWiFi();
    if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
      String wifilog = F("WIFI : Wifi status out sync WiFi.status() = ");
      wifilog += String(WiFi.status());

      addLog(LOG_LEVEL_ERROR, wifilog);
    }
  }

  if (wifiStatus == ESPEASY_WIFI_DISCONNECTED) {
    #ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      static unsigned long lastDisconnectMoment_log = 0;
      static uint8_t lastWiFiStatus_log = 0;
      uint8_t cur_wifi_status = WiFi.status();
      if (lastDisconnectMoment != lastDisconnectMoment_log || 
          lastWiFiStatus_log != cur_wifi_status) {
        lastDisconnectMoment_log = lastDisconnectMoment;
        lastWiFiStatus_log = cur_wifi_status;
        String wifilog = F("WIFI : Disconnected: WiFi.status() = ");
        wifilog += String(cur_wifi_status);

        addLog(LOG_LEVEL_DEBUG, wifilog);
      }
    }
    #endif // ifndef BUILD_NO_DEBUG

    // While connecting to WiFi make sure the device has ample time to do so
    delay(10);
  }

  if (!processedDisconnectAPmode) { processDisconnectAPmode(); }

  if (!processedConnectAPmode) { processConnectAPmode(); }

  if (timerAPoff != 0) { processDisableAPmode(); }

  if (!processedScanDone) { processScanDone(); }

  if (wifi_connect_attempt > 0) {
    // We only want to clear this counter if the connection is currently stable.
    if (wifiStatus == ESPEASY_WIFI_SERVICES_INITIALIZED) {
      if (timePassedSince(lastConnectMoment) > WIFI_CONNECTION_CONSIDERED_STABLE) {
        // Connection considered stable
        wifi_connect_attempt = 0;

        if (!WiFi.getAutoConnect()) {
          WiFi.setAutoConnect(true);
        }
      } else {
        if (WiFi.getAutoConnect()) {
          WiFi.setAutoConnect(false);
        }
      }
    }
  }
}

// ********************************************************************************
// Functions to process the data gathered from the events.
// These functions are called from Setup() or Loop() and thus may call delay() or yield()
// ********************************************************************************
void processDisconnect() {
  if (processedDisconnect) { return; }
  processedDisconnect = true;
  wifiStatus          = ESPEASY_WIFI_DISCONNECTED;
  delay(100); // FIXME TD-er: See https://github.com/letscontrolit/ESPEasy/issues/1987#issuecomment-451644424

  if (Settings.UseRules) {
    eventQueue.add(F("WiFi#Disconnected"));
  }

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("WIFI : Disconnected! Reason: '");
    log += getLastDisconnectReason();
    log += '\'';

    if (lastConnectedDuration > 0) {
      log += F(" Connected for ");
      log += format_msec_duration(lastConnectedDuration);
    }
    addLog(LOG_LEVEL_INFO, log);
  }

  if (Settings.WiFiRestart_connection_lost()) {
    setWifiMode(WIFI_OFF);
    delay(100);
  }
  logConnectionStatus();
}

void processConnect() {
  if (processedConnect) { return; }
  processedConnect = true;
  wifiStatus      |= ESPEASY_WIFI_CONNECTED;
  delay(100); // FIXME TD-er: See https://github.com/letscontrolit/ESPEasy/issues/1987#issuecomment-451644424
  ++wifi_reconnects;

  if (wifiStatus < ESPEASY_WIFI_CONNECTED) { return; }

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    const long connect_duration = timeDiff(last_wifi_connect_attempt_moment, lastConnectMoment);
    String     log              = F("WIFI : Connected! AP: ");
    log += WiFi.SSID();
    log += " (";
    log += WiFi.BSSIDstr();
    log += F(") Ch: ");
    log += RTC.lastWiFiChannel;

    if ((connect_duration > 0) && (connect_duration < 30000)) {
      // Just log times when they make sense.
      log += F(" Duration: ");
      log += connect_duration;
      log += F(" ms");
    }
    addLog(LOG_LEVEL_INFO, log);
  }

  if (Settings.UseRules) {
    if (bssid_changed) {
      eventQueue.add(F("WiFi#ChangedAccesspoint"));
    }

    if (channel_changed) {
      eventQueue.add(F("WiFi#ChangedWiFichannel"));
    }
  } 

  if (useStaticIP()) {
    markGotIP(); // in static IP config the got IP event is never fired.
  }
  saveToRTC();

  logConnectionStatus();
}

void processGotIP() {
  if (processedGotIP) {
    return;
  }
  if ((wifiStatus & ESPEASY_WIFI_CONNECTED) == 0) {
    // Only process GotIP events if we are connected.
    return;
  }
  IPAddress ip = NetworkLocalIP();

  if (!useStaticIP()) {
    if ((ip[0] == 0) && (ip[1] == 0) && (ip[2] == 0) && (ip[3] == 0)) {
      return;
    }
  }
  processedGotIP = true;
  wifiStatus    |= ESPEASY_WIFI_GOT_IP;
  const IPAddress gw       = NetworkGatewayIP();
  const IPAddress subnet   = NetworkSubnetMask();
  const long dhcp_duration = timeDiff(lastConnectMoment, lastGetIPmoment);

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("WIFI : ");

    if (useStaticIP()) {
      log += F("Static IP: ");
    } else {
      log += F("DHCP IP: ");
    }
    log += formatIP(ip);
    log += " (";
    log += NetworkGetHostname();
    log += F(") GW: ");
    log += formatIP(gw);
    log += F(" SN: ");
    log += formatIP(subnet);

    if ((dhcp_duration > 0) && (dhcp_duration < 30000)) {
      // Just log times when they make sense.
      log += F("   duration: ");
      log += dhcp_duration;
      log += F(" ms");
    }
    addLog(LOG_LEVEL_INFO, log);
  }

  // Might not work in core 2.5.0
  // See https://github.com/esp8266/Arduino/issues/5839
  if ((Settings.IP_Octet != 0) && (Settings.IP_Octet != 255))
  {
    ip[3] = Settings.IP_Octet;

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = F("IP   : Fixed IP octet:");
      log += formatIP(ip);
      addLog(LOG_LEVEL_INFO, log);
    }
    WiFi.config(ip, gw, subnet);
  }

  // First try to get the time, since that may be used in logs
  if (node_time.systemTimePresent()) {
    node_time.initTime();
  }
#ifdef USES_MQTT
  mqtt_reconnect_count        = 0;
  MQTTclient_should_reconnect = true;
  timermqtt_interval          = 100;
  setIntervalTimer(TIMER_MQTT);
#endif // USES_MQTT
  sendGratuitousARP_now();

  if (Settings.UseRules)
  {
    eventQueue.add(F("WiFi#Connected"));
  }
  statusLED(true);

  //  WiFi.scanDelete();

  if (wifiSetup) {
    // Wifi setup was active, Apparently these settings work.
    wifiSetup = false;
    SaveSettings();
  }
  logConnectionStatus();
}

// A client disconnected from the AP on this node.
void processDisconnectAPmode() {
  if (processedDisconnectAPmode) { return; }
  processedDisconnectAPmode = true;

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    const int nrStationsConnected = WiFi.softAPgetStationNum();
    String    log                 = F("AP Mode: Client disconnected: ");
    log += formatMAC(lastMacDisconnectedAPmode);
    log += F(" Connected devices: ");
    log += nrStationsConnected;
    addLog(LOG_LEVEL_INFO, log);
  }
}

// Client connects to AP on this node
void processConnectAPmode() {
  if (processedConnectAPmode) { return; }
  processedConnectAPmode = true;
  // Extend timer to switch off AP.
  timerAPoff = millis() + WIFI_AP_OFF_TIMER_DURATION;

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("AP Mode: Client connected: ");
    log += formatMAC(lastMacConnectedAPmode);
    log += F(" Connected devices: ");
    log += WiFi.softAPgetStationNum();
    addLog(LOG_LEVEL_INFO, log);
  }

  // Start DNS, only used if the ESP has no valid WiFi config
  // It will reply with it's own address on all DNS requests
  // (captive portal concept)
  if (!dnsServerActive) {
    dnsServerActive = true;
    dnsServer.start(DNS_PORT, "*", apIP);
  }
}

// Switch of AP mode when timeout reached and no client connected anymore.
void processDisableAPmode() {
  if (timerAPoff == 0) { return; }

  if (WifiIsAP(WiFi.getMode())) {
    // disable AP after timeout and no clients connected.
    if (timeOutReached(timerAPoff) && (WiFi.softAPgetStationNum() == 0)) {
      setAP(false);
    }
  }

  if (!WifiIsAP(WiFi.getMode())) {
    timerAPoff = 0;
  }
}

void processScanDone() {
  if (processedScanDone) { return; }

  // Better act on the scan done event, as it may get triggered for normal wifi begin calls.
  int8_t scanCompleteStatus = WiFi.scanComplete();
  switch (scanCompleteStatus) {
    case 0: // Nothing (yet) found
      if (timePassedSince(lastGetScanMoment) > 5000) {
        processedScanDone = true;
      }
      return;
    case -1: // WIFI_SCAN_RUNNING
      return;
    case -2: // WIFI_SCAN_FAILED
      addLog(LOG_LEVEL_ERROR, F("WiFi  : Scan failed"));
      processedScanDone = true;
      return;
  }

  lastGetScanMoment = millis();
  processedScanDone = true;

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("WIFI  : Scan finished, found: ");
    log += scanCompleteStatus;
    addLog(LOG_LEVEL_INFO, log);
  }

  int bestScanID           = -1;
  int32_t bestRssi         = -1000;
  uint8_t bestWiFiSettings = RTC.lastWiFiSettingsIndex;

  if (selectValidWiFiSettings() && scanCompleteStatus > 0) {
    const uint8_t startWiFiSettings = RTC.lastWiFiSettingsIndex;
    bool done = false;
    while (!done) {
      String ssid_to_check = getLastWiFiSettingsSSID(); 
      for (int i = 0; i < scanCompleteStatus; ++i) {
        if (WiFi.SSID(i) == ssid_to_check) {
          int32_t rssi = WiFi.RSSI(i);

          if (bestRssi < rssi) {
            bestRssi         = rssi;
            bestScanID       = i;
            bestWiFiSettings = RTC.lastWiFiSettingsIndex;
          }
        }
      }

      // Select the next WiFi settings.
      // RTC.lastWiFiSettingsIndex may be updated.
      if (!selectNextWiFiSettings()) {
        done = true; 
      }
      if (startWiFiSettings == RTC.lastWiFiSettingsIndex) {
        done = true; 
      }
    }

    if (bestScanID >= 0) {
      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        String log = F("WIFI  : Selected: ");
        log += formatScanResult(bestScanID, " ");
        addLog(LOG_LEVEL_INFO, log);
      }
      RTC.lastWiFiSettingsIndex = bestWiFiSettings;
      uint8_t *scanbssid = WiFi.BSSID(bestScanID);

      if (scanbssid) {
        for (int i = 0; i < 6; ++i) {
          RTC.lastBSSID[i] = *(scanbssid + i);
        }
      }
    }
  }
}


void markWiFi_services_initialized() {
  addLog(LOG_LEVEL_DEBUG, F("WiFi : WiFi services initialized"));
  wifiStatus            = ESPEASY_WIFI_SERVICES_INITIALIZED;
  wifiConnectInProgress = false;
  
  processedDHCPTimeout  = true;  // FIXME TD-er:  Is this ever happening?
}

#ifdef HAS_ETHERNET

void processEthernetConnected() {
  if (Settings.UseRules)
  {
    eventQueue.add(F("ETHERNET#Connected"));
  }
  statusLED(true);
}

void processEthernetDisconnected() {
  if (Settings.UseRules)
  {
    eventQueue.add(F("ETHERNET#Disconnected"));
  }
}

#endif