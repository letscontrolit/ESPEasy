/*********************************************************************************************\
   Get value count from sensor type
  \*********************************************************************************************/

byte getValueCountFromSensorType(byte sensorType)
{
  byte valueCount = 0;

  switch (sensorType)
  {
    case SENSOR_TYPE_SINGLE:                      // single value sensor, used for Dallas, BH1750, etc
    case SENSOR_TYPE_SWITCH:
    case SENSOR_TYPE_DIMMER:
      valueCount = 1;
      break;
    case SENSOR_TYPE_LONG:                      // single LONG value, stored in two floats (rfid tags)
      valueCount = 1;
      break;
    case SENSOR_TYPE_TEMP_HUM:
    case SENSOR_TYPE_TEMP_BARO:
    case SENSOR_TYPE_DUAL:
      valueCount = 2;
      break;
    case SENSOR_TYPE_TEMP_HUM_BARO:
    case SENSOR_TYPE_TRIPLE:
#ifdef PLUGIN_186
    case SENSOR_TYPE_WIND:
#endif
      valueCount = 3;
      break;
    case SENSOR_TYPE_QUAD:
      valueCount = 4;
      break;
  }
  return valueCount;
}


/*********************************************************************************************\
   Workaround for removing trailing white space when String() converts a float with 0 decimals
  \*********************************************************************************************/
String toString(float value, byte decimals)
{
  String sValue = String(value, decimals);
  sValue.trim();
  return sValue;
}

/*********************************************************************************************\
   Parse a string and get the xth command or parameter
  \*********************************************************************************************/
String parseString(String& string, byte indexFind)
{
  String tmpString = string;
  tmpString += ",";
  tmpString.replace(" ", ",");
  String locateString = "";
  byte count = 0;
  int index = tmpString.indexOf(',');
  while (index > 0)
  {
    count++;
    locateString = tmpString.substring(0, index);
    tmpString = tmpString.substring(index + 1);
    index = tmpString.indexOf(',');
    if (count == indexFind)
    {
      locateString.toLowerCase();
      return locateString;
    }
  }
  return "";
}


/*********************************************************************************************\
   Parse a string and get the xth command or parameter
  \*********************************************************************************************/
int getParamStartPos(String& string, byte indexFind)
{
  String tmpString = string;
  byte count = 0;
  tmpString.replace(" ", ",");
  for (int x = 0; x < tmpString.length(); x++)
  {
    if (tmpString.charAt(x) == ',')
    {
      count++;
      if (count == (indexFind - 1))
        return x + 1;
    }
  }
  return -1;
}


/*********************************************************************************************\
   set pin mode & state (info table)
  \*********************************************************************************************/
boolean setPinState(byte plugin, byte index, byte mode, uint16_t value)
{
  // plugin number and index form a unique key
  // first check if this pin is already known
  boolean reUse = false;
  for (byte x = 0; x < PINSTATE_TABLE_MAX; x++)
    if ((pinStates[x].plugin == plugin) && (pinStates[x].index == index))
    {
      pinStates[x].mode = mode;
      pinStates[x].value = value;
      reUse = true;
      break;
    }

  if (!reUse)
  {
    for (byte x = 0; x < PINSTATE_TABLE_MAX; x++)
      if (pinStates[x].plugin == 0)
      {
        pinStates[x].plugin = plugin;
        pinStates[x].index = index;
        pinStates[x].mode = mode;
        pinStates[x].value = value;
        break;
      }
  }
}


/*********************************************************************************************\
   get pin mode & state (info table)
  \*********************************************************************************************/
boolean getPinState(byte plugin, byte index, byte *mode, uint16_t *value)
{
  for (byte x = 0; x < PINSTATE_TABLE_MAX; x++)
    if ((pinStates[x].plugin == plugin) && (pinStates[x].index == index))
    {
      *mode = pinStates[x].mode;
      *value = pinStates[x].value;
      return true;
    }
  return false;
}


/*********************************************************************************************\
   check if pin mode & state is known (info table)
  \*********************************************************************************************/
boolean hasPinState(byte plugin, byte index)
{
  for (byte x = 0; x < PINSTATE_TABLE_MAX; x++)
    if ((pinStates[x].plugin == plugin) && (pinStates[x].index == index))
    {
      return true;
    }
  return false;
}


/*********************************************************************************************\
   report pin mode & state (info table) using json
  \*********************************************************************************************/
String getPinStateJSON(boolean search, byte plugin, byte index, String& log, uint16_t noSearchValue)
{
  printToWebJSON = true;
  byte mode = PIN_MODE_INPUT;
  uint16_t value = noSearchValue;
  String reply = "";
  boolean found = false;

  if (search)
  {
    for (byte x = 0; x < PINSTATE_TABLE_MAX; x++)
      if ((pinStates[x].plugin == plugin) && (pinStates[x].index == index))
      {
        mode = pinStates[x].mode;
        value = pinStates[x].value;
        found = true;
        break;
      }
  }

  if (!search || (search && found))
  {
    reply += F("{\n\"log\": \"");
    reply += log.substring(7, 32); // truncate to 25 chars, max MQTT message size = 128 including header...
    reply += F("\",\n\"plugin\": ");
    reply += plugin;
    reply += F(",\n\"pin\": ");
    reply += index;
    reply += F(",\n\"mode\": \"");
    switch (mode)
    {
      case PIN_MODE_UNDEFINED:
        reply += F("undefined");
        break;
      case PIN_MODE_INPUT:
        reply += F("input");
        break;
      case PIN_MODE_OUTPUT:
        reply += F("output");
        break;
      case PIN_MODE_PWM:
        reply += F("PWM");
        break;
      case PIN_MODE_SERVO:
        reply += F("servo");
        break;
    }
    reply += F("\",\n\"state\": ");
    reply += value;
    reply += F("\n}\n");
    return reply;
  }
  return "?";
}


/********************************************************************************************\
  Unsigned long Timer timeOut check
  \*********************************************************************************************/

boolean timeOut(unsigned long timer)
{
  // This routine solves the 49 day bug without the need for separate start time and duration
  //   that would need two 32 bit variables if duration is not static
  // It limits the maximum delay to 24.9 days.

  unsigned long now = millis();
  if (((now >= timer) && ((now - timer) < 1 << 31))  || ((timer >= now) && (timer - now > 1 << 31)))
    return true;

  return false;
}


/********************************************************************************************\
  Status LED
  \*********************************************************************************************/
void statusLED(boolean traffic)
{
  if (Settings.Pin_status_led == -1)
    return;

  static unsigned long timer = 0;
  static byte currentState = 0;

  if (traffic)
  {
    currentState = HIGH;
    digitalWrite(Settings.Pin_status_led, currentState); // blink off
    timer = millis() + 100;
  }

  if (timer == 0 || millis() > timer)
  {
    timer = 0;
    byte state = HIGH;
    if (WiFi.status() == WL_CONNECTED)
      state = LOW;

    if (currentState != state)
    {
      currentState = state;
      pinMode(Settings.Pin_status_led, OUTPUT);
      digitalWrite(Settings.Pin_status_led, state);
    }
  }
}


/********************************************************************************************\
  delay in milliseconds with background processing
  \*********************************************************************************************/
void delayMillis(unsigned long delay)
{
  unsigned long timer = millis() + delay;
  while (millis() < timer)
    backgroundtasks();
}


/********************************************************************************************\
  Parse a command string to event struct
  \*********************************************************************************************/
void parseCommandString(struct EventStruct *event, String& string)
{
  char command[80];
  command[0] = 0;
  char TmpStr1[80];
  TmpStr1[0] = 0;

  string.toCharArray(command, 80);
  event->Par1 = 0;
  event->Par2 = 0;
  event->Par3 = 0;

  if (GetArgv(command, TmpStr1, 2)) event->Par1 = str2int(TmpStr1);
  if (GetArgv(command, TmpStr1, 3)) event->Par2 = str2int(TmpStr1);
  if (GetArgv(command, TmpStr1, 4)) event->Par3 = str2int(TmpStr1);
}

/********************************************************************************************\
  Clear task settings for given task
  \*********************************************************************************************/
