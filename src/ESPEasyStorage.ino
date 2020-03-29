#include "src/Globals/Cache.h"
#include "src/Globals/CRCValues.h"
#include "src/Globals/ResetFactoryDefaultPref.h"
#include "src/Globals/Plugins.h"

/********************************************************************************************\
   SPIFFS error handling
   Look here for error # reference: https://github.com/pellepl/spiffs/blob/master/src/spiffs.h
 \*********************************************************************************************/
String FileError(int line, const char *fname)
{
  String err = F("FS   : Error while reading/writing ");

  err += fname;
  err += F(" in ");
  err += line;
  addLog(LOG_LEVEL_ERROR, err);
  return err;
}

/********************************************************************************************\
   Keep track of number of flash writes.
 \*********************************************************************************************/
void flashCount()
{
  if (RTC.flashDayCounter <= MAX_FLASHWRITES_PER_DAY) {
    RTC.flashDayCounter++;
  }
  RTC.flashCounter++;
  saveToRTC();
}

String flashGuard()
{
  checkRAM(F("flashGuard"));

  if (RTC.flashDayCounter > MAX_FLASHWRITES_PER_DAY)
  {
    String log = F("FS   : Daily flash write rate exceeded! (powercycle to reset this)");
    addLog(LOG_LEVEL_ERROR, log);
    return log;
  }
  flashCount();
  return String();
}

// use this in function that can return an error string. it automaticly returns with an error string if there where too many flash writes.
#define FLASH_GUARD() { String flashErr = flashGuard(); \
                        if (flashErr.length()) return flashErr; }


String appendLineToFile(const String& fname, const String& line) {
  return appendToFile(fname, reinterpret_cast<const uint8_t *>(line.c_str()), line.length());
}

String appendToFile(const String& fname, const uint8_t *data, unsigned int size) {
  fs::File f = tryOpenFile(fname, "a+");

  SPIFFS_CHECK(f,                   fname.c_str());
  SPIFFS_CHECK(f.write(data, size), fname.c_str());
  f.close();
  return "";
}

bool fileExists(const String& fname) {
  return SPIFFS.exists(fname);
}

fs::File tryOpenFile(const String& fname, const String& mode) {
  START_TIMER;
  fs::File f;

  if ((mode == "r") && !fileExists(fname)) {
    return f;
  }
  f = SPIFFS.open(fname, mode.c_str());
  STOP_TIMER(TRY_OPEN_FILE);
  return f;
}

bool tryRenameFile(const String& fname_old, const String& fname_new) {
  if (fileExists(fname_old) && !fileExists(fname_new)) {
    return SPIFFS.rename(fname_old, fname_new);
  }
  return false;
}

bool tryDeleteFile(const String& fname) {
  if (fname.length() > 0)
  {
    bool res = SPIFFS.remove(fname);

    // A call to GarbageCollection() will at most erase a single block. (e.g. 8k block size)
    // A deleted file may have covered more than a single block, so try to clear multiple blocks.
    uint8_t retries = 3;

    while (retries > 0 && GarbageCollection()) {
      --retries;
    }
    return res;
  }
  return false;
}

/********************************************************************************************\
   Fix stuff to clear out differences between releases
 \*********************************************************************************************/
String BuildFixes()
{
  checkRAM(F("BuildFixes"));
  serialPrintln(F("\nBuild changed!"));

  if (Settings.Build < 145)
  {
    String   fname = F(FILE_NOTIFICATION);
    fs::File f     = tryOpenFile(fname, "w");
    SPIFFS_CHECK(f, fname.c_str());

    if (f)
    {
      for (int x = 0; x < 4096; x++)
      {
        // See https://github.com/esp8266/Arduino/commit/b1da9eda467cc935307d553692fdde2e670db258#r32622483
        uint8_t zero_value = 0;
        SPIFFS_CHECK(f.write(&zero_value, 1), fname.c_str());
      }
      f.close();
    }
  }

  if (Settings.Build < 20101)
  {
    serialPrintln(F("Fix reset Pin"));
    Settings.Pin_Reset = -1;
  }

  if (Settings.Build < 20102) {
    // Settings were 'mangled' by using older version
    // Have to patch settings to make sure no bogus data is being used.
    serialPrintln(F("Fix settings with uninitalized data or corrupted by switching between versions"));
    Settings.UseRTOSMultitasking       = false;
    Settings.Pin_Reset                 = -1;
    Settings.SyslogFacility            = DEFAULT_SYSLOG_FACILITY;
    Settings.MQTTUseUnitNameAsClientId = DEFAULT_MQTT_USE_UNITNAME_AS_CLIENTID;
    Settings.StructSize                = sizeof(Settings);
  }

  if (Settings.Build < 20103) {
    Settings.ResetFactoryDefaultPreference = 0;
    Settings.OldRulesEngine(DEFAULT_RULES_OLDENGINE);
  }
  if (Settings.Build < 20105) {
    Settings.I2C_clockSpeed = 400000;
  }

  Settings.Build = BUILD;
  return SaveSettings();
}

/********************************************************************************************\
   Mount FS and check config.dat
 \*********************************************************************************************/
void fileSystemCheck()
{
  checkRAM(F("fileSystemCheck"));
  addLog(LOG_LEVEL_INFO, F("FS   : Mounting..."));

  if (SPIFFS.begin())
  {
    #if defined(ESP8266)
    fs::FSInfo fs_info;
    SPIFFS.info(fs_info);

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = F("FS   : Mount successful, used ");
      log = log + fs_info.usedBytes;
      log = log + F(" bytes of ");
      log = log + fs_info.totalBytes;
      addLog(LOG_LEVEL_INFO, log);
    }

    // Run garbage collection before any file is open.
    uint8_t retries = 3;

    while (retries > 0 && GarbageCollection()) {
      --retries;
    }
    #endif // if defined(ESP8266)

    fs::File f = tryOpenFile(FILE_CONFIG, "r");

    if (!f)
    {
      ResetFactory();
    }

    if (f) { f.close(); }
  }
  else
  {
    String log = F("FS   : Mount failed");
    serialPrintln(log);
    addLog(LOG_LEVEL_ERROR, log);
    ResetFactory();
  }
}

