
#define WIFI_AP_OFF_TIMER_DURATION  60000   // in milliSeconds

//********************************************************************************
// Functions to process the data gathered from the events.
// These functions are called from Setup() or Loop() and thus may call delay() or yield()
//********************************************************************************
void processConnect() {
  if (processedConnect) return;
  processedConnect = true;
  if (wifiStatus < ESPEASY_WIFI_CONNECTED) return;
  const long connect_duration = timeDiff(last_wifi_connect_attempt_moment, lastConnectMoment);
  String log = F("WIFI : Connected! AP: ");
  log += WiFi.SSID();
  log += F(" (");
  log += WiFi.BSSIDstr();
  log += F(") Ch: ");
  log += last_channel;
  if (connect_duration > 0 && connect_duration < 30000) {
    // Just log times when they make sense.
    log += F(" Duration: ");
    log += connect_duration;
    log += F(" ms");
  }
  addLog(LOG_LEVEL_INFO, log);
  if (useStaticIP())
  {
    const IPAddress ip = Settings.IP;
    const IPAddress gw = Settings.Gateway;
    const IPAddress subnet = Settings.Subnet;
    const IPAddress dns = Settings.DNS;
    log = F("IP   : Static IP : ");
    log += formatIP(ip);
    log += F(" GW: ");
    log += formatIP(gw);
    log += F(" SN: ");
    log += formatIP(subnet);
    log += F(" DNS: ");
    log += formatIP(dns);
    addLog(LOG_LEVEL_INFO, log);
    WiFi.config(ip, gw, subnet, dns);
  }
  if (Settings.UseRules && bssid_changed) {
    String event = F("WiFi#ChangedAccesspoint");
    rulesProcessing(event);
  }
  setWifiState(WifiConnectSuccess);
}

void processDisconnect() {
  if (processedDisconnect) return;
  processedDisconnect = true;
  if (Settings.UseRules) {
    String event = F("WiFi#Disconnected");
    rulesProcessing(event);
  }
  String log = F("WIFI : Disconnected! Reason: '");
  log += getLastDisconnectReason();
  log += F("'");
  if (lastConnectedDuration > 0) {
    log += F(" Connected for ");
    log += format_msec_duration(lastConnectedDuration);
  }
  addLog(LOG_LEVEL_INFO, log);
  if (wifi_connect_attempt > 5) {
    setWifiState(WifiConnectionFailed);
  } else {
    switch (lastDisconnectReason) {
      case WIFI_DISCONNECT_REASON_NO_AP_FOUND:
      case WIFI_DISCONNECT_REASON_AUTH_FAIL:
      case WIFI_DISCONNECT_REASON_ASSOC_FAIL:
      case WIFI_DISCONNECT_REASON_HANDSHAKE_TIMEOUT:
        // Disconnect without chance for success on next attempt
        if (selectNextWiFiSettings()) {
          if (!intent_to_reboot)
            setWifiState(WifiTryConnect);
          return;
        } else {
          if (Settings.deepSleep && Settings.deepSleepOnFail) {
            //only one attempt in deepsleep, to conserve battery
            addLog(LOG_LEVEL_ERROR, F("SLEEP: Connection failed, going back to sleep."));
            deepSleep(Settings.Delay);
          }
          setWifiState(WifiConnectionFailed);
          return;
        }
        break;
      default:
        setWifiState(WifiStart);
        if (!intent_to_reboot)
          setWifiState(WifiTryConnect);
        return;
    }
  }
}