void taskClear(byte taskIndex, boolean save)
{
  Settings.TaskDeviceNumber[taskIndex] = 0;
  ExtraTaskSettings.TaskDeviceName[0] = 0;
  Settings.TaskDeviceID[taskIndex] = 0;
  Settings.TaskDeviceDataFeed[taskIndex] = 0;
  Settings.TaskDevicePin1[taskIndex] = -1;
  Settings.TaskDevicePin2[taskIndex] = -1;
  Settings.TaskDevicePin3[taskIndex] = -1;
  Settings.TaskDevicePort[taskIndex] = 0;
  Settings.TaskDeviceSendData[taskIndex] = true;
  Settings.TaskDeviceGlobalSync[taskIndex] = false;
  Settings.TaskDeviceTimer[taskIndex] = 0;

  for (byte x = 0; x < PLUGIN_CONFIGVAR_MAX; x++)
    Settings.TaskDevicePluginConfig[taskIndex][x] = 0;

  for (byte varNr = 0; varNr < VARS_PER_TASK; varNr++)
  {
    ExtraTaskSettings.TaskDeviceFormula[varNr][0] = 0;
    ExtraTaskSettings.TaskDeviceValueNames[varNr][0] = 0;
    ExtraTaskSettings.TaskDeviceValueDecimals[varNr] = 2;
  }

  for (byte varNr = 0; varNr < PLUGIN_EXTRACONFIGVAR_MAX; varNr++)
    ExtraTaskSettings.TaskDevicePluginConfigLong[varNr] = 0;

  if (save)
  {
    SaveTaskSettings(taskIndex);
    SaveSettings();
  }
}


/********************************************************************************************\
  Use DNS to resolve hostname to ip address
  \*********************************************************************************************/
void getIPfromHostName()
{
  IPAddress IP;
  if (Settings.ControllerHostName[0] != 0)
  {
    WiFi.hostByName(Settings.ControllerHostName, IP);
    for (byte x = 0; x < 4; x++)
      Settings.Controller_IP[x] = IP[x];
  }
}


/********************************************************************************************\
  Fix stuff to clear out differences between releases
  \*********************************************************************************************/
void BuildFixes()
{
  Serial.println(F("\nBuild changed!"));

  // fix default send data on active tasks, new to R52
  if (Settings.Build < 52)
  {
    Serial.println(F("Fix SendData"));
    for (byte x = 0; x < TASKS_MAX; x++)
    {
      Settings.TaskDeviceSendData[x] = true;
    }
  }

  if (Settings.Build < 64)
  {
    Serial.println(F("Fix Pin3"));
    for (byte x = 0; x < TASKS_MAX; x++)
    {
      Settings.TaskDevicePin3[x] = -1;
    }
  }

  if (Settings.Build < 79)
  {
    Serial.println(F("Fix status LED Pin"));
    Settings.Pin_status_led = -1;
    Settings.UseSerial = true;
    for (byte x = 0; x < TASKS_MAX; x++)
      Settings.TaskDeviceTimer[x] = Settings.Delay;
  }

  if (Settings.Build < 83)
  {
    Serial.println(F("Fix taskindex"));
    for (byte x = 0; x < TASKS_MAX; x++)
    {
      LoadTaskSettings(x);
      SaveTaskSettings(x);
    }
  }

  if (Settings.Build < 88)
  {
    struct ExtraTaskSettingsStruct_old
    {
      byte    TaskIndex;
      char    TaskDeviceName[26];
      char    TaskDeviceFormula[VARS_PER_TASK][41];
      char    TaskDeviceValueNames[VARS_PER_TASK][26];
      long    TaskDevicePluginConfigLong[PLUGIN_EXTRACONFIGVAR_MAX];
      byte    TaskDeviceValueDecimals[VARS_PER_TASK];
    } ExtraTaskSettings_old;

    Serial.println(F("Fix extratasksettings"));
    for (byte TaskIndex = 0; TaskIndex < TASKS_MAX; TaskIndex++)
    {
      LoadFromFlash(4096 + (TaskIndex * 1024), (byte*)&ExtraTaskSettings_old, sizeof(struct ExtraTaskSettingsStruct_old));

      ExtraTaskSettings.TaskIndex = ExtraTaskSettings_old.TaskIndex;
      strcpy(ExtraTaskSettings.TaskDeviceName, ExtraTaskSettings_old.TaskDeviceName);
      for (byte x = 0; x < VARS_PER_TASK; x++)
      {
        strcpy(ExtraTaskSettings.TaskDeviceFormula[x], ExtraTaskSettings_old.TaskDeviceFormula[x]);
        strcpy(ExtraTaskSettings.TaskDeviceValueNames[x], ExtraTaskSettings_old.TaskDeviceValueNames[x]);
        ExtraTaskSettings.TaskDeviceValueDecimals[x] = 2;
      }
      for (byte x = 0; x < PLUGIN_EXTRACONFIGVAR_MAX; x++)
      {
        ExtraTaskSettings.TaskDevicePluginConfigLong[x] = ExtraTaskSettings_old.TaskDevicePluginConfigLong[x];
      }
      SaveToFlash(4096 + (TaskIndex * 1024), (byte*)&ExtraTaskSettings, sizeof(struct ExtraTaskSettingsStruct));
    }
  }

  if (Settings.Build < 112)
  {
    Serial.println(F("Fix timezone"));
    Settings.TimeZone = Settings.TimeZone_OLD * 60;
  }

  Settings.Build = BUILD;
  SaveSettings();
}

#if FEATURE_SPIFFS
void fileSystemCheck()
{
  if (SPIFFS.begin())
  {
    String log = F("SPIFFS Mount successful");
    addLog(LOG_LEVEL_INFO, log);
    File f = SPIFFS.open("config.txt", "r");
    if (!f)
    {
      log = F("formatting...");
      addLog(LOG_LEVEL_INFO, log);
      SPIFFS.format();
      log = F("format done!");
      addLog(LOG_LEVEL_INFO, log);
      File f = SPIFFS.open("config.txt", "w");
      if (f)
      {
        for (int x = 0; x < 32768; x++)
          f.write(0);
        f.close();
      }
      f = SPIFFS.open("security.txt", "w");
      if (f)
      {
        for (int x = 0; x < 512; x++)
          f.write(0);
        f.close();
      }
      f = SPIFFS.open("rules.txt", "w");
      f.close();
    }
  }
  else
  {
    String log = F("SPIFFS Mount failed");
    addLog(LOG_LEVEL_INFO, log);
  }
}
#endif


/********************************************************************************************\
  Find device index corresponding to task number setting
  \*********************************************************************************************/
byte getDeviceIndex(byte Number)
{
  byte DeviceIndex = 0;
  for (byte x = 0; x <= deviceCount ; x++)
    if (Device[x].Number == Number)
      DeviceIndex = x;
  return DeviceIndex;
}


/********************************************************************************************\
  Find protocol index corresponding to protocol setting
  \*********************************************************************************************/
byte getProtocolIndex(byte Number)
{
  byte ProtocolIndex = 0;
  for (byte x = 0; x <= protocolCount ; x++)
    if (Protocol[x].Number == Number)
      ProtocolIndex = x;
  return ProtocolIndex;
}


/********************************************************************************************\
  Find positional parameter in a char string
  \*********************************************************************************************/
boolean GetArgv(const char *string, char *argv, int argc)
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


/********************************************************************************************\
  Convert a char string to integer
  \*********************************************************************************************/
unsigned long str2int(char *string)
{
  unsigned long temp = atof(string);
  return temp;
}


/********************************************************************************************\
  Convert a char string to IP byte array
  \*********************************************************************************************/
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

    else if (c == '.' || c == 0) // next octet from IP address
    {
      if (value <= 255)
        IP[part++] = value;
      else
        return false;
      value = 0;
    }
    else if (c == ' ') // ignore these
      ;
    else // invalid token
      return false;
  }
  if (part == 4) // correct number of octets
    return true;
  return false;
}


/********************************************************************************************\
  Save settings to SPIFFS
  \*********************************************************************************************/
void SaveSettings(void)
{
#if FEATURE_SPIFFS
  SaveToFile((char*)"config.txt", 0, (byte*)&Settings, sizeof(struct SettingsStruct));
  SaveToFile((char*)"security.txt", 0, (byte*)&SecuritySettings, sizeof(struct SecurityStruct));
#else
  SaveToFlash(0, (byte*)&Settings, sizeof(struct SettingsStruct));
  SaveToFlash(32768, (byte*)&SecuritySettings, sizeof(struct SecurityStruct));
#endif
}


/********************************************************************************************\
  Load settings from SPIFFS
  \*********************************************************************************************/
