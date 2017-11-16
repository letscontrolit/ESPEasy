// clean up tcp connections that are in TIME_WAIT status, to conserve memory
// In future versions of WiFiClient it should be possible to call abort(), but
// this feature is not in all upstream versions yet.
// See https://github.com/esp8266/Arduino/issues/1923
// and https://github.com/letscontrolit/ESPEasy/issues/253

#if defined(ESP8266)
void tcpCleanup()
{
  while(tcp_tw_pcbs!=NULL)
  {
    tcp_abort(tcp_tw_pcbs);
  }
}
#endif

bool isDeepSleepEnabled()
{
  if (!Settings.deepSleep)
    return false;

  //cancel deep sleep loop by pulling the pin GPIO16(D0) to GND
  //recommended wiring: 3-pin-header with 1=RST, 2=D0, 3=GND
  //                    short 1-2 for normal deep sleep / wakeup loop
  //                    short 2-3 to cancel sleep loop for modifying settings
  pinMode(16,INPUT_PULLUP);
  if (!digitalRead(16))
  {
    return false;
  }
  return true;
}

void deepSleep(int delay)
{

  if (!isDeepSleepEnabled())
  {
    //Deep sleep canceled by GPIO16(D0)=LOW
    return;
  }

  //first time deep sleep? offer a way to escape
  if (lastBootCause!=BOOT_CAUSE_DEEP_SLEEP)
  {
    addLog(LOG_LEVEL_INFO, F("SLEEP: Entering deep sleep in 30 seconds."));
    delayBackground(30000);
    //disabled?
    if (!isDeepSleepEnabled())
    {
      addLog(LOG_LEVEL_INFO, F("SLEEP: Deep sleep cancelled (GPIO16 connected to GND)"));
      return;
    }
  }

  deepSleepStart(delay); // Call deepSleepStart function after these checks
}

void deepSleepStart(int delay)
{
  // separate function that is called from above function or directly from rules, usign deepSleep as a one-shot
  String event = F("System#Sleep");
  rulesProcessing(event);


  RTC.deepSleepState = 1;
  saveToRTC();

  if (delay > 4294 || delay < 0)
    delay = 4294;   //max sleep time ~1.2h

  addLog(LOG_LEVEL_INFO, F("SLEEP: Powering down to deepsleep..."));
  #if defined(ESP8266)
    ESP.deepSleep((uint32_t)delay * 1000000, WAKE_RF_DEFAULT);
  #endif
  #if defined(ESP32)
    esp_sleep_enable_timer_wakeup((uint32_t)delay * 1000000);
    esp_deep_sleep_start();
  #endif
}

boolean remoteConfig(struct EventStruct *event, String& string)
{
  boolean success = false;
  String command = parseString(string, 1);

  if (command == F("config"))
  {
    success = true;
    if (parseString(string, 2) == F("task"))
    {
      int configCommandPos1 = getParamStartPos(string, 3);
      int configCommandPos2 = getParamStartPos(string, 4);

      String configTaskName = string.substring(configCommandPos1, configCommandPos2 - 1);
      String configCommand = string.substring(configCommandPos2);

      int8_t index = getTaskIndexByName(configTaskName);
      if (index != -1)
      {
        event->TaskIndex = index;
        success = PluginCall(PLUGIN_SET_CONFIG, event, configCommand);
      }
    }
  }
  return success;
}

int8_t getTaskIndexByName(String TaskNameSearch)
{

  for (byte x = 0; x < TASKS_MAX; x++)
  {
    LoadTaskSettings(x);
    String TaskName = ExtraTaskSettings.TaskDeviceName;
    if ((ExtraTaskSettings.TaskDeviceName[0] != 0 ) && (TaskNameSearch.equalsIgnoreCase(TaskName)))
    {
      return x;
    }
  }
  return -1;
}


void flashCount()
{
  if (RTC.flashDayCounter <= MAX_FLASHWRITES_PER_DAY)
    RTC.flashDayCounter++;
  RTC.flashCounter++;
  saveToRTC();
}

String flashGuard()
{
  if (RTC.flashDayCounter > MAX_FLASHWRITES_PER_DAY)
  {
    String log = F("FS   : Daily flash write rate exceeded! (powercycle to reset this)");
    addLog(LOG_LEVEL_ERROR, log);
    return log;
  }
  flashCount();
  return(String());
}

//use this in function that can return an error string. it automaticly returns with an error string if there where too many flash writes.
#define FLASH_GUARD() { String flashErr=flashGuard(); if (flashErr.length()) return(flashErr); }

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
    case SENSOR_TYPE_WIND:
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
   Format a value to the set number of decimals
  \*********************************************************************************************/
String formatUserVar(struct EventStruct *event, byte rel_index)
{
  return toString(
    UserVar[event->BaseVarIndex + rel_index],
    ExtraTaskSettings.TaskDeviceValueDecimals[rel_index]);
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
  for (unsigned int x = 0; x < tmpString.length(); x++)
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
void setPinState(byte plugin, byte index, byte mode, uint16_t value)
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
  //XXX: fix me, something fishy going on here, this << operator is in the wrong place or parenthesis are wrong
  if (((now >= timer) && ((now - timer) < 1u << 31))  || ((timer >= now) && (timer - now > 1u << 31)))
    return true;

  return false;
}


/********************************************************************************************\
  Status LED
\*********************************************************************************************/
#if defined(ESP32)
  #define PWMRANGE 1024
#endif
#define STATUS_PWM_NORMALVALUE (PWMRANGE>>2)
#define STATUS_PWM_NORMALFADE (PWMRANGE>>8)
#define STATUS_PWM_TRAFFICRISE (PWMRANGE>>1)

