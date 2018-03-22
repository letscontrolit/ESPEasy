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
  return (hostname);
}


//********************************************************************************
// Set Wifi AP Mode config
//********************************************************************************
void WifiAPconfig()
{
  // create and store unique AP SSID/PW to prevent ESP from starting AP mode with default SSID and No password!
  // setup ssid for AP Mode when needed

  String softAPSSID=WifiGetAPssid();
  String pwd = SecuritySettings.WifiAPKey;
  WiFi.softAP(softAPSSID.c_str(),pwd.c_str());
  // We start in STA mode
  WifiAPMode(false);

  String log("WIFI : AP Mode ssid will be ");
  log=log+WifiGetAPssid();

  log=log+F(" with address ");
  log=log+apIP.toString();
  addLog(LOG_LEVEL_INFO, log);


}


bool WifiIsAP()
{
  byte wifimode = wifi_get_opmode();
  return(wifimode == 2 || wifimode == 3); //apmode is enabled
}

//********************************************************************************
// Set Wifi AP Mode
//********************************************************************************
void WifiAPMode(boolean state)
{
  if (WifiIsAP())
  {
    //want to disable?
    if (!state)
    {
      WiFi.mode(WIFI_STA);
      addLog(LOG_LEVEL_INFO, F("WIFI : AP Mode disabled"));
    }
  }
  else
  {
    //want to enable?
    if (state)
    {
      WiFi.mode(WIFI_AP_STA);
      addLog(LOG_LEVEL_INFO, F("WIFI : AP Mode enabled"));
    }
  }
}


//********************************************************************************
// Set Wifi config
//********************************************************************************
bool prepareWiFi() {
  String log = "";
  char hostname[40];
  strncpy(hostname, WifiGetHostname().c_str(), sizeof(hostname));
  wifi_station_set_hostname(hostname);

  //use static ip?
  if (Settings.IP[0] != 0 && Settings.IP[0] != 255)
  {
    char str[20];
    sprintf_P(str, PSTR("%u.%u.%u.%u"), Settings.IP[0], Settings.IP[1], Settings.IP[2], Settings.IP[3]);
    log = F("IP   : Static IP :");
    log += str;
    addLog(LOG_LEVEL_INFO, log);
    IPAddress ip = Settings.IP;
    IPAddress gw = Settings.Gateway;
    IPAddress subnet = Settings.Subnet;
    IPAddress dns = Settings.DNS;
    WiFi.config(ip, gw, subnet, dns);
  }
  return selectValidWiFiSettings();
}

//********************************************************************************
// Configure network and connect to Wifi SSID and SSID2
//********************************************************************************
boolean WifiConnect(byte connectAttempts)
{
  if (prepareWiFi()) {
    //try to connect to one of the access points
    bool connected = WifiConnectAndWait(connectAttempts);
    if (!connected) {
      if (selectNextWiFiSettings()) {
        connected = WifiConnectAndWait(connectAttempts);
      }
    }
    if (connected) {
      return(true);
    }
  }
  addLog(LOG_LEVEL_ERROR, F("WIFI : Could not connect to AP!"));
  //everything failed, activate AP mode (will deactivate automatically after a while if its connected again)
  WifiAPMode(true);
  return(false);
}