boolean LoadSettings()
{
#if FEATURE_SPIFFS
  LoadFromFile((char*)"config.txt", 0, (byte*)&Settings, sizeof(struct SettingsStruct));
  LoadFromFile((char*)"security.txt", 0, (byte*)&SecuritySettings, sizeof(struct SecurityStruct));
#else
  LoadFromFlash(0, (byte*)&Settings, sizeof(struct SettingsStruct));
  LoadFromFlash(32768, (byte*)&SecuritySettings, sizeof(struct SecurityStruct));
#endif
}


/********************************************************************************************\
  Save Task settings to SPIFFS
  \*********************************************************************************************/
void SaveTaskSettings(byte TaskIndex)
{
  ExtraTaskSettings.TaskIndex = TaskIndex;
#if FEATURE_SPIFFS
  SaveToFile((char*)"config.txt", 4096 + (TaskIndex * 1024), (byte*)&ExtraTaskSettings, sizeof(struct ExtraTaskSettingsStruct));
#else
  SaveToFlash(4096 + (TaskIndex * 1024), (byte*)&ExtraTaskSettings, sizeof(struct ExtraTaskSettingsStruct));
#endif
}


/********************************************************************************************\
  Load Task settings from SPIFFS
  \*********************************************************************************************/
void LoadTaskSettings(byte TaskIndex)
{
  if (ExtraTaskSettings.TaskIndex == TaskIndex)
    return;

#if FEATURE_SPIFFS
  LoadFromFile((char*)"config.txt", 4096 + (TaskIndex * 1024), (byte*)&ExtraTaskSettings, sizeof(struct ExtraTaskSettingsStruct));
#else
  LoadFromFlash(4096 + (TaskIndex * 1024), (byte*)&ExtraTaskSettings, sizeof(struct ExtraTaskSettingsStruct));
#endif
  ExtraTaskSettings.TaskIndex = TaskIndex; // Needed when an empty task was requested
}


/********************************************************************************************\
  Save Custom Task settings to SPIFFS
  \*********************************************************************************************/
void SaveCustomTaskSettings(int TaskIndex, byte* memAddress, int datasize)
{
  if (datasize > 512)
    return;
#if FEATURE_SPIFFS
  SaveToFile((char*)"config.txt", 4096 + (TaskIndex * 1024) + 512, memAddress, datasize);
#else
  SaveToFlash(4096 + (TaskIndex * 1024) + 512, memAddress, datasize);
#endif
}


/********************************************************************************************\
  Save Custom Task settings to SPIFFS
  \*********************************************************************************************/
void LoadCustomTaskSettings(int TaskIndex, byte* memAddress, int datasize)
{
  if (datasize > 512)
    return;
#if FEATURE_SPIFFS
  LoadFromFile((char*)"config.txt", 4096 + (TaskIndex * 1024) + 512, memAddress, datasize);
#else
  LoadFromFlash(4096 + (TaskIndex * 1024) + 512, memAddress, datasize);
#endif
}


/********************************************************************************************\
  Save Custom Controller settings to SPIFFS
  \*********************************************************************************************/
void SaveCustomControllerSettings(byte* memAddress, int datasize)
{
  if (datasize > 4096)
    return;
#if FEATURE_SPIFFS
  SaveToFile((char*)"config.txt", 28672, memAddress, datasize);
#else
  SaveToFlash(28672, memAddress, datasize);
#endif
}


/********************************************************************************************\
  Save Custom Controller settings to SPIFFS
  \*********************************************************************************************/
void LoadCustomControllerSettings(byte* memAddress, int datasize)
{
  if (datasize > 4096)
    return;
#if FEATURE_SPIFFS
  LoadFromFile((char*)"config.txt", 28672, memAddress, datasize);
#else
  LoadFromFlash(28672, memAddress, datasize);
#endif
}


#if FEATURE_SPIFFS
/********************************************************************************************\
  Save data into config file on SPIFFS
  \*********************************************************************************************/
void SaveToFile(char* fname, int index, byte* memAddress, int datasize)
{
  File f = SPIFFS.open(fname, "r+");
  if (f)
  {
    f.seek(index, SeekSet);
    byte *pointerToByteToSave = memAddress;
    for (int x = 0; x < datasize ; x++)
    {
      f.write(*pointerToByteToSave);
      pointerToByteToSave++;
    }
    f.close();
    String log = F("FILE : File saved");
    addLog(LOG_LEVEL_INFO, log);
  }
}


/********************************************************************************************\
  Load data from config file on SPIFFS
  \*********************************************************************************************/
void LoadFromFile(char* fname, int index, byte* memAddress, int datasize)
{
  File f = SPIFFS.open(fname, "r+");
  if (f)
  {
    f.seek(index, SeekSet);
    byte *pointerToByteToRead = memAddress;
    for (int x = 0; x < datasize; x++)
    {
      *pointerToByteToRead = f.read();
      pointerToByteToRead++;// next byte
    }
    f.close();
  }
}
#endif


/********************************************************************************************\
  Save data to flash
  \*********************************************************************************************/
void SaveToFlash(int index, byte* memAddress, int datasize)
{
  if (index > 33791) // Limit usable flash area to 32+1k size
  {
    return;
  }
  uint32_t _sector = ((uint32_t)&_SPIFFS_start - 0x40200000) / SPI_FLASH_SEC_SIZE;
  uint8_t* data = new uint8_t[FLASH_EEPROM_SIZE];
  int sectorOffset = index / SPI_FLASH_SEC_SIZE;
  int sectorIndex = index % SPI_FLASH_SEC_SIZE;
  uint8_t* dataIndex = data + sectorIndex;
  _sector += sectorOffset;

  // load entire sector from flash into memory
  noInterrupts();
  spi_flash_read(_sector * SPI_FLASH_SEC_SIZE, reinterpret_cast<uint32_t*>(data), FLASH_EEPROM_SIZE);
  interrupts();

  // store struct into this block
  memcpy(dataIndex, memAddress, datasize);

  noInterrupts();
  // write sector back to flash
  if (spi_flash_erase_sector(_sector) == SPI_FLASH_RESULT_OK)
    if (spi_flash_write(_sector * SPI_FLASH_SEC_SIZE, reinterpret_cast<uint32_t*>(data), FLASH_EEPROM_SIZE) == SPI_FLASH_RESULT_OK)
    {
      //Serial.println("flash save ok");
    }
  interrupts();
  delete [] data;
  String log = F("FLASH: Settings saved");
  addLog(LOG_LEVEL_INFO, log);
  flashWrites++;
}


/********************************************************************************************\
  Load data from flash
  \*********************************************************************************************/
void LoadFromFlash(int index, byte* memAddress, int datasize)
{
  uint32_t _sector = ((uint32_t)&_SPIFFS_start - 0x40200000) / SPI_FLASH_SEC_SIZE;
  uint8_t* data = new uint8_t[FLASH_EEPROM_SIZE];
  int sectorOffset = index / SPI_FLASH_SEC_SIZE;
  int sectorIndex = index % SPI_FLASH_SEC_SIZE;
  uint8_t* dataIndex = data + sectorIndex;
  _sector += sectorOffset;

  // load entire sector from flash into memory
  noInterrupts();
  spi_flash_read(_sector * SPI_FLASH_SEC_SIZE, reinterpret_cast<uint32_t*>(data), FLASH_EEPROM_SIZE);
  interrupts();

  // load struct from this block
  memcpy(memAddress, dataIndex, datasize);
  delete [] data;
}


/********************************************************************************************\
  Erase data on flash
  \*********************************************************************************************/
void ZeroFillFlash()
{
  // this will fill the SPIFFS area with a 64k block of all zeroes.
  uint32_t _sectorStart = ((uint32_t)&_SPIFFS_start - 0x40200000) / SPI_FLASH_SEC_SIZE;
  uint32_t _sectorEnd = _sectorStart + 16 ; //((uint32_t)&_SPIFFS_end - 0x40200000) / SPI_FLASH_SEC_SIZE;
  uint8_t* data = new uint8_t[FLASH_EEPROM_SIZE];

  uint8_t* tmpdata = data;
  for (int x = 0; x < FLASH_EEPROM_SIZE; x++)
  {
    *tmpdata = 0;
    tmpdata++;
  }


  for (uint32_t _sector = _sectorStart; _sector < _sectorEnd; _sector++)
  {
    // write sector to flash
    noInterrupts();
    if (spi_flash_erase_sector(_sector) == SPI_FLASH_RESULT_OK)
      if (spi_flash_write(_sector * SPI_FLASH_SEC_SIZE, reinterpret_cast<uint32_t*>(data), FLASH_EEPROM_SIZE) == SPI_FLASH_RESULT_OK)
      {
        interrupts();
        Serial.print(F("FLASH: Zero Fill Sector: "));
        Serial.println(_sector);
        delay(10);
      }
  }
  interrupts();
  delete [] data;
}