void processGotIP() {
  if (processedGetIP)
    return;
  processedGetIP = true;
  if (wifiStatus < ESPEASY_WIFI_GOT_IP)
    return;
  IPAddress ip = WiFi.localIP();
  if (ip[0] == 0 && ip[1] == 0 && ip[2] == 0 && ip[3] == 0)
    return;
  const IPAddress gw = WiFi.gatewayIP();
  const IPAddress subnet = WiFi.subnetMask();
  String log = F("WIFI : ");
  if (useStaticIP()) {
    log += F("Static IP: ");
  } else {
    log += F("DHCP IP: ");
  }
  log += formatIP(ip);
  log += F(" (");
  log += WifiGetHostname();
  log += F(") GW: ");
  log += formatIP(gw);
  log += F(" SN: ");
  log += formatIP(subnet);

  const long dhcp_duration = timeDiff(lastConnectMoment, lastGetIPmoment);
  if (dhcp_duration > 0 && dhcp_duration < 30000) {
    // Just log times when they make sense.
    log += F("   duration: ");
    log += dhcp_duration;
    log += F(" ms");
  }
  addLog(LOG_LEVEL_INFO, log);

  // fix octet?
  if (Settings.IP_Octet != 0 && Settings.IP_Octet != 255)
  {
    ip[3] = Settings.IP_Octet;
    log = F("IP   : Fixed IP octet:");
    log += formatIP(ip);
    addLog(LOG_LEVEL_INFO, log);
    WiFi.config(ip, gw, subnet);
  }

  #ifdef FEATURE_MDNS

    log = F("WIFI : ");
    if (MDNS.begin(WifiGetHostname().c_str(), WiFi.localIP())) {

      log += F("mDNS started, with name: ");
      log += WifiGetHostname();
      log += F(".local");
    }
    else{
      log += F("mDNS failed");
    }
    addLog(LOG_LEVEL_INFO, log);
  #endif

  // First try to get the time, since that may be used in logs
  if (Settings.UseNTP) {
    initTime();
  }
  timermqtt_interval = 100;
  timermqtt = millis() + timermqtt_interval;
  if (Settings.UseRules)
  {
    String event = F("WiFi#Connected");
    rulesProcessing(event);
  }
  statusLED(true);
//  WiFi.scanDelete();
  wifiStatus = ESPEASY_WIFI_SERVICES_INITIALIZED;
  if (wifiSetup) {
    // Wifi setup was active, Apparently these settings work.
    wifiSetup = false;
    SaveSettings();
  }
}

void processConnectAPmode() {
  if (processedConnectAPmode) return;
  processedConnectAPmode = true;
  String log = F("AP Mode: Client connected: ");
  log += formatMAC(lastMacConnectedAPmode);
  log += F(" Connected devices: ");
  log += WiFi.softAPgetStationNum();
  addLog(LOG_LEVEL_INFO, log);
  setWifiState(WifiClientConnectAP);
}

void processDisconnectAPmode() {
  if (processedDisconnectAPmode) return;
  processedDisconnectAPmode = true;
  const int nrStationsConnected = WiFi.softAPgetStationNum();
  String log = F("AP Mode: Client disconnected: ");
  log += formatMAC(lastMacDisconnectedAPmode);
  log += F(" Connected devices: ");
  log += nrStationsConnected;
  addLog(LOG_LEVEL_INFO, log);
  if (nrStationsConnected == 0) {
    setWifiState(WifiClientDisconnectAP);
  }
}

void processScanDone() {
  if (processedScanDone) return;
  processedScanDone = true;
  String log = F("WIFI  : Scan finished, found: ");
  log += scan_done_number;
  addLog(LOG_LEVEL_INFO, log);

  int bestScanID = -1;
  int32_t bestRssi = -1000;
  uint8_t bestWiFiSettings = lastWiFiSettings;
  if (selectValidWiFiSettings()) {
    bool done = false;
    String lastWiFiSettingsSSID = getLastWiFiSettingsSSID();
    for (int settingNr = 0; !done && settingNr < 2; ++settingNr) {
      for (int i = 0; i < scan_done_number; ++i) {
        if (WiFi.SSID(i) == lastWiFiSettingsSSID) {
          int32_t rssi = WiFi.RSSI(i);
          if (bestRssi < rssi) {
            bestRssi = rssi;
            bestScanID = i;
            bestWiFiSettings = lastWiFiSettings;
          }
        }
      }
      if (!selectNextWiFiSettings()) done = true;
    }
    if (bestScanID >= 0) {
      log = F("WIFI  : Selected: ");
      log += formatScanResult(bestScanID, " ");
      addLog(LOG_LEVEL_INFO, log);
      lastWiFiSettings = bestWiFiSettings;
      uint8_t * scanbssid = WiFi.BSSID(bestScanID);
      if (scanbssid) {
        for (int i = 0; i < 6 ; ++i) {
          lastBSSID[i] = *(scanbssid + i);
        }
      }
    }
    setWifiState(WifiTryConnect);
  }
}

void resetWiFi() {
  addLog(LOG_LEVEL_INFO, F("Reset WiFi."));
  setWifiState(WifiOff);
  setWifiState(WifiStart);
  lastDisconnectMoment = millis();
  processedDisconnect = false;
  wifiStatus = ESPEASY_WIFI_DISCONNECTED;
}

