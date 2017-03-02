char* ramtest;

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

  if (strcasecmp_P(Command, PSTR("background")) == 0)
  {
    success = true;
    unsigned long timer = millis() + Par1;
    Serial.println("start");
    while (millis() < timer)
      backgroundtasks();
    Serial.println("end");
  }
          
  if (strcasecmp_P(Command, PSTR("executeRules")) == 0)
  {
    success = true;
    if (GetArgv(Line, TmpStr1, 2))
    {
      String fileName = TmpStr1;
      String event = "";
      rulesProcessingFile(fileName, event);
    }
  }

  if (strcasecmp_P(Command, PSTR("clearRTCRAM")) == 0)
  {
    success = true;
    RTC.factoryResetCounter = 0;
    RTC.deepSleepState = 0;
    RTC.rebootCounter = 0;
    RTC.flashDayCounter = 0;
    RTC.flashCounter = 0;
    saveToRTC();
  }

  if (strcasecmp_P(Command, PSTR("notify")) == 0)
  {
    success = true;
    String message = "";
    if (GetArgv(Line, TmpStr1, 3))
      message = TmpStr1;

    if (Par1 > 0)
    {
      if (Settings.NotificationEnabled[Par1 - 1] && Settings.Notification[Par1 - 1] != 0)
      {
        byte NotificationProtocolIndex = getNotificationIndex(Settings.Notification[Par1 - 1]);
        struct EventStruct TempEvent;
        TempEvent.NotificationProtocolIndex = Par1 - 1;
        if (NPlugin_id[NotificationProtocolIndex] != 0)
          NPlugin_ptr[NotificationProtocolIndex](NPLUGIN_NOTIFY, &TempEvent, message);
      }
    }
  }

  if (strcasecmp_P(Command, PSTR("resetFlashWriteCounter")) == 0)
  {
    success = true;
    RTC.flashDayCounter = 0;
  }

  if (strcasecmp_P(Command, PSTR("udptest")) == 0)
  {
    success = true;
    for (byte x = 0; x < Par2; x++)
    {
      String event = "Test ";
      event += x;
      SendUDPCommand(Par1, (char*)event.c_str(), event.length());
    }
  }

  if (strcasecmp_P(Command, PSTR("sdcard")) == 0)
  {
    success = true;
    File root = SD.open("/");
    root.rewindDirectory();
    printDirectory(root, 0);
    root.close();
  }

  if (strcasecmp_P(Command, PSTR("sdremove")) == 0)
  {
    success = true;
    String fname = Line;
    fname = fname.substring(9);
    Serial.print(F("Removing:"));
    Serial.println(fname.c_str());
    SD.remove((char*)fname.c_str());
  }

  if (strcasecmp_P(Command, PSTR("lowmem")) == 0)
  {
    Serial.print(lowestRAM);
    Serial.print(F(" : "));
    Serial.println(lowestRAMid);
    success = true;
  }

  if (strcasecmp_P(Command, PSTR("malloc")) == 0)
  {
    ramtest = (char *)malloc(Par1);
    success = true;
  }

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
    Serial.print(F("DeviceStruct: "));
    Serial.println(sizeof(Device));
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

  if (strcasecmp_P(Command, PSTR("deepSleep")) == 0)
  {
    success = true;
    if (Par1 > 0)
      deepSleep(Par1);
  }

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
        if (line.substring(0, 15) == F("HTTP/1.1 200 OK"))
          addLog(LOG_LEVEL_DEBUG, line);
        delay(1);
      }
      client.flush();
      client.stop();
    }
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
    WifiConnect(true, 1);
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

void printDirectory(File dir, int numTabs) {
  while (true) {

    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
}