/********************************************************************************************\
   Garbage collection
 \*********************************************************************************************/
bool GarbageCollection() {
  #ifdef CORE_POST_2_6_0

  // Perform garbage collection
  START_TIMER;

  if (SPIFFS.gc()) {
    addLog(LOG_LEVEL_INFO, F("FS   : Success garbage collection"));
    STOP_TIMER(SPIFFS_GC_SUCCESS);
    return true;
  }
  STOP_TIMER(SPIFFS_GC_FAIL);
  return false;
  #else // ifdef CORE_POST_2_6_0

  // Not supported, so nothing was removed.
  return false;
  #endif // ifdef CORE_POST_2_6_0
}

/********************************************************************************************\
   Save settings to SPIFFS
 \*********************************************************************************************/
String SaveSettings(void)
{
  checkRAM(F("SaveSettings"));
  MD5Builder md5;
  uint8_t    tmp_md5[16] = { 0 };
  String     err;

  Settings.StructSize = sizeof(Settings);

  // FIXME @TD-er: As discussed in #1292, the CRC for the settings is now disabled.

  /*
     memcpy( Settings.ProgmemMd5, CRCValues.runTimeMD5, 16);
     md5.begin();
     md5.add((uint8_t *)&Settings, sizeof(Settings)-16);
     md5.calculate();
     md5.getBytes(tmp_md5);
     if (memcmp(tmp_md5, Settings.md5, 16) != 0) {
      // Settings have changed, save to file.
      memcpy(Settings.md5, tmp_md5, 16);
   */
  Settings.validate();
  err = SaveToFile((char *)FILE_CONFIG, 0, (byte *)&Settings, sizeof(Settings));

  if (err.length()) {
    return err;
  }

  // Must check this after saving, or else it is not possible to fix multiple
  // issues which can only corrected on different pages.
  if (!SettingsCheck(err)) { return err; }

  //  }

  SecuritySettings.validate();
  memcpy(SecuritySettings.ProgmemMd5, CRCValues.runTimeMD5, 16);
  md5.begin();
  md5.add((uint8_t *)&SecuritySettings, sizeof(SecuritySettings) - 16);
  md5.calculate();
  md5.getBytes(tmp_md5);

  if (memcmp(tmp_md5, SecuritySettings.md5, 16) != 0) {
    // Settings have changed, save to file.
    memcpy(SecuritySettings.md5, tmp_md5, 16);
    err = SaveToFile((char *)FILE_SECURITY, 0, (byte *)&SecuritySettings, sizeof(SecuritySettings));

    if (WifiIsAP(WiFi.getMode())) {
      // Security settings are saved, may be update of WiFi settings or hostname.
      wifiSetupConnect         = true;
      wifiConnectAttemptNeeded = true;
    }
  }
  afterloadSettings();
  return err;
}

void afterloadSettings() {
  ExtraTaskSettings.clear(); // make sure these will not contain old settings.
  ResetFactoryDefaultPreference_struct pref(Settings.ResetFactoryDefaultPreference);
  DeviceModel model = pref.getDeviceModel();

  // TODO TD-er: Try to get the information from more locations to make it more persistent
  // Maybe EEPROM location?

  if (modelMatchingFlashSize(model)) {
    ResetFactoryDefaultPreference = Settings.ResetFactoryDefaultPreference;
  }
  msecTimerHandler.setEcoMode(Settings.EcoPowerMode());

  if (!Settings.UseRules) {
    eventQueue.clear();
  }
  set_mDNS(); // To update changes in hostname.
}

/********************************************************************************************\
   Load settings from SPIFFS
 \*********************************************************************************************/
String LoadSettings()
{
  checkRAM(F("LoadSettings"));
  String  err;
  uint8_t calculatedMd5[16];
  MD5Builder md5;

  err = LoadFromFile((char *)FILE_CONFIG, 0, (byte *)&Settings, sizeof(SettingsStruct));

  if (err.length()) {
    return err;
  }
  Settings.validate();

  // FIXME @TD-er: As discussed in #1292, the CRC for the settings is now disabled.

  /*
     if (Settings.StructSize > 16) {
      md5.begin();
      md5.add((uint8_t *)&Settings, Settings.StructSize -16);
      md5.calculate();
      md5.getBytes(calculatedMd5);
     }
     if (memcmp (calculatedMd5, Settings.md5,16)==0){
      addLog(LOG_LEVEL_INFO,  F("CRC  : Settings CRC           ...OK"));
      if (memcmp(Settings.ProgmemMd5, CRCValues.runTimeMD5, 16)!=0)
        addLog(LOG_LEVEL_INFO, F("CRC  : binary has changed since last save of Settings"));
     }
     else{
      addLog(LOG_LEVEL_ERROR, F("CRC  : Settings CRC           ...FAIL"));
     }
   */

  err = LoadFromFile((char *)FILE_SECURITY, 0, (byte *)&SecuritySettings, sizeof(SecurityStruct));
  md5.begin();
  md5.add((uint8_t *)&SecuritySettings, sizeof(SecuritySettings) - 16);
  md5.calculate();
  md5.getBytes(calculatedMd5);

  if (memcmp(calculatedMd5, SecuritySettings.md5, 16) == 0) {
    addLog(LOG_LEVEL_INFO, F("CRC  : SecuritySettings CRC   ...OK "));

    if (memcmp(SecuritySettings.ProgmemMd5, CRCValues.runTimeMD5, 16) != 0) {
      addLog(LOG_LEVEL_INFO, F("CRC  : binary has changed since last save of Settings"));
    }
  }
  else {
    addLog(LOG_LEVEL_ERROR, F("CRC  : SecuritySettings CRC   ...FAIL"));
  }

  //  setupStaticIPconfig();
  // FIXME TD-er: Must check if static/dynamic IP was changed and trigger a reconnect? Or is a reboot better when changing those settings?
  afterloadSettings();
  SecuritySettings.validate();
  return err;
}

