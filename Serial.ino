//********************************************************************************
// Serial Interface to configure and save settings to eeprom
//********************************************************************************

#define INPUT_COMMAND_SIZE          80
void ExecuteCommand(char *Line)
{
  char TmpStr1[40];
  TmpStr1[0] = 0;
  char Command[40];
  Command[0] = 0;
  int Par1 = 0;
  int Par2 = 0;

  GetArgv(Line, Command, 1);
  if (GetArgv(Line, TmpStr1, 2)) Par1 = str2int(TmpStr1);
  if (GetArgv(Line, TmpStr1, 3)) Par2 = str2int(TmpStr1);

  // ****************************************
  // commands to execute io tasks
  // ****************************************

  if (strcasecmp(Command, "ExtWiredOut") == 0)
  {
    mcp23017(Par1, Par2);
  }

  if (strcasecmp(Command, "LCDWrite") == 0)
  {
    GetArgv(Line, TmpStr1, 4);
    TmpStr1[25] = 0;
    for (byte x=0; x < 25; x++)
      if (TmpStr1[x]=='_')
        TmpStr1[x]=' ';
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

  if (strcasecmp(Command, "ServerIP") == 0)
  {
    if (GetArgv(Line, TmpStr1, 2))
      if (!str2ip(TmpStr1, Settings.Server_IP))
        Serial.println("?");
  }

  if (strcasecmp(Command, "ServerPort") == 0)
  {
    if (GetArgv(Line, TmpStr1, 2))
      Settings.ServerPort = str2int(TmpStr1);
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

  if (strcasecmp(Command, "WifiScan") == 0)
  {
    Serial.println("scan start");
    int n = WiFi.scanNetworks();
    Serial.println("scan done");
    if (n == 0)
      Serial.println("no networks found");
    else
    {
      Serial.print(n);
      Serial.println(" networks found");
      for (int i = 0; i < n; ++i)
      {
        // Print SSID and RSSI for each network found
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.print(WiFi.SSID(i));
        Serial.print(" (");
        Serial.print(WiFi.RSSI(i));
        Serial.print(")");
        Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
        delay(10);
      }
    }
    Serial.println("");
  }

  if (strcasecmp(Command, "WifiConnect") == 0)
  {
    if (Settings.WifiSSID[0] != 0)
    {
      WiFi.mode(WIFI_STA);
      WiFi.begin(Settings.WifiSSID, Settings.WifiKey);
      while (WiFi.status() != WL_CONNECTED)
      {
        delay(100);
        Serial.print(".");
      }
      Serial.println("");
    }
    else
      Serial.println("No SSID!");
  }

  if (strcasecmp(Command, "WifiDisconnect") == 0)
  {
    WiFi.disconnect();
  }

  if (strcasecmp(Command, "Reboot") == 0)
  {
    Reboot();
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
    sprintf(str, "%u.%u.%u.%u", Settings.Server_IP[0], Settings.Server_IP[1], Settings.Server_IP[2], Settings.Server_IP[3]);
    Serial.println();
    Serial.println("Generic settings");
    Serial.print("  Version   :"); Serial.println((int)Settings.Version);
    Serial.print("  Unit      :"); Serial.println((int)Settings.Unit);
    Serial.print("  WifiSSID  :"); Serial.println(Settings.WifiSSID);
    Serial.print("  WifiKey   :");  Serial.println(Settings.WifiKey);
    Serial.print("  ServerIP  :"); Serial.println(str);
    Serial.print("  ServerPort:"); Serial.println(Settings.ServerPort);
    Serial.print("  Delay     :"); Serial.println(Settings.Delay);
    Serial.println();
    Serial.println("Device settings");
    Serial.print("  Dallas    :"); Serial.println(Settings.Dallas);
    Serial.print("  DHT       :"); Serial.println(Settings.DHT);
    Serial.print("  DHTType   :"); Serial.println(Settings.DHTType);
    Serial.print("  BMP       :"); Serial.println(Settings.BMP);
    Serial.print("  LUX       :"); Serial.println(Settings.LUX);
    Serial.print("  RFID      :"); Serial.println(Settings.RFID);
    Serial.print("  Analog    :"); Serial.println(Settings.Analog);
  }

  if (strcasecmp(Command, "Freemem") == 0)
  {
    Serial.println(FreeMem());
  }
}
#define INPUT_BUFFER_SIZE          128

//********************************************************************************
// workaround for strcasecmp, issue with lib of header files.??????
//********************************************************************************
int strcasecmp(const char * str1, const char * str2) {
  int d = 0;
  while (1) {
    int c1 = tolower(*str1++);
    int c2 = tolower(*str2++);
    if (((d = c1 - c2) != 0) || (c2 == '\0')) {
      break;
    }
  }
  return d;
}

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

  Settings.Version         = VERSION;
  Settings.Unit            = UNIT;
  strcpy(Settings.WifiSSID, DEFAULT_SSID);
  strcpy(Settings.WifiKey, DEFAULT_KEY);
  str2ip((char*)DEFAULT_SERVER, Settings.Server_IP);
  Settings.ServerPort      = DEFAULT_PORT;
  Settings.Delay           = DEFAULT_DELAY;
  Settings.Dallas          = DEFAULT_DALLAS_IDX;
  Settings.DHT             = DEFAULT_DHT_IDX;
  Settings.DHTType         = DEFAULT_DHT_TYPE;
  Settings.BMP             = DEFAULT_BMP_IDX;
  Settings.LUX             = DEFAULT_LUX_IDX;
  Settings.RFID            = DEFAULT_RFID_IDX;
  Settings.Analog          = DEFAULT_ANALOG_IDX;

  Save_Settings();
  Reboot();
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

