void WifiAPMode(boolean state)
{
  if (state)
  {
    Serial.println("WIFI : Starting AP Mode");
    WiFi.softAP(ap_ssid, Settings.WifiAPKey);
    WiFi.mode(WIFI_AP_STA);
  }
  else
  {
    Serial.println("WIFI : Ending AP Mode");
    WiFi.mode(WIFI_STA);
  }
}

boolean WifiConnect()
{
  Serial.println("WIFI : Connecting...");
  if (WiFi.status() != WL_CONNECTED)
  {
    if (Settings.WifiSSID[0] != 0)
    {
      //WiFi.mode(WIFI_STA);
      WiFi.begin(Settings.WifiSSID, Settings.WifiKey);
      for (byte x = 0; x < 10; x++)
      {
        if (WiFi.status() != WL_CONNECTED)
        {
          delay(500);
          Serial.println(".");
        }
        else
          break;
      }

      // fix ip if last octet is set
      if (Settings.IP_Octet != 0 && Settings.IP_Octet != 255)
      {
        IPAddress ip = WiFi.localIP();
        IPAddress gw = WiFi.gatewayIP();
        IPAddress subnet = WiFi.subnetMask();
        ip[3] = Settings.IP_Octet;
        Serial.print("IP   : Fixed IP :");
        Serial.println(ip);
        WiFi.config(ip, gw, subnet);
      }
    }
    else
      Serial.println("WIFI : No SSID!");
  }
}

boolean WifiDisconnect()
{
  WiFi.disconnect();
}

void WifiScan()
{
  Serial.println("WIFI : SSID Scan start");
  int n = WiFi.scanNetworks();
  Serial.println("WIFI : Scan done");
  if (n == 0)
    Serial.println("WIFI : No networks found");
  else
  {
    Serial.print("WIFI : ");
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      Serial.print("WIFI : ");
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      //Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
      Serial.println("");
      delay(10);
    }
  }
  Serial.println("");
}

void WifiCheck()
{
    if (WiFi.status() != WL_CONNECTED)
      {
        NC_Count++;
        if (NC_Count > 10 && !AP_Mode)
          {
            C_Count=0;
            AP_Mode=true;
            WifiAPMode(true);
          }
      }
      else
        {
          if (NC_Count !=0)  // there was a disconnect before...
          {
            C_Count++;
            if (C_Count > 30)
              {
                Serial.println("WIFI : Return to STA mode");
                NC_Count=0;
                AP_Mode=false;
                WifiAPMode(false);
              }
          }
        }
}
