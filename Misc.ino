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

  for (byte x = 0; x < PLUGIN_CONFIGVAR_MAX; x++)
    Settings.TaskDevicePluginConfig[taskIndex][x] = 0;

  for (byte varNr = 0; varNr < VARS_PER_TASK; varNr++)
  {
    ExtraTaskSettings.TaskDeviceFormula[varNr][0] = 0;
    ExtraTaskSettings.TaskDeviceValueNames[varNr][0] = 0;
  }
  if (save)
  {
    SaveTaskSettings(taskIndex);
    SaveSettings();
  }
}

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

  Settings.Build = BUILD;
  SaveSettings();
}

#if FEATURE_SPIFFS
void fileSystemCheck()
{
  if (SPIFFS.begin())
  {
    String log = F("SPIFFS Mount succesfull");
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
* Find device index corresponding to task number setting
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
* Find protocol index corresponding to protocol setting
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
* Find positional parameter in a char string
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
* Convert a char string to integer
\*********************************************************************************************/
unsigned long str2int(char *string)
{
  unsigned long temp = atof(string);
  return temp;
}


/********************************************************************************************\
* Convert a char string to IP byte array
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
* Save settings to SPIFFS
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
* Load settings from SPIFFS
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
* Save Task settings to SPIFFS
\*********************************************************************************************/
void SaveTaskSettings(byte TaskIndex)
{
#if FEATURE_SPIFFS
  SaveToFile((char*)"config.txt", 4096 + (TaskIndex * 1024), (byte*)&ExtraTaskSettings, sizeof(struct ExtraTaskSettingsStruct));
#else
  SaveToFlash(4096 + (TaskIndex * 1024), (byte*)&ExtraTaskSettings, sizeof(struct ExtraTaskSettingsStruct));
#endif
}


/********************************************************************************************\
* Load Task settings from SPIFFS
\*********************************************************************************************/
void LoadTaskSettings(byte TaskIndex)
{
#if FEATURE_SPIFFS
  if (ExtraTaskSettings.TaskIndex == TaskIndex)
    return;
  LoadFromFile((char*)"config.txt", 4096 + (TaskIndex * 1024), (byte*)&ExtraTaskSettings, sizeof(struct ExtraTaskSettingsStruct));
  ExtraTaskSettings.TaskIndex = TaskIndex; // store active index
#else
  LoadFromFlash(4096 + (TaskIndex * 1024), (byte*)&ExtraTaskSettings, sizeof(struct ExtraTaskSettingsStruct));
#endif
}


/********************************************************************************************\
* Save Custom Task settings to SPIFFS
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
* Save Custom Task settings to SPIFFS
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


#if FEATURE_SPIFFS
/********************************************************************************************\
* Save data into config file on SPIFFS
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
* Load data from config file on SPIFFS
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
* Save data to flash
\*********************************************************************************************/
#define FLASH_EEPROM_SIZE 4096
extern "C" {
#include "spi_flash.h"
}
extern "C" uint32_t _SPIFFS_start;
extern "C" uint32_t _SPIFFS_end;
extern "C" uint32_t _SPIFFS_page;
extern "C" uint32_t _SPIFFS_block;

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
}


/********************************************************************************************\
* Load data from flash
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
* Erase data on flash
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
* Erase all content on flash (except sketch)
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
* Check SPIFFS area settings
\*********************************************************************************************/
int SpiffsSectors()
{
  uint32_t _sectorStart = ((uint32_t)&_SPIFFS_start - 0x40200000) / SPI_FLASH_SEC_SIZE;
  uint32_t _sectorEnd = ((uint32_t)&_SPIFFS_end - 0x40200000) / SPI_FLASH_SEC_SIZE;
  return _sectorEnd - _sectorStart;
}


/********************************************************************************************\
* Check flash chip (beyond sketch size)
\*********************************************************************************************/
void CheckFlash(int start, int end)
{
  //uint32_t _sectorStart = (ESP.getSketchSize() / SPI_FLASH_SEC_SIZE) + 1;
  //uint32_t _sectorEnd = _sectorStart + (ESP.getFlashChipRealSize() / SPI_FLASH_SEC_SIZE);

  uint32_t _sectorStart = start;
  uint32_t _sectorEnd = end;

  uint8_t* data = new uint8_t[FLASH_EEPROM_SIZE];

  uint8_t* tmpdata = data;
  for (int x = 0; x < FLASH_EEPROM_SIZE; x++)
  {
    *tmpdata = 0xA5;
    tmpdata++;
  }

  for (uint32_t _sector = _sectorStart; _sector < _sectorEnd; _sector++)
  {
    boolean success = 0;
    Serial.print(F("FLASH: Verify Sector: "));
    Serial.print(_sector);
    Serial.print(F(" : "));
    delay(10);
    noInterrupts();
    //if (spi_flash_erase_sector(_sector) == SPI_FLASH_RESULT_OK)
      //if (spi_flash_write(_sector * SPI_FLASH_SEC_SIZE, reinterpret_cast<uint32_t*>(data), FLASH_EEPROM_SIZE) == SPI_FLASH_RESULT_OK)
        if (spi_flash_read(_sector * SPI_FLASH_SEC_SIZE, reinterpret_cast<uint32_t*>(data), FLASH_EEPROM_SIZE) == SPI_FLASH_RESULT_OK)
          success = true;
    interrupts();
    if (success)
      Serial.println(F("OK"));
    else
      Serial.println(F("Fail"));
  }
  delete [] data;
}


/********************************************************************************************\
* Reset all settings to factory defaults
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
#else
  EraseFlash();
  ZeroFillFlash();
#endif

  LoadSettings();
  // now we set all parameters that need to be non-zero as default value
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
  Settings.Protocol        = DEFAULT_PROTOCOL;
  strcpy_P(Settings.Name, PSTR(DEFAULT_NAME));
  Settings.SerialLogLevel  = 2;
  Settings.WebLogLevel     = 2;
  Settings.BaudRate        = 115200;
  Settings.MessageDelay = 1000;
  Settings.deepSleep = false;
  Settings.CustomCSS = false;
  for (byte x = 0; x < TASKS_MAX; x++)
  {
    Settings.TaskDevicePin1[x] = -1;
    Settings.TaskDevicePin2[x] = -1;
    Settings.TaskDevicePin3[x] = -1;
    Settings.TaskDevicePin1PullUp[x] = true;
    Settings.TaskDevicePin1Inversed[x] = false;
    Settings.TaskDeviceSendData[x] = true;
  }
  Settings.Build = BUILD;
  SaveSettings();
  delay(1000);
  WiFi.persistent(true); // use SDK storage of SSID/WPA parameters
  WiFi.disconnect(); // this will store empty ssid/wpa into sdk storage
  WiFi.persistent(false); // Do not use SDK storage of SSID/WPA parameters
  ESP.reset();
}


/********************************************************************************************\
* If RX and TX tied together, perform emergency reset to get the system out of boot loops
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
* Get free system mem
\*********************************************************************************************/
extern "C" {
#include "user_interface.h"
}

unsigned long FreeMem(void)
{
  return system_get_free_heap_size();
}


/********************************************************************************************\
* In memory convert float to long
\*********************************************************************************************/
unsigned long float2ul(float f)
{
  unsigned long ul;
  memcpy(&ul, &f, 4);
  return ul;
}


/********************************************************************************************\
* In memory convert long to float
\*********************************************************************************************/
float ul2float(unsigned long ul)
{
  float f;
  memcpy(&f, &ul, 4);
  return f;
}


/********************************************************************************************\
* Add to log
\*********************************************************************************************/
void addLog(byte loglevel, String& string)
{
  addLog(loglevel, string.c_str());
}

void addLog(byte loglevel, const char *line)
{
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
* Delayed reboot, in case of issues, do not reboot with high frequency as it might not help...
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
* Save a byte to RTC memory
\*********************************************************************************************/
#define RTC_BASE 65 // system doc says user area starts at 64, but it does not work (?)
void saveToRTC(byte Par1)
{
  byte buf[3] = {0xAA, 0x55, 0};
  buf[2] = Par1;
  system_rtc_mem_write(RTC_BASE, buf, 3);
}


/********************************************************************************************\
* Read a byte from RTC memory
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
* Convert a string like "Sun,12:30" into a 32 bit integer
\*********************************************************************************************/
unsigned long string2TimeLong(String &str)
{
  // format 0000WWWWAAAABBBBCCCCDDDD
  // WWWW=weekday, AAAA=hours tens digit, BBBB=hours, CCCC=minutes tens digit DDDD=minutes

  char command[20];
  char TmpStr1[10];
  int w, x, y;
  unsigned long a;
  str.toCharArray(command, 20);
  unsigned long lngTime;

  if (GetArgv(command, TmpStr1, 1))
  {
    String day = TmpStr1;
    String weekDays = F("AllSunMonTueWedThuFriSat");
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
* Convert  a 32 bit integer into a string like "Sun,12:30"
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
                  String value = String(UserVar[y * VARS_PER_TASK + z]);
                  if (valueFormat == "R")
                  {
                    int filler = lineSize - newString.length() - value.length() - tmpString.length() ;
                    for (byte f = 0; f < filler; f++)
                      newString += " ";
                  }
                  newString += String(value);
                }
            }
          }
        }
      }
      leftBracketIndex = tmpString.indexOf('[');
      count++;
    }
    newString += tmpString;
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

  IPAddress ip = WiFi.localIP();
  char strIP[20];
  sprintf_P(strIP, PSTR("%u.%u.%u.%u"), ip[0], ip[1], ip[2], ip[3]);
  newString.replace("%ip%", strIP);

  // padding spaces
  while (newString.length() < lineSize)
    newString += " ";

  return newString;
}


/********************************************************************************************\
* Calculate function for simple expressions
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

#define is_operator(c)  (c == '+' || c == '-' || c == '*' || c == '/' )

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
* Time stuff
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
  now();
}

void checkTime()
{
  now();
  if (tm.Minute != PrevMinutes)
  {
    PluginCall(PLUGIN_CLOCK_IN, 0, dummyString);
    PrevMinutes = tm.Minute;
  }
}


unsigned long getNtpTime()
{
  WiFiUDP udp;
  udp.begin(123);
  String log = F("NTP  : NTP sync requested");
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
  while (millis() - beginWait < 1500) {
    int size = udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      log = F("NTP  : NTP replied!");
      addLog(LOG_LEVEL_DEBUG_MORE, log);
      return secsSince1900 - 2208988800UL + Settings.TimeZone * SECS_PER_HOUR;
    }
  }
  log = F("NTP  : No reply");
  addLog(LOG_LEVEL_DEBUG_MORE, log);
  return 0;
}
#endif