/********************************************************************************************\
   Disable Plugin, based on bootFailedCount
 \*********************************************************************************************/
byte disablePlugin(byte bootFailedCount) {
  for (taskIndex_t i = 0; i < TASKS_MAX && bootFailedCount > 0; ++i) {
    if (Settings.TaskDeviceEnabled[i]) {
      --bootFailedCount;

      if (bootFailedCount == 0) {
        Settings.TaskDeviceEnabled[i] = false;
      }
    }
  }
  return bootFailedCount;
}

/********************************************************************************************\
   Disable Controller, based on bootFailedCount
 \*********************************************************************************************/
byte disableController(byte bootFailedCount) {
  for (controllerIndex_t i = 0; i < CONTROLLER_MAX && bootFailedCount > 0; ++i) {
    if (Settings.ControllerEnabled[i]) {
      --bootFailedCount;

      if (bootFailedCount == 0) {
        Settings.ControllerEnabled[i] = false;
      }
    }
  }
  return bootFailedCount;
}

/********************************************************************************************\
   Disable Notification, based on bootFailedCount
 \*********************************************************************************************/
byte disableNotification(byte bootFailedCount) {
  for (byte i = 0; i < NOTIFICATION_MAX && bootFailedCount > 0; ++i) {
    if (Settings.NotificationEnabled[i]) {
      --bootFailedCount;

      if (bootFailedCount == 0) {
        Settings.NotificationEnabled[i] = false;
      }
    }
  }
  return bootFailedCount;
}

#include "src/DataStructs/StorageLayout.h"

/********************************************************************************************\
   Offsets in settings files
 \*********************************************************************************************/
bool getSettingsParameters(SettingsType settingsType, int index, int& max_index, int& offset, int& max_size, int& struct_size) {
  // The defined offsets should be used with () just in case they are the result of a formula in the defines.
  struct_size = 0;

  switch (settingsType) {
    case BasicSettings_Type:
    {
      max_index   = 1;
      offset      = 0;
      max_size    = (DAT_BASIC_SETTINGS_SIZE);
      struct_size = sizeof(SettingsStruct);
      break;
    }
    case TaskSettings_Type:
    {
      max_index   = TASKS_MAX;
      offset      = (DAT_OFFSET_TASKS) + (index * (DAT_TASKS_DISTANCE));
      max_size    = DAT_TASKS_SIZE;
      struct_size = sizeof(ExtraTaskSettingsStruct);
      break;
    }
    case CustomTaskSettings_Type:
    {
      getSettingsParameters(TaskSettings_Type, index, max_index, offset, max_size, struct_size);
      offset  += (DAT_TASKS_CUSTOM_OFFSET);
      max_size = DAT_TASKS_CUSTOM_SIZE;
      break;

      // struct_size may differ.
    }
    case ControllerSettings_Type:
    {
      max_index   = CONTROLLER_MAX;
      offset      = (DAT_OFFSET_CONTROLLER) + (index * (DAT_CONTROLLER_SIZE));
      max_size    = DAT_CONTROLLER_SIZE;
      struct_size = sizeof(ControllerSettingsStruct);
      break;
    }
    case CustomControllerSettings_Type:
    {
      max_index = CONTROLLER_MAX;
      offset    = (DAT_OFFSET_CUSTOM_CONTROLLER) + (index * (DAT_CUSTOM_CONTROLLER_SIZE));
      max_size  = DAT_CUSTOM_CONTROLLER_SIZE;

      // struct_size may differ.
    }    break;
    case NotificationSettings_Type:
    {
      max_index   = NOTIFICATION_MAX;
      offset      = index * (DAT_NOTIFICATION_SIZE);
      max_size    = DAT_NOTIFICATION_SIZE;
      struct_size = sizeof(NotificationSettingsStruct);
      break;
    }
    default:
    {
      max_index = -1;
      offset    = -1;
      return false;
    }
  }
  return index >= 0 && index < max_index;
}

int getMaxFilePos(SettingsType settingsType) {
  int max_index, offset, max_size;
  int struct_size = 0;

  getSettingsParameters(settingsType, 0,          max_index, offset, max_size, struct_size);
  getSettingsParameters(settingsType, max_index - 1, offset, max_size);
  return offset + max_size - 1;
}

int getFileSize(SettingsType settingsType) {
  if (settingsType == NotificationSettings_Type) {
    return getMaxFilePos(settingsType);
  }

  int max_file_pos = 0;

  for (int st = 0; st < SettingsType_MAX; ++st) {
    int filePos = getMaxFilePos(static_cast<SettingsType>(st));

    if (filePos > max_file_pos) {
      max_file_pos = filePos;
    }
  }
  return max_file_pos;
}

bool getAndLogSettingsParameters(bool read, SettingsType settingsType, int index, int& offset, int& max_size) {
#ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG_DEV)) {
    String log = read ? F("Read") : F("Write");
    log += F(" settings: ");
    log += getSettingsTypeString(settingsType);
    log += F(" index: ");
    log += index;
    addLog(LOG_LEVEL_DEBUG_DEV, log);
  }
#endif // ifndef BUILD_NO_DEBUG
  return getSettingsParameters(settingsType, index, offset, max_size);
}