void WifiScanAsync() {
  setWifiState(WifiStartScan);
  addLog(LOG_LEVEL_INFO, F("WIFI  : Start network scan"));
  #ifdef ESP32
    bool async = true;
    bool show_hidden = false;
    bool passive = false;
    uint32_t max_ms_per_chan = 300;
    WiFi.scanNetworks(async, show_hidden, passive, max_ms_per_chan);
  #else
      // 2.4.x only and it doesn't work like expected.
//    WiFi.scanNetworksAsync(onScanFinished);
  #endif
}

//********************************************************************************
// WiFi state
//********************************************************************************
// 0  Wifi off
// 1  Start                        => STA
// 2  Try connect                  => STA      => successful, goto 7
// 3  Connection fails             => AP       => no client in N seconds, goto 1
// 4  Client connects              => AP
// 5  Credentials changed          => STA_AP   => goto 2
// 6  Connect successful (AP on)   => STA_AP   => disable AP after N seconds, goto 8
// 7  Connect successful           => STA
// 8  Disable AP after N seconds   => STA
//********************************************************************************

bool WifiIsAP(WiFiMode_t wifimode)
{
  #if defined(ESP32)
    return (wifimode == WIFI_MODE_AP) || (wifimode == WIFI_MODE_APSTA);
  #else
    return (wifimode == WIFI_AP) || (wifimode == WIFI_AP_STA);
  #endif
}

bool WifiIsSTA(WiFiMode_t wifimode)
{
  #if defined(ESP32)
    return ((wifimode & WIFI_MODE_STA) != 0);
  #else
    return ((wifimode & WIFI_STA) != 0);
  #endif
}

void setWifiState(WifiState state) {
  currentWifiState = state;
  switch (state) {
    case WifiOff:
      WifiDisconnect();
      changeWifiMode(WIFI_OFF);
      break;
    case WifiStart:
      changeWifiMode(WIFI_STA);
      break;
    case WifiConnectionFailed:
      addLog(LOG_LEVEL_INFO, F("WIFI : Connection Failed"));
      if (WIFI_AP != WiFi.getMode()) {
        changeWifiMode(WIFI_AP);
        wifiSetup = true;
        timerAPoff = millis() + WIFI_AP_OFF_TIMER_DURATION;
      }
      break;
    case WifiClientConnectAP:
      timerAPoff = 0; // Disable timer to switch AP off.
      changeWifiMode(WIFI_AP);
      break;
    case WifiClientDisconnectAP:
      if (WifiIsAP(WiFi.getMode())) {
        timerAPoff = millis() + WIFI_AP_OFF_TIMER_DURATION;
      }
      break;
    case WifiCredentialsChanged:
      lastWiFiSettings = 0; // Force to load the first settings.
      wifi_connect_attempt = 0;
      // fall through
    case WifiTryConnect:
      if (WifiIsAP(WiFi.getMode())) {
        timerAPoff = 0; // Disable timer to switch AP off.
        changeWifiMode(WIFI_AP_STA);
      } else {
        changeWifiMode(WIFI_STA);
      }
      if (prepareWiFi()) {
        tryConnectWiFi();
      } else {
        setWifiState(WifiConnectionFailed);
      }
      break;
    case WifiConnectSuccess:
      if (WifiIsAP(WiFi.getMode())) {
        timerAPoff = millis() + WIFI_AP_OFF_TIMER_DURATION;
      } else {
        timerAPoff = 0; // Disable timer to switch AP off.
        changeWifiMode(WIFI_STA);
      }
      wifi_connect_attempt = 0;
      break;
    case WifiDisableAP:
      if (WifiIsAP(WiFi.getMode())) {
        timerAPoff = 0; // Disable timer to switch AP off.
        changeWifiMode(WIFI_STA);
      }
      break;
    case WifiEnableAP:
      if (wifiStatus == ESPEASY_WIFI_SERVICES_INITIALIZED) {
        changeWifiMode(WIFI_AP_STA);
      } else {
        changeWifiMode(WIFI_AP);
      }
      timerAPoff = millis() + WIFI_AP_OFF_TIMER_DURATION;
      break;
    case WifiStartScan:
      if (WifiIsAP(WiFi.getMode())) {
        changeWifiMode(WIFI_AP_STA);
      } else changeWifiMode(WIFI_STA);
      break;
  }
}

