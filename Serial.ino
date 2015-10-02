/********************************************************************************************\
* Process data from Serial Interface
\*********************************************************************************************/

#define INPUT_COMMAND_SIZE          80
void ExecuteCommand(char *Line)
{
  char TmpStr1[80];
  TmpStr1[0] = 0;
  char Command[80];
  Command[0] = 0;
  int Par1 = 0;
  int Par2 = 0;

  GetArgv(Line, Command, 1);
  if (GetArgv(Line, TmpStr1, 2)) Par1 = str2int(TmpStr1);
  if (GetArgv(Line, TmpStr1, 3)) Par2 = str2int(TmpStr1);

  // ****************************************
  // commands for debugging
  // ****************************************

  if (strcasecmp_P(Command, PSTR("Pullup")) == 0)
  {
    Plugin_009_Config(18, 1);
  }

  #if FEATURE_SPIFFS
  if (strcasecmp_P(Command, PSTR("format")) == 0)
  {
    Serial.println(F("formatting..."));
    SPIFFS.format();
    Serial.println(F("format done!"));
  }
  #endif
    
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

  if (strcasecmp_P(Command, PSTR("DomoticzGet")) == 0)
  {
    float value = 0;
    if (Domoticz_getData(Par2, &value))
    {
      Serial.print(F("DomoticzGet "));
      Serial.println(value);
    }
    else
      Serial.println(F("Error getting data"));
  }

  if (strcasecmp_P(Command, PSTR("LCDWrite")) == 0)
  {
    GetArgv(Line, TmpStr1, 4);
    TmpStr1[25] = 0;
    for (byte x = 0; x < 25; x++)
      if (TmpStr1[x] == '_')
        TmpStr1[x] = ' ';
    lcd.setCursor(Par2 - 1, Par1 - 1);
    lcd.print(TmpStr1);
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
      pinMode(0,INPUT);
      pinMode(2,INPUT);
      pinMode(15,INPUT);
      ESP.reset();
    }
    
  if (strcasecmp_P(Command, PSTR("Restart")) == 0)
    ESP.restart();

  if (strcasecmp_P(Command, PSTR("Reset")) == 0)
    ResetFactory();

  if (strcasecmp_P(Command, PSTR("Save")) == 0)
    SaveSettings();

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

