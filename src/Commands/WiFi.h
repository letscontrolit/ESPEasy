#ifndef COMMAND_WIFI_H
#define COMMAND_WIFI_H


#define WIFI_MODE_MAX (WiFiMode_t)4

bool Command_Wifi_SSID (struct EventStruct *event, const char* Line)
{
  return Command_GetORSetString(F("Wifi SSID:"), 
    Line, 
    SecuritySettings.WifiSSID,
    sizeof(SecuritySettings.WifiSSID),
    1);
}

bool Command_Wifi_Key (struct EventStruct *event, const char* Line)
{
  return Command_GetORSetString(F("Wifi Key:"), 
    Line, 
    SecuritySettings.WifiKey,
    sizeof(SecuritySettings.WifiKey),
    1);
}

bool Command_Wifi_SSID2 (struct EventStruct *event, const char* Line)
{
  return Command_GetORSetString(F("Wifi2 SSID:"), 
    Line, 
    SecuritySettings.WifiSSID2,
    sizeof(SecuritySettings.WifiSSID2),
    1);
}

bool Command_Wifi_Key2 (struct EventStruct *event, const char* Line)
{
  return Command_GetORSetString(F("Wifi2 Key:"), 
    Line, 
    SecuritySettings.WifiKey2,
    sizeof(SecuritySettings.WifiKey2),
    1);
}

bool Command_Wifi_Scan (struct EventStruct *event, const char* Line)
{
  WifiScan();
  return true;
}

bool Command_Wifi_Connect (struct EventStruct *event, const char* Line)
{
  WiFiConnectRelaxed();
  return true;
}

bool Command_Wifi_Disconnect (struct EventStruct *event, const char* Line)
{
  WifiDisconnect();
  return true;
}

bool Command_Wifi_APMode (struct EventStruct *event, const char* Line)
{
  setAP(true);
  return true;
}

bool Command_Wifi_STAMode (struct EventStruct *event, const char* Line)
{
  setSTA(true);
  return true;
}

bool Command_Wifi_Mode (struct EventStruct *event, const char* Line)
{
  char TmpStr1[INPUT_COMMAND_SIZE];
  if (GetArgv(Line, TmpStr1, 2)) {
    WiFiMode_t mode = WIFI_MODE_MAX;
    if(event->Par1 > 0)
    {
      mode = (WiFiMode_t)(event->Par1-1);        
    }
    else
    {
           if (strcmp_P(TmpStr1, PSTR("off")        ) == 0) mode = WIFI_OFF; 
      else if (strcmp_P(TmpStr1, PSTR("sta")        ) == 0) mode = WIFI_STA ; 
      else if (strcmp_P(TmpStr1, PSTR("ap")         ) == 0) mode = WIFI_AP  ; 
      else if (strcmp_P(TmpStr1, PSTR("ap+sta")     ) == 0) mode = WIFI_AP_STA ; 
    }
    if( mode >= WIFI_OFF && mode < WIFI_MODE_MAX )
    {      
      setWifiMode(mode);
      setAPinternal(WifiIsAP(mode));
    }
    else
    {
      Serial.println();
      Serial.println(F("Wifi Mode: invalid arguments"));
    }
  }
  else{
    Serial.println();
    Serial.print(F("WiFi Mode:"));
    Serial.println(toString(WiFi.getMode()));
  }
  return true;
}

bool Command_WiFi_Erase(struct EventStruct *event, const char* Line)
{
  WiFi.persistent(true); // use SDK storage of SSID/WPA parameters
  WifiDisconnect(); // this will store empty ssid/wpa into sdk storage
  WiFi.persistent(false); // Do not use SDK storage of SSID/WPA parameters
  return true;
}

#endif // COMMAND_WIFI_H