//********************************************************************************
// Set Wifi AP Mode
//********************************************************************************
void setWifiMode(WiFiMode_t wifimode) {
  switch (wifimode) {
    case WIFI_OFF:
      addLog(LOG_LEVEL_INFO, F("WIFI : Switch off WiFi"));
      break;
    case WIFI_STA:
      addLog(LOG_LEVEL_INFO, F("WIFI : Set WiFi to STA"));
      break;
    case WIFI_AP:
      addLog(LOG_LEVEL_INFO, F("WIFI : Set WiFi to AP"));
      break;
    case WIFI_AP_STA:
      addLog(LOG_LEVEL_INFO, F("WIFI : Set WiFi to AP+STA"));
      break;
    default:
      break;
  }
  WiFi.mode(wifimode);
}

void changeWifiMode(WiFiMode_t wifimode)
{
  if (wifimode == WiFi.getMode()) return;
  if (WiFi.getMode() == WIFI_OFF) {
    // Any mode can be selected, starting from off mode.
    addLog(LOG_LEVEL_INFO, F("WIFI : Switch on WiFi"));
    setWifiMode(wifimode);
    return;
  }
  switch (wifimode) {
    case WIFI_OFF:
      WifiDisconnect();
      setWifiMode(WIFI_OFF);
      return;
    case WIFI_STA:
      if (WifiIsAP(WiFi.getMode())) {
        // Change from AP mode to STA mode, must disconnect clients.
        dnsServerActive = false;
        dnsServer.stop();
        if (WiFi.softAPdisconnect(true)) {
          String log("WIFI : AP Mode Disabled for SSID: ");
          log += WifiGetAPssid();
          addLog(LOG_LEVEL_INFO, log);
        } else {
          String log("WIFI : Error while disabling AP Mode with SSID: ");
          log += WifiGetAPssid();
          addLog(LOG_LEVEL_ERROR, log);
        }
      }
      setWifiMode(wifimode);
      WiFi.reconnect();
      break;
    case WIFI_AP_STA:
    case WIFI_AP:
    {
      if (WifiIsSTA(WiFi.getMode())) {
        // Should disable WiFi first and then restart in AP mode.
        WifiDisconnect();
        if (wifimode == WIFI_AP_STA) {
          WiFi.reconnect();
        }
//        setWifiMode(WIFI_OFF);
        delay(100);
      }
      // create and store unique AP SSID/PW to prevent ESP from starting AP mode with default SSID and No password!
      // setup ssid for AP Mode when needed
      setWifiMode(wifimode);
      WiFi.setAutoConnect(false);
      String softAPSSID=WifiGetAPssid();
      String pwd = SecuritySettings.WifiAPKey;
      IPAddress subnet(DEFAULT_AP_SUBNET);
      if (!WiFi.softAPConfig(apIP, apIP, subnet)) {
        addLog(LOG_LEVEL_ERROR, "WIFI : [AP] softAPConfig failed!");
      }
      if (WiFi.softAP(softAPSSID.c_str(),pwd.c_str())) {
        String log("WIFI : AP Mode ssid will be ");
        log += softAPSSID;
        log += F(" with address ");
        log += WiFi.softAPIP().toString();
        addLog(LOG_LEVEL_INFO, log);
      } else {
        String log("WIFI : Error while starting AP Mode with SSID: ");
        log += softAPSSID;
        log += F(" IP: ");
        log += apIP.toString();
        addLog(LOG_LEVEL_ERROR, log);
      }
      #ifdef ESP32

      #else
        if(wifi_softap_dhcps_status() != DHCP_STARTED) {
          if(!wifi_softap_dhcps_start()) {
            addLog(LOG_LEVEL_ERROR, "WIFI : [AP] wifi_softap_dhcps_start failed!");
          }
        }
      #endif
      // Start DNS, only used if the ESP has no valid WiFi config
      // It will reply with it's own address on all DNS requests
      // (captive portal concept)
      dnsServerActive = true;
      dnsServer.start(DNS_PORT, "*", apIP);
      break;
    }
    default:
      break;
  }
}