bool getSettingsParameters(SettingsType settingsType, int index, int& offset, int& max_size) {
  int max_index = -1;
  int struct_size;

  if (!getSettingsParameters(settingsType, index, max_index, offset, max_size, struct_size)) {
    return false;
  }

  if ((index >= 0) && (index < max_index)) { return true; }
  offset = -1;
  return false;
}

/********************************************************************************************\
   Save Task settings to SPIFFS
 \*********************************************************************************************/
String SaveTaskSettings(taskIndex_t TaskIndex)
{
  checkRAM(F("SaveTaskSettings"));

  if (ExtraTaskSettings.TaskIndex != TaskIndex) {
    return F("SaveTaskSettings taskIndex does not match");
  }
  String err = SaveToFile(TaskSettings_Type,
                          TaskIndex,
                          (char *)FILE_CONFIG,
                          (byte *)&ExtraTaskSettings,
                          sizeof(struct ExtraTaskSettingsStruct));

  if (err.length() == 0) {
    err = checkTaskSettings(TaskIndex);
  }
  return err;
}

/********************************************************************************************\
   Load Task settings from SPIFFS
 \*********************************************************************************************/
String LoadTaskSettings(taskIndex_t TaskIndex)
{
  if (ExtraTaskSettings.TaskIndex == TaskIndex) {
    return String(); // already loaded
  }

  if (!validTaskIndex(TaskIndex)) {
    return String(); // Un-initialized task index.
  }
  checkRAM(F("LoadTaskSettings"));

  START_TIMER
  ExtraTaskSettings.clear();
  String result = "";
  result =
    LoadFromFile(TaskSettings_Type, TaskIndex, (char *)FILE_CONFIG, (byte *)&ExtraTaskSettings, sizeof(struct ExtraTaskSettingsStruct));

  // After loading, some settings may need patching.
  ExtraTaskSettings.TaskIndex = TaskIndex; // Needed when an empty task was requested

  if (ExtraTaskSettings.TaskDeviceValueNames[0][0] == 0) {
    // if field set empty, reload defaults
    struct EventStruct TempEvent;
    TempEvent.TaskIndex = TaskIndex;
    String tmp;

    // the plugin call should populate ExtraTaskSettings with its default values.
    PluginCall(PLUGIN_GET_DEVICEVALUENAMES, &TempEvent, tmp);
  }
  ExtraTaskSettings.validate();
  STOP_TIMER(LOAD_TASK_SETTINGS);

  return result;
}

/********************************************************************************************\
   Save Custom Task settings to SPIFFS
 \*********************************************************************************************/
String SaveCustomTaskSettings(taskIndex_t TaskIndex, byte *memAddress, int datasize)
{
  checkRAM(F("SaveCustomTaskSettings"));
  return SaveToFile(CustomTaskSettings_Type, TaskIndex, (char *)FILE_CONFIG, memAddress, datasize);
}

/********************************************************************************************\
   Save array of Strings to Custom Task settings
   Use maxStringLength = 0 to optimize for size (strings will be concatenated)
 \*********************************************************************************************/
String SaveCustomTaskSettings(taskIndex_t TaskIndex, String strings[], uint16_t nrStrings, uint16_t maxStringLength)
{
  checkRAM(F("SaveCustomTaskSettings"));
  const uint16_t bufferSize = 128;

  // FIXME TD-er: For now stack allocated, may need to be heap allocated?
  byte buffer[bufferSize];

  String   result;
  int      writePos        = 0;
  uint16_t stringCount     = 0;
  uint16_t stringReadPos   = 0;
  uint16_t nextStringPos   = 0;
  uint16_t curStringLength = 0;

  if (maxStringLength != 0) {
    // Specified string length, check given strings
    for (int i = 0; i < nrStrings; ++i) {
      if (strings[i].length() >= maxStringLength) {
        result += getCustomTaskSettingsError(i);
      }
    }
  }

  while (stringCount < nrStrings && writePos < DAT_TASKS_CUSTOM_SIZE) {
    ZERO_FILL(buffer);

    for (int i = 0; i < bufferSize && stringCount < nrStrings; ++i) {
      if (stringReadPos == 0) {
        // We're at the start of a string
        curStringLength = strings[stringCount].length();

        if (maxStringLength != 0) {
          if (curStringLength >= maxStringLength) {
            curStringLength = maxStringLength - 1;
          }
        }
      }

      uint16_t curPos = writePos + i;

      if (curPos >= nextStringPos) {
        if (stringReadPos < curStringLength) {
          buffer[i] = strings[stringCount][stringReadPos];
          ++stringReadPos;
        } else {
          buffer[i]     = 0;
          stringReadPos = 0;
          ++stringCount;

          if (maxStringLength == 0) {
            nextStringPos += curStringLength + 1;
          } else {
            nextStringPos += maxStringLength;
          }
        }
      }
    }

    // Buffer is filled, now write to flash
    // As we write in parts, only count as single write.
    if (RTC.flashDayCounter > 0) {
      RTC.flashDayCounter--;
    }
    result   += SaveToFile(CustomTaskSettings_Type, TaskIndex, (char *)FILE_CONFIG, &(buffer[0]), bufferSize, writePos);
    writePos += bufferSize;
  }

  if ((writePos >= DAT_TASKS_CUSTOM_SIZE) && (stringCount < nrStrings)) {
    result += F("Error: Not all strings fit in custom task settings.");
  }
  return result;
}

String getCustomTaskSettingsError(byte varNr) {
  String error = F("Error: Text too long for line ");

  error += varNr + 1;
  error += '\n';
  return error;
}

/********************************************************************************************\
   Clear custom task settings
 \*********************************************************************************************/