void statusLED(boolean traffic)
{
  static int gnStatusValueCurrent = -1;
  static long int gnLastUpdate = millis();

  if (Settings.Pin_status_led == -1)
    return;

  if (gnStatusValueCurrent<0)
    pinMode(Settings.Pin_status_led, OUTPUT);

  int nStatusValue = gnStatusValueCurrent;

  if (traffic)
  {
    nStatusValue += STATUS_PWM_TRAFFICRISE; //ramp up fast
  }
  else
  {

    if (WiFi.status() == WL_CONNECTED)
    {
      long int delta=millis()-gnLastUpdate;
      if (delta>0 || delta<0 )
      {
        nStatusValue -= STATUS_PWM_NORMALFADE; //ramp down slowly
        nStatusValue = std::max(nStatusValue, STATUS_PWM_NORMALVALUE);
        gnLastUpdate=millis();
      }
    }
    //AP mode is active
    else if (WifiIsAP())
    {
      nStatusValue = ((millis()>>1) & PWMRANGE) - (PWMRANGE>>2); //ramp up for 2 sec, 3/4 luminosity
    }
    //Disconnected
    else
    {
      nStatusValue = (millis()>>1) & (PWMRANGE>>2); //ramp up for 1/2 sec, 1/4 luminosity
    }
  }

  nStatusValue = constrain(nStatusValue, 0, PWMRANGE);

  if (gnStatusValueCurrent != nStatusValue)
  {
    gnStatusValueCurrent = nStatusValue;

    long pwm = nStatusValue * nStatusValue; //simple gamma correction
    pwm >>= 10;
    if (Settings.Pin_status_led_Inversed)
      pwm = PWMRANGE-pwm;

    #if defined(ESP8266)
      analogWrite(Settings.Pin_status_led, pwm);
    #endif
  }
}


/********************************************************************************************\
  delay in milliseconds with background processing
  \*********************************************************************************************/
void delayBackground(unsigned long delay)
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
  event->Par4 = 0;
  event->Par5 = 0;

  if (GetArgv(command, TmpStr1, 2)) event->Par1 = str2int(TmpStr1);
  if (GetArgv(command, TmpStr1, 3)) event->Par2 = str2int(TmpStr1);
  if (GetArgv(command, TmpStr1, 4)) event->Par3 = str2int(TmpStr1);
  if (GetArgv(command, TmpStr1, 5)) event->Par4 = str2int(TmpStr1);
  if (GetArgv(command, TmpStr1, 6)) event->Par5 = str2int(TmpStr1);
}

/********************************************************************************************\
  Clear task settings for given task
  \*********************************************************************************************/
void taskClear(byte taskIndex, boolean save)
{
  Settings.TaskDeviceNumber[taskIndex] = 0;
  ExtraTaskSettings.TaskDeviceName[0] = 0;
  Settings.TaskDeviceDataFeed[taskIndex] = 0;
  Settings.TaskDevicePin1[taskIndex] = -1;
  Settings.TaskDevicePin2[taskIndex] = -1;
  Settings.TaskDevicePin3[taskIndex] = -1;
  Settings.TaskDevicePort[taskIndex] = 0;
  Settings.TaskDeviceGlobalSync[taskIndex] = false;
  Settings.TaskDeviceTimer[taskIndex] = 0;
  Settings.TaskDeviceEnabled[taskIndex] = false;

  for (byte controllerNr = 0; controllerNr < CONTROLLER_MAX; controllerNr++)
  {
    Settings.TaskDeviceID[controllerNr][taskIndex] = 0;
    Settings.TaskDeviceSendData[controllerNr][taskIndex] = true;
  }

  for (byte x = 0; x < PLUGIN_CONFIGVAR_MAX; x++)
    Settings.TaskDevicePluginConfig[taskIndex][x] = 0;

  for (byte varNr = 0; varNr < VARS_PER_TASK; varNr++)
  {
    ExtraTaskSettings.TaskDeviceFormula[varNr][0] = 0;
    ExtraTaskSettings.TaskDeviceValueNames[varNr][0] = 0;
    ExtraTaskSettings.TaskDeviceValueDecimals[varNr] = 2;
  }

  for (byte varNr = 0; varNr < PLUGIN_EXTRACONFIGVAR_MAX; varNr++)
  {
    ExtraTaskSettings.TaskDevicePluginConfigLong[varNr] = 0;
    ExtraTaskSettings.TaskDevicePluginConfig[varNr] = 0;
  }

  if (save)
  {
    SaveTaskSettings(taskIndex);
    SaveSettings();
  }
}

/********************************************************************************************\
  SPIFFS error handling
  Look here for error # reference: https://github.com/pellepl/spiffs/blob/master/src/spiffs.h
  \*********************************************************************************************/
#define SPIFFS_CHECK(result, fname) if (!(result)) { return(FileError(__LINE__, fname)); }
String FileError(int line, const char * fname)
{
   String err("FS   : Error while reading/writing ");
   err=err+fname;
   err=err+" in ";
   err=err+line;
   addLog(LOG_LEVEL_ERROR, err);
   return(err);
}


/********************************************************************************************\
  Fix stuff to clear out differences between releases
  \*********************************************************************************************/
String BuildFixes()
{
  Serial.println(F("\nBuild changed!"));

  if (Settings.Build < 145)
  {
    String fname=F(FILE_NOTIFICATION);
    fs::File f = SPIFFS.open(fname, "w");
    SPIFFS_CHECK(f, fname.c_str());

    if (f)
    {
      for (int x = 0; x < 4096; x++)
      {
        SPIFFS_CHECK(f.write(0), fname.c_str());
      }
      f.close();
    }
  }
  Settings.Build = BUILD;
  return(SaveSettings());
}


/********************************************************************************************\
  Mount FS and check config.dat
  \*********************************************************************************************/
void fileSystemCheck()
{
  addLog(LOG_LEVEL_INFO, F("FS   : Mounting..."));
  if (SPIFFS.begin())
  {
    #if defined(ESP8266)
      fs::FSInfo fs_info;
      SPIFFS.info(fs_info);

      String log = F("FS   : Mount successful, used ");
      log=log+fs_info.usedBytes;
      log=log+F(" bytes of ");
      log=log+fs_info.totalBytes;
      addLog(LOG_LEVEL_INFO, log);
    #endif

    fs::File f = SPIFFS.open(FILE_CONFIG, "r");
    if (!f)
    {
      ResetFactory();
    }
    f.close();
  }
  else
  {
    String log = F("FS   : Mount failed");
    Serial.println(log);
    addLog(LOG_LEVEL_ERROR, log);
    ResetFactory();
  }
}


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
  Get notificatoin protocol index (plugin index), by NPlugin_id
  \*********************************************************************************************/
