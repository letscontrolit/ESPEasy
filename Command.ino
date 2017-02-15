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
  if (strcasecmp_P(Command, PSTR("sysload")) == 0)
  {
    success = true;
    Serial.print(100 - (100 * loopCounterLast / loopCounterMax));
    Serial.print(F("% (LC="));
    Serial.print(int(loopCounterLast / 30));
    Serial.println(F(")"));
  }

  if (strcasecmp_P(Command, PSTR("SerialFloat")) == 0)
  {
    success = true;
    pinMode(1, INPUT);
    pinMode(3, INPUT);
    delay(60000);
  }

  if (strcasecmp_P(Command, PSTR("meminfo")) == 0)
  {
    success = true;
    Serial.print(F("SecurityStruct         : "));
    Serial.println(sizeof(SecuritySettings));
    Serial.print(F("SettingsStruct         : "));
    Serial.println(sizeof(Settings));
    Serial.print(F("ExtraTaskSettingsStruct: "));
    Serial.println(sizeof(ExtraTaskSettings));
  }

  if (strcasecmp_P(Command, PSTR("TaskClear")) == 0)
  {
    success = true;
    taskClear(Par1 - 1, true);
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

  if (strcasecmp_P(Command, PSTR("TaskValueSet")) == 0)
  {
    success = true;
    if (GetArgv(Line, TmpStr1, 4))
    {
      float result = 0;
      byte error = Calculate(TmpStr1, &result);
      UserVar[(VARS_PER_TASK * (Par1 - 1)) + Par2 - 1] = result;
    }
  }

  if (strcasecmp_P(Command, PSTR("TaskRun")) == 0)
  {
    success = true;
    SensorSendTask(Par1 - 1);
  }

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

  if (strcasecmp_P(Command, PSTR("SendTo")) == 0)
  {
    success = true;
    String event = Line;
    event = event.substring(7);
    int index = event.indexOf(',');
    if (index > 0)
    {
      event = event.substring(index + 1);
      SendUDPCommand(Par1, (char*)event.c_str(), event.length());
    }
  }

  if (strcasecmp_P(Command, PSTR("Publish")) == 0)
  {
    success = true;
    String event = Line;
    event = event.substring(8);
    int index = event.indexOf(',');
    if (index > 0)
    {
      String topic = event.substring(0, index);
      String value = event.substring(index + 1);
      MQTTclient.publish(topic.c_str(), value.c_str(), Settings.MQTTRetainFlag);
    }
  }

  if (strcasecmp_P(Command, PSTR("SendToUDP")) == 0)
  {
    success = true;
    String strLine = Line;
    String ip = parseString(strLine, 2);
    String port = parseString(strLine, 3);
    int msgpos = getParamStartPos(strLine, 4);
    String message = strLine.substring(msgpos);
    byte ipaddress[4];
    str2ip((char*)ip.c_str(), ipaddress);
    IPAddress UDP_IP(ipaddress[0], ipaddress[1], ipaddress[2], ipaddress[3]);
    portUDP.beginPacket(UDP_IP, port.toInt());
    portUDP.write(message.c_str(), message.length());
    portUDP.endPacket();
  }

  if (strcasecmp_P(Command, PSTR("SendToHTTP")) == 0)
  {
    success = true;
    String strLine = Line;
    String host = parseString(strLine, 2);
    String port = parseString(strLine, 3);
    int pathpos = getParamStartPos(strLine, 4);
    String path = strLine.substring(pathpos);
    WiFiClient client;
    if (client.connect(host.c_str(), port.toInt()))
    {
      client.print(String("GET ") + path + " HTTP/1.1\r\n" +
                   "Host: " + host + "\r\n" +
                   "Connection: close\r\n\r\n");

      unsigned long timer = millis() + 200;
      while (!client.available() && millis() < timer)
        delay(1);

      while (client.available()) {
        String line = client.readStringUntil('\n');
        if (line.substring(0, 15) == "HTTP/1.1 200 OK")
          addLog(LOG_LEVEL_DEBUG, line);
        delay(1);
      }
      client.flush();
      client.stop();
    }
  }

#ifdef PLUGIN_BUILD_TESTING
  // ****************************************
  // special commands for Blynk
  // ****************************************

  if (strcasecmp(Command, "BlynkGet") == 0)
  {
    String event = Line;
    event = event.substring(9);
    int index = event.indexOf(',');
    if (index > 0)
    {
      int index = event.lastIndexOf(',');
      String blynkcommand = event.substring(index+1);
      float value = 0;
      if (Blynk_get(blynkcommand, &value))
      {
        UserVar[(VARS_PER_TASK * (Par1 - 1)) + Par2 - 1] = value;
      }
      else
        status = F("Error getting data");
    }
    else
    {
      if (!Blynk_get(event))
      {
        status = F("Error getting data");
      }
    }

  }
#endif

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
    WifiConnect(1);
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

  yield();

  if (success)
    status += F("\nOk");
  else
    status += F("\nUnknown command!");
  SendStatus(source, status);
  yield();
}
