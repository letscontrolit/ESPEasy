//********************************************************************************
// Set Wifi AP Mode config
//********************************************************************************
void WifiAPconfig()
{
  // create and store unique AP SSID/PW to prevent ESP from starting AP mode with default SSID and No password!
  char ap_ssid[20];
  ap_ssid[0] = 0;
  strcpy(ap_ssid, "ESP_");
  sprintf_P(ap_ssid, PSTR("%s%u"), ap_ssid, Settings.Unit);
  // setup ssid for AP Mode when needed
  WiFi.softAP(ap_ssid, SecuritySettings.WifiAPKey);
  // We start in STA mode
  WiFi.mode(WIFI_STA);
}


//********************************************************************************
// Set Wifi AP Mode
//********************************************************************************
void WifiAPMode(boolean state)
{
  if (state)
  {
    AP_Mode = true;
    WiFi.mode(WIFI_AP_STA);
  }
  else
  {
    AP_Mode = false;
    WiFi.mode(WIFI_STA);
  }
}


//********************************************************************************
// Connect to Wifi AP
//********************************************************************************
boolean WifiConnect(byte connectAttempts)
{
  String log = "";

  char hostName[sizeof(Settings.Name)];
  strcpy(hostName,Settings.Name);
  for(byte x=0; x< sizeof(hostName); x++)
    if (hostName[x] == ' ')
      hostName[x] = '-';
  wifi_station_set_hostname(hostName);

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


  if (WiFi.status() != WL_CONNECTED)
  {
    if ((SecuritySettings.WifiSSID[0] != 0)  && (strcasecmp(SecuritySettings.WifiSSID, "ssid") != 0))
    {
      for (byte tryConnect = 1; tryConnect <= connectAttempts; tryConnect++)
      {
        log = F("WIFI : Connecting... ");
        log += tryConnect;
        addLog(LOG_LEVEL_INFO, log);

        if (tryConnect == 1)
          WiFi.begin(SecuritySettings.WifiSSID, SecuritySettings.WifiKey);
        else
          WiFi.begin();

        for (byte x = 0; x < 20; x++)
        {
          if (WiFi.status() != WL_CONNECTED)
          {
            delay(500);
          }
          else
            break;
        }
        if (WiFi.status() == WL_CONNECTED)
        {
          log = F("WIFI : Connected! IP: ");
          IPAddress ip = WiFi.localIP();
          char str[20];
          sprintf_P(str, PSTR("%u.%u.%u.%u"), ip[0], ip[1], ip[2], ip[3]);
          log += str;
          addLog(LOG_LEVEL_INFO, log);
          break;
        }
        else
        {
          log = F("WIFI : Disconnecting!");
          addLog(LOG_LEVEL_INFO, log);
          ETS_UART_INTR_DISABLE();
          wifi_station_disconnect();
          ETS_UART_INTR_ENABLE();
          delay(1000);
        }
      }

      // fix ip if last octet is set
      if (Settings.IP_Octet != 0 && Settings.IP_Octet != 255)
      {
        IPAddress ip = WiFi.localIP();
        IPAddress gw = WiFi.gatewayIP();
        IPAddress subnet = WiFi.subnetMask();
        ip[3] = Settings.IP_Octet;
        log = F("IP   : Fixed IP :");
        log += ip;
        addLog(LOG_LEVEL_INFO, log);
        WiFi.config(ip, gw, subnet);
      }
    }
    else
    {
      log = F("WIFI : No SSID!");
      addLog(LOG_LEVEL_INFO, log);
      NC_Count = 1;
      WifiAPMode(true);
    }
  }
}


//********************************************************************************
// Disconnect from Wifi AP
//********************************************************************************
boolean WifiDisconnect()
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

  String log = "";

  if (WiFi.status() != WL_CONNECTED)
  {
    NC_Count++;
    if (NC_Count > 2)
    {
      WifiConnect(2);
      C_Count=0;
      if (WiFi.status() != WL_CONNECTED)
        WifiAPMode(true);
      NC_Count = 0;
    }
  }
  else
  {
    C_Count++;
    NC_Count = 0;
    if (C_Count > 2) // close AP after timeout if a Wifi connection is established...
    {
      byte wifimode = wifi_get_opmode();
      if (wifimode == 2 || wifimode == 3) //apmode is active
      {
        WifiAPMode(false);
        log = F("WIFI : AP Mode inactive");
        addLog(LOG_LEVEL_INFO, log);
      }
    }
  }
}
