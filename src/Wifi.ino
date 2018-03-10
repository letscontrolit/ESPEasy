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
  return (hostname);
}


//********************************************************************************
// Set Wifi AP Mode config
//********************************************************************************
void WifiAPconfig()
{
  // create and store unique AP SSID/PW to prevent ESP from starting AP mode with default SSID and No password!
  // setup ssid for AP Mode when needed
  WiFi.softAP(WifiGetAPssid().c_str(), SecuritySettings.WifiAPKey);
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
  #if defined(ESP8266)
    byte wifimode = wifi_get_opmode();
  #endif
  #if defined(ESP32)
    byte wifimode = WiFi.getMode();
  #endif
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
// Configure network and connect to Wifi SSID and SSID2
//********************************************************************************
boolean WifiConnect(byte connectAttempts)
{
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
  if (Settings.IP[0] != 0 && Settings.IP[0] != 255)
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

  //try to connect to one of the access points
  if (WifiConnectSSID(SecuritySettings.WifiSSID,  SecuritySettings.WifiKey,  connectAttempts) ||
      WifiConnectSSID(SecuritySettings.WifiSSID2, SecuritySettings.WifiKey2, connectAttempts))
  {
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
    if (Settings.UseRules)
    {
      String event = F("WiFi#Connected");
      rulesProcessing(event);
    }
    return(true);
  }

  addLog(LOG_LEVEL_ERROR, F("WIFI : Could not connect to AP!"));

  //everything failed, activate AP mode (will deactivate automatically after a while if its connected again)
  WifiAPMode(true);

  return(false);
}


//********************************************************************************
// Connect to Wifi specific SSID
//********************************************************************************
boolean WifiConnectSSID(char WifiSSID[], char WifiKey[], byte connectAttempts)
{
  String log;

  //already connected, need to disconnect first
  if (WiFi.status() == WL_CONNECTED)
    return(true);

  //no ssid specified
  if ((WifiSSID[0] == 0)  || (strcasecmp(WifiSSID, "ssid") == 0))
    return(false);

  for (byte tryConnect = 1; tryConnect <= connectAttempts; tryConnect++)
  {
    log = F("WIFI : Connecting ");
    log += WifiSSID;
    log += F(" attempt #");
    log += tryConnect;
    addLog(LOG_LEVEL_INFO, log);

    if (tryConnect == 1)
      WiFi.begin(WifiSSID, WifiKey);
    else
      WiFi.begin();

    //wait until it connects
    for (byte x = 0; x < 200; x++)
    {
      if (!WiFiConnected(50))
      {
        statusLED(false);
        // No delay needed, since the WiFi check has a delay
      }
      else
        break;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
      if (Settings.UseNTP) {
        initTime();
      }
      log = F("WIFI : Connected! IP: ");
      log += formatIP(WiFi.localIP());
      log += F(" (");
      log += WifiGetHostname();
      log += F(")");

      addLog(LOG_LEVEL_INFO, log);
      statusLED(true);
      return(true);
    }
    else
    {
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
  }


  return false;
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

  if (WiFi.status() != WL_CONNECTED)
  {
    NC_Count++;
    //give it time to automatically reconnect
    if (NC_Count > 2)
    {
      WifiConnect(2);

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
  if (WiFi.status() != WL_CONNECTED) {
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
