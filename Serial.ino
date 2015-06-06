//********************************************************************************
// Serial Interface to configure and save settings to eeprom
//********************************************************************************

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

  if (strcasecmp(Command, "DomoticzSend") == 0)
  {
    if (GetArgv(Line, TmpStr1, 4))
      {
        UserVar[10 - 1] = atof(TmpStr1);
        Domoticz_sendData(Par1, Par2, 10);
      }
  }

  if (strcasecmp(Command, "DomoticzGet") == 0)
  {
    float value=0;
    if (Domoticz_getData(Par2, &value))
      {
        Serial.print("DomoticzGet ");
        Serial.println(value);
      }
    else
      Serial.println("Error getting data");
  }

  if (strcasecmp(Command, "UDP") == 0)
  {
    if (GetArgv(Line, TmpStr1, 2))
      {
        IPAddress broadcastIP(255,255,255,255);
        portTX.beginPacket(broadcastIP,Settings.UDPPort);
        portTX.write(TmpStr1);
        portTX.endPacket();
      }
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
    lcd(Par1, Par2, TmpStr1);
  }

  // ****************************************
  // configure settings commands:
  // ****************************************
  if (strcasecmp(Command, "Analog") == 0)
    Settings.Analog = Par1;

  if (strcasecmp(Command, "Dallas") == 0)
    Settings.Dallas = Par1;

  if (strcasecmp(Command, "DHT") == 0)
    Settings.DHT = Par1;

  if (strcasecmp(Command, "DHTType") == 0)
    Settings.DHTType = Par1;

  if (strcasecmp(Command, "BMP") == 0)
    Settings.BMP = Par1;

  if (strcasecmp(Command, "LUX") == 0)
    Settings.LUX = Par1;

  if (strcasecmp(Command, "RFID") == 0)
    Settings.RFID = Par1;

  if (strcasecmp(Command, "Unit") == 0)
    Settings.Unit = Par1;

  if (strcasecmp(Command, "Delay") == 0)
    Settings.Delay = Par1;

  if (strcasecmp(Command, "Pulse") == 0)
    Settings.Pulse1 = Par1;

  if (strcasecmp(Command, "ControllerIP") == 0)
  {
    if (GetArgv(Line, TmpStr1, 2))
      if (!str2ip(TmpStr1, Settings.Controller_IP))
        Serial.println("?");
  }

  if (strcasecmp(Command, "IP") == 0)
  {
    if (GetArgv(Line, TmpStr1, 2))
      Settings.IP_Octet = str2int(TmpStr1);
  }

  if (strcasecmp(Command, "ControllerPort") == 0)
  {
    if (GetArgv(Line, TmpStr1, 2))
      Settings.ControllerPort = str2int(TmpStr1);
  }

  if (strcasecmp(Command, "WifiSSID") == 0)
  {
    GetArgv(Line, TmpStr1, 2);
    TmpStr1[25] = 0;
    strcpy(Settings.WifiSSID, TmpStr1);
  }

  if (strcasecmp(Command, "WifiKey") == 0)
  {
    GetArgv(Line, TmpStr1, 2);
    TmpStr1[25] = 0;
    strcpy(Settings.WifiKey, TmpStr1);
  }

  if (strcasecmp(Command, "WifiAPKey") == 0)
  {
    GetArgv(Line, TmpStr1, 2);
    TmpStr1[25] = 0;
    strcpy(Settings.WifiAPKey, TmpStr1);
  }

  if (strcasecmp(Command, "WifiScan") == 0)
    WifiScan();

  if (strcasecmp(Command, "WifiConnect") == 0)
    WifiConnect();

  if (strcasecmp(Command, "WifiDisconnect") == 0)
    WifiDisconnect();

  if (strcasecmp(Command, "Reboot") == 0)
  {
    ESP.reset();
  }

  if (strcasecmp(Command, "Reset") == 0)
  {
    ResetFactory();
  }

  if (strcasecmp(Command, "Save") == 0)
  {
    Save_Settings();
  }

  if (strcasecmp(Command, "Settings") == 0)
  {
    char str[20];
    Serial.println();

    Serial.println("System Info");
    IPAddress ip = WiFi.localIP();
    sprintf(str, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
    Serial.print("  IP Address   : "); Serial.println(str);
    Serial.print("  Board Type   : "); Serial.println((int)Settings.BoardType);
    Serial.print("         SDA   : "); Serial.println((int)Settings.Pin_i2c_sda);
    Serial.print("         SCL   : "); Serial.println((int)Settings.Pin_i2c_scl);
    Serial.print("         In 1  : "); Serial.println((int)Settings.Pin_wired_in_1);
    Serial.print("         In 2  : "); Serial.println((int)Settings.Pin_wired_in_2);
    Serial.print("         Out 1 : "); Serial.println((int)Settings.Pin_wired_out_1);
    Serial.print("         Out 2 : "); Serial.println((int)Settings.Pin_wired_out_2);
    Serial.println();
    
    Serial.println("Generic settings");
    Serial.print("  Version          : "); Serial.println((int)Settings.Version);
    Serial.print("  Unit             : "); Serial.println((int)Settings.Unit);
    Serial.print("  WifiSSID         : "); Serial.println(Settings.WifiSSID);
    Serial.print("  WifiKey          : ");  Serial.println(Settings.WifiKey);
    sprintf(str, "%u.%u.%u.%u", Settings.Controller_IP[0], Settings.Controller_IP[1], Settings.Controller_IP[2], Settings.Controller_IP[3]);
    Serial.print("  ControllerIP     : "); Serial.println(str);
    Serial.print("  ControllerPort   : "); Serial.println(Settings.ControllerPort);
    Serial.print("  Fixed IP octet   : "); Serial.println(Settings.IP_Octet);
    Serial.print("  WifiKey (APmode) : ");  Serial.println(Settings.WifiAPKey);

    Serial.println();
    Serial.println("Device settings");
    Serial.print("  Delay   : "); Serial.println(Settings.Delay);
    Serial.print("  Dallas  : "); Serial.println(Settings.Dallas);
    Serial.print("  DHT     : "); Serial.println(Settings.DHT);
    Serial.print("  DHTType : "); Serial.println(Settings.DHTType);
    Serial.print("  BMP     : "); Serial.println(Settings.BMP);
    Serial.print("  LUX     : "); Serial.println(Settings.LUX);
    Serial.print("  RFID    : "); Serial.println(Settings.RFID);
    Serial.print("  Analog  : "); Serial.println(Settings.Analog);
    Serial.print("  Pulse   : "); Serial.println(Settings.Pulse1);
  }

  if (strcasecmp(Command, "Freemem") == 0)
  {
    Serial.println(FreeMem());
  }
}
#define INPUT_BUFFER_SIZE          128