/********************************************************************************************\
  Erase all content on flash (except sketch)
  \*********************************************************************************************/
void EraseFlash()
{
  uint32_t _sectorStart = (ESP.getSketchSize() / SPI_FLASH_SEC_SIZE) + 1;
  uint32_t _sectorEnd = _sectorStart + (ESP.getFlashChipRealSize() / SPI_FLASH_SEC_SIZE);

  for (uint32_t _sector = _sectorStart; _sector < _sectorEnd; _sector++)
  {
    noInterrupts();
    if (spi_flash_erase_sector(_sector) == SPI_FLASH_RESULT_OK)
    {
      interrupts();
      Serial.print(F("FLASH: Erase Sector: "));
      Serial.println(_sector);
      delay(10);
    }
    interrupts();
  }
}


/********************************************************************************************\
  Check SPIFFS area settings
  \*********************************************************************************************/
int SpiffsSectors()
{
  uint32_t _sectorStart = ((uint32_t)&_SPIFFS_start - 0x40200000) / SPI_FLASH_SEC_SIZE;
  uint32_t _sectorEnd = ((uint32_t)&_SPIFFS_end - 0x40200000) / SPI_FLASH_SEC_SIZE;
  return _sectorEnd - _sectorStart;
}


/********************************************************************************************\
  Reset all settings to factory defaults
  \*********************************************************************************************/
void ResetFactory(void)
{
  // Direct Serial is allowed here, since this is only an emergency task.

  byte bootCount = 0;
  if (readFromRTC(&bootCount))
  {
    Serial.print(F("RESET: Reboot count: "));
    Serial.println(bootCount);
    if (bootCount > 3)
    {
      Serial.println(F("RESET: To many reset attempts"));
      return;
    }
  }
  else
    Serial.println(F("RESET: Cold boot"));

  bootCount++;
  saveToRTC(bootCount);

#if FEATURE_SPIFFS
  File f = SPIFFS.open("config.txt", "w");
  if (f)
  {
    for (int x = 0; x < 32768; x++)
      f.write(0);
    f.close();

  }
  f = SPIFFS.open("security.txt", "w");
  if (f)
  {
    for (int x = 0; x < 512; x++)
      f.write(0);
    f.close();
  }
  f = SPIFFS.open("rules.txt", "w");
  f.close();
#else
  EraseFlash();
  ZeroFillFlash();
#endif

  LoadSettings();
  // now we set all parameters that need to be non-zero as default value

#if DEFAULT_USE_STATIC_IP
  str2ip((char*)DEFAULT_IP, Settings.IP);
  str2ip((char*)DEFAULT_DNS, Settings.DNS);
  str2ip((char*)DEFAULT_GW, Settings.Gateway);
  str2ip((char*)DEFAULT_SUBNET, Settings.Subnet);
#endif

#if DEFAULT_MQTT_TEMPLATE
  strcpy_P(Settings.MQTTsubscribe, PSTR(DEFAULT_MQTT_SUB));
  strcpy_P(Settings.MQTTpublish, PSTR(DEFAULT_MQTT_PUB));
#endif

  Settings.PID             = ESP_PROJECT_PID;
  Settings.Version         = VERSION;
  Settings.Unit            = UNIT;
  strcpy_P(SecuritySettings.WifiSSID, PSTR(DEFAULT_SSID));
  strcpy_P(SecuritySettings.WifiKey, PSTR(DEFAULT_KEY));
  strcpy_P(SecuritySettings.WifiAPKey, PSTR(DEFAULT_AP_KEY));
  SecuritySettings.Password[0] = 0;
  str2ip((char*)DEFAULT_SERVER, Settings.Controller_IP);
  Settings.ControllerPort      = DEFAULT_PORT;
  Settings.Delay           = DEFAULT_DELAY;
  Settings.Pin_i2c_sda     = 4;
  Settings.Pin_i2c_scl     = 5;
  Settings.Pin_status_led  = -1;
  Settings.Protocol        = DEFAULT_PROTOCOL;
  strcpy_P(Settings.Name, PSTR(DEFAULT_NAME));
  Settings.SerialLogLevel  = 2;
  Settings.WebLogLevel     = 2;
  Settings.BaudRate        = 115200;
  Settings.MessageDelay = 1000;
  Settings.deepSleep = false;
  Settings.CustomCSS = false;
  Settings.InitSPI = false;
  for (byte x = 0; x < TASKS_MAX; x++)
  {
    Settings.TaskDevicePin1[x] = -1;
    Settings.TaskDevicePin2[x] = -1;
    Settings.TaskDevicePin3[x] = -1;
    Settings.TaskDevicePin1PullUp[x] = true;
    Settings.TaskDevicePin1Inversed[x] = false;
    Settings.TaskDeviceSendData[x] = true;
    Settings.TaskDeviceTimer[x] = Settings.Delay;
  }
  Settings.Build = BUILD;
  Settings.UseSerial = true;
  SaveSettings();
  delay(1000);
  WiFi.persistent(true); // use SDK storage of SSID/WPA parameters
  WiFi.disconnect(); // this will store empty ssid/wpa into sdk storage
  WiFi.persistent(false); // Do not use SDK storage of SSID/WPA parameters
  ESP.reset();
}


/********************************************************************************************\
  If RX and TX tied together, perform emergency reset to get the system out of boot loops
  \*********************************************************************************************/

void emergencyReset()
{
  // Direct Serial is allowed here, since this is only an emergency task.
  Serial.begin(115200);
  Serial.write(0xAA);
  Serial.write(0x55);
  delay(1);
  if (Serial.available() == 2)
    if (Serial.read() == 0xAA && Serial.read() == 0x55)
    {
      Serial.println(F("System will reset in 10 seconds..."));
      delay(10000);
      ResetFactory();
    }
}


/********************************************************************************************\
  Get free system mem
  \*********************************************************************************************/
unsigned long FreeMem(void)
{
  return system_get_free_heap_size();
}


/********************************************************************************************\
  In memory convert float to long
  \*********************************************************************************************/
unsigned long float2ul(float f)
{
  unsigned long ul;
  memcpy(&ul, &f, 4);
  return ul;
}


/********************************************************************************************\
  In memory convert long to float
  \*********************************************************************************************/
float ul2float(unsigned long ul)
{
  float f;
  memcpy(&f, &ul, 4);
  return f;
}


/********************************************************************************************\
  Add to log
  \*********************************************************************************************/
void addLog(byte loglevel, String& string)
{
  addLog(loglevel, string.c_str());
}

void addLog(byte loglevel, const char *line)
{
  if (Settings.UseSerial)
    if (loglevel <= Settings.SerialLogLevel)
      Serial.println(line);

  if (loglevel <= Settings.SyslogLevel)
    syslog(line);

  if (loglevel <= Settings.WebLogLevel)
  {
    logcount++;
    if (logcount > 9)
      logcount = 0;
    Logging[logcount].timeStamp = millis();
    Logging[logcount].Message = line;
  }
}


/********************************************************************************************\
  Delayed reboot, in case of issues, do not reboot with high frequency as it might not help...
  \*********************************************************************************************/
void delayedReboot(int rebootDelay)
{
  // Direct Serial is allowed here, since this is only an emergency task.
  while (rebootDelay != 0 )
  {
    Serial.print(F("Delayed Reset "));
    Serial.println(rebootDelay);
    rebootDelay--;
    delay(1000);
  }
  ESP.reset();
}


/********************************************************************************************\
  Save a byte to RTC memory
  \*********************************************************************************************/
#define RTC_BASE 65 // system doc says user area starts at 64, but it does not work (?)
void saveToRTC(byte Par1)
{
  byte buf[3] = {0xAA, 0x55, 0};
  buf[2] = Par1;
  system_rtc_mem_write(RTC_BASE, buf, 3);
}


/********************************************************************************************\
  Read a byte from RTC memory
  \*********************************************************************************************/
boolean readFromRTC(byte* data)
{
  byte buf[3] = {0, 0, 0};
  system_rtc_mem_read(RTC_BASE, buf, 3);
  if (buf[0] == 0xAA && buf[1] == 0x55)
  {
    *data = buf[2];
    return true;
  }
  return false;
}


/********************************************************************************************\
  Convert a string like "Sun,12:30" into a 32 bit integer
  \*********************************************************************************************/