String ClearCustomTaskSettings(taskIndex_t TaskIndex)
{
  // addLog(LOG_LEVEL_DEBUG, F("Clearing custom task settings"));
  return ClearInFile(CustomTaskSettings_Type, TaskIndex, (char *)FILE_CONFIG);
}

/********************************************************************************************\
   Load Custom Task settings from SPIFFS
 \*********************************************************************************************/
String LoadCustomTaskSettings(taskIndex_t TaskIndex, byte *memAddress, int datasize)
{
  START_TIMER;
  checkRAM(F("LoadCustomTaskSettings"));
  String result = LoadFromFile(CustomTaskSettings_Type, TaskIndex, (char *)FILE_CONFIG, memAddress, datasize);
  STOP_TIMER(LOAD_CUSTOM_TASK_STATS);
  return result;
}

/********************************************************************************************\
   Load array of Strings from Custom Task settings
   Use maxStringLength = 0 to optimize for size (strings will be concatenated)
 \*********************************************************************************************/
String LoadCustomTaskSettings(taskIndex_t TaskIndex, String strings[], uint16_t nrStrings, uint16_t maxStringLength)
{
  START_TIMER;
  checkRAM(F("LoadCustomTaskSettings"));

  const uint16_t bufferSize = 128;

  // FIXME TD-er: For now stack allocated, may need to be heap allocated?
  if (maxStringLength >= bufferSize) { return F("Max 128 chars allowed"); }
  char buffer[bufferSize];

  String   result;
  uint16_t readPos       = 0;
  uint16_t nextStringPos = 0;
  uint16_t stringCount   = 0;
  String   tmpString;
  tmpString.reserve(bufferSize);

  while (stringCount < nrStrings && readPos < DAT_TASKS_CUSTOM_SIZE) {
    result += LoadFromFile(CustomTaskSettings_Type,
                           TaskIndex,
                           (char *)FILE_CONFIG,
                           (byte *)&buffer,
                           bufferSize,
                           readPos);

    for (int i = 0; i < bufferSize && stringCount < nrStrings; ++i) {
      uint16_t curPos = readPos + i;

      if (curPos >= nextStringPos) {
        if (buffer[i] == 0) {
          if (maxStringLength != 0) {
            // Specific string length, so we have to set the next string position.
            nextStringPos += maxStringLength;
          }
          strings[stringCount] = tmpString;
          tmpString            = "";
          tmpString.reserve(bufferSize);
          ++stringCount;
        } else {
          tmpString += buffer[i];
        }
      }
    }
    readPos += bufferSize;
  }

  if ((tmpString.length() != 0) && (stringCount < nrStrings)) {
    result              += F("Incomplete custom settings for task ");
    result              += (TaskIndex + 1);
    strings[stringCount] = tmpString;
  }
  STOP_TIMER(LOAD_CUSTOM_TASK_STATS);
  return result;
}

/********************************************************************************************\
   Save Controller settings to SPIFFS
 \*********************************************************************************************/
String SaveControllerSettings(controllerIndex_t ControllerIndex, ControllerSettingsStruct& controller_settings)
{
  checkRAM(F("SaveControllerSettings"));
  controller_settings.validate(); // Make sure the saved controller settings have proper values.
  return SaveToFile(ControllerSettings_Type, ControllerIndex,
                    (char *)FILE_CONFIG, (byte *)&controller_settings, sizeof(controller_settings));
}

/********************************************************************************************\
   Load Controller settings to SPIFFS
 \*********************************************************************************************/
String LoadControllerSettings(controllerIndex_t ControllerIndex, ControllerSettingsStruct& controller_settings) {
  checkRAM(F("LoadControllerSettings"));
  String result =
    LoadFromFile(ControllerSettings_Type, ControllerIndex,
                 (char *)FILE_CONFIG, (byte *)&controller_settings, sizeof(controller_settings));
  controller_settings.validate(); // Make sure the loaded controller settings have proper values.
  return result;
}

/********************************************************************************************\
   Clear Custom Controller settings
 \*********************************************************************************************/
String ClearCustomControllerSettings(controllerIndex_t ControllerIndex)
{
  checkRAM(F("ClearCustomControllerSettings"));

  // addLog(LOG_LEVEL_DEBUG, F("Clearing custom controller settings"));
  return ClearInFile(CustomControllerSettings_Type, ControllerIndex, (char *)FILE_CONFIG);
}

/********************************************************************************************\
   Save Custom Controller settings to SPIFFS
 \*********************************************************************************************/
String SaveCustomControllerSettings(controllerIndex_t ControllerIndex, byte *memAddress, int datasize)
{
  checkRAM(F("SaveCustomControllerSettings"));
  return SaveToFile(CustomControllerSettings_Type, ControllerIndex, (char *)FILE_CONFIG, memAddress, datasize);
}

/********************************************************************************************\
   Load Custom Controller settings to SPIFFS
 \*********************************************************************************************/
String LoadCustomControllerSettings(controllerIndex_t ControllerIndex, byte *memAddress, int datasize)
{
  checkRAM(F("LoadCustomControllerSettings"));
  return LoadFromFile(CustomControllerSettings_Type, ControllerIndex, (char *)FILE_CONFIG, memAddress, datasize);
}

/********************************************************************************************\
   Save Controller settings to SPIFFS
 \*********************************************************************************************/
String SaveNotificationSettings(int NotificationIndex, byte *memAddress, int datasize)
{
  checkRAM(F("SaveNotificationSettings"));
  return SaveToFile(NotificationSettings_Type, NotificationIndex, (char *)FILE_NOTIFICATION, memAddress, datasize);
}

/********************************************************************************************\
   Load Controller settings to SPIFFS
 \*********************************************************************************************/
