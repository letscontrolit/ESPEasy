#include "src/Globals/ESPEasyWiFiEvent.h"

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
      WiFiConnectRelaxed();
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

    if ((wifiStatus & ESPEASY_WIFI_GOT_IP) && (wifiStatus & ESPEASY_WIFI_CONNECTED) && WiFi.isConnected()) {
      wifiStatus = ESPEASY_WIFI_SERVICES_INITIALIZED;
      wifiConnectInProgress = false;
      resetAPdisableTimer();
    }
  } else if (!WiFiConnected()) {
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
      String wifilog = F("WIFI : Disconnected: WiFi.status() = ");
      wifilog += String(WiFi.status());

      addLog(LOG_LEVEL_DEBUG, wifilog);
    }
    #endif // ifndef BUILD_NO_DEBUG

    // While connecting to WiFi make sure the device has ample time to do so
    delay(10);
  }

  if (!processedDisconnectAPmode) { processDisconnectAPmode(); }

  if (!processedConnectAPmode) { processConnectAPmode(); }

  if (timerAPoff != 0) { processDisableAPmode(); }

  if (!processedScanDone) { processScanDone(); }
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
    String event = F("WiFi#Disconnected");
    rulesProcessing(event);
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
    log += last_channel;

    if ((connect_duration > 0) && (connect_duration < 30000)) {
      // Just log times when they make sense.
      log += F(" Duration: ");
      log += connect_duration;
      log += F(" ms");
    }
    addLog(LOG_LEVEL_INFO, log);
  }

  if (Settings.UseRules && bssid_changed) {
    String event = F("WiFi#ChangedAccesspoint");
    rulesProcessing(event);
  }

  if (Settings.UseRules && channel_changed) {
    String event = F("WiFi#ChangedWiFichannel");
    rulesProcessing(event);
  }

  if (useStaticIP()) {
    markGotIP(); // in static IP config the got IP event is never fired.
  }

  if (!WiFi.getAutoConnect()) {
    WiFi.setAutoConnect(true);
  }
  logConnectionStatus();
}

void processGotIP() {
  if (processedGotIP) {
    return;
  }
  IPAddress ip = WiFi.localIP();

  if (!useStaticIP()) {
    if ((ip[0] == 0) && (ip[1] == 0) && (ip[2] == 0) && (ip[3] == 0)) {
      return;
    }
  }
  processedGotIP = true;
  wifiStatus    |= ESPEASY_WIFI_GOT_IP;
  const IPAddress gw       = WiFi.gatewayIP();
  const IPAddress subnet   = WiFi.subnetMask();
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
    log += WifiGetHostname();
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

  #ifdef FEATURE_MDNS
  addLog(LOG_LEVEL_INFO, F("WIFI : Starting mDNS..."));
  bool mdns_started = MDNS.begin(WifiGetHostname().c_str());

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("WIFI : ");

    if (mdns_started) {
      log += F("mDNS started, with name: ");
      log += WifiGetHostname();
      log += F(".local");
    }
    else {
      log += F("mDNS failed");
    }
    addLog(LOG_LEVEL_INFO, log);
  }
  #endif // ifdef FEATURE_MDNS

  // First try to get the time, since that may be used in logs
  if (systemTimePresent()) {
    initTime();
  }
#ifdef USES_MQTT
  mqtt_reconnect_count        = 0;
  MQTTclient_should_reconnect = true;
  timermqtt_interval          = 100;
  setIntervalTimer(TIMER_MQTT);
#endif //USES_MQTT
  sendGratuitousARP_now();

  if (Settings.UseRules)
  {
    String event = F("WiFi#Connected");
    rulesProcessing(event);
  }
  statusLED(true);

  //  WiFi.scanDelete();
  setWebserverRunning(true);
  #ifdef FEATURE_MDNS

  if (mdns_started) {
    MDNS.addService("http", "tcp", 80);
  }
  #endif // ifdef FEATURE_MDNS
  wifi_connect_attempt = 0;

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
  resetAPdisableTimer();

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
  resetAPdisableTimer();

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("AP Mode: Client connected: ");
    log += formatMAC(lastMacConnectedAPmode);
    log += F(" Connected devices: ");
    log += WiFi.softAPgetStationNum();
    addLog(LOG_LEVEL_INFO, log);
  }
  setWebserverRunning(true);

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
  bool APmodeActive = WifiIsAP(WiFi.getMode());

  if (APmodeActive) {
    // disable AP after timeout and no clients connected.
    if (timeOutReached(timerAPoff) && (WiFi.softAPgetStationNum() == 0)) {
      setAP(false);
      APmodeActive = false;
    }
  }

  if (!APmodeActive) {
    timerAPoff = 0;
  }
}

void processScanDone() {
  if (processedScanDone) { return; }
  processedScanDone = true;

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("WIFI  : Scan finished, found: ");
    log += scan_done_number;
    addLog(LOG_LEVEL_INFO, log);
  }

  int bestScanID           = -1;
  int32_t bestRssi         = -1000;
  uint8_t bestWiFiSettings = lastWiFiSettings;

  if (selectValidWiFiSettings()) {
    bool   done                 = false;
    String lastWiFiSettingsSSID = getLastWiFiSettingsSSID();

    for (int settingNr = 0; !done && settingNr < 2; ++settingNr) {
      for (int i = 0; i < scan_done_number; ++i) {
        if (WiFi.SSID(i) == lastWiFiSettingsSSID) {
          int32_t rssi = WiFi.RSSI(i);

          if (bestRssi < rssi) {
            bestRssi         = rssi;
            bestScanID       = i;
            bestWiFiSettings = lastWiFiSettings;
          }
        }
      }

      if (!selectNextWiFiSettings()) { done = true; }
    }

    if (bestScanID >= 0) {
      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        String log = F("WIFI  : Selected: ");
        log += formatScanResult(bestScanID, " ");
        addLog(LOG_LEVEL_INFO, log);
      }
      lastWiFiSettings = bestWiFiSettings;
      uint8_t *scanbssid = WiFi.BSSID(bestScanID);

      if (scanbssid) {
        for (int i = 0; i < 6; ++i) {
          lastBSSID[i] = *(scanbssid + i);
        }
      }
    }
  }
}