unsigned long string2TimeLong(String &str)
{
  // format 0000WWWWAAAABBBBCCCCDDDD
  // WWWW=weekday, AAAA=hours tens digit, BBBB=hours, CCCC=minutes tens digit DDDD=minutes

  char command[20];
  char TmpStr1[10];
  int w, x, y;
  unsigned long a;
  str.toLowerCase();
  str.toCharArray(command, 20);
  unsigned long lngTime = 0;

  if (GetArgv(command, TmpStr1, 1))
  {
    String day = TmpStr1;
    String weekDays = F("allsunmontuewedthufrisat");
    y = weekDays.indexOf(TmpStr1) / 3;
    if (y == 0)
      y = 0xf; // wildcard is 0xf
    lngTime |= (unsigned long)y << 16;
  }

  if (GetArgv(command, TmpStr1, 2))
  {
    y = 0;
    for (x = strlen(TmpStr1) - 1; x >= 0; x--)
    {
      w = TmpStr1[x];
      if (w >= '0' && w <= '9' || w == '*')
      {
        a = 0xffffffff  ^ (0xfUL << y); // create mask to clean nibble position y
        lngTime &= a; // maak nibble leeg
        if (w == '*')
          lngTime |= (0xFUL << y); // fill nibble with wildcard value
        else
          lngTime |= (w - '0') << y; // fill nibble with token
        y += 4;
      }
      else if (w == ':');
      else
      {
        break;
      }
    }
  }
  return lngTime;
}


/********************************************************************************************\
  Convert  a 32 bit integer into a string like "Sun,12:30"
  \*********************************************************************************************/
String timeLong2String(unsigned long lngTime)
{
  unsigned long x = 0;
  String time = "";

  x = (lngTime >> 16) & 0xf;
  if (x == 0x0f)
    x = 0;
  String weekDays = F("AllSunMonTueWedThuFriSat");
  time = weekDays.substring(x * 3, x * 3 + 3);
  time += ",";

  x = (lngTime >> 12) & 0xf;
  if (x == 0xf)
    time += "*";
  else if (x == 0xe)
    time += "-";
  else
    time += x;

  x = (lngTime >> 8) & 0xf;
  if (x == 0xf)
    time += "*";
  else if (x == 0xe)
    time += "-";
  else
    time += x;

  time += ":";

  x = (lngTime >> 4) & 0xf;
  if (x == 0xf)
    time += "*";
  else if (x == 0xe)
    time += "-";
  else
    time += x;

  x = (lngTime) & 0xf;
  if (x == 0xf)
    time += "*";
  else if (x == 0xe)
    time += "-";
  else
    time += x;

  return time;
}


String parseTemplate(String &tmpString, byte lineSize)
{
  String newString = "";
  String tmpStringMid = "";

  // replace task template variables
  int leftBracketIndex = tmpString.indexOf('[');
  if (leftBracketIndex == -1)
    newString = tmpString;
  else
  {
    byte count = 0;
    byte currentTaskIndex = ExtraTaskSettings.TaskIndex;
    while (leftBracketIndex >= 0 && count < 10 - 1)
    {
      newString += tmpString.substring(0, leftBracketIndex);
      tmpString = tmpString.substring(leftBracketIndex + 1);
      int rightBracketIndex = tmpString.indexOf(']');
      if (rightBracketIndex)
      {
        tmpStringMid = tmpString.substring(0, rightBracketIndex);
        tmpString = tmpString.substring(rightBracketIndex + 1);
        int hashtagIndex = tmpStringMid.indexOf('#');
        String deviceName = tmpStringMid.substring(0, hashtagIndex);
        String valueName = tmpStringMid.substring(hashtagIndex + 1);
        String valueFormat = "";
        hashtagIndex = valueName.indexOf('#');
        if (hashtagIndex >= 0)
        {
          valueFormat = valueName.substring(hashtagIndex + 1);
          valueName = valueName.substring(0, hashtagIndex);
        }
        for (byte y = 0; y < TASKS_MAX; y++)
        {
          LoadTaskSettings(y);
          if (ExtraTaskSettings.TaskDeviceName[0] != 0)
          {
            if (deviceName.equalsIgnoreCase(ExtraTaskSettings.TaskDeviceName))
            {
              for (byte z = 0; z < VARS_PER_TASK; z++)
                if (valueName.equalsIgnoreCase(ExtraTaskSettings.TaskDeviceValueNames[z]))
                {
                  // here we know the task and value, so find the uservar
                  String value = "";
                  byte DeviceIndex = getDeviceIndex(Settings.TaskDeviceNumber[y]);
                  if (Device[DeviceIndex].VType == SENSOR_TYPE_LONG)
                    value = (unsigned long)UserVar[y * VARS_PER_TASK + z] + ((unsigned long)UserVar[y * VARS_PER_TASK + z + 1] << 16);
                  else
                    value = toString(UserVar[y * VARS_PER_TASK + z], ExtraTaskSettings.TaskDeviceValueDecimals[z]);

                  if (valueFormat == "R")
                  {
                    int filler = lineSize - newString.length() - value.length() - tmpString.length() ;
                    for (byte f = 0; f < filler; f++)
                      newString += " ";
                  }
                  newString += String(value);
                  break;
                }
              break;
            }
          }
        }
      }
      leftBracketIndex = tmpString.indexOf('[');
      count++;
    }
    newString += tmpString;
    LoadTaskSettings(currentTaskIndex);
  }

  // replace other system variables like %sysname%, %systime%, %ip%
  newString.replace("%sysname%", Settings.Name);

#if FEATURE_TIME
  String strTime = "";
  if (hour() < 10)
    strTime += " ";
  strTime += hour();
  strTime += ":";
  if (minute() < 10)
    strTime += "0";
  strTime += minute();
  newString.replace("%systime%", strTime);
#endif

  newString.replace("%uptime%", String(wdcounter / 2));

#if FEATURE_ADC_VCC
  newString.replace("%vcc%", String(vcc));
#endif

  IPAddress ip = WiFi.localIP();
  char strIP[20];
  sprintf_P(strIP, PSTR("%u.%u.%u.%u"), ip[0], ip[1], ip[2], ip[3]);
  newString.replace("%ip%", strIP);

  newString.replace("%sysload%", String(100 - (100 * loopCounterLast / loopCounterMax)));

  // padding spaces
  while (newString.length() < lineSize)
    newString += " ";

  return newString;
}


/********************************************************************************************\
  Calculate function for simple expressions
  \*********************************************************************************************/
#define CALCULATE_OK                            0
#define CALCULATE_ERROR_STACK_OVERFLOW          1
#define CALCULATE_ERROR_BAD_OPERATOR            2
#define CALCULATE_ERROR_PARENTHESES_MISMATCHED  3
#define CALCULATE_ERROR_UNKNOWN_TOKEN           4
#define STACK_SIZE 10 // was 50
#define TOKEN_MAX 20

float globalstack[STACK_SIZE];
float *sp = globalstack - 1;
float *sp_max = &globalstack[STACK_SIZE - 1];

#define is_operator(c)  (c == '+' || c == '-' || c == '*' || c == '/' || c == '^')

int push(float value)
{
  if (sp != sp_max) // Full
  {
    *(++sp) = value;
    return 0;
  }
  else
    return CALCULATE_ERROR_STACK_OVERFLOW;
}

float pop()
{
  if (sp != (globalstack - 1)) // empty
    return *(sp--);
}

float apply_operator(char op, float first, float second)
{
  switch (op)
  {
    case '+':
      return first + second;
    case '-':
      return first - second;
    case '*':
      return first * second;
    case '/':
      return first / second;
    case '^':
      return pow(first, second);
    default:
      return 0;
  }
}

char *next_token(char *linep)
{
  while (isspace(*(linep++)));
  while (*linep && !isspace(*(linep++)));
  return linep;
}

int RPNCalculate(char* token)
{
  if (token[0] == 0)
    return 0; // geen moeite doen voor een lege string

  if (is_operator(token[0]))
  {
    float second = pop();
    float first = pop();

    if (push(apply_operator(token[0], first, second)))
      return CALCULATE_ERROR_STACK_OVERFLOW;
  }
  else // Als er nog een is, dan deze ophalen
    if (push(atof(token))) // is het een waarde, dan op de stack plaatsen
      return CALCULATE_ERROR_STACK_OVERFLOW;

  return 0;
}

