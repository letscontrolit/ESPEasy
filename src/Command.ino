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
Command commandStringToEnum(const char * cmd) {
  String tmpcmd;
  tmpcmd = cmd;
  tmpcmd.toLowerCase();
  String log = F("Command: ");
  log += tmpcmd;
  addLog(LOG_LEVEL_INFO, log);
  char cmd_lc[INPUT_COMMAND_SIZE];
  tmpcmd.toCharArray(cmd_lc, tmpcmd.length() + 1);
  switch (cmd_lc[0]) {
    case 'a': {
           if (strcmp_P(cmd_lc, PSTR("accessinfo")            ) == 0) return cmd_accessinfo;
      break;
    }
    case 'b': {
           if (strcmp_P(cmd_lc, PSTR("background")            ) == 0) return cmd_background;
      else if (strcmp_P(cmd_lc, PSTR("blynkget")              ) == 0) return cmd_BlynkGet;
      else if (strcmp_P(cmd_lc, PSTR("build")                 ) == 0) return cmd_build;
      break;
    }
    case 'c': {
           if (strcmp_P(cmd_lc, PSTR("clearaccessblock")      ) == 0) return cmd_clearaccessblock;
      else if (strcmp_P(cmd_lc, PSTR("clearrtcram")           ) == 0) return cmd_clearRTCRAM;
      else if (strcmp_P(cmd_lc, PSTR("config")                ) == 0) return cmd_config;
      break;
    }
    case 'd': {
           if (strcmp_P(cmd_lc, PSTR("debug")                 ) == 0) return cmd_Debug;
      else if (strcmp_P(cmd_lc, PSTR("delay")                 ) == 0) return cmd_Delay;
      else if (strcmp_P(cmd_lc, PSTR("deepsleep")             ) == 0) return cmd_deepSleep;
      break;
    }
    case 'e': {
           if (strcmp_P(cmd_lc, PSTR("erase")                 ) == 0) return cmd_Erase;
      else if (strcmp_P(cmd_lc, PSTR("event")                 ) == 0) return cmd_Event;
      else if (strcmp_P(cmd_lc, PSTR("executerules")          ) == 0) return cmd_executeRules;
      break;
    }
    case 'i': {
           if (strcmp_P(cmd_lc, PSTR("i2cscanner")            ) == 0) return cmd_i2cscanner;
      else if (strcmp_P(cmd_lc, PSTR("ip")                    ) == 0) return cmd_IP;
      break;
    }
    case 'l': {
           if (strcmp_P(cmd_lc, PSTR("load")                  ) == 0) return cmd_Load;
      else if (strcmp_P(cmd_lc, PSTR("logentry")              ) == 0) return cmd_logentry;
      else if (strcmp_P(cmd_lc, PSTR("lowmem")                ) == 0) return cmd_lowmem;
      break;
    }
    case 'm': {
           if (strcmp_P(cmd_lc, PSTR("malloc")                ) == 0) return cmd_malloc;
      else if (strcmp_P(cmd_lc, PSTR("meminfo")               ) == 0) return cmd_meminfo;
      break;
    }
    case 'n': {
           if (strcmp_P(cmd_lc, PSTR("name")                  ) == 0) return cmd_Name;
      else if (strcmp_P(cmd_lc, PSTR("notify")                ) == 0) return cmd_notify;
      else if (strcmp_P(cmd_lc, PSTR("nosleep")               ) == 0) return cmd_NoSleep;
      break;
    }
    case 'p': {
           if (strcmp_P(cmd_lc, PSTR("password")              ) == 0) return cmd_Password;
      else if (strcmp_P(cmd_lc, PSTR("publish")               ) == 0) return cmd_Publish;
      break;
    }
    case 'r': {
           if (strcmp_P(cmd_lc, PSTR("reboot")                ) == 0) return cmd_Reboot;
      else if (strcmp_P(cmd_lc, PSTR("reset")                 ) == 0) return cmd_Reset;
      else if (strcmp_P(cmd_lc, PSTR("restart")               ) == 0) return cmd_Restart;
      else if (strcmp_P(cmd_lc, PSTR("resetflashwritecounter")) == 0) return cmd_resetFlashWriteCounter;
      else if (strcmp_P(cmd_lc, PSTR("rules")                 ) == 0) return cmd_Rules;
      break;
    }
    case 's': {
           if (strcmp_P(cmd_lc, PSTR("sdcard")                ) == 0) return cmd_sdcard;
      else if (strcmp_P(cmd_lc, PSTR("sdremove")              ) == 0) return cmd_sdremove;
      else if (strcmp_P(cmd_lc, PSTR("sysload")               ) == 0) return cmd_sysload;
      else if (strcmp_P(cmd_lc, PSTR("save")                  ) == 0) return cmd_Save;
      else if (strcmp_P(cmd_lc, PSTR("sendto")                ) == 0) return cmd_SendTo;
      else if (strcmp_P(cmd_lc, PSTR("sendtohttp")            ) == 0) return cmd_SendToHTTP;
      else if (strcmp_P(cmd_lc, PSTR("sendtoudp")             ) == 0) return cmd_SendToUDP;
      else if (strcmp_P(cmd_lc, PSTR("serialfloat")           ) == 0) return cmd_SerialFloat;
      else if (strcmp_P(cmd_lc, PSTR("settings")              ) == 0) return cmd_Settings;
      break;
    }
    case 't': {
           if (strcmp_P(cmd_lc, PSTR("taskclear")             ) == 0) return cmd_TaskClear;
      else if (strcmp_P(cmd_lc, PSTR("taskclearall")          ) == 0) return cmd_TaskClearAll;
      else if (strcmp_P(cmd_lc, PSTR("taskrun")               ) == 0) return cmd_TaskRun;
      else if (strcmp_P(cmd_lc, PSTR("taskvalueset")          ) == 0) return cmd_TaskValueSet;
      else if (strcmp_P(cmd_lc, PSTR("taskvaluesetandrun")    ) == 0) return cmd_TaskValueSetAndRun;
      else if (strcmp_P(cmd_lc, PSTR("timerset")              ) == 0) return cmd_TimerSet;
      else if (strcmp_P(cmd_lc, PSTR("timerpause")            ) == 0) return cmd_TimerPause;
      else if (strcmp_P(cmd_lc, PSTR("timerresume")           ) == 0) return cmd_TimerResume;
      break;
    }
    case 'u': {
           if (strcmp_P(cmd_lc, PSTR("udptest")               ) == 0) return cmd_udptest;
      else if (strcmp_P(cmd_lc, PSTR("unit")                  ) == 0) return cmd_Unit;
      break;
    }
    case 'w': {
           if (strcmp_P(cmd_lc, PSTR("wdconfig")              ) == 0) return cmd_wdconfig;
      else if (strcmp_P(cmd_lc, PSTR("wdread")                ) == 0) return cmd_wdread;
      else if (strcmp_P(cmd_lc, PSTR("wifiapmode")            ) == 0) return cmd_WifiAPMode;
      else if (strcmp_P(cmd_lc, PSTR("wificonnect")           ) == 0) return cmd_WifiConnect;
      else if (strcmp_P(cmd_lc, PSTR("wifidisconnect")        ) == 0) return cmd_WifiDisconnect;
      else if (strcmp_P(cmd_lc, PSTR("wifikey2")              ) == 0) return cmd_WifiKey2;
      else if (strcmp_P(cmd_lc, PSTR("wifikey")               ) == 0) return cmd_WifiKey;
      else if (strcmp_P(cmd_lc, PSTR("wifissid2")             ) == 0) return cmd_WifiSSID2;
      else if (strcmp_P(cmd_lc, PSTR("wifissid")              ) == 0) return cmd_WifiSSID;
      else if (strcmp_P(cmd_lc, PSTR("wifiscan")              ) == 0) return cmd_WifiScan;
      break;
    }
    default:
      addLog(LOG_LEVEL_INFO, F("Command unknown"));
      return cmd_Unknown;
  }
  addLog(LOG_LEVEL_INFO, F("Command unknown"));
  return cmd_Unknown;
}

