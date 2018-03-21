// clean up tcp connections that are in TIME_WAIT status, to conserve memory
// In future versions of WiFiClient it should be possible to call abort(), but
// this feature is not in all upstream versions yet.
// See https://github.com/esp8266/Arduino/issues/1923
// and https://github.com/letscontrolit/ESPEasy/issues/253
#if defined(ESP8266)
  #include <md5.h>
#endif
#if defined(ESP8266)

struct tcp_pcb;
extern struct tcp_pcb* tcp_tw_pcbs;
extern "C" void tcp_abort (struct tcp_pcb* pcb);

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

bool readyForSleep()
{
  if (!isDeepSleepEnabled())
    return false;
  return timeOutReached(timerAwakeFromDeepSleep + 1000 * Settings.deepSleep);
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
  ESP.deepSleep((uint32_t)delay * 1000000, WAKE_RF_DEFAULT);
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
  Status LED
\*********************************************************************************************/
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
      long int delta = timePassedSince(gnLastUpdate);
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

    analogWrite(Settings.Pin_status_led, pwm);
  }
}


/********************************************************************************************\
  delay in milliseconds with background processing
  \*********************************************************************************************/
void delayBackground(unsigned long delay)
{
  unsigned long timer = millis() + delay;
  while (!timeOutReached(timer))
    backgroundtasks();
}


/********************************************************************************************\
  Parse a command string to event struct
  \*********************************************************************************************/
void parseCommandString(struct EventStruct *event, const String& string)
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
    fs::FSInfo fs_info;
    SPIFFS.info(fs_info);

    String log = F("FS   : Mount successful, used ");
    log=log+fs_info.usedBytes;
    log=log+F(" bytes of ");
    log=log+fs_info.totalBytes;
    addLog(LOG_LEVEL_INFO, log);

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
  for (byte x = 0; x <= deviceCount ; x++) {
    if (Device[x].Number == Number) {
      return x;
    }
  }
  return 0;
}

/********************************************************************************************\
  Find name of plugin given the plugin device index..
  \*********************************************************************************************/
String getPluginNameFromDeviceIndex(byte deviceIndex) {
  String deviceName = "";
  Plugin_ptr[deviceIndex](PLUGIN_GET_DEVICENAME, 0, deviceName);
  return deviceName;
}


/********************************************************************************************\
  Find protocol index corresponding to protocol setting
  \*********************************************************************************************/
byte getProtocolIndex(byte Number)
{
  for (byte x = 0; x <= protocolCount ; x++) {
    if (Protocol[x].Number == Number) {
      return x;
    }
  }
  return 0;
}

/********************************************************************************************\
  Get notificatoin protocol index (plugin index), by NPlugin_id
  \*********************************************************************************************/