//********************************************************************************
// Determine Wifi AP name to set. (also used for mDNS)
//********************************************************************************
String WifiGetAPssid()
{
  String ssid(Settings.Name);
  ssid+=F("_");
  ssid+=Settings.Unit;
  return (ssid);
}

//********************************************************************************
// Determine hostname: basically WifiGetAPssid with spaces changed to -
//********************************************************************************
String WifiGetHostname()
{
  String hostname(WifiGetAPssid());
  hostname.replace(F(" "), F("-"));
  hostname.replace(F("_"), F("-")); // See RFC952
  return (hostname);
}


bool useStaticIP() {
  return (Settings.IP[0] != 0 && Settings.IP[0] != 255);
}

//********************************************************************************
// Set Wifi config
//********************************************************************************
bool prepareWiFi() {
  if (!selectValidWiFiSettings()) {
    addLog(LOG_LEVEL_ERROR, F("WIFI : No valid wifi settings"));
    return false;
  }
  String log = "";
  char hostname[40];
  strncpy(hostname, WifiGetHostname().c_str(), sizeof(hostname));
  #if defined(ESP8266)
    wifi_station_set_hostname(hostname);
  #endif
  #if defined(ESP32)
    WiFi.setHostname(hostname);
  #endif
  return true;
}


//********************************************************************************
// Manage WiFi credentials
//********************************************************************************
const char* getLastWiFiSettingsSSID() {
  return lastWiFiSettings == 0 ? SecuritySettings.WifiSSID : SecuritySettings.WifiSSID2;
}

const char* getLastWiFiSettingsPassphrase() {
  return lastWiFiSettings == 0 ? SecuritySettings.WifiKey : SecuritySettings.WifiKey2;
}

bool selectNextWiFiSettings() {
  uint8_t tmp = lastWiFiSettings;
  lastWiFiSettings = (lastWiFiSettings + 1) % 2;
  if (!wifiSettingsValid(getLastWiFiSettingsSSID(), getLastWiFiSettingsPassphrase())) {
    // other settings are not correct, switch back.
    lastWiFiSettings = tmp;
    return false; // Nothing changed.
  }
  return true;
}

bool selectValidWiFiSettings() {
  if (wifiSettingsValid(getLastWiFiSettingsSSID(), getLastWiFiSettingsPassphrase()))
    return true;
  return selectNextWiFiSettings();
}

bool wifiSettingsValid(const char* ssid, const char* pass) {
  if (ssid[0] == 0 || (strcasecmp(ssid, "ssid") == 0)) {
    return false;
  }
//  if (pass[0] == 0) return false; // Allow for empty pass
  if (strlen(ssid) > 32) return false;
  if (strlen(pass) > 64) return false;
  return true;
}

bool wifiConnectTimeoutReached() {
  if (wifi_connect_attempt == 0) return true;
  if (timeDiff(last_wifi_connect_attempt_moment, lastDisconnectMoment) >0 ) {
    // Connection attempt was already ended.
    return true;
  }
  if (WifiIsAP(WiFi.getMode())) {
    // Initial setup of WiFi, may take much longer since accesspoint is still active.
    return timeOutReached(last_wifi_connect_attempt_moment + 20000);
  }
  // wait until it connects + add some device specific random offset to prevent
  // all nodes overloading the accesspoint when turning on at the same time.
  #if defined(ESP8266)
  const unsigned int randomOffset_in_sec = wifi_connect_attempt == 1 ? 0 : 1000 * ((ESP.getChipId() & 0xF));
  #endif
  #if defined(ESP32)
  const unsigned int randomOffset_in_sec = wifi_connect_attempt == 1 ? 0 : 1000 * ((ESP.getEfuseMac() & 0xF));
  #endif
  return timeOutReached(last_wifi_connect_attempt_moment + DEFAULT_WIFI_CONNECTION_TIMEOUT + randomOffset_in_sec);
}