void ExecuteCommand(byte source, const char *Line)
{
  checkRAM(F("ExecuteCommand"));
  String status = "";
  boolean success = false;
  char TmpStr1[INPUT_COMMAND_SIZE];
  TmpStr1[0] = 0;
  char cmd[INPUT_COMMAND_SIZE];
  cmd[0] = 0;
  int Par1 = 0;
  int Par2 = 0;
  int Par3 = 0;

  GetArgv(Line, cmd, 1);
  if (GetArgv(Line, TmpStr1, 2)) Par1 = str2int(TmpStr1);
  if (GetArgv(Line, TmpStr1, 3)) Par2 = str2int(TmpStr1);
  if (GetArgv(Line, TmpStr1, 4)) Par3 = str2int(TmpStr1);

  const Command cmd_enum = commandStringToEnum(cmd);
  switch (cmd_enum) {

  // ****************************************
  // commands for debugging
  // ****************************************

  case cmd_background:
  {
    success = true;
    unsigned long timer = millis() + Par1;
    Serial.println("start");
    while (!timeOutReached(timer))
      backgroundtasks();
    Serial.println("end");
    break;
  }

  case cmd_executeRules:
  {
    success = true;
    if (GetArgv(Line, TmpStr1, 2))
    {
      String fileName = TmpStr1;
      String event = "";
      rulesProcessingFile(fileName, event);
    }
    break;
  }

  case cmd_clearRTCRAM:
  {
    success = true;
    initRTC();
    break;
  }

  case cmd_notify:
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
    break;
  }

  case cmd_resetFlashWriteCounter:
  {
    success = true;
    RTC.flashDayCounter = 0;
    break;
  }

  case cmd_udptest:
  {
    success = true;
    for (byte x = 0; x < Par2; x++)
    {
      String event = "Test ";
      event += x;
      SendUDPCommand(Par1, (char*)event.c_str(), event.length());
    }
    break;
  }
#ifdef FEATURE_SD
  case cmd_sdcard:
  {
    success = true;
    File root = SD.open("/");
    root.rewindDirectory();
    printDirectory(root, 0);
    root.close();
    break;
  }

  case cmd_sdremove:
  {
    success = true;
    String fname = Line;
    fname = fname.substring(9);
    Serial.print(F("Removing:"));
    Serial.println(fname.c_str());
    SD.remove((char*)fname.c_str());
    break;
  }
#endif

  case cmd_lowmem:
  {
    Serial.print(lowestRAM);
    Serial.print(F(" : "));
    Serial.println(lowestRAMfunction);
    success = true;
    break;
  }

  case cmd_malloc:
  {
    ramtest = (char *)malloc(Par1);
    success = true;
    break;
  }

  case cmd_sysload:
  {
    success = true;
    Serial.print(100 - (100 * loopCounterLast / loopCounterMax));
    Serial.print(F("% (LC="));
    Serial.print(int(loopCounterLast / 30));
    Serial.println(F(")"));
    break;
  }

  case cmd_SerialFloat:
  {
    success = true;
    pinMode(1, INPUT);
    pinMode(3, INPUT);
    delay(60000);
    break;
  }

  case cmd_accessinfo:
  {
    success = true;
    Serial.print(F("Allowed IP range : "));
    Serial.println(describeAllowedIPrange());
    break;
  }

  case cmd_clearaccessblock:
  {
    success = true;
    clearAccessBlock();
    Serial.print(F("Allowed IP range : "));
    Serial.println(describeAllowedIPrange());
    break;
  }

  case cmd_meminfo:
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
    break;
  }

  case cmd_TaskClear:
  {
    success = true;
    taskClear(Par1 - 1, true);
    break;
  }

  //quickly clear all tasks, without saving (used by test suite)
  case cmd_TaskClearAll:
  {
    success = true;
    for (byte t=0; t<TASKS_MAX; t++)
      taskClear(t, false);
    break;
  }

  case cmd_wdconfig:
  {
    success = true;
    Wire.beginTransmission(Par1);  // address
    Wire.write(Par2);              // command
    Wire.write(Par3);              // data
    Wire.endTransmission();
    break;
  }

  case cmd_wdread:
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
    break;
  }

  case cmd_build:
  {
    success = true;
    Settings.Build = Par1;
    SaveSettings();
    break;
  }

  case cmd_NoSleep:
  {
    success = true;
    Settings.deepSleep = 0;
    break;
  }

  case cmd_i2cscanner:
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
    break;
  }

  // ****************************************
  // commands for rules
  // ****************************************

  case cmd_config:
  {
    success = true;
    struct EventStruct TempEvent;
    String request = Line;
    remoteConfig(&TempEvent, request);
    break;
  }

  case cmd_deepSleep:
  {
    success = true;
    if (Par1 > 0)
      deepSleepStart(Par1); // call the second part of the function to avoid check and enable one-shot operation
    break;
  }

  case cmd_TaskValueSet:
  {
    success = true;
    if (GetArgv(Line, TmpStr1, 4))
    {
      float result = 0;
      Calculate(TmpStr1, &result);
      UserVar[(VARS_PER_TASK * (Par1 - 1)) + Par2 - 1] = result;
    }
    break;
  }

  case cmd_TaskValueSetAndRun:
  {
    success = true;
    if (GetArgv(Line, TmpStr1, 4))
    {
      float result = 0;
      Calculate(TmpStr1, &result);
      UserVar[(VARS_PER_TASK * (Par1 - 1)) + Par2 - 1] = result;
      SensorSendTask(Par1 - 1);
    }
    break;
  }

  // it does nothing, just print on the log (INFO) the content of the line in the rule:
  // log example of command: logentry,S=[task#value] T=[task1#value]:
  // ACT  : logentry,S=24 T=23.1
  // Command: logentry
  case cmd_logentry:
  {
    success = true;
    break;
  }

  case cmd_TaskRun:
  {
    success = true;
    SensorSendTask(Par1 - 1);
    break;
  }

  case cmd_TimerSet:
  {
    if (Par1>=1 && Par1<=RULES_TIMER_MAX)
    {
      success = true;
      if (Par2)
      {
        //start new timer
        RulesTimer[Par1 - 1].interval = Par2*1000;
        RulesTimer[Par1 - 1].paused = false;
        RulesTimer[Par1 - 1].timestamp = millis() + (1000 * Par2);
      }
      else
      {
        //disable existing timer
        RulesTimer[Par1 - 1].interval = 0;
        RulesTimer[Par1 - 1].paused = false;
        RulesTimer[Par1 - 1].timestamp = 0L;
      }
    }
    else
    {
      addLog(LOG_LEVEL_ERROR, F("TIMER: invalid timer number"));
    }
    break;
  }

  case cmd_TimerPause:
  {
    if (Par1>=1 && Par1<=RULES_TIMER_MAX)
    {
       success = true;
       if (RulesTimer[Par1 - 1].paused == false)
       {
          long delta = timePassedSince(RulesTimer[Par1 - 1].timestamp);
          if(RulesTimer[Par1 - 1].timestamp != 0L && delta < 0)
          {
            String event = F("Rules#TimerPause=");
            event += Par1;
            rulesProcessing(event);
            RulesTimer[Par1 - 1].paused = true;
            RulesTimer[Par1 - 1].interval = -delta; // set remaind time
          }
       }
       else
       {
         addLog(LOG_LEVEL_INFO, F("TIMER: already paused"));
       }
    }
    else
    {
      addLog(LOG_LEVEL_ERROR, F("TIMER: invalid timer number"));
    }
    break;
  }

  case cmd_TimerResume:
  {
    if (Par1>=1 && Par1<=RULES_TIMER_MAX)
    {
       success = true;
       if (RulesTimer[Par1 - 1].paused == true)
       {
          if(RulesTimer[Par1 - 1].interval > 0 && RulesTimer[Par1 - 1].timestamp != 0L)
          {
            String event = F("Rules#TimerResume=");
            event += Par1;
            rulesProcessing(event);
            RulesTimer[Par1 - 1].timestamp = millis() + (RulesTimer[Par1 - 1].interval);
            RulesTimer[Par1 - 1].paused = false;
          }
       }
       else
       {
         addLog(LOG_LEVEL_INFO, F("TIMER: already resumed"));
       }
    }
    else
    {
      addLog(LOG_LEVEL_ERROR, F("TIMER: invalid timer number"));
    }
    break;
  }

  case cmd_Delay:
  {
    success = true;
    delayBackground(Par1);
    break;
  }

  case cmd_Rules:
  {
    success = true;
    if (Par1 == 1)
      Settings.UseRules = true;
    else
      Settings.UseRules = false;
    break;
  }

  case cmd_Event:
  {
    success = true;
    String event = Line;
    event = event.substring(6);
    event.replace("$", "#");
    if (Settings.UseRules)
      rulesProcessing(event);
    break;
  }

  case cmd_SendTo:
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
    break;
  }
  case cmd_Publish:
  {
    if (wifiStatus == ESPEASY_WIFI_SERVICES_INITIALIZED) {
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
    break;
  }

  case cmd_SendToUDP:
  {
    if (wifiStatus == ESPEASY_WIFI_SERVICES_INITIALIZED) {
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
    break;
  }

  case cmd_SendToHTTP:
  {
    if (wifiStatus == ESPEASY_WIFI_SERVICES_INITIALIZED) {
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
    break;
  }

  // ****************************************
  // special commands for Blynk
  // ****************************************

#ifdef CPLUGIN_012
  //FIXME: this should go to PLUGIN_WRITE in _C012.ino
  case cmd_BlynkGet:
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
    break;
  }
#endif

  // ****************************************
  // configure settings commands
  // ****************************************
  case cmd_WifiSSID:
  {
    success = true;
    strcpy(SecuritySettings.WifiSSID, Line + 9);
    break;
  }

  case cmd_WifiKey:
  {
    success = true;
    strcpy(SecuritySettings.WifiKey, Line + 8);
    break;
  }

  case cmd_WifiSSID2:
  {
    success = true;
    strcpy(SecuritySettings.WifiSSID2, Line + 10);
    break;
  }

  case cmd_WifiKey2:
  {
    success = true;
    strcpy(SecuritySettings.WifiKey2, Line + 9);
    break;
  }

  case cmd_WifiScan:
  {
    success = true;
    WifiScan();
    break;
  }

  case cmd_WifiConnect:
  {
    success = true;
    WiFiConnectRelaxed();
    break;
  }

  case cmd_WifiDisconnect:
  {
    success = true;
    WifiDisconnect();
    break;
  }

  case cmd_WifiAPMode:
  {
    setAP(true);
    success = true;
    break;
  }

  case cmd_Unit:
  {
    success = true;
    Settings.Unit=Par1;
    break;
  }

  case cmd_Name:
  {
    success = true;
    strcpy(Settings.Name, Line + 5);
    break;
  }

  case cmd_Password:
  {
    success = true;
    strcpy(SecuritySettings.Password, Line + 9);
    break;
  }

  case cmd_Reboot:
  {
    success = true;
    pinMode(0, INPUT);
    pinMode(2, INPUT);
    pinMode(15, INPUT);
    #if defined(ESP8266)
      ESP.reset();
    #endif
    #if defined(ESP32)
      ESP.restart();
    #endif
    break;
  }

  case cmd_Restart:
  {
    success = true;
    ESP.restart();
    break;
  }

  case cmd_Erase:
  {
    success = true;
    WiFi.persistent(true); // use SDK storage of SSID/WPA parameters
    WifiDisconnect(); // this will store empty ssid/wpa into sdk storage
    WiFi.persistent(false); // Do not use SDK storage of SSID/WPA parameters
    break;
  }

  case cmd_Reset:
  {
    success = true;
    ResetFactory();
    #if defined(ESP8266)
      ESP.reset();
    #endif
    #if defined(ESP32)
      ESP.restart();
    #endif
    break;
  }

  case cmd_Save:
  {
    success = true;
    SaveSettings();
    break;
  }

  case cmd_Load:
  {
    success = true;
    LoadSettings();
    break;
  }

  case cmd_Debug:
  {
    success = true;
    Settings.SerialLogLevel = Par1;
    break;
  }

  case cmd_IP:
  {
    success = true;
    if (GetArgv(Line, TmpStr1, 2)) {
      if (!str2ip(TmpStr1, Settings.IP))
        Serial.println("?");
    }
    break;
  }

  case cmd_Settings:
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
    break;
  }
  default:
    success = false;
  }

  yield();

  if (success)
    status += F("\nOk");
  else
    status += F("\nUnknown command!");
  SendStatus(source, status);
  yield();
}

#ifdef FEATURE_SD
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
#endif