// operators
// precedence   operators         associativity
// 3            !                 right to left
// 2            * / %             left to right
// 1            + - ^             left to right
int op_preced(const char c)
{
  switch (c)
  {
    case '^':
      return 3;
    case '*':
    case '/':
      return 2;
    case '+':
    case '-':
      return 1;
  }
  return 0;
}

bool op_left_assoc(const char c)
{
  switch (c)
  {
    case '^':
    case '*':
    case '/':
    case '+':
    case '-':
      return true;     // left to right
      //case '!': return false;    // right to left
  }
  return false;
}

unsigned int op_arg_count(const char c)
{
  switch (c)
  {
    case '^':
    case '*':
    case '/':
    case '+':
    case '-':
      return 2;
      //case '!': return 1;
  }
  return 0;
}


int Calculate(const char *input, float* result)
{
  const char *strpos = input, *strend = input + strlen(input);
  char token[25];
  char c, *TokenPos = token;
  char stack[32];       // operator stack
  unsigned int sl = 0;  // stack length
  char     sc;          // used for record stack element
  int error = 0;

  //*sp=0; // bug, it stops calculating after 50 times
  sp = globalstack - 1;

  while (strpos < strend)
  {
    // read one token from the input stream
    c = *strpos;
    if (c != ' ')
    {
      // If the token is a number (identifier), then add it to the token queue.
      if ((c >= '0' && c <= '9') || c == '.')
      {
        *TokenPos = c;
        ++TokenPos;
      }

      // If the token is an operator, op1, then:
      else if (is_operator(c))
      {
        *(TokenPos) = 0;
        error = RPNCalculate(token);
        TokenPos = token;
        if (error)return error;
        while (sl > 0)
        {
          sc = stack[sl - 1];
          // While there is an operator token, op2, at the top of the stack
          // op1 is left-associative and its precedence is less than or equal to that of op2,
          // or op1 has precedence less than that of op2,
          // The differing operator priority decides pop / push
          // If 2 operators have equal priority then associativity decides.
          if (is_operator(sc) && ((op_left_assoc(c) && (op_preced(c) <= op_preced(sc))) || (op_preced(c) < op_preced(sc))))
          {
            // Pop op2 off the stack, onto the token queue;
            *TokenPos = sc;
            ++TokenPos;
            *(TokenPos) = 0;
            error = RPNCalculate(token);
            TokenPos = token;
            if (error)return error;
            sl--;
          }
          else
            break;
        }
        // push op1 onto the stack.
        stack[sl] = c;
        ++sl;
      }
      // If the token is a left parenthesis, then push it onto the stack.
      else if (c == '(')
      {
        stack[sl] = c;
        ++sl;
      }
      // If the token is a right parenthesis:
      else if (c == ')')
      {
        bool pe = false;
        // Until the token at the top of the stack is a left parenthesis,
        // pop operators off the stack onto the token queue
        while (sl > 0)
        {
          *(TokenPos) = 0;
          error = RPNCalculate(token);
          TokenPos = token;
          if (error)return error;
          sc = stack[sl - 1];
          if (sc == '(')
          {
            pe = true;
            break;
          }
          else
          {
            *TokenPos = sc;
            ++TokenPos;
            sl--;
          }
        }
        // If the stack runs out without finding a left parenthesis, then there are mismatched parentheses.
        if (!pe)
          return CALCULATE_ERROR_PARENTHESES_MISMATCHED;

        // Pop the left parenthesis from the stack, but not onto the token queue.
        sl--;

        // If the token at the top of the stack is a function token, pop it onto the token queue.
        if (sl > 0)
          sc = stack[sl - 1];

      }
      else
        return CALCULATE_ERROR_UNKNOWN_TOKEN;
    }
    ++strpos;
  }
  // When there are no more tokens to read:
  // While there are still operator tokens in the stack:
  while (sl > 0)
  {
    sc = stack[sl - 1];
    if (sc == '(' || sc == ')')
      return CALCULATE_ERROR_PARENTHESES_MISMATCHED;

    *(TokenPos) = 0;
    error = RPNCalculate(token);
    TokenPos = token;
    if (error)return error;
    *TokenPos = sc;
    ++TokenPos;
    --sl;
  }

  *(TokenPos) = 0;
  error = RPNCalculate(token);
  TokenPos = token;
  if (error)
  {
    *result = 0;
    return error;
  }
  *result = *sp;
  return CALCULATE_OK;
}


/********************************************************************************************\
  Time stuff
  \*********************************************************************************************/
#if FEATURE_TIME

#define SECS_PER_MIN  (60UL)
#define SECS_PER_HOUR (3600UL)
#define SECS_PER_DAY  (SECS_PER_HOUR * 24UL)
#define DAYS_PER_WEEK (7UL)
#define SECS_PER_WEEK (SECS_PER_DAY * DAYS_PER_WEEK)
#define SECS_PER_YEAR (SECS_PER_WEEK * 52UL)
#define SECS_YR_2000  (946684800UL) // the time at the start of y2k
#define LEAP_YEAR(Y)     ( ((1970+Y)>0) && !((1970+Y)%4) && ( ((1970+Y)%100) || !((1970+Y)%400) ) )

struct  timeStruct {
  uint8_t Second;
  uint8_t Minute;
  uint8_t Hour;
  uint8_t Wday;   // day of week, sunday is day 1
  uint8_t Day;
  uint8_t Month;
  uint8_t Year;   // offset from 1970;
} tm;

uint32_t syncInterval = 3600;  // time sync will be attempted after this many seconds
uint32_t sysTime = 0;
uint32_t prevMillis = 0;
uint32_t nextSyncTime = 0;

byte PrevMinutes = 0;

void breakTime(unsigned long timeInput, struct timeStruct &tm) {
  uint8_t year;
  uint8_t month, monthLength;
  uint32_t time;
  unsigned long days;
  const uint8_t monthDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

  time = (uint32_t)timeInput;
  tm.Second = time % 60;
  time /= 60; // now it is minutes
  tm.Minute = time % 60;
  time /= 60; // now it is hours
  tm.Hour = time % 24;
  time /= 24; // now it is days
  tm.Wday = ((time + 4) % 7) + 1;  // Sunday is day 1

  year = 0;
  days = 0;
  while ((unsigned)(days += (LEAP_YEAR(year) ? 366 : 365)) <= time) {
    year++;
  }
  tm.Year = year; // year is offset from 1970

  days -= LEAP_YEAR(year) ? 366 : 365;
  time  -= days; // now it is days in this year, starting at 0

  days = 0;
  month = 0;
  monthLength = 0;
  for (month = 0; month < 12; month++) {
    if (month == 1) { // february
      if (LEAP_YEAR(year)) {
        monthLength = 29;
      } else {
        monthLength = 28;
      }
    } else {
      monthLength = monthDays[month];
    }

    if (time >= monthLength) {
      time -= monthLength;
    } else {
      break;
    }
  }
  tm.Month = month + 1;  // jan is month 1
  tm.Day = time + 1;     // day of month
}

void setTime(unsigned long t) {
  sysTime = (uint32_t)t;
  nextSyncTime = (uint32_t)t + syncInterval;
  prevMillis = millis();  // restart counting from now (thanks to Korman for this fix)
}

unsigned long now() {
  // calculate number of seconds passed since last call to now()
  while (millis() - prevMillis >= 1000) {
    // millis() and prevMillis are both unsigned ints thus the subtraction will always be the absolute value of the difference
    sysTime++;
    prevMillis += 1000;
  }
  if (nextSyncTime <= sysTime) {
    unsigned long  t = getNtpTime();
    if (t != 0) {
      if (Settings.DST)
        t += SECS_PER_HOUR; // add one hour if DST active
      setTime(t);
    } else {
      nextSyncTime = sysTime + syncInterval;
    }
  }
  breakTime(sysTime, tm);
  return (unsigned long)sysTime;
}

int hour()
{
  return tm.Hour;
}

int minute()
{
  return tm.Minute;
}

int weekday()
{
  return tm.Wday;
}

void initTime()
{
  nextSyncTime = 0;
  now();
}

void checkTime()
{
  now();
  if (tm.Minute != PrevMinutes)
  {
    PluginCall(PLUGIN_CLOCK_IN, 0, dummyString);
    PrevMinutes = tm.Minute;
    if (Settings.UseRules)
    {
      String weekDays = F("AllSunMonTueWedThuFriSat");
      String event = F("Clock#Time=");
      event += weekDays.substring(weekday() * 3, weekday() * 3 + 3);
      event += ",";
      if (hour() < 10)
        event += "0";
      event += hour();
      event += ":";
      if (minute() < 10)
        event += "0";
      event += minute();
      rulesProcessing(event);
    }
  }
}