byte getNotificationProtocolIndex(byte Number)
{

  for (byte x = 0; x <= notificationCount ; x++)
    if (Notification[x].Number == Number)
      return(x);

  return(NPLUGIN_NOT_FOUND);
}

/********************************************************************************************\
  Find positional parameter in a char string
  \*********************************************************************************************/
boolean GetArgv(const char *string, char *argv, unsigned int argc)
{
  unsigned int string_pos = 0, argv_pos = 0, argc_pos = 0;
  char c, d;
  boolean parenthesis = false;

  while (string_pos < strlen(string))
  {
    c = string[string_pos];
    d = string[string_pos + 1];

    if       (!parenthesis && c == ' ' && d == ' ') {}
    else if  (!parenthesis && c == ' ' && d == ',') {}
    else if  (!parenthesis && c == ',' && d == ' ') {}
    else if  (!parenthesis && c == ' ' && d >= 33 && d <= 126) {}
    else if  (!parenthesis && c == ',' && d >= 33 && d <= 126) {}
    else if  (c == '"') {
      parenthesis = true;
    }
    else
    {
      argv[argv_pos++] = c;
      argv[argv_pos] = 0;

      if ((!parenthesis && (d == ' ' || d == ',' || d == 0)) || (parenthesis && d == '"')) // end of word
      {
        if (d == '"')
          parenthesis = false;
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

  for (unsigned int x = 0; x <= strlen(string); x++)
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
String SaveSettings(void)
{
  String err;
  err=SaveToFile((char*)FILE_CONFIG, 0, (byte*)&Settings, sizeof(struct SettingsStruct));
  if (err.length())
    return(err);

  return(SaveToFile((char*)FILE_SECURITY, 0, (byte*)&SecuritySettings, sizeof(struct SecurityStruct)));
}


/********************************************************************************************\
  Load settings from SPIFFS
  \*********************************************************************************************/
String LoadSettings()
{
  String err;
  err=LoadFromFile((char*)FILE_CONFIG, 0, (byte*)&Settings, sizeof(struct SettingsStruct));
  if (err.length())
    return(err);

  return(LoadFromFile((char*)FILE_SECURITY, 0, (byte*)&SecuritySettings, sizeof(struct SecurityStruct)));
}


/********************************************************************************************\
  Save Task settings to SPIFFS
  \*********************************************************************************************/
String SaveTaskSettings(byte TaskIndex)
{
  ExtraTaskSettings.TaskIndex = TaskIndex;
  return(SaveToFile((char*)FILE_CONFIG, DAT_OFFSET_TASKS + (TaskIndex * DAT_TASKS_SIZE), (byte*)&ExtraTaskSettings, sizeof(struct ExtraTaskSettingsStruct)));
}


/********************************************************************************************\
  Load Task settings from SPIFFS
  \*********************************************************************************************/
String LoadTaskSettings(byte TaskIndex)
{
  //already loaded
  if (ExtraTaskSettings.TaskIndex == TaskIndex)
    return(String());

  String result = "";
  result = LoadFromFile((char*)FILE_CONFIG, DAT_OFFSET_TASKS + (TaskIndex * DAT_TASKS_SIZE), (byte*)&ExtraTaskSettings, sizeof(struct ExtraTaskSettingsStruct));
  ExtraTaskSettings.TaskIndex = TaskIndex; // Needed when an empty task was requested
  return result;
}


/********************************************************************************************\
  Save Custom Task settings to SPIFFS
  \*********************************************************************************************/
String SaveCustomTaskSettings(int TaskIndex, byte* memAddress, int datasize)
{
  if (datasize > DAT_TASKS_SIZE)
    return F("SaveCustomTaskSettings too big");
  return(SaveToFile((char*)FILE_CONFIG, DAT_OFFSET_TASKS + (TaskIndex * DAT_TASKS_SIZE) + DAT_TASKS_CUSTOM_OFFSET, memAddress, datasize));
}


/********************************************************************************************\
  Load Custom Task settings to SPIFFS
  \*********************************************************************************************/
String LoadCustomTaskSettings(int TaskIndex, byte* memAddress, int datasize)
{
  if (datasize > DAT_TASKS_SIZE)
    return (String(F("LoadCustomTaskSettings too big")));
  return(LoadFromFile((char*)FILE_CONFIG, DAT_OFFSET_TASKS + (TaskIndex * DAT_TASKS_SIZE) + DAT_TASKS_CUSTOM_OFFSET, memAddress, datasize));
}

/********************************************************************************************\
  Save Controller settings to SPIFFS
  \*********************************************************************************************/
String SaveControllerSettings(int ControllerIndex, byte* memAddress, int datasize)
{
  if (datasize > DAT_CONTROLLER_SIZE)
    return F("SaveControllerSettings too big");
  return SaveToFile((char*)FILE_CONFIG, DAT_OFFSET_CONTROLLER + (ControllerIndex * DAT_CONTROLLER_SIZE), memAddress, datasize);
}


/********************************************************************************************\
  Load Controller settings to SPIFFS
  \*********************************************************************************************/
String LoadControllerSettings(int ControllerIndex, byte* memAddress, int datasize)
{
  if (datasize > DAT_CONTROLLER_SIZE)
    return F("LoadControllerSettings too big");

  return(LoadFromFile((char*)FILE_CONFIG, DAT_OFFSET_CONTROLLER + (ControllerIndex * DAT_CONTROLLER_SIZE), memAddress, datasize));
}

/********************************************************************************************\
  Save Custom Controller settings to SPIFFS
  \*********************************************************************************************/
String SaveCustomControllerSettings(int ControllerIndex,byte* memAddress, int datasize)
{
  if (datasize > DAT_CUSTOM_CONTROLLER_SIZE)
    return F("SaveCustomControllerSettings too big");
  return SaveToFile((char*)FILE_CONFIG, DAT_OFFSET_CUSTOM_CONTROLLER + (ControllerIndex * DAT_CUSTOM_CONTROLLER_SIZE), memAddress, datasize);
}


/********************************************************************************************\
  Load Custom Controller settings to SPIFFS
  \*********************************************************************************************/
String LoadCustomControllerSettings(int ControllerIndex,byte* memAddress, int datasize)
{
  if (datasize > DAT_CUSTOM_CONTROLLER_SIZE)
    return(F("LoadCustomControllerSettings too big"));
  return(LoadFromFile((char*)FILE_CONFIG, DAT_OFFSET_CUSTOM_CONTROLLER + (ControllerIndex * DAT_CUSTOM_CONTROLLER_SIZE), memAddress, datasize));
}

/********************************************************************************************\
  Save Controller settings to SPIFFS
  \*********************************************************************************************/
String SaveNotificationSettings(int NotificationIndex, byte* memAddress, int datasize)
{
  if (datasize > DAT_NOTIFICATION_SIZE)
    return F("SaveNotificationSettings too big");
  return SaveToFile((char*)FILE_NOTIFICATION, NotificationIndex * DAT_NOTIFICATION_SIZE, memAddress, datasize);
}


/********************************************************************************************\
  Load Controller settings to SPIFFS
  \*********************************************************************************************/
String LoadNotificationSettings(int NotificationIndex, byte* memAddress, int datasize)
{
  if (datasize > DAT_NOTIFICATION_SIZE)
    return(F("LoadNotificationSettings too big"));
  return(LoadFromFile((char*)FILE_NOTIFICATION, NotificationIndex * DAT_NOTIFICATION_SIZE, memAddress, datasize));
}




/********************************************************************************************\
  Init a file with zeros on SPIFFS
  \*********************************************************************************************/
String InitFile(const char* fname, int datasize)
{

  FLASH_GUARD();

  fs::File f = SPIFFS.open(fname, "w");
  SPIFFS_CHECK(f, fname);

  for (int x = 0; x < datasize ; x++)
  {
    SPIFFS_CHECK(f.write(0), fname);
  }
  f.close();

  //OK
  return String();
}

/********************************************************************************************\
  Save data into config file on SPIFFS
  \*********************************************************************************************/
String SaveToFile(char* fname, int index, byte* memAddress, int datasize)
{

  FLASH_GUARD();

  fs::File f = SPIFFS.open(fname, "r+");
  SPIFFS_CHECK(f, fname);

  SPIFFS_CHECK(f.seek(index, fs::SeekSet), fname);
  byte *pointerToByteToSave = memAddress;
  for (int x = 0; x < datasize ; x++)
  {
    SPIFFS_CHECK(f.write(*pointerToByteToSave), fname);
    pointerToByteToSave++;
  }
  f.close();
  String log = F("FILE : Saved ");
  log=log+fname;
  addLog(LOG_LEVEL_INFO, log);

  //OK
  return String();
}


/********************************************************************************************\
  Load data from config file on SPIFFS
  \*********************************************************************************************/
String LoadFromFile(char* fname, int index, byte* memAddress, int datasize)
{
  // addLog(LOG_LEVEL_INFO, String(F("FILE : Load size "))+datasize);

  fs::File f = SPIFFS.open(fname, "r+");
  SPIFFS_CHECK(f, fname);

  // addLog(LOG_LEVEL_INFO, String(F("FILE : File size "))+f.size());

  SPIFFS_CHECK(f.seek(index, fs::SeekSet), fname);
  byte *pointerToByteToRead = memAddress;
  for (int x = 0; x < datasize; x++)
  {
    int readres=f.read();
    SPIFFS_CHECK(readres >=0, fname);
    *pointerToByteToRead = readres;
    pointerToByteToRead++;// next byte
  }
  f.close();

  return(String());
}


/********************************************************************************************\
  Check SPIFFS area settings
  \*********************************************************************************************/
int SpiffsSectors()
{
  #if defined(ESP8266)
    uint32_t _sectorStart = ((uint32_t)&_SPIFFS_start - 0x40200000) / SPI_FLASH_SEC_SIZE;
    uint32_t _sectorEnd = ((uint32_t)&_SPIFFS_end - 0x40200000) / SPI_FLASH_SEC_SIZE;
    return _sectorEnd - _sectorStart;
  #endif
  #if defined(ESP32)
    return 32;
  #endif
}


/********************************************************************************************\
  Reset all settings to factory defaults
  \*********************************************************************************************/
void ResetFactory(void)
{

  // Direct Serial is allowed here, since this is only an emergency task.
  Serial.println(F("RESET: Resetting factory defaults..."));
  delay(1000);
  if (readFromRTC())
  {
    Serial.print(F("RESET: Warm boot, reset count: "));
    Serial.println(RTC.factoryResetCounter);
    if (RTC.factoryResetCounter >= 3)
    {
      Serial.println(F("RESET: Too many resets, protecting your flash memory (powercycle to solve this)"));
      return;
    }
  }
  else
  {
    Serial.println(F("RESET: Cold boot"));
    initRTC();
  }

  RTC.flashCounter=0; //reset flashcounter, since we're already counting the number of factory-resets. we dont want to hit a flash-count limit during reset.
  RTC.factoryResetCounter++;
  saveToRTC();

  //always format on factory reset, in case of corrupt SPIFFS
  SPIFFS.end();
  Serial.println(F("RESET: formatting..."));
  SPIFFS.format();
  Serial.println(F("RESET: formatting done..."));
  if (!SPIFFS.begin())
  {
    Serial.println(F("RESET: FORMAT SPIFFS FAILED!"));
    return;
  }


  //pad files with extra zeros for future extensions
  String fname;

  fname=F(FILE_CONFIG);
  InitFile(fname.c_str(), 65536);

  fname=F(FILE_SECURITY);
  InitFile(fname.c_str(), 4096);

  fname=F(FILE_NOTIFICATION);
  InitFile(fname.c_str(), 4096);

  fname=F(FILE_RULES);
  InitFile(fname.c_str(), 0);

  LoadSettings();
  // now we set all parameters that need to be non-zero as default value

#if DEFAULT_USE_STATIC_IP
  str2ip((char*)DEFAULT_IP, Settings.IP);
  str2ip((char*)DEFAULT_DNS, Settings.DNS);
  str2ip((char*)DEFAULT_GW, Settings.Gateway);
  str2ip((char*)DEFAULT_SUBNET, Settings.Subnet);
#endif

  Settings.PID             = ESP_PROJECT_PID;
  Settings.Version         = VERSION;
  Settings.Unit            = UNIT;
  strcpy_P(SecuritySettings.WifiSSID, PSTR(DEFAULT_SSID));
  strcpy_P(SecuritySettings.WifiKey, PSTR(DEFAULT_KEY));
  strcpy_P(SecuritySettings.WifiAPKey, PSTR(DEFAULT_AP_KEY));
  SecuritySettings.Password[0] = 0;
  Settings.Delay           = DEFAULT_DELAY;
  Settings.Pin_i2c_sda     = 4;
  Settings.Pin_i2c_scl     = 5;
  Settings.Pin_status_led  = -1;
  Settings.Pin_status_led_Inversed  = true;
  Settings.Pin_sd_cs       = -1;
  Settings.Protocol[0]        = DEFAULT_PROTOCOL;
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
    for (byte y = 0; y < CONTROLLER_MAX; y++)
      Settings.TaskDeviceSendData[y][x] = true;
    Settings.TaskDeviceTimer[x] = Settings.Delay;
  }
  Settings.Build = BUILD;
  Settings.UseSerial = true;
  SaveSettings();

#if DEFAULT_CONTROLLER
  ControllerSettingsStruct ControllerSettings;
  strcpy_P(ControllerSettings.Subscribe, PSTR(DEFAULT_SUB));
  strcpy_P(ControllerSettings.Publish, PSTR(DEFAULT_PUB));
  str2ip((char*)DEFAULT_SERVER, ControllerSettings.IP);
  ControllerSettings.HostName[0]=0;
  ControllerSettings.Port = DEFAULT_PORT;
  SaveControllerSettings(0, (byte*)&ControllerSettings, sizeof(ControllerSettings));
#endif

  Serial.println("RESET: Succesful, rebooting. (you might need to press the reset button if you've justed flashed the firmware)");
  //NOTE: this is a known ESP8266 bug, not our fault. :)
  delay(1000);
  WiFi.persistent(true); // use SDK storage of SSID/WPA parameters
  WiFi.disconnect(); // this will store empty ssid/wpa into sdk storage
  WiFi.persistent(false); // Do not use SDK storage of SSID/WPA parameters
  #if defined(ESP8266)
    ESP.reset();
  #endif
  #if defined(ESP32)
    ESP.restart();
  #endif
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
      Serial.println(F("\n\n\rSystem will reset to factory defaults in 10 seconds..."));
      delay(10000);
      ResetFactory();
    }
}


/********************************************************************************************\
  Get free system mem
  \*********************************************************************************************/
unsigned long FreeMem(void)
{
  #if defined(ESP8266)
    return system_get_free_heap_size();
  #endif
  #if defined(ESP32)
    return ESP.getFreeHeap();
  #endif
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
  Init critical variables for logging (important during initial factory reset stuff )
  \*********************************************************************************************/
void initLog()
{
  //make sure addLog doesnt do any stuff before initalisation of Settings is complete.
  Settings.UseSerial=true;
  Settings.SyslogLevel=0;
  Settings.SerialLogLevel=2; //logging during initialisation
  Settings.WebLogLevel=2;
  Settings.SDLogLevel=0;
  for (int l=0; l<10; l++)
  {
    Logging[l].Message=0;
  }
}

/********************************************************************************************\
  Logging
  \*********************************************************************************************/
void addLog(byte loglevel, String& string)
{
  addLog(loglevel, string.c_str());
}

void addLog(byte logLevel, const __FlashStringHelper* flashString)
{
    String s(flashString);
    addLog(logLevel, s.c_str());
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
    if (Logging[logcount].Message == 0)
      Logging[logcount].Message =  (char *)malloc(128);
    strncpy(Logging[logcount].Message, line, 127);
    Logging[logcount].Message[127]=0; //make sure its null terminated!

  }

  if (loglevel <= Settings.SDLogLevel)
  {
    File logFile = SD.open("log.dat", FILE_WRITE);
    if (logFile)
      logFile.println(line);
    logFile.close();
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
   #if defined(ESP8266)
     ESP.reset();
   #endif
   #if defined(ESP32)
     ESP.restart();
   #endif
}


/********************************************************************************************\
  Save RTC struct to RTC memory
  \*********************************************************************************************/
boolean saveToRTC()
{
  #if defined(ESP8266)
    if (!system_rtc_mem_write(RTC_BASE_STRUCT, (byte*)&RTC, sizeof(RTC)) || !readFromRTC())
    {
      addLog(LOG_LEVEL_ERROR, F("RTC  : Error while writing to RTC"));
      return(false);
    }
    else
    {
      return(true);
    }
  #endif
  #if defined(ESP32)
    boolean ret = false;
  #endif
}


/********************************************************************************************\
  Initialize RTC memory
  \*********************************************************************************************/
void initRTC()
{
  memset(&RTC, 0, sizeof(RTC));
  RTC.ID1 = 0xAA;
  RTC.ID2 = 0x55;
  saveToRTC();

  memset(&UserVar, 0, sizeof(UserVar));
  saveUserVarToRTC();
}

/********************************************************************************************\
  Read RTC struct from RTC memory
  \*********************************************************************************************/
boolean readFromRTC()
{
  #if defined(ESP8266)
    if (!system_rtc_mem_read(RTC_BASE_STRUCT, (byte*)&RTC, sizeof(RTC)))
      return(false);

    if (RTC.ID1 == 0xAA && RTC.ID2 == 0x55)
      return true;
    else
      return false;
  #endif
  #if defined(ESP32)
    boolean ret = false;
  #endif
}


/********************************************************************************************\
  Save values to RTC memory
\*********************************************************************************************/
boolean saveUserVarToRTC()
{
  #if defined(ESP8266)
    //addLog(LOG_LEVEL_DEBUG, F("RTCMEM: saveUserVarToRTC"));
    byte* buffer = (byte*)&UserVar;
    size_t size = sizeof(UserVar);
    uint32_t sum = getChecksum(buffer, size);
    boolean ret = system_rtc_mem_write(RTC_BASE_USERVAR, buffer, size);
    ret &= system_rtc_mem_write(RTC_BASE_USERVAR+(size>>2), (byte*)&sum, 4);
  #endif
  #if defined(ESP32)
    boolean ret = false;
  #endif
  return ret;
}


/********************************************************************************************\
  Read RTC struct from RTC memory
\*********************************************************************************************/
boolean readUserVarFromRTC()
{
  #if defined(ESP8266)
    //addLog(LOG_LEVEL_DEBUG, F("RTCMEM: readUserVarFromRTC"));
    byte* buffer = (byte*)&UserVar;
    size_t size = sizeof(UserVar);
    boolean ret = system_rtc_mem_read(RTC_BASE_USERVAR, buffer, size);
    uint32_t sumRAM = getChecksum(buffer, size);
    uint32_t sumRTC = 0;
    ret &= system_rtc_mem_read(RTC_BASE_USERVAR+(size>>2), (byte*)&sumRTC, 4);
    if (!ret || sumRTC != sumRAM)
    {
      addLog(LOG_LEVEL_ERROR, F("RTC  : Checksum error on reading RTC user var"));
      memset(buffer, 0, size);
    }
  #endif
  #if defined(ESP32)
    boolean ret = false;
  #endif
  return ret;
}


uint32_t getChecksum(byte* buffer, size_t size)
{
  uint32_t sum = 0x82662342;   //some magic to avoid valid checksum on new, uninitialized ESP
  for (size_t i=0; i<size; i++)
    sum += buffer[i];
  return sum;
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
    String weekDays = F("allsunmontuewedthufrisatwrkwkd");
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
      if ( (w >= '0' && w <= '9') || w == '*')
      {
        a = 0xffffffff  ^ (0xfUL << y); // create mask to clean nibble position y
        lngTime &= a; // maak nibble leeg
        if (w == '*')
          lngTime |= (0xFUL << y); // fill nibble with wildcard value
        else
          lngTime |= (w - '0') << y; // fill nibble with token
        y += 4;
      }
      else
        if (w == ':');
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
  String weekDays = F("AllSunMonTueWedThuFriSatWrkWkd");
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

// returns the current Date separated by the given delimiter
// date format example with '-' delimiter: 2016-12-31 (YYYY-MM-DD)
String getDateString(char delimiter)
{
  String reply = String(year());
  if (delimiter != '\0')
  	reply += delimiter;
  if (month() < 10)
    reply += "0";
  reply += month();
  if (delimiter != '\0')
  	reply += delimiter;
  if (day() < 10)
  	reply += F("0");
  reply += day();
  return reply;
}

String getDayString()
{
  String reply;
  if (day() < 10)
    reply += F("0");
  reply += day();
  return reply;
}
String getMonthString()
{
  String reply;
  if (month() < 10)
    reply += F("0");
  reply += month();
  return reply;
}
String getYearString()
{
  String reply = String(year());
  return reply;
}
String getYearStringShort()
{
  String dummy = String(year());
  String reply = dummy.substring(2);
  return reply;
}

// returns the current Date without delimiter
// date format example: 20161231 (YYYYMMDD)
String getDateString()
{
	return getDateString('\0');
}

// returns the current Time separated by the given delimiter
// time format example with ':' delimiter: 23:59:59 (HH:MM:SS)
String getTimeString(char delimiter)
{
	String reply;
	if (hour() < 10)
		reply += F("0");
  reply += String(hour());
  if (delimiter != '\0')
  	reply += delimiter;
  if (minute() < 10)
    reply += F("0");
  reply += minute();
  if (delimiter != '\0')
  	reply += delimiter;
  if (second() < 10)
  	reply += F("0");
  reply += second();
  return reply;
}

String getHourString()
{
  String reply;
  if (hour() < 10)
    reply += F("0");
  reply += String(hour());
  return reply;
}
String getMinuteString()
{
  String reply;
  if (minute() < 10)
    reply += F("0");
  reply += minute();
  return reply;
}
String getSecondString()
{
  String reply;
  if (second() < 10)
    reply += F("0");
  reply += second();
  return reply;
}

// returns the current Time without delimiter
// time format example: 235959 (HHMMSS)
String getTimeString()
{
	return getTimeString('\0');
}

// returns the current Date and Time separated by the given delimiter
// if called like this: getDateTimeString('\0', '\0', '\0');
// it will give back this: 20161231235959  (YYYYMMDDHHMMSS)
String getDateTimeString(char dateDelimiter, char timeDelimiter,  char dateTimeDelimiter)
{
	String ret = getDateString(dateDelimiter);
	if (dateTimeDelimiter != '\0')
		ret += dateTimeDelimiter;
	ret += getTimeString(timeDelimiter);
	return ret;
}

/********************************************************************************************\
  Match clock event
  \*********************************************************************************************/
boolean matchClockEvent(unsigned long clockEvent, unsigned long clockSet)
{
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

  if (((clockSet >> (16)) & 0xf) == 0x8)     // if weekday nibble has the wildcard value 0x8 (workdays)
    if (weekday() >= 2 and weekday() <= 6)   // and we have a working day today...
    {
      Mask = 0xffffffff  ^ (0xFUL << (16));  // Mask to wipe nibble position.
      clockEvent &= Mask;                    // clear nibble
      clockEvent |= (0x8UL << (16));         // fill with wildcard value 0x8
    }

  if (((clockSet >> (16)) & 0xf) == 0x9)     // if weekday nibble has the wildcard value 0x9 (weekends)
    if (weekday() == 1 or weekday() == 7)    // and we have a weekend day today...
    {
      Mask = 0xffffffff  ^ (0xFUL << (16));  // Mask to wipe nibble position.
      clockEvent &= Mask;                    // clear nibble
      clockEvent |= (0x9UL << (16));         // fill with wildcard value 0x9
    }

  if (clockEvent == clockSet)
    return true;
  return false;
}


/********************************************************************************************\
  Parse string template
  \*********************************************************************************************/

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
              boolean match = false;
              for (byte z = 0; z < VARS_PER_TASK; z++)
                if (valueName.equalsIgnoreCase(ExtraTaskSettings.TaskDeviceValueNames[z]))
                {
                  // here we know the task and value, so find the uservar
                  match = true;
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
              if (!match) // try if this is a get config request
              {
                struct EventStruct TempEvent;
                TempEvent.TaskIndex = y;
                String tmpName = valueName;
                if (PluginCall(PLUGIN_GET_CONFIG, &TempEvent, tmpName))
                  newString += tmpName;
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
  newString.replace(F("%sysname%"), Settings.Name);

  newString.replace(F("%systime%"), getTimeString(':'));

  newString.replace(F("%syshour%"), getHourString());
  newString.replace(F("%sysmin%"), getMinuteString());
  newString.replace(F("%syssec%"), getSecondString());
  newString.replace(F("%sysday%"), getDayString());
  newString.replace(F("%sysmonth%"), getMonthString());
  newString.replace(F("%sysyear%"), getYearString());
  newString.replace(F("%sysyears%"), getYearStringShort());

  newString.replace(F("%uptime%"), String(wdcounter / 2));

#if FEATURE_ADC_VCC
  newString.replace(F("%vcc%"), String(vcc));
#endif

  IPAddress ip = WiFi.localIP();
  char strIP[20];
  sprintf_P(strIP, PSTR("%u.%u.%u.%u"), ip[0], ip[1], ip[2], ip[3]);
  newString.replace(F("%ip%"), strIP);

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
  else
    return 0.0;
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

  if (is_operator(token[0]) && token[1] == 0)
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
  char c, oc, *TokenPos = token;
  char stack[32];       // operator stack
  unsigned int sl = 0;  // stack length
  char     sc;          // used for record stack element
  int error = 0;

  //*sp=0; // bug, it stops calculating after 50 times
  sp = globalstack - 1;
  oc=c=0;
  while (strpos < strend)
  {
    // read one token from the input stream
    oc = c;
    c = *strpos;
    if (c != ' ')
    {
      // If the token is a number (identifier), then add it to the token queue.
      if ((c >= '0' && c <= '9') || c == '.' || (c == '-' && is_operator(oc)))
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

int year()
{
  return 1970 + tm.Year;
}

byte month()
{
	return tm.Month;
}

byte day()
{
	return tm.Day;
}


byte hour()
{
  return tm.Hour;
}

byte minute()
{
  return tm.Minute;
}

byte second()
{
	return tm.Second;
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


/********************************************************************************************\
  Rules processing
  \*********************************************************************************************/
void rulesProcessing(String& event)
{
  unsigned long timer = millis();
  String log = "";

  log = F("EVENT: ");
  log += event;
  addLog(LOG_LEVEL_INFO, log);

  for (byte x = 1; x < RULESETS_MAX + 1; x++)
  {
    #if defined(ESP8266)
      String fileName = F("rules");
    #endif
    #if defined(ESP32)
      String fileName = F("/rules");
    #endif
    fileName += x;
    fileName += F(".txt");
    if (SPIFFS.exists(fileName))
      rulesProcessingFile(fileName, event);
  }

  log = F("EVENT: Processing time:");
  log += millis() - timer;
  log += F(" milliSeconds");
  addLog(LOG_LEVEL_DEBUG, log);

}

/********************************************************************************************\
  Rules processing
  \*********************************************************************************************/
String rulesProcessingFile(String fileName, String& event)
{
  fs::File f = SPIFFS.open(fileName, "r+");
  SPIFFS_CHECK(f, fileName.c_str());

  static byte nestingLevel;
  int data = 0;
  String log = "";

  nestingLevel++;
  if (nestingLevel > RULES_MAX_NESTING_LEVEL)
  {
    log = F("EVENT: Error: Nesting level exceeded!");
    addLog(LOG_LEVEL_ERROR, log);
    nestingLevel--;
    return(log);
  }


  // int pos = 0;
  String line = "";
  boolean match = false;
  boolean codeBlock = false;
  boolean isCommand = false;
  boolean conditional = false;
  boolean condition = false;
  boolean ifBranche = false;

  while (f.available())
  {
    data = f.read();

    SPIFFS_CHECK(data >= 0, fileName.c_str());

    if (data != 10)
      line += char(data);

    if (data == 10)    // if line complete, parse this rule
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
            if (eventTrigger == "*") // wildcard, always process
              match = true;
            else
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
            if (event.charAt(0) == '!')
            {
              action.replace(F("%eventvalue%"), event); // substitute %eventvalue% with literal event string if starting with '!'
            }
            else
            {
              int equalsPos = event.indexOf("=");
              if (equalsPos > 0)
              {
                String tmpString = event.substring(equalsPos + 1);
                action.replace(F("%eventvalue%"), tmpString); // substitute %eventvalue% in actions with the actual value from the event
              }
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
    //pos++;
  }

  nestingLevel--;
  return(String());
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
    int pos = rule.indexOf('#');
    if (pos == -1) // no # sign in rule, use 'wildcard' match...
      tmpEvent = event.substring(0,rule.length());

    if (tmpEvent.equalsIgnoreCase(rule))
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
        if (matchClockEvent(clockEvent, clockSet))
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
      if (RulesTimer[x] <= millis()) // timer finished?
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


void SendValueLogger(byte TaskIndex)
{
  String logger;

  LoadTaskSettings(TaskIndex);
  byte BaseVarIndex = TaskIndex * VARS_PER_TASK;
  byte DeviceIndex = getDeviceIndex(Settings.TaskDeviceNumber[TaskIndex]);
  byte sensorType = Device[DeviceIndex].VType;
  for (byte varNr = 0; varNr < Device[DeviceIndex].ValueCount; varNr++)
  {
    logger += getDateString('-');
    logger += F(" ");
    logger += getTimeString(':');
    logger += F(",");
    logger += Settings.Unit;
    logger += F(",");
    logger += ExtraTaskSettings.TaskDeviceName;
    logger += F(",");
    logger += ExtraTaskSettings.TaskDeviceValueNames[varNr];
    logger += F(",");

    if (sensorType == SENSOR_TYPE_LONG)
      logger += (unsigned long)UserVar[BaseVarIndex] + ((unsigned long)UserVar[BaseVarIndex + 1] << 16);
    else
      logger += String(UserVar[BaseVarIndex + varNr], ExtraTaskSettings.TaskDeviceValueDecimals[varNr]);
    logger += F("\r\n");
  }

  addLog(LOG_LEVEL_DEBUG, logger);

  String filename = F("VALUES.CSV");
  File logFile = SD.open(filename, FILE_WRITE);
  if (logFile)
    logFile.print(logger);
  logFile.close();
}


void checkRAM( const __FlashStringHelper* flashString)
{
  uint32_t freeRAM = FreeMem();

  if (freeRAM < lowestRAM)
  {
    lowestRAM = freeRAM;
    lowestRAMfunction = flashString;
  }
}

#ifdef PLUGIN_BUILD_TESTING

#define isdigit(n) (n >= '0' && n <= '9')

/********************************************************************************************\
  Generate a tone of specified frequency on pin
  \*********************************************************************************************/
void tone(uint8_t _pin, unsigned int frequency, unsigned long duration) {
  analogWriteFreq(frequency);
  //NOTE: analogwrite reserves IRAM and uninitalized ram.
  analogWrite(_pin,100);
  delay(duration);
  analogWrite(_pin,0);
}

/********************************************************************************************\
  Play RTTTL string on specified pin
  \*********************************************************************************************/
void play_rtttl(uint8_t _pin, const char *p )
{
  #define OCTAVE_OFFSET 0
  // FIXME: Absolutely no error checking in here

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


#ifdef FEATURE_ARDUINO_OTA
/********************************************************************************************\
  Allow updating via the Arduino OTA-protocol. (this allows you to upload directly from platformio)
  \*********************************************************************************************/

void ArduinoOTAInit()
{
  // Default port is 8266
  ArduinoOTA.setPort(8266);
	ArduinoOTA.setHostname(Settings.Name);

  if (SecuritySettings.Password[0]!=0)
    ArduinoOTA.setPassword(SecuritySettings.Password);

  ArduinoOTA.onStart([]() {
      Serial.println(F("OTA  : Start upload"));
      SPIFFS.end(); //important, otherwise it fails
  });

  ArduinoOTA.onEnd([]() {
      Serial.println(F("\nOTA  : End"));
      //"dangerous": if you reset during flash you have to reflash via serial
      //so dont touch device until restart is complete
      Serial.println(F("\nOTA  : DO NOT RESET OR POWER OFF UNTIL BOOT+FLASH IS COMPLETE."));
      delay(100);
      #if defined(ESP8266)
        ESP.reset();
      #endif
      #if defined(ESP32)
        ESP.restart();
      #endif
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {

      Serial.printf("OTA  : Progress %u%%\r", (progress / (total / 100)));
  });

  ArduinoOTA.onError([](ota_error_t error) {
      Serial.print(F("\nOTA  : Error (will reboot): "));
      if (error == OTA_AUTH_ERROR) Serial.println(F("Auth Failed"));
      else if (error == OTA_BEGIN_ERROR) Serial.println(F("Begin Failed"));
      else if (error == OTA_CONNECT_ERROR) Serial.println(F("Connect Failed"));
      else if (error == OTA_RECEIVE_ERROR) Serial.println(F("Receive Failed"));
      else if (error == OTA_END_ERROR) Serial.println(F("End Failed"));

      delay(100);
      #if defined(ESP8266)
       ESP.reset();
      #endif
      #if defined(ESP32)
        ESP.restart();
      #endif
  });
  ArduinoOTA.begin();

  String log = F("OTA  : Arduino OTA enabled on port 8266");
  addLog(LOG_LEVEL_INFO, log);

}

#endif

String getBearing(int degrees)
{
  const char* bearing[] = {
    PSTR("N"),
    PSTR("NNE"),
    PSTR("NE"),
    PSTR("ENE"),
    PSTR("E"),
    PSTR("ESE"),
    PSTR("SE"),
    PSTR("SSE"),
    PSTR("S"),
    PSTR("SSW"),
    PSTR("SW"),
    PSTR("WSW"),
    PSTR("W"),
    PSTR("WNW"),
    PSTR("NW"),
    PSTR("NNW")
  };

    return(bearing[int(degrees/22.5)]);

}

//escapes special characters in strings for use in html-forms
void htmlEscape(String & html)
{
  html.replace("&",  "&amp;");
  html.replace("\"", "&quot;");
  html.replace("'",  "&#039;");
  html.replace("<",  "&lt;");
  html.replace(">",  "&gt;");
}


// Compute the dew point temperature, given temperature and humidity (temp in Celcius)
// Formula: http://www.ajdesigner.com/phphumidity/dewpoint_equation_dewpoint_temperature.php
// Td = (f/100)^(1/8) * (112 + 0.9*T) + 0.1*T - 112
float compute_dew_point_temp(float temperature, float humidity_percentage) {
  return pow(humidity_percentage / 100.0, 0.125) *
         (112.0 + 0.9*temperature) + 0.1*temperature - 112.0;
}

// Compute the humidity given temperature and dew point temperature (temp in Celcius)
// Formula: http://www.ajdesigner.com/phphumidity/dewpoint_equation_relative_humidity.php
// f = 100 * ((112 - 0.1*T + Td) / (112 + 0.9 * T))^8
float compute_humidity_from_dewpoint(float temperature, float dew_temperature) {
  return 100.0 * pow((112.0 - 0.1 * temperature + dew_temperature) /
                     (112.0 + 0.9 * temperature), 8);
}