//********************************************************************************
// Start connect to WiFi and check later to see if connected.
//********************************************************************************
void WiFiConnectRelaxed() {
  wifiConnected = false;
  wifi_connect_attempt = 0;
  if (prepareWiFi()) {
    tryConnectWiFi();
    return;
  }
  addLog(LOG_LEVEL_ERROR, F("WIFI : Could not connect to AP, no valid WiFi settings!"));
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

bool anyValidWifiSettings() {
  if (wifiSettingsValid(SecuritySettings.WifiSSID, SecuritySettings.WifiKey))
    return true;
  if (wifiSettingsValid(SecuritySettings.WifiSSID2, SecuritySettings.WifiKey2))
    return true;
  addLog(LOG_LEVEL_ERROR, F("WIFI : No valid WiFi settings!"));
  return false;
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
  if (wifiSetupConnect) {
    // Initial setup of WiFi, may take much longer since accesspoint is still active.
    return timeOutReached(wifi_connect_timer + 20000);
  }
  // wait until it connects + add some device specific random offset to prevent
  // all nodes overloading the accesspoint when turning on at the same time.
  const unsigned int randomOffset_in_sec = wifi_connect_attempt == 1 ? 0 : 1000 * ((ESP.getChipId() & 0xF));
  return timeOutReached(wifi_connect_timer + DEFAULT_WIFI_CONNECTION_TIMEOUT + randomOffset_in_sec);
}

//********************************************************************************
// Simply start the WiFi connection sequence
//********************************************************************************
bool tryConnectWiFi() {
  if (wifiSetup && !wifiSetupConnect)
    return false;
  if (WiFi.status() == WL_CONNECTED)
    return(true);   //already connected, need to disconnect first
  if (!wifiConnectTimeoutReached())
    return true;    // timeout not reached yet, thus no need to retry again.
  if (!anyValidWifiSettings())
    return false;

  if (wifi_connect_attempt != 0 && ((wifi_connect_attempt % 3) == 0)) {
    // Change to other wifi settings.
    if (selectNextWiFiSettings())
      WiFi.disconnect();
  }
  if (wifi_connect_attempt > 6) {
    //everything failed, activate AP mode (will deactivate automatically after a while if its connected again)
    WifiAPMode(true);
  }
  String ssid;
  ssid.reserve(32);
  String passphrase;
  passphrase.reserve(64);
  ssid = getLastWiFiSettingsSSID();
  passphrase = getLastWiFiSettingsPassphrase();
  String log = F("WIFI : Connecting ");
  log += ssid;
  log += F(" attempt #");
  log += wifi_connect_attempt;
  addLog(LOG_LEVEL_INFO, log);

  wifi_connect_timer = millis();
  switch (wifi_connect_attempt) {
    case 0:
      if (lastBSSID[0] == 0)
        WiFi.begin(ssid.c_str(), passphrase.c_str());
      else
        WiFi.begin(ssid.c_str(), passphrase.c_str(), 0, &lastBSSID[0]);
      break;
    default:
      WiFi.begin(ssid.c_str(), passphrase.c_str());
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
    case WL_CONNECTED:
      checkWifiJustConnected();
      break;
    default:
     break;
  }
  return true; // Sent
}

//********************************************************************************
// Connect to Wifi specific SSID
//********************************************************************************
boolean WifiConnectAndWait(byte connectAttempts)
{
  String log;

  wifiConnected = false;
  wifi_connect_attempt = 0;
  for (byte tryConnect = 0; tryConnect < connectAttempts; tryConnect++)
  {
    if (tryConnectWiFi()) {
      do {
        if (checkWifiJustConnected())
          return true;
        delay(50);
      } while (!wifiConnectTimeoutReached());
    }
    // log = F("WIFI : Disconnecting!");
    // addLog(LOG_LEVEL_INFO, log);
    #if defined(ESP8266)
      ETS_UART_INTR_DISABLE();
      wifi_station_disconnect();
      ETS_UART_INTR_ENABLE();
    #endif
    for (byte x = 0; x < 20; x++)
    {
      statusLED(true);
      delay(50);
    }
  }
  return false;
}

bool checkWifiJustConnected() {
  if (wifiConnected) return true;
  delay(1);
  if (WiFi.status() != WL_CONNECTED) {
    statusLED(false);
    return false;
  }
  wifiConnected = true;
  String log = F("WIFI : WiFi connect attempt took: ");
  log += timePassedSince(wifi_connect_timer);
  log += F(" ms");
  addLog(LOG_LEVEL_INFO, log);

  // fix octet?
  if (Settings.IP_Octet != 0 && Settings.IP_Octet != 255)
  {
    IPAddress ip = WiFi.localIP();
    IPAddress gw = WiFi.gatewayIP();
    IPAddress subnet = WiFi.subnetMask();
    ip[3] = Settings.IP_Octet;
    log = F("IP   : Fixed IP octet:");
    log += ip;
    addLog(LOG_LEVEL_INFO, log);
    WiFi.config(ip, gw, subnet);
  }

  #ifdef FEATURE_MDNS

    String log = F("WIFI : ");
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
  uint8_t* curBSSID = WiFi.BSSID();
  bool changed = false;
  for (byte i=0; i < 6; ++i) {
    if (lastBSSID[i] != *(curBSSID + i)) {
      changed = true;
      lastBSSID[i] = *(curBSSID + i);
    }
  }
  if (Settings.UseRules)
  {
    if (changed) {
      String event = F("WiFi#ChangedAccesspoint");
      rulesProcessing(event);
    }
    String event = F("WiFi#Connected");
    rulesProcessing(event);
  }
  log = F("WIFI : Connected! IP: ");
  log += formatIP(WiFi.localIP());
  log += F(" (");
  log += WifiGetHostname();
  log += F(") AP: ");
  log += WiFi.SSID();
  log += F(" AP BSSID: ");
  log += WiFi.BSSIDstr();

  addLog(LOG_LEVEL_INFO, log);
  statusLED(true);
  return true;
}

//********************************************************************************
// Disconnect from Wifi AP
//********************************************************************************
void WifiDisconnect()
{
  WiFi.disconnect();
  wifiConnected = false;
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

  if (WiFi.status() != WL_CONNECTED)
  {
    NC_Count++;
    //give it time to automaticly reconnect
    if (NC_Count > 2)
    {
      if (wifiConnected) {
        wifi_connect_attempt = 0;
        wifiConnected = false;
        WiFiConnectRelaxed();
      }
      C_Count=0;
      NC_Count = 0;
    }
  }
  //connected
  else
  {
    C_Count++;
    NC_Count = 0;
    if (C_Count > 2) // disable AP after timeout if a Wifi connection is established...
    {
      WifiAPMode(false);
    }
  }
  if (!wifiConnected) {
    if (tryConnectWiFi())
      checkWifiJustConnected();
  }
}