unsigned long getNtpTime()
{
  WiFiUDP udp;
  udp.begin(123);
  for (byte x = 1; x < 4; x++)
  {
    String log = F("NTP  : NTP sync request:");
    log += x;
    addLog(LOG_LEVEL_DEBUG_MORE, log);

    const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
    byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

    IPAddress timeServerIP;
    const char* ntpServerName = "pool.ntp.org";

    if (Settings.NTPHost[0] != 0)
      WiFi.hostByName(Settings.NTPHost, timeServerIP);
    else
      WiFi.hostByName(ntpServerName, timeServerIP);

    char host[20];
    sprintf_P(host, PSTR("%u.%u.%u.%u"), timeServerIP[0], timeServerIP[1], timeServerIP[2], timeServerIP[3]);
    log = F("NTP  : NTP send to ");
    log += host;
    addLog(LOG_LEVEL_DEBUG_MORE, log);

    while (udp.parsePacket() > 0) ; // discard any previously received packets

    memset(packetBuffer, 0, NTP_PACKET_SIZE);
    packetBuffer[0] = 0b11100011;   // LI, Version, Mode
    packetBuffer[1] = 0;     // Stratum, or type of clock
    packetBuffer[2] = 6;     // Polling Interval
    packetBuffer[3] = 0xEC;  // Peer Clock Precision
    packetBuffer[12]  = 49;
    packetBuffer[13]  = 0x4E;
    packetBuffer[14]  = 49;
    packetBuffer[15]  = 52;
    udp.beginPacket(timeServerIP, 123); //NTP requests are to port 123
    udp.write(packetBuffer, NTP_PACKET_SIZE);
    udp.endPacket();

    uint32_t beginWait = millis();
    while (millis() - beginWait < 1000) {
      int size = udp.parsePacket();
      if (size >= NTP_PACKET_SIZE) {
        udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
        unsigned long secsSince1900;
        // convert four bytes starting at location 40 to a long integer
        secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
        secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
        secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
        secsSince1900 |= (unsigned long)packetBuffer[43];
        log = F("NTP  : NTP replied: ");
        log += millis() - beginWait;
        log += F(" mSec");
        addLog(LOG_LEVEL_DEBUG_MORE, log);
        return secsSince1900 - 2208988800UL + Settings.TimeZone * SECS_PER_MIN;
      }
    }
    log = F("NTP  : No reply");
    addLog(LOG_LEVEL_DEBUG_MORE, log);
  }
  return 0;
}
#endif


/********************************************************************************************\
  Rules processing
  \*********************************************************************************************/
void rulesProcessing(String& event)
{
  static uint8_t* data;
  static byte nestingLevel;

  String log = "";

  nestingLevel++;
  if (nestingLevel > RULES_MAX_NESTING_LEVEL)
  {
    log = F("EVENT: Error: Nesting level exceeded!");
    addLog(LOG_LEVEL_ERROR, log);
    nestingLevel--;
    return;
  }

  log = F("EVENT: ");
  log += event;
  addLog(LOG_LEVEL_INFO, log);

  // load rules from flash memory, stored in offset block 10
  if (data == NULL)
  {
    data = new uint8_t[RULES_MAX_SIZE];
#if FEATURE_SPIFFS
    File f = SPIFFS.open("rules.txt", "r+");
    if (f)
    {
      byte *pointerToByteToRead = data;
      for (int x = 0; x < f.size(); x++)
      {
        *pointerToByteToRead = f.read();
        pointerToByteToRead++;// next byte
      }
      data[f.size()] = 0;
      f.close();
    }
#else
    uint32_t _sector = ((uint32_t)&_SPIFFS_start - 0x40200000) / SPI_FLASH_SEC_SIZE;
    _sector += 10;
    noInterrupts();
    spi_flash_read(_sector * SPI_FLASH_SEC_SIZE, reinterpret_cast<uint32_t*>(data), RULES_MAX_SIZE);
    interrupts();
#endif
    data[RULES_MAX_SIZE - 1] = 0; // make sure it's terminated!
  }

  int pos = 0;
  String line = "";
  boolean match = false;
  boolean codeBlock = false;
  boolean isCommand = false;
  boolean conditional = false;
  boolean condition = false;
  boolean ifBranche = false;

  while (data[pos] != 0)
  {
    if (data[pos] != 0 && data[pos] != 10)
      line += (char)data[pos];

    if (data[pos] == 10)    // if line complete, parse this rule
    {
      line.replace("\r", "");
      if (line.substring(0, 2) != "//" && line.length() > 0)
      {
        isCommand = true;

        int comment = line.indexOf("//");
        if (comment > 0)
          line = line.substring(0, comment);

        line = parseTemplate(line, line.length());
        line.trim();

        String lineOrg = line; // store original line for future use
        line.toLowerCase(); // convert all to lower case to make checks easier

        String eventTrigger = "";
        String action = "";

        if (!codeBlock)  // do not check "on" rules if a block of actions is to be processed
        {
          if (line.startsWith("on "))
          {
            line = line.substring(3);
            int split = line.indexOf(" do");
            if (split != -1)
            {
              eventTrigger = line.substring(0, split);
              action = lineOrg.substring(split + 7);
              action.trim();
            }
            match = ruleMatch(event, eventTrigger);
            if (action.length() > 0) // single on/do/action line, no block
            {
              isCommand = true;
              codeBlock = false;
            }
            else
            {
              isCommand = false;
              codeBlock = true;
            }
          }
        }
        else
        {
          action = lineOrg;
        }

        String lcAction = action;
        lcAction.toLowerCase();
        if (lcAction == "endon") // Check if action block has ended, then we will wait for a new "on" rule
        {
          isCommand = false;
          codeBlock = false;
        }

        if (match) // rule matched for one action or a block of actions
        {
          int split = lcAction.indexOf("if "); // check for optional "if" condition
          if (split != -1)
          {
            conditional = true;
            String check = lcAction.substring(split + 3);
            condition = conditionMatch(check);
            ifBranche = true;
            isCommand = false;
          }

          if (lcAction == "else") // in case of an "else" block of actions, set ifBranche to false
          {
            ifBranche = false;
            isCommand = false;
          }

          if (lcAction == "endif") // conditional block ends here
          {
            conditional = false;
            isCommand = false;
          }

          // process the action if it's a command and unconditional, or conditional and the condition matches the if or else block.
          if (isCommand && ((!conditional) || (conditional && (condition == ifBranche))))
          {
            int equalsPos = event.indexOf("=");
            if (equalsPos > 0)
            {
              String tmpString = event.substring(equalsPos + 1);
              action.replace("%eventvalue%", tmpString); // substitute %eventvalue% in actions with the actual value from the event
            }
            log = F("ACT  : ");
            log += action;
            addLog(LOG_LEVEL_INFO, log);

            struct EventStruct TempEvent;
            parseCommandString(&TempEvent, action);
            yield();
            if (!PluginCall(PLUGIN_WRITE, &TempEvent, action))
              ExecuteCommand(VALUE_SOURCE_SYSTEM, action.c_str());
            yield();
          }
        }
      }

      line = "";
    }
    pos++;
  }

  nestingLevel--;
  if (nestingLevel == 0)
  {
    delete [] data;
    data = NULL;
  }
}


/********************************************************************************************\
  Check if an event matches to a given rule
  \*********************************************************************************************/
