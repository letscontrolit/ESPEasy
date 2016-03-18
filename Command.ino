#define INPUT_COMMAND_SIZE          80
void ExecuteCommand(byte source, const char *Line)
{
  String status = "";
  boolean success = false;
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

  if (strcasecmp_P(Command, PSTR("pinstates")) == 0)
  {
    success = true;
    for (byte x = 0; x < PINSTATE_TABLE_MAX; x++)
      if (pinStates[x].plugin != 0)
      {
        status += F("\nPlugin: ");
        status += pinStates[x].plugin;
        status += F(" index: ");
        status += pinStates[x].index;
        status += F(" mode: ");
        status += pinStates[x].mode;
        status += F(" value: ");
        status += pinStates[x].value;
      }
  }

  if (strcasecmp_P(Command, PSTR("timer")) == 0)
  {
    success = true;
    if (GetArgv(Line, TmpStr1, 3))
    {
      setSystemTimer(Par1 * 1000, Par2, Par3, 0, 0);
    }
  }

  if (strcasecmp_P(Command, PSTR("cmdtimer")) == 0)
  {
    success = true;
    if (GetArgv(Line, TmpStr1, 3))
    {
      String demo = TmpStr1;
      setSystemCMDTimer(Par1 * 1000, demo);
    }
  }

  if (strcasecmp_P(Command, PSTR("TaskClear")) == 0)
  {
    success = true;
    taskClear(Par1 - 1, true);
  }

  if (strcasecmp_P(Command, PSTR("TaskGlobalSync")) == 0)
  {
    success = true;
    if (Par2 == 1)
      Settings.TaskDeviceGlobalSync[Par1 -1] = true;
    else
      Settings.TaskDeviceGlobalSync[Par1 -1] = false;
  }


  if (strcasecmp_P(Command, PSTR("wdconfig")) == 0)
  {
    success = true;
    Wire.beginTransmission(Par1);  // address
    Wire.write(Par2);              // command
    Wire.write(Par3);              // data
    Wire.endTransmission();
  }

  if (strcasecmp_P(Command, PSTR("wdread")) == 0)
  {
    success = true;
    Wire.beginTransmission(Par1);  // address
    Wire.write(0x83);              // command to set pointer
    Wire.write(Par2);              // pointer value
    Wire.endTransmission();
    Wire.requestFrom((uint8_t)Par1, (uint8_t)1);
    if (Wire.available())
    {
      byte value = Wire.read();
      status = F("Reg value: ");
      status += value;
    }
  }

  if (strcasecmp_P(Command, PSTR("VariableSet")) == 0)
  {
    success = true;
    if (GetArgv(Line, TmpStr1, 3))
      UserVar[Par1 - 1] = atof(TmpStr1);
  }

  if (strcasecmp_P(Command, PSTR("build")) == 0)
  {
    success = true;
    Settings.Build = Par1;
    SaveSettings();
  }

  if (strcasecmp_P(Command, PSTR("NoSleep")) == 0)
  {
    success = true;
    Settings.deepSleep = 0;
  }


  // ****************************************
  // commands for rules
  // ****************************************

  if (strcasecmp_P(Command, PSTR("TimerSet")) == 0)
  {
    success = true;
    RulesTimer[Par1 - 1] = millis() + (1000 * Par2);
  }

  if (strcasecmp_P(Command, PSTR("Delay")) == 0)
  {
    success = true;
    delayMillis(Par1);
  }

  if (strcasecmp_P(Command, PSTR("Rules")) == 0)
  {
    success = true;
    if (Par1 == 1)
      Settings.UseRules = true;
    else
      Settings.UseRules = false;
  }

  if (strcasecmp_P(Command, PSTR("Event")) == 0)
  {
    success = true;
    String event = Line;
    event = event.substring(6);
    event.replace("$", "#");
    if (Settings.UseRules)
      rulesProcessing(event);
  }

  // ****************************************
  // special commands for old nodo plugin
  // ****************************************

  if (strcasecmp_P(Command, PSTR("DomoticzSend")) == 0)
  {
    success = true;
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
    success = true;
    float value = 0;
    if (Domoticz_getData(Par2, &value))
    {
      status = F("DomoticzGet ");
      status += value;
    }
    else
      status = F("Error getting data");
  }

  // ****************************************
  // configure settings commands
  // ****************************************
  if (strcasecmp_P(Command, PSTR("WifiSSID")) == 0)
  {
    success = true;
    strcpy(SecuritySettings.WifiSSID, Line + 9);
  }

  if (strcasecmp_P(Command, PSTR("WifiKey")) == 0)
  {
    success = true;
    strcpy(SecuritySettings.WifiKey, Line + 8);
  }

  if (strcasecmp_P(Command, PSTR("WifiScan")) == 0)
  {
    success = true;
    WifiScan();
  }
  
  if (strcasecmp_P(Command, PSTR("WifiConnect")) == 0)
  {
    success = true;
    WifiConnect();
  }
  
  if (strcasecmp_P(Command, PSTR("WifiDisconnect")) == 0)
  {
    success = true;
    WifiDisconnect();
  }
  
  if (strcasecmp_P(Command, PSTR("Reboot")) == 0)
  {
    success = true;
    pinMode(0, INPUT);
    pinMode(2, INPUT);
    pinMode(15, INPUT);
    ESP.reset();
  }

  if (strcasecmp_P(Command, PSTR("Restart")) == 0)
  {
    success = true;
    ESP.restart();
  }
  if (strcasecmp_P(Command, PSTR("Erase")) == 0)
  {
    success = true;
    EraseFlash();
    ZeroFillFlash();
    saveToRTC(0);
    WiFi.persistent(true); // use SDK storage of SSID/WPA parameters
    WiFi.disconnect(); // this will store empty ssid/wpa into sdk storage
    WiFi.persistent(false); // Do not use SDK storage of SSID/WPA parameters
  }

  if (strcasecmp_P(Command, PSTR("Reset")) == 0)
  {
    success = true;
    ResetFactory();
  }

  if (strcasecmp_P(Command, PSTR("Save")) == 0)
  {
    success = true;
    SaveSettings();
  }

  if (strcasecmp_P(Command, PSTR("Load")) == 0)
  {
    success = true;
    LoadSettings();
  }

  if (strcasecmp_P(Command, PSTR("FlashDump")) == 0)
  {
    success = true;
    uint32_t _sectorStart = ((uint32_t)&_SPIFFS_start - 0x40200000) / SPI_FLASH_SEC_SIZE;
    uint32_t _sectorEnd = ((uint32_t)&_SPIFFS_end - 0x40200000) / SPI_FLASH_SEC_SIZE;

    Serial.print(F("Sketch size        : "));
    Serial.println(ESP.getSketchSize());
    Serial.print(F("Sketch free space  : "));
    Serial.println(ESP.getFreeSketchSpace());
    Serial.print(F("Flash size         : "));
    Serial.println(ESP.getFlashChipRealSize());
    Serial.print(F("SPIFFS start sector: "));
    Serial.println(_sectorStart);
    Serial.print(F("SPIFFS end sector  : "));
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

  if (strcasecmp_P(Command, PSTR("flashcheck")) == 0)
  {
    success = true;
    CheckFlash(Par1, Par2);
  }

  if (strcasecmp_P(Command, PSTR("Debug")) == 0)
  {
    success = true;
    Settings.SerialLogLevel = Par1;
  }

  if (strcasecmp_P(Command, PSTR("IP")) == 0)
  {
    success = true;
    if (GetArgv(Line, TmpStr1, 2))
      if (!str2ip(TmpStr1, Settings.IP))
        Serial.println("?");
  }

  if (strcasecmp_P(Command, PSTR("Settings")) == 0)
  {
    success = true;
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

  if (success)
    status += F("\nOk");
  else  
    status += F("\nUnknown command!");
  SendStatus(source,status);
}