String LoadNotificationSettings(int NotificationIndex, byte *memAddress, int datasize)
{
  checkRAM(F("LoadNotificationSettings"));
  return LoadFromFile(NotificationSettings_Type, NotificationIndex, (char *)FILE_NOTIFICATION, memAddress, datasize);
}

/********************************************************************************************\
   Init a file with zeros on SPIFFS
 \*********************************************************************************************/
String InitFile(const char *fname, int datasize)
{
  checkRAM(F("InitFile"));
  FLASH_GUARD();

  fs::File f = tryOpenFile(fname, "w");

  if (f) {
    SPIFFS_CHECK(f, fname);

    for (int x = 0; x < datasize; x++)
    {
      // See https://github.com/esp8266/Arduino/commit/b1da9eda467cc935307d553692fdde2e670db258#r32622483
      uint8_t zero_value = 0;
      SPIFFS_CHECK(f.write(&zero_value, 1), fname);
    }
    f.close();
  }

  // OK
  return String();
}

/********************************************************************************************\
   Save data into config file on SPIFFS
 \*********************************************************************************************/
String SaveToFile(const char *fname, int index, const byte *memAddress, int datasize)
{
  return doSaveToFile(fname, index, memAddress, datasize, "r+");
}

// See for mode description: https://github.com/esp8266/Arduino/blob/master/doc/filesystem.rst
String doSaveToFile(const char *fname, int index, const byte *memAddress, int datasize, const char *mode)
{
#ifndef ESP32

  if (allocatedOnStack(memAddress)) {
    String log = F("SaveToFile: ");
    log += fname;
    log += F(" ERROR, Data allocated on stack");
    addLog(LOG_LEVEL_ERROR, log);

    //    return log;  // FIXME TD-er: Should this be considered a breaking error?
  }
#endif // ifndef ESP32

  if (index < 0) {
    String log = F("SaveToFile: ");
    log += fname;
    log += F(" ERROR, invalid position in file");
    addLog(LOG_LEVEL_ERROR, log);
    return log;
  }
  START_TIMER;
  checkRAM(F("SaveToFile"));
  FLASH_GUARD();

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("SaveToFile: free stack: ");
    log += getCurrentFreeStack();
    addLog(LOG_LEVEL_INFO, log);
  }
  delay(1);
  unsigned long timer = millis() + 50;
  fs::File f          = tryOpenFile(fname, mode);

  if (f) {
    clearAllCaches();
    SPIFFS_CHECK(f,                          fname);
    SPIFFS_CHECK(f.seek(index, fs::SeekSet), fname);
    const byte *pointerToByteToSave = memAddress;

    for (int x = 0; x < datasize; x++)
    {
      // See https://github.com/esp8266/Arduino/commit/b1da9eda467cc935307d553692fdde2e670db258#r32622483
      uint8_t byteToSave = *pointerToByteToSave;
      SPIFFS_CHECK(f.write(&byteToSave, 1), fname);
      pointerToByteToSave++;

      if (x % 256 == 0) {
        // one page written, do some background tasks
        timer = millis() + 50;
        delay(0);
      }

      if (timeOutReached(timer)) {
        timer += 50;
        delay(0);
      }
    }
    f.close();

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = F("FILE : Saved ");
      log = log + fname;
      addLog(LOG_LEVEL_INFO, log);
    }
  } else {
    String log = F("SaveToFile: ");
    log += fname;
    log += F(" ERROR, Cannot save to file");
    addLog(LOG_LEVEL_ERROR, log);
    return log;
  }
  STOP_TIMER(SAVEFILE_STATS);

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("SaveToFile: free stack after: ");
    log += getCurrentFreeStack();
    addLog(LOG_LEVEL_INFO, log);
  }

  // OK
  return String();
}

/********************************************************************************************\
   Clear a certain area in a file (set to 0)
 \*********************************************************************************************/
String ClearInFile(char *fname, int index, int datasize)
{
  if (index < 0) {
    String log = F("ClearInFile: ");
    log += fname;
    log += F(" ERROR, invalid position in file");
    addLog(LOG_LEVEL_ERROR, log);
    return log;
  }

  checkRAM(F("ClearInFile"));
  FLASH_GUARD();

  fs::File f = tryOpenFile(fname, "r+");

  if (f) {
    SPIFFS_CHECK(f,                          fname);

    SPIFFS_CHECK(f.seek(index, fs::SeekSet), fname);

    for (int x = 0; x < datasize; x++)
    {
      // See https://github.com/esp8266/Arduino/commit/b1da9eda467cc935307d553692fdde2e670db258#r32622483
      uint8_t zero_value = 0;
      SPIFFS_CHECK(f.write(&zero_value, 1), fname);
    }
    f.close();
  } else {
    String log = F("ClearInFile: ");
    log += fname;
    log += F(" ERROR, Cannot save to file");
    addLog(LOG_LEVEL_ERROR, log);
    return log;
  }

  // OK
  return String();
}

/********************************************************************************************\
   Load data from config file on SPIFFS
 \*********************************************************************************************/
String LoadFromFile(char *fname, int offset, byte *memAddress, int datasize)
{
  if (offset < 0) {
    String log = F("LoadFromFile: ");
    log += fname;
    log += F(" ERROR, invalid position in file");
    addLog(LOG_LEVEL_ERROR, log);
    return log;
  }
  delay(1);
  START_TIMER;

  checkRAM(F("LoadFromFile"));
  fs::File f = tryOpenFile(fname, "r");
  SPIFFS_CHECK(f,                            fname);
  SPIFFS_CHECK(f.seek(offset, fs::SeekSet),  fname);
  SPIFFS_CHECK(f.read(memAddress, datasize), fname);
  f.close();

  STOP_TIMER(LOADFILE_STATS);
  delay(1);

  return String();
}