boolean ruleMatch(String& event, String& rule)
{
  boolean match = false;
  String tmpEvent = event;
  String tmpRule = rule;

  // Special handling of literal string events, they should start with '!'
  if (event.charAt(0) == '!')
  {
    if (event.equalsIgnoreCase(rule))
      return true;
    else
      return false;
  }

  if (event.startsWith("Clock#Time")) // clock events need different handling...
  {
    int pos1 = event.indexOf("=");
    int pos2 = rule.indexOf("=");
    if (pos1 > 0 && pos2 > 0)
    {
      tmpEvent = event.substring(0, pos1);
      tmpRule  = rule.substring(0, pos2);
      if (tmpRule.equalsIgnoreCase(tmpEvent)) // if this is a clock rule
      {
        tmpEvent = event.substring(pos1 + 1);
        tmpRule  = rule.substring(pos2 + 1);
        unsigned long clockEvent = string2TimeLong(tmpEvent);
        unsigned long clockSet = string2TimeLong(tmpRule);
        unsigned long Mask;
        for (byte y = 0; y < 8; y++)
        {
          if (((clockSet >> (y * 4)) & 0xf) == 0xf)  // if nibble y has the wildcard value 0xf
          {
            Mask = 0xffffffff  ^ (0xFUL << (y * 4)); // Mask to wipe nibble position y.
            clockEvent &= Mask;                      // clear nibble
            clockEvent |= (0xFUL << (y * 4));        // fill with wildcard value 0xf
          }
        }
        if (clockEvent == clockSet)
          return true;
        else
          return false;
      }
    }
  }


  // parse event into verb and value
  float value = 0;
  int pos = event.indexOf("=");
  if (pos)
  {
    tmpEvent = event.substring(pos + 1);
    value = tmpEvent.toFloat();
    tmpEvent = event.substring(0, pos);
  }

  // parse rule
  int comparePos = 0;
  char compare = ' ';
  comparePos = rule.indexOf(">");
  if (comparePos > 0)
  {
    compare = '>';
  }
  else
  {
    comparePos = rule.indexOf("<");
    if (comparePos > 0)
    {
      compare = '<';
    }
    else
    {
      comparePos = rule.indexOf("=");
      if (comparePos > 0)
      {
        compare = '=';
      }
    }
  }

  float ruleValue = 0;

  if (comparePos > 0)
  {
    tmpRule = rule.substring(comparePos + 1);
    ruleValue = tmpRule.toFloat();
    tmpRule = rule.substring(0, comparePos);
  }

  switch (compare)
  {
    case '>':
      if (tmpRule.equalsIgnoreCase(tmpEvent) && value > ruleValue)
        match = true;
      break;

    case '<':
      if (tmpRule.equalsIgnoreCase(tmpEvent) && value < ruleValue)
        match = true;
      break;

    case '=':
      if (tmpRule.equalsIgnoreCase(tmpEvent) && value == ruleValue)
        match = true;
      break;

    case ' ':
      if (tmpRule.equalsIgnoreCase(tmpEvent))
        match = true;
      break;
  }

  return match;
}


/********************************************************************************************\
  Check expression
  \*********************************************************************************************/
boolean conditionMatch(String& check)
{
  boolean match = false;

  int comparePos = 0;
  char compare = ' ';
  comparePos = check.indexOf(">");
  if (comparePos > 0)
  {
    compare = '>';
  }
  else
  {
    comparePos = check.indexOf("<");
    if (comparePos > 0)
    {
      compare = '<';
    }
    else
    {
      comparePos = check.indexOf("=");
      if (comparePos > 0)
      {
        compare = '=';
      }
    }
  }

  float Value1 = 0;
  float Value2 = 0;

  if (comparePos > 0)
  {
    String tmpCheck = check.substring(comparePos + 1);
    Value2 = tmpCheck.toFloat();
    tmpCheck = check.substring(0, comparePos);
    Value1 = tmpCheck.toFloat();
  }
  else
    return false;

  switch (compare)
  {
    case '>':
      if (Value1 > Value2)
        match = true;
      break;

    case '<':
      if (Value1 < Value2)
        match = true;
      break;

    case '=':
      if (Value1 == Value2)
        match = true;
      break;
  }
  return match;
}


/********************************************************************************************\
  Check rule timers
  \*********************************************************************************************/
void rulesTimers()
{
  for (byte x = 0; x < RULES_TIMER_MAX; x++)
  {
    if (RulesTimer[x] != 0L) // timer active?
    {
      if (RulesTimer[x] < millis()) // timer finished?
      {
        RulesTimer[x] = 0L; // turn off this timer
        String event = F("Rules#Timer=");
        event += x + 1;
        rulesProcessing(event);
      }
    }
  }
}


/********************************************************************************************\
  Generate rule events based on task refresh
  \*********************************************************************************************/

void createRuleEvents(byte TaskIndex)
{
  LoadTaskSettings(TaskIndex);
  byte BaseVarIndex = TaskIndex * VARS_PER_TASK;
  byte DeviceIndex = getDeviceIndex(Settings.TaskDeviceNumber[TaskIndex]);
  byte sensorType = Device[DeviceIndex].VType;
  for (byte varNr = 0; varNr < Device[DeviceIndex].ValueCount; varNr++)
  {
    String eventString = ExtraTaskSettings.TaskDeviceName;
    eventString += F("#");
    eventString += ExtraTaskSettings.TaskDeviceValueNames[varNr];
    eventString += F("=");

    if (sensorType == SENSOR_TYPE_LONG)
      eventString += (unsigned long)UserVar[BaseVarIndex] + ((unsigned long)UserVar[BaseVarIndex + 1] << 16);
    else
      eventString += UserVar[BaseVarIndex + varNr];

    rulesProcessing(eventString);
  }
}


#ifdef PLUGIN_BUILD_TESTING

#define isdigit(n) (n >= '0' && n <= '9')

/********************************************************************************************\
  Generate a tone of specified frequency on pin
  \*********************************************************************************************/
void tone(uint8_t _pin, unsigned int frequency, unsigned long duration) {
  analogWriteFreq(frequency);
  analogWrite(_pin,100);
  delay(duration);
  analogWrite(_pin,0);
}

/********************************************************************************************\
  Play RTTTL string on specified pin
  \*********************************************************************************************/
void play_rtttl(uint8_t _pin, char *p )
{
  #define OCTAVE_OFFSET 0
  // Absolutely no error checking in here

  int notes[] = { 0,
    262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494,
    523, 554, 587, 622, 659, 698, 740, 784, 831, 880, 932, 988,
    1047, 1109, 1175, 1245, 1319, 1397, 1480, 1568, 1661, 1760, 1865, 1976,
    2093, 2217, 2349, 2489, 2637, 2794, 2960, 3136, 3322, 3520, 3729, 3951
  };



  byte default_dur = 4;
  byte default_oct = 6;
  int bpm = 63;
  int num;
  long wholenote;
  long duration;
  byte note;
  byte scale;

  // format: d=N,o=N,b=NNN:
  // find the start (skip name, etc)

  while(*p != ':') p++;    // ignore name
  p++;                     // skip ':'

  // get default duration
  if(*p == 'd')
  {
    p++; p++;              // skip "d="
    num = 0;
    while(isdigit(*p))
    {
      num = (num * 10) + (*p++ - '0');
    }
    if(num > 0) default_dur = num;
    p++;                   // skip comma
  }

  // get default octave
  if(*p == 'o')
  {
    p++; p++;              // skip "o="
    num = *p++ - '0';
    if(num >= 3 && num <=7) default_oct = num;
    p++;                   // skip comma
  }

  // get BPM
  if(*p == 'b')
  {
    p++; p++;              // skip "b="
    num = 0;
    while(isdigit(*p))
    {
      num = (num * 10) + (*p++ - '0');
    }
    bpm = num;
    p++;                   // skip colon
  }

  // BPM usually expresses the number of quarter notes per minute
  wholenote = (60 * 1000L / bpm) * 4;  // this is the time for whole note (in milliseconds)

  // now begin note loop
  while(*p)
  {
    // first, get note duration, if available
    num = 0;
    while(isdigit(*p))
    {
      num = (num * 10) + (*p++ - '0');
    }

    if (num) duration = wholenote / num;
    else duration = wholenote / default_dur;  // we will need to check if we are a dotted note after

    // now get the note
    note = 0;

    switch(*p)
    {
      case 'c':
        note = 1;
        break;
      case 'd':
        note = 3;
        break;
      case 'e':
        note = 5;
        break;
      case 'f':
        note = 6;
        break;
      case 'g':
        note = 8;
        break;
      case 'a':
        note = 10;
        break;
      case 'b':
        note = 12;
        break;
      case 'p':
      default:
        note = 0;
    }
    p++;

    // now, get optional '#' sharp
    if(*p == '#')
    {
      note++;
      p++;
    }

    // now, get optional '.' dotted note
    if(*p == '.')
    {
      duration += duration/2;
      p++;
    }

    // now, get scale
    if(isdigit(*p))
    {
      scale = *p - '0';
      p++;
    }
    else
    {
      scale = default_oct;
    }

    scale += OCTAVE_OFFSET;

    if(*p == ',')
      p++;       // skip comma for next note (or we may be at the end)

    // now play the note
    if(note)
    {
      tone(_pin, notes[(scale - 4) * 12 + note], duration);
    }
    else
    {
      delay(duration/10);
    }
  }
}

#endif
