char* ramtest;

//Reads a string from a stream until a terminator-character.
//We make sure we're not reading more than maxSize bytes and we're not busy for longer than timeout mS.
bool safeReadStringUntil(Stream &input, String &str, char terminator, unsigned int maxSize=1024, unsigned int timeout=1000)
{
    int c;
    const unsigned long timer = millis() + timeout;
    str="";

    do {
        //read character
        c = input.read();
        if(c >= 0) {

            //found terminator, we're ok
            if (c==terminator)
            {
                return(true);
            }
            //found character, add to string
            else
            {
                str+=char(c);
                //string at max size?
                if (str.length()>=maxSize)
                {
                    addLog(LOG_LEVEL_ERROR, F("Not enough bufferspace to read all input data!"));
                    return(false);
                }
            }
        }
        yield();
    } while(!timeOutReached(timer));

    addLog(LOG_LEVEL_ERROR, F("Timeout while reading input data!"));
    return(false);

}


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
    while (!timeOutReached(timer))
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
    initRTC();
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
        byte NotificationProtocolIndex = getNotificationProtocolIndex(Settings.Notification[Par1 - 1]);
        if (NotificationProtocolIndex!=NPLUGIN_NOT_FOUND)
        {
          struct EventStruct TempEvent;
          // TempEvent.NotificationProtocolIndex = NotificationProtocolIndex;
          TempEvent.NotificationIndex=Par1-1;
          NPlugin_ptr[NotificationProtocolIndex](NPLUGIN_NOTIFY, &TempEvent, message);
        }
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
    Serial.println(lowestRAMfunction);
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

  //quickly clear all tasks, without saving (used by test suite)
  if (strcasecmp_P(Command, PSTR("TaskClearAll")) == 0)
  {
    success = true;
    for (byte t=0; t<TASKS_MAX; t++)
      taskClear(t, false);
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

  if (strcasecmp_P(Command, PSTR("i2cscanner")) == 0)
  {
    success = true;

    byte error, address;
    for (address = 1; address <= 127; address++ )
    {
      Wire.beginTransmission(address);
      error = Wire.endTransmission();
      if (error == 0)
      {
        Serial.print(F("I2C  : Found 0x"));
        Serial.println(String(address, HEX));
      }
      else if (error == 4)
      {
        Serial.print(F("I2C  : Error at 0x"));
        Serial.println(String(address, HEX));
      }
    }
  }

  // ****************************************
  // commands for rules
  // ****************************************

  if (strcasecmp_P(Command, PSTR("config")) == 0)
  {
    success = true;
    struct EventStruct TempEvent;
    String request = Line;
    remoteConfig(&TempEvent, request);
  }

  if (strcasecmp_P(Command, PSTR("deepSleep")) == 0)
  {
    success = true;
    if (Par1 > 0)
      deepSleepStart(Par1); // call the second part of the function to avoid check and enable one-shot operation
  }

  if (strcasecmp_P(Command, PSTR("TaskValueSet")) == 0)
  {
    success = true;
    if (GetArgv(Line, TmpStr1, 4))
    {
      float result = 0;
      Calculate(TmpStr1, &result);
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
    if (Par1>=1 && Par1<=RULES_TIMER_MAX)
    {
      success = true;
      if (Par2)
        //start new timer
        RulesTimer[Par1 - 1] = millis() + (1000 * Par2);
      else
        //disable existing timer
        RulesTimer[Par1 - 1] = 0L;
    }
    else
    {
      addLog(LOG_LEVEL_ERROR, F("TIMER: invalid timer number"));
    }
  }

  if (strcasecmp_P(Command, PSTR("Delay")) == 0)
  {
    success = true;
    delayBackground(Par1);
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
  if (strcasecmp_P(Command, PSTR("Publish")) == 0 && WiFi.status() == WL_CONNECTED)
  {
    // ToDo TD-er: Not sure about this function, but at least it sends to an existing MQTTclient
    int enabledMqttController = firstEnabledMQTTController();
    if (enabledMqttController >= 0) {
      success = true;
      String event = Line;
      event = event.substring(8);
      int index = event.indexOf(',');
      if (index > 0)
      {
        String topic = event.substring(0, index);
        String value = event.substring(index + 1);
        MQTTpublish(enabledMqttController, topic.c_str(), value.c_str(), Settings.MQTTRetainFlag);
      }
    }
  }

  if (strcasecmp_P(Command, PSTR("SendToUDP")) == 0 && WiFi.status() == WL_CONNECTED)
  {
    success = true;
    String strLine = Line;
    String ip = parseString(strLine, 2);
    String port = parseString(strLine, 3);
    int msgpos = getParamStartPos(strLine, 4);
    String message = strLine.substring(msgpos);
    IPAddress UDP_IP;
    if(UDP_IP.fromString(ip)) {
      portUDP.beginPacket(UDP_IP, port.toInt());
      #if defined(ESP8266)
        portUDP.write(message.c_str(), message.length());
      #endif
      #if defined(ESP32)
        portUDP.write((uint8_t*)message.c_str(), message.length());
      #endif
      portUDP.endPacket();
    }
  }

  if (strcasecmp_P(Command, PSTR("SendToHTTP")) == 0 && WiFi.status() == WL_CONNECTED)
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
      while (!client.available() && !timeOutReached(timer))
        delay(1);

      while (client.available()) {
        // String line = client.readStringUntil('\n');
        String line;
        safeReadStringUntil(client, line, '\n');


        if (line.substring(0, 15) == F("HTTP/1.1 200 OK"))
          addLog(LOG_LEVEL_DEBUG, line);
        delay(1);
      }
      client.flush();
      client.stop();
    }
  }

   // ****************************************
   // special commands for Blynk
   // ****************************************
   #ifdef CPLUGIN_012
     //FIXME: this should go to PLUGIN_WRITE in _C012.ino
   if (strcasecmp_P(Command, PSTR("BlynkGet")) == 0)
   {
     byte first_enabled_blynk_controller = firstEnabledBlynkController();
     if (first_enabled_blynk_controller == -1) {
       status = F("Controller not enabled");
     } else {
       String strLine = Line;
       strLine = strLine.substring(9);
       int index = strLine.indexOf(',');
       if (index > 0)
       {
         int index = strLine.lastIndexOf(',');
         String blynkcommand = strLine.substring(index+1);
         float value = 0;
         if (Blynk_get(blynkcommand, first_enabled_blynk_controller, &value))
         {
           UserVar[(VARS_PER_TASK * (Par1 - 1)) + Par2 - 1] = value;
         }
         else
           status = F("Error getting data");
       }
       else
       {
         if (!Blynk_get(strLine, first_enabled_blynk_controller))
         {
           status = F("Error getting data");
         }
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

  if (strcasecmp_P(Command, PSTR("WifiSSID2")) == 0)
  {
    success = true;
    strcpy(SecuritySettings.WifiSSID2, Line + 10);
  }

  if (strcasecmp_P(Command, PSTR("WifiKey2")) == 0)
  {
    success = true;
    strcpy(SecuritySettings.WifiKey2, Line + 9);
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

  if (strcasecmp_P(Command, PSTR("WifiAPMode")) == 0)
  {
    WifiAPMode(true);
    success = true;
  }

  if (strcasecmp_P(Command, PSTR("Unit")) == 0)
  {
    success = true;
    Settings.Unit=Par1;
  }

  if (strcasecmp_P(Command, PSTR("Name")) == 0)
  {
    success = true;
    strcpy(Settings.Name, Line + 5);
  }

  if (strcasecmp_P(Command, PSTR("Password")) == 0)
  {
    success = true;
    strcpy(SecuritySettings.Password, Line + 9);
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
    #if defined(ESP8266)
      ESP.reset();
    #endif
    #if defined(ESP32)
      ESP.restart();
    #endif

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
    if (GetArgv(Line, TmpStr1, 2)) {
      if (!str2ip(TmpStr1, Settings.IP))
        Serial.println("?");
    }
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
    Serial.print(F("  Name          : ")); Serial.println(Settings.Name);
    Serial.print(F("  Unit          : ")); Serial.println((int)Settings.Unit);
    Serial.print(F("  WifiSSID      : ")); Serial.println(SecuritySettings.WifiSSID);
    Serial.print(F("  WifiKey       : ")); Serial.println(SecuritySettings.WifiKey);
    Serial.print(F("  WifiSSID2     : ")); Serial.println(SecuritySettings.WifiSSID2);
    Serial.print(F("  WifiKey2      : ")); Serial.println(SecuritySettings.WifiKey2);
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