/********************************************************************************************\
   Wrapper functions to handle errors in accessing settings
 \*********************************************************************************************/
String getSettingsFileIndexRangeError(bool read, SettingsType settingsType, int index) {
  if (settingsType >= SettingsType_MAX) {
    String error = F("Unknown settingsType: ");
    error += static_cast<int>(settingsType);
    return error;
  }
  String error = read ? F("Load") : F("Save");
  error += getSettingsTypeString(settingsType);
  error += F(" index out of range: ");
  error += index;
  return error;
}

String getSettingsFileDatasizeError(bool read, SettingsType settingsType, int index, int datasize, int max_size) {
  String error = read ? F("Load") : F("Save");

  error += getSettingsTypeString(settingsType);
  error += '(';
  error += index;
  error += F(") datasize(");
  error += datasize;
  error += F(") > max_size(");
  error += max_size;
  error += ')';
  return error;
}

String LoadFromFile(SettingsType settingsType, int index, char *fname, byte *memAddress, int datasize, int offset_in_block) {
  bool read = true;
  int  offset, max_size;

  if (!getAndLogSettingsParameters(read, settingsType, index, offset, max_size)) {
    return getSettingsFileIndexRangeError(read, settingsType, index);
  }

  if ((datasize + offset_in_block) > max_size) {
    return getSettingsFileDatasizeError(read, settingsType, index, datasize, max_size);
  }
  return LoadFromFile(fname, (offset + offset_in_block), memAddress, datasize);
}

String LoadFromFile(SettingsType settingsType, int index, char *fname, byte *memAddress, int datasize) {
  return LoadFromFile(settingsType, index, fname, memAddress, datasize, 0);
}

String SaveToFile(SettingsType settingsType, int index, char *fname, byte *memAddress, int datasize) {
  return SaveToFile(settingsType, index, fname, memAddress, datasize, 0);
}

String SaveToFile(SettingsType settingsType, int index, char *fname, byte *memAddress, int datasize, int posInBlock) {
  bool read = false;
  int  offset, max_size;

  if (!getAndLogSettingsParameters(read, settingsType, index, offset, max_size)) {
    return getSettingsFileIndexRangeError(read, settingsType, index);
  }

  if ((datasize > max_size) || ((posInBlock + datasize) > max_size)) {
    return getSettingsFileDatasizeError(read, settingsType, index, datasize, max_size);
  }
  return SaveToFile(fname, offset + posInBlock, memAddress, datasize);
}

String ClearInFile(SettingsType settingsType, int index, char *fname) {
  bool read = false;
  int  offset, max_size;

  if (!getAndLogSettingsParameters(read, settingsType, index, offset, max_size)) {
    return getSettingsFileIndexRangeError(read, settingsType, index);
  }
  return ClearInFile(fname, offset, max_size);
}

/********************************************************************************************\
   Check SPIFFS area settings
 \*********************************************************************************************/
int SpiffsSectors()
{
  checkRAM(F("SpiffsSectors"));
  #if defined(ESP8266)
    # ifdef CORE_POST_2_6_0
  uint32_t _sectorStart = ((uint32_t)&_FS_start - 0x40200000) / SPI_FLASH_SEC_SIZE;
  uint32_t _sectorEnd   = ((uint32_t)&_FS_end - 0x40200000) / SPI_FLASH_SEC_SIZE;
    # else // ifdef CORE_POST_2_6_0
  uint32_t _sectorStart = ((uint32_t)&_SPIFFS_start - 0x40200000) / SPI_FLASH_SEC_SIZE;
  uint32_t _sectorEnd   = ((uint32_t)&_SPIFFS_end - 0x40200000) / SPI_FLASH_SEC_SIZE;
    # endif // ifdef CORE_POST_2_6_0

  return _sectorEnd - _sectorStart;
  #endif // if defined(ESP8266)
  #if defined(ESP32)
  return 32;
  #endif // if defined(ESP32)
}

size_t SpiffsUsedBytes() {
  size_t result = 1; // Do not output 0, this may be used in divisions.

  #ifdef ESP32
  result = SPIFFS.usedBytes();
  #endif // ifdef ESP32
  #ifdef ESP8266
  fs::FSInfo fs_info;
  SPIFFS.info(fs_info);
  result = fs_info.usedBytes;
  #endif // ifdef ESP8266
  return result;
}

size_t SpiffsTotalBytes() {
  size_t result = 1; // Do not output 0, this may be used in divisions.

  #ifdef ESP32
  result = SPIFFS.totalBytes();
  #endif // ifdef ESP32
  #ifdef ESP8266
  fs::FSInfo fs_info;
  SPIFFS.info(fs_info);
  result = fs_info.totalBytes;
  #endif // ifdef ESP8266
  return result;
}

size_t SpiffsBlocksize() {
  size_t result = 8192; // Some default viable for most 1 MB SPIFFS filesystems

  #ifdef ESP32
  result = 8192;        // Just assume 8k, since we cannot query it
  #endif // ifdef ESP32
  #ifdef ESP8266
  fs::FSInfo fs_info;
  SPIFFS.info(fs_info);
  result = fs_info.blockSize;
  #endif // ifdef ESP8266
  return result;
}

size_t SpiffsPagesize() {
  size_t result = 256; // Most common

  #ifdef ESP32
  result = 256;        // Just assume 256, since we cannot query it
  #endif // ifdef ESP32
  #ifdef ESP8266
  fs::FSInfo fs_info;
  SPIFFS.info(fs_info);
  result = fs_info.pageSize;
  #endif // ifdef ESP8266
  return result;
}

size_t SpiffsFreeSpace() {
  int freeSpace = SpiffsTotalBytes() - SpiffsUsedBytes();

  if (freeSpace < static_cast<int>(2 * SpiffsBlocksize())) {
    // Not enough free space left to store anything
    // There needs to be minimum of 2 free blocks.
    return 0;
  }
  return freeSpace - 2 * SpiffsBlocksize();
}