byte SerialInByte;
int SerialInByteCounter = 0;
char InputBuffer_Serial[INPUT_BUFFER_SIZE + 2];

//********************************************************************************
void serial()
//********************************************************************************
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

//********************************************************************************
boolean GetArgv(char *string, char *argv, int argc)
//********************************************************************************
{
  int string_pos = 0, argv_pos = 0, argc_pos = 0;
  char c, d;

  while (string_pos < strlen(string))
  {
    c = string[string_pos];
    d = string[string_pos + 1];

    if       (c == ' ' && d == ' ') {}
    else if  (c == ' ' && d == ',') {}
    else if  (c == ',' && d == ' ') {}
    else if  (c == ' ' && d >= 33 && d <= 126) {}
    else if  (c == ',' && d >= 33 && d <= 126) {}
    else
    {
      argv[argv_pos++] = c;
      argv[argv_pos] = 0;

      if (d == ' ' || d == ',' || d == 0)
      {
        argv[argv_pos] = 0;
        argc_pos++;

        if (argc_pos == argc)
        {
          return true;
        }

        argv[0] = 0;
        argv_pos = 0;
        string_pos++;
      }
    }
    string_pos++;
  }
  return false;
}

//********************************************************************************
unsigned long str2int(char *string)
//********************************************************************************
{
  unsigned long temp = atof(string);
  return temp;
}
boolean str2ip(char *string, byte* IP)
{
  byte c;
  byte part = 0;
  int value = 0;

  for (int x = 0; x <= strlen(string); x++)
  {
    c = string[x];
    if (isdigit(c))
    {
      value *= 10;
      value += c - '0';
    }

    else if (c == '.' || c == 0) // volgende deel uit IP adres
    {
      if (value <= 255)
        IP[part++] = value;
      else
        return false;
      value = 0;
    }
    else if (c == ' ') // deze tekens negeren
      ;
    else // ongeldig teken
      return false;
  }
  if (part == 4) // correct aantal delen van het IP adres
    return true;
  return false;
}