byte getNotificationProtocolIndex(byte Number)
{
  for (byte x = 0; x <= notificationCount ; x++) {
    if (Notification[x].Number == Number) {
      return(x);
    }
  }
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
  Clear custom task settings
  \*********************************************************************************************/
String ClearCustomTaskSettings(int TaskIndex)
{
  // addLog(LOG_LEVEL_DEBUG, F("Clearing custom task settings"));
  return(ClearInFile((char*)FILE_CONFIG, DAT_OFFSET_TASKS + (TaskIndex * DAT_TASKS_SIZE) + DAT_TASKS_CUSTOM_OFFSET, DAT_TASKS_SIZE));
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
  Clear Custom Controller settings
  \*********************************************************************************************/
String ClearCustomControllerSettings(int ControllerIndex)
{
  // addLog(LOG_LEVEL_DEBUG, F("Clearing custom controller settings"));
  return(ClearInFile((char*)FILE_CONFIG, DAT_OFFSET_CUSTOM_CONTROLLER + (ControllerIndex * DAT_CUSTOM_CONTROLLER_SIZE), DAT_CUSTOM_CONTROLLER_SIZE));
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
  Clear a certain area in a file (set to 0)
  \*********************************************************************************************/
String ClearInFile(char* fname, int index, int datasize)
{
  FLASH_GUARD();

  fs::File f = SPIFFS.open(fname, "r+");
  SPIFFS_CHECK(f, fname);

  SPIFFS_CHECK(f.seek(index, fs::SeekSet), fname);
  for (int x = 0; x < datasize ; x++)
  {
    SPIFFS_CHECK(f.write(0), fname);
  }
  f.close();

  //OK
  return String();

}

/********************************************************************************************\
  Load data from config file on SPIFFS
  \*********************************************************************************************/
String LoadFromFile(char* fname, int index, byte* memAddress, int datasize)
{
  checkRAM(F("LoadFromFile"));
  // addLog(LOG_LEVEL_INFO, String(F("FILE : Load size "))+datasize);

  fs::File f = SPIFFS.open(fname, "r+");
  SPIFFS_CHECK(f, fname);
  SPIFFS_CHECK(f.seek(index, fs::SeekSet), fname);
  SPIFFS_CHECK(f.read(memAddress,datasize), fname);
  f.close();

  return(String());
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

  fname=F("rules1.txt");
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

	// advanced Settings
	Settings.UseRules 		= DEFAULT_USE_RULES;

	Settings.MQTTRetainFlag	= DEFAULT_MQTT_RETAIN;
	Settings.MessageDelay	= DEFAULT_MQTT_DELAY;

    Settings.UseNTP			= DEFAULT_USE_NTP;
	strcpy_P(Settings.NTPHost, PSTR(DEFAULT_NTP_HOST));
	Settings.TimeZone		= DEFAULT_TIME_ZONE;
    Settings.DST 			= DEFAULT_USE_DST;

	str2ip((char*)DEFAULT_SYSLOG_IP, Settings.Syslog_IP);

	Settings.SyslogLevel	= DEFAULT_SYSLOG_LEVEL;
	Settings.SerialLogLevel	= DEFAULT_SERIAL_LOG_LEVEL;
	Settings.WebLogLevel	= DEFAULT_WEB_LOG_LEVEL;
	Settings.SDLogLevel		= DEFAULT_SD_LOG_LEVEL;
	Settings.UseValueLogger = DEFAULT_USE_SD_LOG;

	Settings.UseSerial		= DEFAULT_USE_SERIAL;
	Settings.BaudRate		= DEFAULT_SERIAL_BAUD;

/*
	Settings.GlobalSync						= DEFAULT_USE_GLOBAL_SYNC;
	Settings.UDPPort						= DEFAULT_SYNC_UDP_PORT;

	Settings.IP_Octet						= DEFAULT_IP_OCTET;
	Settings.WDI2CAddress					= DEFAULT_WD_IC2_ADDRESS;
	Settings.UseSSDP						= DEFAULT_USE_SSDP;
	Settings.ConnectionFailuresThreshold	= DEFAULT_CON_FAIL_THRES;
	Settings.WireClockStretchLimit			= DEFAULT_I2C_CLOCK_LIMIT;
*/






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
  return system_get_free_heap_size();
}


/********************************************************************************************\
  Check if string is valid float
  \*********************************************************************************************/
boolean isFloat(const String& tBuf) {
  return isNumerical(tBuf, false);
}

boolean isValidFloat(float f) {
  if (f == NAN)      return false; //("NaN");
  if (f == INFINITY) return false; //("INFINITY");
  if (-f == INFINITY)return false; //("-INFINITY");
  if (isnan(f))      return false; //("isnan");
  if (isinf(f))      return false; //("isinf");
  return true;
}

boolean isInt(const String& tBuf) {
  return isNumerical(tBuf, true);
}

boolean isNumerical(const String& tBuf, bool mustBeInteger) {
  boolean decPt = false;
  int firstDec = 0;
  if(tBuf.charAt(0) == '+' || tBuf.charAt(0) == '-')
    firstDec = 1;
  for(unsigned int x=firstDec; x < tBuf.length(); ++x) {
    if(tBuf.charAt(x) == '.') {
      if (mustBeInteger) return false;
      // Only one decimal point allowed
      if(decPt) return false;
      else decPt = true;
    }
    else if(tBuf.charAt(x) < '0' || tBuf.charAt(x) > '9') return false;
  }
  return true;
}

// convert old and new time string to nr of seconds
float timeStringToSeconds(String tBuf) {
	float sec = 0;
	int split = tBuf.indexOf(':');
	if (split < 0) { // assume only hours
		sec += tBuf.toFloat() * 60 * 60;
	} else {
		sec += tBuf.substring(0, split).toFloat() * 60 * 60;
		tBuf = tBuf.substring(split +1);
		split = tBuf.indexOf(':');
		if (split < 0) { //old format
			sec += tBuf.toFloat() * 60;
		} else { //new format
			sec += tBuf.substring(0, split).toFloat() * 60;
			sec += tBuf.substring(split +1).toFloat();
		}
	}
	return sec;
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

bool SerialAvailableForWrite() {
  if (!Settings.UseSerial) return false;
  if (!Serial.availableForWrite()) return false; // UART FIFO overflow or TX disabled.
  return true;
}

boolean loglevelActiveFor(byte destination, byte logLevel) {
  byte logLevelSettings = 0;
  switch (destination) {
    case LOG_TO_SERIAL: {
      if (!SerialAvailableForWrite()) return false;
      logLevelSettings = Settings.SerialLogLevel;
      break;
    }
    case LOG_TO_SYSLOG: {
      logLevelSettings = Settings.SyslogLevel;
      break;
    }
    case LOG_TO_WEBLOG: {
      logLevelSettings = Settings.WebLogLevel;
      break;
    }
    case LOG_TO_SDCARD: {
      logLevelSettings = Settings.SDLogLevel;
      break;
    }
    default:
      return false;
  }
  return loglevelActive(logLevel, logLevelSettings);
}


boolean loglevelActive(byte logLevel, byte logLevelSettings) {
  return (logLevel <= logLevelSettings);
}

void addLog(byte logLevel, const char *line)
{
  if (loglevelActiveFor(LOG_TO_SERIAL, logLevel)) {
    Serial.println(line);
  }
  if (loglevelActiveFor(LOG_TO_SYSLOG, logLevel)) {
    syslog(line);
  }
  if (loglevelActiveFor(LOG_TO_WEBLOG, logLevel)) {
    Logging.add(line);
  }
  if (loglevelActiveFor(LOG_TO_SDCARD, logLevel)) {
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
  ESP.reset();
}


/********************************************************************************************\
  Save RTC struct to RTC memory
  \*********************************************************************************************/
boolean saveToRTC()
{
  if (!system_rtc_mem_write(RTC_BASE_STRUCT, (byte*)&RTC, sizeof(RTC)) || !readFromRTC())
  {
    addLog(LOG_LEVEL_ERROR, F("RTC  : Error while writing to RTC"));
    return(false);
  }
  else
  {
    return(true);
  }
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
  if (!system_rtc_mem_read(RTC_BASE_STRUCT, (byte*)&RTC, sizeof(RTC)))
    return(false);

  if (RTC.ID1 == 0xAA && RTC.ID2 == 0x55)
    return true;
  else
    return false;
}


/********************************************************************************************\
  Save values to RTC memory
\*********************************************************************************************/
boolean saveUserVarToRTC()
{
  //addLog(LOG_LEVEL_DEBUG, F("RTCMEM: saveUserVarToRTC"));
  byte* buffer = (byte*)&UserVar;
  size_t size = sizeof(UserVar);
  uint32 sum = getChecksum(buffer, size);
  boolean ret = system_rtc_mem_write(RTC_BASE_USERVAR, buffer, size);
  ret &= system_rtc_mem_write(RTC_BASE_USERVAR+(size>>2), (byte*)&sum, 4);
  return ret;
}


/********************************************************************************************\
  Read RTC struct from RTC memory
\*********************************************************************************************/
boolean readUserVarFromRTC()
{
  //addLog(LOG_LEVEL_DEBUG, F("RTCMEM: readUserVarFromRTC"));
  byte* buffer = (byte*)&UserVar;
  size_t size = sizeof(UserVar);
  boolean ret = system_rtc_mem_read(RTC_BASE_USERVAR, buffer, size);
  uint32 sumRAM = getChecksum(buffer, size);
  uint32 sumRTC = 0;
  ret &= system_rtc_mem_read(RTC_BASE_USERVAR+(size>>2), (byte*)&sumRTC, 4);
  if (!ret || sumRTC != sumRAM)
  {
    addLog(LOG_LEVEL_ERROR, F("RTC  : Checksum error on reading RTC user var"));
    memset(buffer, 0, size);
  }
  return ret;
}


uint32 getChecksum(byte* buffer, size_t size)
{
  uint32 sum = 0x82662342;   //some magic to avoid valid checksum on new, uninitialized ESP
  for (size_t i=0; i<size; i++)
    sum += buffer[i];
  return sum;
}

/********************************************************************************************\
  Parse string template
  \*********************************************************************************************/

String parseTemplate(String &tmpString, byte lineSize)
{
  String newString = "";
  String tmpStringMid = "";
  newString.reserve(lineSize);

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
      if (rightBracketIndex >= 0)
      {
        tmpStringMid = tmpString.substring(0, rightBracketIndex);
        tmpString = tmpString.substring(rightBracketIndex + 1);
        int hashtagIndex = tmpStringMid.indexOf('#');
        if (hashtagIndex >= 0) {
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
            if (Settings.TaskDeviceEnabled[y])
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

                      int oidx;
                      if ((oidx = valueFormat.indexOf('O'))>=0) // Output
                      {
                        valueFormat.remove(oidx);
                        oidx = valueFormat.indexOf('!'); // inverted or active low
                        float val = value.toFloat();
                        if (oidx >= 0) {
                            valueFormat.remove(oidx);
                      	  value = val == 0 ? " ON" : "OFF";
                        } else {
                      	  value = val == 0 ? "OFF" : " ON";
                        }
                      }

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
        }
      }
      leftBracketIndex = tmpString.indexOf('[');
      count++;
    }
    newString += tmpString;

    if (currentTaskIndex!=255)
      LoadTaskSettings(currentTaskIndex);
  }

  parseSystemVariables(newString, false);
  parseStandardConversions(newString, false);

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
    return(0);
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


void checkRuleSets(){
for (byte x=0; x < RULESETS_MAX; x++){
  #if defined(ESP8266)
    String fileName = F("rules");
  #endif
  #if defined(ESP32)
    String fileName = F("/rules");
  #endif
  fileName += x+1;
  fileName += F(".txt");
  if (SPIFFS.exists(fileName))
    activeRuleSets[x] = true;
  else
    activeRuleSets[x] = false;

  if (Settings.SerialLogLevel == LOG_LEVEL_DEBUG_DEV){
    Serial.print(fileName);
    Serial.print(" ");
    Serial.println(activeRuleSets[x]);
    }
  }
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

  for (byte x = 0; x < RULESETS_MAX; x++)
  {
    #if defined(ESP8266)
      String fileName = F("rules");
    #endif
    #if defined(ESP32)
      String fileName = F("/rules");
    #endif
    fileName += x+1;
    fileName += F(".txt");
    if(activeRuleSets[x])
      rulesProcessingFile(fileName, event);
  }

  log += F(" Processing time:");
  log += timePassedSince(timer);
  log += F(" milliSeconds");
  addLog(LOG_LEVEL_DEBUG, log);

}

/********************************************************************************************\
  Rules processing
  \*********************************************************************************************/
String rulesProcessingFile(String fileName, String& event)
{
  checkRAM(F("rulesProcessingFile"));
  if (Settings.SerialLogLevel == LOG_LEVEL_DEBUG_DEV){
    Serial.print(F("RuleDebug Processing:"));
    Serial.println(fileName);
    Serial.println(F("     flags CMI  parse output:"));
    }
  fs::File f = SPIFFS.open(fileName, "r+");
  SPIFFS_CHECK(f, fileName.c_str());

  static byte nestingLevel = 0;
  int data = 0;
  String log = "";

  nestingLevel++;
  if (nestingLevel > RULES_MAX_NESTING_LEVEL)
  {
    log = F("EVENT: Error: Nesting level exceeded!");
    addLog(LOG_LEVEL_ERROR, log);
    nestingLevel--;
    return (log);
  }

  String line = "";
  boolean match = false;
  boolean codeBlock = false;
  boolean isCommand = false;
  boolean conditional = false;
  boolean condition = false;
  boolean ifBranche = false;

  byte buf[RULES_BUFFER_SIZE];
  int len = 0;
  while (f.available())
  {
    len = f.read((byte*)buf, RULES_BUFFER_SIZE);
    for (int x = 0; x < len; x++) {
      data = buf[x];

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

          if (match || !codeBlock) {
            // only parse [xxx#yyy] if we have a matching ruleblock or need to eval the "on" (no codeBlock)
            // This to avoid waisting CPU time...
            line = parseTemplate(line, line.length());
          }
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
            match = false;
          }

          if (Settings.SerialLogLevel == LOG_LEVEL_DEBUG_DEV){
            Serial.print(F("RuleDebug: "));
            Serial.print(codeBlock);
            Serial.print(match);
            Serial.print(isCommand);
            Serial.print(": ");
            Serial.println(line);
          }

          if (match) // rule matched for one action or a block of actions
          {
            int split = lcAction.indexOf("if "); // check for optional "if" condition
            if (split != -1)
            {
              conditional = true;
              String check = lcAction.substring(split + 3);
              condition = conditionMatchExtended(check);
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
              // Use a tmp string to call PLUGIN_WRITE, since PluginCall may inadvertenly alter the string.
              String tmpAction(action);
              if (!PluginCall(PLUGIN_WRITE, &TempEvent, tmpAction)) {
                if (!tmpAction.equals(action)) {
                  String log = F("PLUGIN_WRITE altered the string: ");
                  log += action;
                  log += F(" to: ");
                  log += tmpAction;
                  addLog(LOG_LEVEL_ERROR, log);
                }
                ExecuteCommand(VALUE_SOURCE_SYSTEM, action.c_str());
              }
              yield();
            }
          }
        }

        line = "";
      }
    }
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
    if (pos == -1) // no # sign in rule, use 'wildcard' match on event 'source'
      {
        tmpEvent = event.substring(0,rule.length());
        tmpRule = rule;
      }

    pos = rule.indexOf('*');
    if (pos != -1) // a * sign in rule, so use a'wildcard' match on message
      {
        tmpEvent = event.substring(0,pos-1);
        tmpRule = rule.substring(0,pos-1);
      }

    if (tmpEvent.equalsIgnoreCase(tmpRule))
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

boolean conditionMatchExtended(String& check) {
	int condAnd = -1;
	int condOr = -1;
	boolean rightcond = false;
	boolean leftcond = conditionMatch(check); // initial check

	do {
		condAnd = check.indexOf(F(" and "));
		condOr  = check.indexOf(F(" or "));

		if (condAnd > 0 || condOr > 0) { // we got AND/OR
			if (condAnd > 0	&& ((condOr < 0 && condOr < condAnd) || (condOr > 0 && condOr > condAnd))) { //AND is first
				check = check.substring(condAnd + 5);
				rightcond = conditionMatch(check);
				leftcond = (leftcond && rightcond);
			} else { //OR is first
				check = check.substring(condOr + 4);
				rightcond = conditionMatch(check);
				leftcond = (leftcond || rightcond);
			}
		}
	} while (condAnd > 0 || condOr > 0);
	return leftcond;
}

boolean conditionMatch(const String& check)
{
  boolean match = false;

  char compare    = ' ';

  int posStart = check.length();
  int posEnd = posStart;
  int comparePos  = 0;

  if ((comparePos = check.indexOf("!="))>0 && comparePos<posStart) {
	  posStart = comparePos;
	  posEnd = posStart+2;
	  compare = '!'+'=';
  }
  if ((comparePos = check.indexOf("<>"))>0 && comparePos<posStart) {
	  posStart = comparePos;
	  posEnd = posStart+2;
	  compare = '!'+'=';
  }
  if ((comparePos = check.indexOf(">="))>0 && comparePos<posStart) {
	  posStart = comparePos;
	  posEnd = posStart+2;
	  compare = '>'+'=';
  }
  if ((comparePos = check.indexOf("<="))>0 && comparePos<posStart) {
	  posStart = comparePos;
	  posEnd = posStart+2;
	  compare = '<'+'=';
  }
  if ((comparePos = check.indexOf("<"))>0 && comparePos<posStart) {
	  posStart = comparePos;
	  posEnd = posStart+1;
	  compare = '<';
  }
  if ((comparePos = check.indexOf(">"))>0 && comparePos<posStart) {
	  posStart = comparePos;
	  posEnd = posStart+1;
	  compare = '>';
  }
  if ((comparePos = check.indexOf("="))>0 && comparePos<posStart) {
	  posStart = comparePos;
	  posEnd = posStart+1;
	  compare = '=';
  }

  float Value1 = 0;
  float Value2 = 0;

  if (compare > ' ')
  {
    String tmpCheck1 = check.substring(0, posStart);
    String tmpCheck2 = check.substring(posEnd);
    if (!isFloat(tmpCheck1) || !isFloat(tmpCheck2)) {
        Value1 = timeStringToSeconds(tmpCheck1);
        Value2 = timeStringToSeconds(tmpCheck2);
    } else {
        Value1 = tmpCheck1.toFloat();
        Value2 = tmpCheck2.toFloat();
    }
  }
  else
    return false;

  switch (compare)
  {
  case '>'+'=':
	  if (Value1 >= Value2)
		  match = true;
	  break;

  case '<'+'=':
	  if (Value1 <= Value2)
		  match = true;
	  break;

  case '!'+'=':
	  if (Value1 != Value2)
		  match = true;
	  break;

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
      if (timeOutReached(RulesTimer[x])) // timer finished?
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
  uint16_t freeRAM = FreeMem();

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
void play_rtttl(uint8_t _pin, char *p )
{
  #define OCTAVE_OFFSET 0
  // Absolutely no error checking in here

  const int notes[] = { 0,
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
      ESP.reset();
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
      ESP.reset();
  });
  ArduinoOTA.begin();

  String log = F("OTA  : Arduino OTA enabled on port 8266");
  addLog(LOG_LEVEL_INFO, log);

}

#endif