//********************************************************************************
// Simply start the WiFi connection sequence
//********************************************************************************
bool tryConnectWiFi() {
//  if (wifiSetup && !wifiSetupConnect)
//    return false;
  if (wifiStatus != ESPEASY_WIFI_DISCONNECTED) {
    if (!WifiIsAP(WiFi.getMode())) {
      // Only when not in AP mode.
      return(true);   //already connected, need to disconnect first
    }
  }
  if (!wifiConnectTimeoutReached())
    return true;    // timeout not reached yet, thus no need to retry again.
  if (!selectValidWiFiSettings()) {
    addLog(LOG_LEVEL_ERROR, F("WIFI : No valid WiFi settings!"));
    return false;
  }
  const char* ssid = getLastWiFiSettingsSSID();
  const char* passphrase = getLastWiFiSettingsPassphrase();
  String log = F("WIFI : Connecting ");
  log += ssid;
  log += F(" attempt #");
  log += wifi_connect_attempt;
  addLog(LOG_LEVEL_INFO, log);

  last_wifi_connect_attempt_moment = millis();
  switch (wifi_connect_attempt) {
    case 0:
      if (lastBSSID[0] == 0)
        WiFi.begin(ssid, passphrase);
      else
        WiFi.begin(ssid, passphrase, 0, &lastBSSID[0]);
      break;
    default:
      WiFi.begin(ssid, passphrase);
  }
  ++wifi_connect_attempt;
  switch (WiFi.status()) {
    case WL_NO_SSID_AVAIL: {
      log = F("WIFI : No SSID found matching: ");
      log += ssid;
      addLog(LOG_LEVEL_INFO, log);
      return false;
    }
    case WL_CONNECT_FAILED: {
      log = F("WIFI : Connection failed to: ");
      log += ssid;
      addLog(LOG_LEVEL_INFO, log);
      return false;
    }
    default:
     break;
  }
  return true; // Sent
}

//********************************************************************************
// Disconnect from Wifi AP
//********************************************************************************
void WifiDisconnect()
{
  #if defined(ESP32)
    WiFi.disconnect();
  #else
    ETS_UART_INTR_DISABLE();
    wifi_station_disconnect();
    ETS_UART_INTR_ENABLE();
  #endif
}


//********************************************************************************
// Scan all Wifi Access Points
//********************************************************************************
void WifiScan()
{
  // Direct Serial is allowed here, since this function will only be called from serial input.
  Serial.println(F("WIFI : SSID Scan start"));
  int n = WiFi.scanNetworks();
  if (n == 0)
    Serial.println(F("WIFI : No networks found"));
  else
  {
    Serial.print(F("WIFI : "));
    Serial.print(n);
    Serial.println(F(" networks found"));
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      Serial.print(F("WIFI : "));
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.println(formatScanResult(i, " "));
      delay(10);
    }
  }
  Serial.println("");
}

String formatScanResult(int i, const String& separator) {
  String result = WiFi.SSID(i);
  result += separator;
  result += WiFi.BSSIDstr(i);
  result += separator;
  result += F("Ch:");
  result += WiFi.channel(i);
  result += F(" (");
  result += WiFi.RSSI(i);
  result += F("dBm) ");
  switch (WiFi.encryptionType(i)) {
  #ifdef ESP32
    case WIFI_AUTH_OPEN: result += F("open"); break;
    case WIFI_AUTH_WEP:  result += F("WEP"); break;
    case WIFI_AUTH_WPA_PSK: result += F("WPA/PSK"); break;
    case WIFI_AUTH_WPA2_PSK: result += F("WPA2/PSK"); break;
    case WIFI_AUTH_WPA_WPA2_PSK: result += F("WPA/WPA2/PSK"); break;
    case WIFI_AUTH_WPA2_ENTERPRISE: result += F("WPA2 Enterprise"); break;
  #else
    case ENC_TYPE_WEP: result += F("WEP"); break;
    case ENC_TYPE_TKIP: result += F("WPA/PSK"); break;
    case ENC_TYPE_CCMP: result += F("WPA2/PSK"); break;
    case ENC_TYPE_NONE: result += F("open"); break;
    case ENC_TYPE_AUTO: result += F("WPA/WPA2/PSK"); break;
  #endif
    default:
      break;
  }
  return result;
}


//********************************************************************************
// Check if we are still connected to a Wifi AP
//********************************************************************************
void WifiCheck()
{
  if(wifiSetup)
    return;

  if (WifiIsAP(WiFi.getMode())) {
    // disable AP after timeout.
    if (timerAPoff && timeOutReached(timerAPoff)) {
      setWifiState(WifiDisableAP);
    }
  }

  if (wifiStatus == ESPEASY_WIFI_DISCONNECTED)
  {
    NC_Count++;
    if (!WifiIsAP(WiFi.getMode()))
      setWifiState(WifiTryConnect);
  }
  //connected
  else if (wifiStatus == ESPEASY_WIFI_SERVICES_INITIALIZED)
  {
    C_Count++;
    NC_Count = 0;
  }
}

