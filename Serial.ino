/********************************************************************************************\
* Process data from Serial Interface
\*********************************************************************************************/
#define INPUT_COMMAND_SIZE          80
void ExecuteCommand(const char *Line)
{
  char TmpStr1[80];
  TmpStr1[0] = 0;
  char Command[80];
  Command[0] = 0;
  int Par1 = 0;
  int Par2 = 0;
  int Par3 = 0;

  GetArgv(Line, Command, 1);
  if (GetArgv(Line, TmpStr1, 2)) Par1 = str2int(TmpStr1);
  if (GetArgv(Line, TmpStr1, 3)) Par2 = str2int(TmpStr1);
  if (GetArgv(Line, TmpStr1, 4)) Par3 = str2int(TmpStr1);

  // ****************************************
  // commands for debugging
  // ****************************************

  if (strcasecmp_P(Command, PSTR("getudptaskinfo")) == 0) // local task, remote unit, remote task
  {
    char data[6];
    data[0]= 255;
    data[1]= 2;
    data[2]= Settings.Unit; // source unit
    data[3]= Par2; // dest unit
    data[4]= Par1-1; // local task
    data[5]= Par3-1; // task index to request
    sendUDP(Par2, (byte*) &data, sizeof(data));
  }

  if (strcasecmp_P(Command, PSTR("getudptaskdata")) == 0) // local task, remote unit, remote task
  {
    char data[6];
    data[0]= 255;
    data[1]= 4;
    data[2]= Settings.Unit; // source unit
    data[3]= Par2; // dest unit
    data[4]= Par1-1; // local task
    data[5]= Par3-1; // task index to request
    sendUDP(Par2, (byte*) &data, sizeof(data));
  }

  if (strcasecmp_P(Command, PSTR("TaskClear")) == 0)
  {
    taskClear(Par1 - 1,true);
  }

  if (strcasecmp_P(Command, PSTR("resetinfo")) == 0)
  {
    Serial.print(F("getResetInfo: "));
    Serial.println(ESP.getResetInfo());
  }
  
  if (strcasecmp_P(Command, PSTR("wdconfig")) == 0)
  {
    Wire.beginTransmission(Par1);  // address
    Wire.write(Par2);              // command
    Wire.write(Par3);              // data
    Wire.endTransmission();
  }

  if (strcasecmp_P(Command, PSTR("wdread")) == 0)
  {
    Wire.beginTransmission(Par1);  // address
    Wire.write(0x83);              // command to set pointer
    Wire.write(Par2);              // pointer value
    Wire.endTransmission();
    Wire.requestFrom((uint8_t)Par1, (uint8_t)1);
    if (Wire.available())
    {
      byte value = Wire.read();
      if(printToWeb)
      {
        printWebString += F("Reg value: ");
        printWebString += value;
      }      
      Serial.print(F("Reg value: "));
      Serial.println(value);
    }
  }

#if FEATURE_TIME
  if (strcasecmp_P(Command, PSTR("ntp")) == 0)
    getNtpTime();
#endif

  if (strcasecmp_P(Command, PSTR("setsdk")) == 0)
  {
    WiFi.persistent(true); // use SDK storage of SSID/WPA parameters
    WiFi.disconnect();
    WiFi.begin(SecuritySettings.WifiSSID, SecuritySettings.WifiKey);
    WiFi.persistent(false);
  }

  if (strcasecmp_P(Command, PSTR("clearsdk")) == 0)
  {
    WiFi.persistent(true); // use SDK storage of SSID/WPA parameters
    WiFi.disconnect();
    WiFi.persistent(false);
    WiFi.begin(SecuritySettings.WifiSSID, SecuritySettings.WifiKey);
  }

  if (strcasecmp_P(Command, PSTR("getssid")) == 0)
  {
    struct station_config conf;
    if (wifi_station_get_config(&conf))
    {
      Serial.print(F("SDK current: "));
      Serial.println(String(reinterpret_cast<char*>(conf.ssid)));
    }
    struct station_config sconf;
    if (wifi_station_get_config_default(&sconf))
    {
      Serial.print(F("SDK default: "));
      Serial.println(String(reinterpret_cast<char*>(sconf.ssid)));
    }
  }

  if (strcasecmp_P(Command, PSTR("VariableSet")) == 0)
  {
    if (GetArgv(Line, TmpStr1, 3))
      UserVar[Par1 - 1] = atof(TmpStr1);
  }

  if (strcasecmp_P(Command, PSTR("build")) == 0)
  {
    Settings.Build = Par1;
    SaveSettings();
  }

  if (strcasecmp_P(Command, PSTR("NoSleep")) == 0)
  {
    Settings.deepSleep = 0;
  }

  // ****************************************
  // special commands for old nodo plugin
  // ****************************************

  if (strcasecmp_P(Command, PSTR("DomoticzSend")) == 0)
  {
    if (GetArgv(Line, TmpStr1, 4))
    {
      struct EventStruct TempEvent;
      TempEvent.TaskIndex = 0;
      TempEvent.BaseVarIndex = (VARS_PER_TASK * TASKS_MAX) - 1;
      TempEvent.idx = Par2;
      TempEvent.sensorType = Par1;
      UserVar[(VARS_PER_TASK * TASKS_MAX) - 1] = atof(TmpStr1);
      sendData(&TempEvent);
    }
  }

  if (strcasecmp(Command, "DomoticzGet") == 0)
  {
    float value = 0;
    if (Domoticz_getData(Par2, &value))
    {
      Serial.print("DomoticzGet ");
      Serial.println(value);
    }
    else
      Serial.println("Error getting data");
  }
  
  // ****************************************
  // configure settings commands
  // ****************************************
  if (strcasecmp_P(Command, PSTR("WifiSSID")) == 0)
    strcpy(SecuritySettings.WifiSSID, Line + 9);

  if (strcasecmp_P(Command, PSTR("WifiKey")) == 0)
    strcpy(SecuritySettings.WifiKey, Line + 8);

  if (strcasecmp_P(Command, PSTR("WifiScan")) == 0)
    WifiScan();

  if (strcasecmp_P(Command, PSTR("WifiConnect")) == 0)
    WifiConnect();

  if (strcasecmp_P(Command, PSTR("WifiDisconnect")) == 0)
    WifiDisconnect();

  if (strcasecmp_P(Command, PSTR("Reboot")) == 0)
  {
    pinMode(0, INPUT);
    pinMode(2, INPUT);
    pinMode(15, INPUT);
    ESP.reset();
  }

  if (strcasecmp_P(Command, PSTR("Restart")) == 0)
    ESP.restart();

  if (strcasecmp_P(Command, PSTR("Erase")) == 0)
  {
    EraseFlash();
    saveToRTC(0);
    WiFi.persistent(true); // use SDK storage of SSID/WPA parameters
    WiFi.disconnect(); // this will store empty ssid/wpa into sdk storage
    WiFi.persistent(false); // Do not use SDK storage of SSID/WPA parameters
  }

  if (strcasecmp_P(Command, PSTR("Reset")) == 0)
    ResetFactory();

  if (strcasecmp_P(Command, PSTR("Save")) == 0)
    SaveSettings();

  if (strcasecmp_P(Command, PSTR("Load")) == 0)
    LoadSettings();

  if (strcasecmp_P(Command, PSTR("FlashDump")) == 0)
  {
    uint32_t _sectorStart = ((uint32_t)&_SPIFFS_start - 0x40200000) / SPI_FLASH_SEC_SIZE;
    uint32_t _sectorEnd = ((uint32_t)&_SPIFFS_end - 0x40200000) / SPI_FLASH_SEC_SIZE;

    Serial.print(F("Flash start sector: "));
    Serial.println(_sectorStart);
    Serial.print(F("Flash end sector  : "));
    Serial.println(_sectorEnd);
    char data[80];
    if (Par2 == 0) Par2 = Par1;
    for (int x = Par1; x <= Par2; x++)
    {
      LoadFromFlash(x * 1024, (byte*)&data, sizeof(data));
      Serial.print(F("Offset: "));
      Serial.print(x);
      Serial.print(" : ");
      Serial.println(data);
    }
  }

  if (strcasecmp_P(Command, PSTR("Delay")) == 0)
    Settings.Delay = Par1;

  if (strcasecmp_P(Command, PSTR("Debug")) == 0)
    Settings.SerialLogLevel = Par1;

  if (strcasecmp_P(Command, PSTR("IP")) == 0)
  {
    if (GetArgv(Line, TmpStr1, 2))
      if (!str2ip(TmpStr1, Settings.IP))
        Serial.println("?");
  }

  if (strcasecmp_P(Command, PSTR("Settings")) == 0)
  {
    char str[20];
    Serial.println();

    Serial.println(F("System Info"));
    IPAddress ip = WiFi.localIP();
    sprintf_P(str, PSTR("%u.%u.%u.%u"), ip[0], ip[1], ip[2], ip[3]);
    Serial.print(F("  IP Address    : ")); Serial.println(str);
    Serial.print(F("  Build         : ")); Serial.println((int)BUILD);
    Serial.print(F("  Unit          : ")); Serial.println((int)Settings.Unit);
    Serial.print(F("  WifiSSID      : ")); Serial.println(SecuritySettings.WifiSSID);
    Serial.print(F("  WifiKey       : ")); Serial.println(SecuritySettings.WifiKey);
    Serial.print(F("  Free mem      : ")); Serial.println(FreeMem());
  }
}


/********************************************************************************************\
* Get data from Serial Interface
\*********************************************************************************************/
#define INPUT_BUFFER_SIZE          128

byte SerialInByte;
int SerialInByteCounter = 0;
char InputBuffer_Serial[INPUT_BUFFER_SIZE + 2];

void serial()
{
  while (Serial.available())
  {
    yield();
    SerialInByte = Serial.read();
    if (SerialInByte == 255) // binary data...
    {
      Serial.flush();
      return;
    }

    if (isprint(SerialInByte))
    {
      if (SerialInByteCounter < INPUT_BUFFER_SIZE) // add char to string if it still fits
        InputBuffer_Serial[SerialInByteCounter++] = SerialInByte;
    }

    if (SerialInByte == '\n')
    {
      InputBuffer_Serial[SerialInByteCounter] = 0; // serial data completed
      Serial.write('>');
      Serial.println(InputBuffer_Serial);
      ExecuteCommand(InputBuffer_Serial);
      SerialInByteCounter = 0;
      InputBuffer_Serial[0] = 0; // serial data processed, clear buffer
    }
  }
}

