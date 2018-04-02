

//********************************************************************************
// Functions to process the data gathered from the events.
// These functions are called from Setup() or Loop() and thus may call delay() or yield()
//********************************************************************************
void processConnect() {
  if (processedConnect) return;
  processedConnect = true;
  if (wifiStatus < ESPEASY_WIFI_CONNECTED) return;

  String log = F("WIFI : Connected! AP: ");
  log += WiFi.SSID();
  log += F(" (");
  log += WiFi.BSSIDstr();
  log += F(") Ch: ");
  log += last_channel;
  const long connect_duration = timeDiff(last_wifi_connect_attempt_moment, lastConnectMoment);
  if (connect_duration > 0 && connect_duration < 30000) {
    // Just log times when they make sense.
    log += F(" Duration: ");
    log += connect_duration;
    log += F(" ms");
  }
  addLog(LOG_LEVEL_INFO, log);

  if (Settings.UseRules && bssid_changed) {
    String event = F("WiFi#ChangedAccesspoint");
    rulesProcessing(event);
  }
  wifi_connect_attempt = 0;
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

  if (Settings.deepSleep && Settings.deepSleepOnFail) {
    //only one attempt in deepsleep, to conserve battery
    addLog(LOG_LEVEL_ERROR, F("SLEEP: Connection failed, going back to sleep."));
    deepSleep(Settings.Delay);
  }
  if (wifi_connect_attempt > 6) {
    //everything failed, activate AP mode (will deactivate automatically after a while if its connected again)
    WifiAPMode(true);
  }

  if (!intent_to_reboot)
    WiFiConnectRelaxed();
}


void processGotIP() {
  if (processedGetIP)
    return;
  processedGetIP = true;
  if (wifiStatus < ESPEASY_WIFI_GOT_IP)
    return;
  IPAddress ip = WiFi.localIP();
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
  wifiStatus = ESPEASY_WIFI_SERVICES_INITIALIZED;
}

void processConnectAPmode() {
  if (processedConnectAPmode) return;
  processedConnectAPmode = true;
  String log = F("AP Mode: Client connected: ");
  log += formatMAC(lastMacConnectedAPmode);
  log += F(" Connected devices: ");
  log += WiFi.softAPgetStationNum();
  addLog(LOG_LEVEL_INFO, log);
}

void processDisconnectAPmode() {
  if (processedDisconnectAPmode) return;
  processedDisconnectAPmode = true;
  String log = F("AP Mode: Client disconnected: ");
  log += formatMAC(lastMacDisconnectedAPmode);
  log += F(" Connected devices: ");
  log += WiFi.softAPgetStationNum();
  addLog(LOG_LEVEL_INFO, log);
}

void resetWiFi() {
  addLog(LOG_LEVEL_INFO, F("Reset WiFi."));
  WiFiMode_t currentMode = WiFi.getMode();
  WiFi.mode(WIFI_OFF);
  WiFi.mode(currentMode);
  lastDisconnectMoment = millis();
  processedDisconnect = false;
  wifiStatus = ESPEASY_WIFI_DISCONNECTED;
}


//********************************************************************************
// Determine Wifi AP name to set. (also used for mDNS)
//********************************************************************************
String WifiGetAPssid()
{
  String ssid(Settings.Name);
  ssid+=F("-");
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

bool WifiIsAP()
{
  byte wifimode = WiFi.getMode();
  #if defined(ESP32)
    return ((wifimode & WIFI_MODE_AP) != 0);
  #else
    return ((wifimode & WIFI_AP) != 0);
  #endif
}

//********************************************************************************
// Set Wifi AP Mode
//********************************************************************************
void WifiAPMode(boolean state)
{
  const bool currentState = WifiIsAP();
  const bool mustChange = currentState != state;
  if (!mustChange) return;
  if (state) {
    // create and store unique AP SSID/PW to prevent ESP from starting AP mode with default SSID and No password!
    // setup ssid for AP Mode when needed

    String softAPSSID=WifiGetAPssid();
    String pwd = SecuritySettings.WifiAPKey;
    IPAddress subnet(DEFAULT_AP_SUBNET);
    WiFi.softAPConfig(apIP, apIP, subnet);
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
      WiFi.softAPdisconnect(true);
    }
    return;
  }
  // Must disable AP mode.
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

  if (wifiSetupConnect) {
    wifi_connect_attempt = 0;
    WifiAPMode(false);
  } else if (WifiIsAP()) {
    // wifiSetupConnect is not active, so no connect attempt initiated from the setup page.
    if (WiFi.softAPgetStationNum() > 0 // client is connected to AP
        || !timeOutReached(last_wifi_connect_attempt_moment + 5000))  // Slow retry
    {
      // Either a client is connected, or may connect, so disable STA.
      WiFi.enableSTA(false);
      return false;
    }
  }
  if (!WiFi.enableSTA(true)) {
    // Cannot enable STA mode.
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

  //use static ip?
  if (useStaticIP())
  {
    const IPAddress ip = Settings.IP;
    log = F("IP   : Static IP :");
    log += ip;
    addLog(LOG_LEVEL_INFO, log);
    const IPAddress gw = Settings.Gateway;
    const IPAddress subnet = Settings.Subnet;
    const IPAddress dns = Settings.DNS;
    WiFi.config(ip, gw, subnet, dns);
  }
  return true;
}

//********************************************************************************
// Start connect to WiFi and check later to see if connected.
//********************************************************************************
void WiFiConnectRelaxed() {
  if (prepareWiFi()) {
    tryConnectWiFi();
    return;
  }
  //everything failed, activate AP mode (will deactivate automatically after a while if its connected again)
  WifiAPMode(true);
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
  if (pass[0] == 0) return false;
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
  if (wifiSetupConnect) {
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
  if (wifiSetup && !wifiSetupConnect)
    return false;
  if (wifiStatus != ESPEASY_WIFI_DISCONNECTED)
    return(true);   //already connected, need to disconnect first
  if (!wifiConnectTimeoutReached())
    return true;    // timeout not reached yet, thus no need to retry again.
  if (!selectValidWiFiSettings()) {
    addLog(LOG_LEVEL_ERROR, F("WIFI : No valid WiFi settings!"));
    return false;
  }

  if (wifi_connect_attempt != 0 && ((wifi_connect_attempt % 3) == 0)) {
    // Change to other wifi settings.
    selectNextWiFiSettings();
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
    case 1:
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
  WiFi.disconnect();
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
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println("");
      delay(10);
    }
  }
  Serial.println("");
}


//********************************************************************************
// Check if we are still connected to a Wifi AP
//********************************************************************************
void WifiCheck()
{
  if(wifiSetup)
    return;

  if (wifiStatus == ESPEASY_WIFI_DISCONNECTED)
  {
    NC_Count++;
    WiFiConnectRelaxed();
  }
  //connected
  else if (wifiStatus == ESPEASY_WIFI_SERVICES_INITIALIZED)
  {
    C_Count++;
    NC_Count = 0;
    if (WifiIsAP()) {
      // disable AP after timeout if a Wifi connection is established...
      if (timerAPoff && timeOutReached(timerAPoff)) {
        timerAPoff = 0;
        WifiAPMode(false);
//      } else if (timeOutReached(lastGetIPmoment + 2000)) {
//        WifiAPMode(false);
      }
    }
  }
}

//********************************************************************************
// Return subnet range of WiFi.
//********************************************************************************
bool getSubnetRange(IPAddress& low, IPAddress& high)
{
  if (WifiIsAP()) {
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