//********************************************************************************
void Save_Settings(void)
//********************************************************************************
{
  char ByteToSave, *pointerToByteToSave = pointerToByteToSave = (char*)&Settings; //pointer to settings struct

  for (int x = 0; x < sizeof(struct SettingsStruct) ; x++)
  {
    EEPROM.write(x, *pointerToByteToSave);
    pointerToByteToSave++;
  }
  EEPROM.commit();
}

boolean LoadSettings()
{
  byte x;

  char ByteToSave, *pointerToByteToRead = (char*)&Settings; //pointer to settings struct

  for (int x = 0; x < sizeof(struct SettingsStruct); x++)
  {
    *pointerToByteToRead = EEPROM.read(x);
    pointerToByteToRead++;// next byte
  }
}

//********************************************************************************
void ResetFactory(void)
//********************************************************************************
{
  Serial.println("Reset!");
  Settings.PID             = ESP_PROJECT_PID;
  Settings.Version         = VERSION;
  Settings.Unit            = UNIT;
  strcpy(Settings.WifiSSID, DEFAULT_SSID);
  strcpy(Settings.WifiKey, DEFAULT_KEY);
  strcpy(Settings.WifiAPKey, DEFAULT_AP_KEY);
  str2ip((char*)DEFAULT_SERVER, Settings.Controller_IP);
  Settings.ControllerPort      = DEFAULT_PORT;
  Settings.IP_Octet        = 0;
  Settings.Delay           = DEFAULT_DELAY;
  Settings.Dallas          = DEFAULT_DALLAS_IDX;
  Settings.DHT             = DEFAULT_DHT_IDX;
  Settings.DHTType         = DEFAULT_DHT_TYPE;
  Settings.BMP             = DEFAULT_BMP_IDX;
  Settings.LUX             = DEFAULT_LUX_IDX;
  Settings.RFID            = DEFAULT_RFID_IDX;
  Settings.Analog          = DEFAULT_ANALOG_IDX;
  Settings.Pulse1          = DEFAULT_PULSE1_IDX;
  Settings.BoardType       = 0;
  Settings.Pin_i2c_sda     = 0;
  Settings.Pin_i2c_scl     = 2;
  Settings.Pin_wired_in_1  = 4;
  Settings.Pin_wired_in_2  = 5;
  Settings.Pin_wired_out_1 = 12;
  Settings.Pin_wired_out_2 = 13;
  Settings.Syslog_IP[0]    = 0;
  Settings.Syslog_IP[1]    = 0;
  Settings.Syslog_IP[2]    = 0;
  Settings.Syslog_IP[3]    = 0;
  Settings.UDPPort         = 0;
  Settings.Switch1         = DEFAULT_SWITCH1_IDX;
  Save_Settings();
  WifiDisconnect();
  ESP.reset();
}

extern "C" {
#include "user_interface.h"
}
//********************************************************************************
unsigned long FreeMem(void)
//********************************************************************************
{
  return system_get_free_heap_size();
}

