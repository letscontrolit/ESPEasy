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
  // commands to execute io tasks
  // ****************************************

  if (strcasecmp(Command, "GPIO") == 0)
  {
    if (Par1 >= 0 && Par1 <= 16)
    {
      pinMode(Par1, OUTPUT);
      digitalWrite(Par1, Par2);
      if (printToWeb)
      {
        printWebString += "GPIO ";
        printWebString += Par1;
        printWebString += " Set to ";
        printWebString += Par2;
        printWebString += "<BR>";
      }
    }
  }

  if (strcasecmp(Command, "PWM") == 0)
  {
    if (Par1 >= 0 && Par1 <= 1023)
    {
      pinMode(Par1, OUTPUT);
      analogWrite(Par1, Par2);
      if (printToWeb)
      {
        printWebString += "GPIO ";
        printWebString += Par1;
        printWebString += " Set PWM to ";
        printWebString += Par2;
        printWebString += "<BR>";
      }
    }
  }

  if (strcasecmp(Command, "ExtRead") == 0)
  {
    uint8_t address = 0x7f;
    Wire.requestFrom(address, (uint8_t)Par1);
    if (Wire.available())
    {
      for (byte x = 0; x < Par1; x++)
        Serial.println(Wire.read());
    }
  }

  if (strcasecmp(Command, "ExtGPIO") == 0)
    extender(1, Par1, Par2);
  if (strcasecmp(Command, "ExtPWM") == 0)
    extender(3, Par1, Par2);
  if (strcasecmp(Command, "ExtGPIORead") == 0)
  {
    byte value = extender(2, Par1, 0);
    Serial.println(value);
  }
  if (strcasecmp(Command, "ExtADCRead") == 0)
  {
    int value = extender(4, Par1, 0);
    Serial.println(value);
  }

  if (strcasecmp(Command, "DomoticzSend") == 0)
  {
    if (GetArgv(Line, TmpStr1, 4))
    {
      UserVar[(VARS_PER_TASK * TASKS_MAX) - 1] = atof(TmpStr1);
      sendData(0, Par1, Par2, VARS_PER_TASK * TASKS_MAX -1);
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

  if (strcasecmp(Command, "ExtWiredOut") == 0)
  {
    mcp23017(Par1, Par2);
  }

  if (strcasecmp(Command, "LCDWrite") == 0)
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
  // configure settings commands:
  // ****************************************
  if (strcasecmp(Command, "WifiSSID") == 0)
    strcpy(Settings.WifiSSID, Line+9);

  if (strcasecmp(Command, "WifiKey") == 0)
    strcpy(Settings.WifiKey, Line+8);

  if (strcasecmp(Command, "WifiScan") == 0)
    WifiScan();

  if (strcasecmp(Command, "WifiConnect") == 0)
    WifiConnect();

  if (strcasecmp(Command, "WifiDisconnect") == 0)
    WifiDisconnect();

  if (strcasecmp(Command, "Reboot") == 0)
    ESP.reset();

  if (strcasecmp(Command, "Reset") == 0)
    ResetFactory();

  if (strcasecmp(Command, "Save") == 0)
    Save_Settings();

  if (strcasecmp(Command, "Delay") == 0)
    Settings.Delay = Par1;

  if (strcasecmp(Command, "Debug") == 0)
    Settings.SerialLogLevel = Par1;

  if (strcasecmp(Command, "IP") == 0)
  {
    if (GetArgv(Line, TmpStr1, 2))
      if (!str2ip(TmpStr1, Settings.IP))
        Serial.println("?");
  }

  if (strcasecmp(Command, "Settings") == 0)
  {
    char str[20];
    Serial.println();

    Serial.println("System Info");
    IPAddress ip = WiFi.localIP();
    sprintf_P(str, PSTR("%u.%u.%u.%u"), ip[0], ip[1], ip[2], ip[3]);
    Serial.print("  IP Address    : "); Serial.println(str);
    Serial.print("  Build         : "); Serial.println((int)BUILD);
    Serial.print("  Unit          : "); Serial.println((int)Settings.Unit);
    Serial.print("  WifiSSID      : "); Serial.println(Settings.WifiSSID);
    Serial.print("  WifiKey       : "); Serial.println(Settings.WifiKey);
    Serial.print("  Settings size : "); Serial.println(sizeof(struct SettingsStruct));
    Serial.print("  Free mem      : "); Serial.println(FreeMem());
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