//********************************************************************************
// Return subnet range of WiFi.
//********************************************************************************
bool getSubnetRange(IPAddress& low, IPAddress& high)
{
  if (WifiIsAP(WiFi.getMode())) {
    // WiFi is active as accesspoint, do not check.
    return false;
  }
  if (wifiStatus < ESPEASY_WIFI_GOT_IP) {
    return false;
  }
  const IPAddress ip = WiFi.localIP();
  const IPAddress subnet = WiFi.subnetMask();
  low = ip;
  high = ip;
  // Compute subnet range.
  for (byte i=0; i < 4; ++i) {
    if (subnet[i] != 255) {
      low[i] = low[i] & subnet[i];
      high[i] = high[i] | ~subnet[i];
    }
  }
  return true;
}


String getLastDisconnectReason() {
  switch (lastDisconnectReason) {
    case WIFI_DISCONNECT_REASON_UNSPECIFIED:                return F("Unspecified");
    case WIFI_DISCONNECT_REASON_AUTH_EXPIRE:                return F("Auth expire");
    case WIFI_DISCONNECT_REASON_AUTH_LEAVE:                 return F("Auth leave");
    case WIFI_DISCONNECT_REASON_ASSOC_EXPIRE:               return F("Assoc expire");
    case WIFI_DISCONNECT_REASON_ASSOC_TOOMANY:              return F("Assoc toomany");
    case WIFI_DISCONNECT_REASON_NOT_AUTHED:                 return F("Not authed");
    case WIFI_DISCONNECT_REASON_NOT_ASSOCED:                return F("Not assoced");
    case WIFI_DISCONNECT_REASON_ASSOC_LEAVE:                return F("Assoc leave");
    case WIFI_DISCONNECT_REASON_ASSOC_NOT_AUTHED:           return F("Assoc not authed");
    case WIFI_DISCONNECT_REASON_DISASSOC_PWRCAP_BAD:        return F("Disassoc pwrcap bad");
    case WIFI_DISCONNECT_REASON_DISASSOC_SUPCHAN_BAD:       return F("Disassoc supchan bad");
    case WIFI_DISCONNECT_REASON_IE_INVALID:                 return F("IE invalid");
    case WIFI_DISCONNECT_REASON_MIC_FAILURE:                return F("Mic failure");
    case WIFI_DISCONNECT_REASON_4WAY_HANDSHAKE_TIMEOUT:     return F("4way handshake timeout");
    case WIFI_DISCONNECT_REASON_GROUP_KEY_UPDATE_TIMEOUT:   return F("Group key update timeout");
    case WIFI_DISCONNECT_REASON_IE_IN_4WAY_DIFFERS:         return F("IE in 4way differs");
    case WIFI_DISCONNECT_REASON_GROUP_CIPHER_INVALID:       return F("Group cipher invalid");
    case WIFI_DISCONNECT_REASON_PAIRWISE_CIPHER_INVALID:    return F("Pairwise cipher invalid");
    case WIFI_DISCONNECT_REASON_AKMP_INVALID:               return F("AKMP invalid");
    case WIFI_DISCONNECT_REASON_UNSUPP_RSN_IE_VERSION:      return F("Unsupp RSN IE version");
    case WIFI_DISCONNECT_REASON_INVALID_RSN_IE_CAP:         return F("Invalid RSN IE cap");
    case WIFI_DISCONNECT_REASON_802_1X_AUTH_FAILED:         return F("802 1X auth failed");
    case WIFI_DISCONNECT_REASON_CIPHER_SUITE_REJECTED:      return F("Cipher suite rejected");
    case WIFI_DISCONNECT_REASON_BEACON_TIMEOUT:             return F("Beacon timeout");
    case WIFI_DISCONNECT_REASON_NO_AP_FOUND:                return F("No AP found");
    case WIFI_DISCONNECT_REASON_AUTH_FAIL:                  return F("Auth fail");
    case WIFI_DISCONNECT_REASON_ASSOC_FAIL:                 return F("Assoc fail");
    case WIFI_DISCONNECT_REASON_HANDSHAKE_TIMEOUT:          return F("Handshake timeout");
  }
  return F("Unknown");
}