bool SpiffsFull() {
  return SpiffsFreeSpace() == 0;
}

/********************************************************************************************\
   Handling cached data
 \*********************************************************************************************/
String createCacheFilename(unsigned int count) {
  String fname;

  fname.reserve(16);
  #ifdef ESP32
  fname = '/';
  #endif // ifdef ESP32
  fname += "cache_";
  fname += String(count);
  fname += ".bin";
  return fname;
}

// Match string with an integer between '_' and ".bin"
int getCacheFileCountFromFilename(const String& fname) {
  int startpos = fname.indexOf('_');

  if (startpos < 0) { return -1; }
  int endpos = fname.indexOf(F(".bin"));

  if (endpos < 0) { return -1; }

  //  String digits = fname.substring(startpos + 1, endpos);
  int result;

  if (validIntFromString(fname.substring(startpos + 1, endpos), result)) {
    return result;
  }
  return -1;
}

// Look into the filesystem to see if there are any cache files present on the filesystem
// Return true if any found.
bool getCacheFileCounters(uint16_t& lowest, uint16_t& highest, size_t& filesizeHighest) {
  lowest          = 65535;
  highest         = 0;
  filesizeHighest = 0;
#ifdef ESP8266
  Dir dir = SPIFFS.openDir("cache");

  while (dir.next()) {
    String filename = dir.fileName();
    int    count    = getCacheFileCountFromFilename(filename);

    if (count >= 0) {
      if (lowest > count) {
        lowest = count;
      }

      if (highest < count) {
        highest         = count;
        filesizeHighest = dir.fileSize();
      }
    }
  }
#endif // ESP8266
#ifdef ESP32
  File root = SPIFFS.open("/cache");
  File file = root.openNextFile();

  while (file)
  {
    if (!file.isDirectory()) {
      int count = getCacheFileCountFromFilename(file.name());

      if (count >= 0) {
        if (lowest > count) {
          lowest = count;
        }

        if (highest < count) {
          highest         = count;
          filesizeHighest = file.size();
        }
      }
    }
    file = root.openNextFile();
  }
#endif // ESP32

  if (lowest <= highest) {
    return true;
  }
  lowest  = 0;
  highest = 0;
  return false;
}

/********************************************************************************************\
   Get partition table information
 \*********************************************************************************************/
#ifdef ESP32
String getPartitionType(byte pType, byte pSubType) {
  esp_partition_type_t partitionType       = static_cast<esp_partition_type_t>(pType);
  esp_partition_subtype_t partitionSubType = static_cast<esp_partition_subtype_t>(pSubType);

  if (partitionType == ESP_PARTITION_TYPE_APP) {
    if ((partitionSubType >= ESP_PARTITION_SUBTYPE_APP_OTA_MIN) &&
        (partitionSubType < ESP_PARTITION_SUBTYPE_APP_OTA_MAX)) {
      String result = F("OTA partition ");
      result += (partitionSubType - ESP_PARTITION_SUBTYPE_APP_OTA_MIN);
      return result;
    }

    switch (partitionSubType) {
      case ESP_PARTITION_SUBTYPE_APP_FACTORY: return F("Factory app");
      case ESP_PARTITION_SUBTYPE_APP_TEST:    return F("Test app");
      default: break;
    }
  }

  if (partitionType == ESP_PARTITION_TYPE_DATA) {
    switch (partitionSubType) {
      case ESP_PARTITION_SUBTYPE_DATA_OTA:      return F("OTA selection");
      case ESP_PARTITION_SUBTYPE_DATA_PHY:      return F("PHY init data");
      case ESP_PARTITION_SUBTYPE_DATA_NVS:      return F("NVS");
      case ESP_PARTITION_SUBTYPE_DATA_COREDUMP: return F("COREDUMP");
      case ESP_PARTITION_SUBTYPE_DATA_ESPHTTPD: return F("ESPHTTPD");
      case ESP_PARTITION_SUBTYPE_DATA_FAT:      return F("FAT");
      case ESP_PARTITION_SUBTYPE_DATA_SPIFFS:   return F("SPIFFS");
      default: break;
    }
  }
  String result = F("Unknown(");
  result += partitionSubType;
  result += ')';
  return result;
}

String getPartitionTableHeader(const String& itemSep, const String& lineEnd) {
  String result;

  result += F("Address");
  result += itemSep;
  result += F("Size");
  result += itemSep;
  result += F("Label");
  result += itemSep;
  result += F("Partition Type");
  result += itemSep;
  result += F("Encrypted");
  result += lineEnd;
  return result;
}

String getPartitionTable(byte pType, const String& itemSep, const String& lineEnd) {
  esp_partition_type_t partitionType = static_cast<esp_partition_type_t>(pType);
  String result;
  esp_partition_iterator_t _mypartiterator = esp_partition_find(partitionType, ESP_PARTITION_SUBTYPE_ANY, NULL);

  if (_mypartiterator) {
    do {
      const esp_partition_t *_mypart = esp_partition_get(_mypartiterator);
      result += formatToHex(_mypart->address);
      result += itemSep;
      result += formatToHex_decimal(_mypart->size, 1024);
      result += itemSep;
      result += _mypart->label;
      result += itemSep;
      result += getPartitionType(_mypart->type, _mypart->subtype);
      result += itemSep;
      result += (_mypart->encrypted ? F("Yes") : F("-"));
      result += lineEnd;
    } while ((_mypartiterator = esp_partition_next(_mypartiterator)) != NULL);
  }
  esp_partition_iterator_release(_mypartiterator);
  return result;
}

#endif // ifdef ESP32
