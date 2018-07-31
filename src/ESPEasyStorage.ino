/********************************************************************************************\
  SPIFFS error handling
  Look here for error # reference: https://github.com/pellepl/spiffs/blob/master/src/spiffs.h
  \*********************************************************************************************/
#define SPIFFS_CHECK(result, fname) if (!(result)) { return(FileError(__LINE__, fname)); }
String FileError(int line, const char * fname)
{
   String err = F("FS   : Error while reading/writing ");
   err += fname;
   err += F(" in ");
   err += line;
   addLog(LOG_LEVEL_ERROR, err);
   return(err);
}

/********************************************************************************************\
  Keep track of number of flash writes.
  \*********************************************************************************************/

void flashCount()
{
  if (RTC.flashDayCounter <= MAX_FLASHWRITES_PER_DAY)
    RTC.flashDayCounter++;
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
  return(String());
}

//use this in function that can return an error string. it automaticly returns with an error string if there where too many flash writes.
#define FLASH_GUARD() { String flashErr=flashGuard(); if (flashErr.length()) return(flashErr); }

/********************************************************************************************\
  Fix stuff to clear out differences between releases
  \*********************************************************************************************/
String BuildFixes()
{
  checkRAM(F("BuildFixes"));
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

  if (Settings.Build < 20101)
  {
    Serial.println(F("Fix reset Pin"));
    Settings.Pin_Reset = -1;
  }
  if (Settings.Build < 20102) {
    // Settings were 'mangled' by using older version
    // Have to patch settings to make sure no bogus data is being used.
    Serial.println(F("Fix settings with uninitalized data or corrupted by switching between versions"));
    Settings.UseRTOSMultitasking = false;
    Settings.Pin_Reset = -1;
    Settings.SyslogFacility = DEFAULT_SYSLOG_FACILITY;
    Settings.MQTTUseUnitNameAsClientId = DEFAULT_MQTT_USE_UNITNANE_AS_CLIENTID;
    Settings.StructSize = sizeof(Settings);
  }

  Settings.Build = BUILD;
  return(SaveSettings());
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
        log=log+fs_info.usedBytes;
        log=log+F(" bytes of ");
        log=log+fs_info.totalBytes;
        addLog(LOG_LEVEL_INFO, log);
      }
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
  Save settings to SPIFFS
  \*********************************************************************************************/
String SaveSettings(void)
{
  checkRAM(F("SaveSettings"));
  MD5Builder md5;
  uint8_t tmp_md5[16] = {0};
  String err;

  Settings.StructSize = sizeof(struct SettingsStruct);

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
    err=SaveToFile((char*)FILE_CONFIG, 0, (byte*)&Settings, sizeof(Settings));
    if (err.length())
     return(err);
//  }

  memcpy( SecuritySettings.ProgmemMd5, CRCValues.runTimeMD5, 16);
  md5.begin();
  md5.add((uint8_t *)&SecuritySettings, sizeof(SecuritySettings)-16);
  md5.calculate();
  md5.getBytes(tmp_md5);
  if (memcmp(tmp_md5, SecuritySettings.md5, 16) != 0) {
    // Settings have changed, save to file.
    memcpy(SecuritySettings.md5, tmp_md5, 16);
    err=SaveToFile((char*)FILE_SECURITY, 0, (byte*)&SecuritySettings, sizeof(SecuritySettings));
    if (WifiIsAP(WiFi.getMode())) {
      // Security settings are saved, may be update of WiFi settings or hostname.
      wifiSetupConnect = true;
    }
  }
  return (err);
}

/********************************************************************************************\
  Load settings from SPIFFS
  \*********************************************************************************************/
String LoadSettings()
{
  checkRAM(F("LoadSettings"));
  String err;
  uint8_t calculatedMd5[16];
  MD5Builder md5;

  err=LoadFromFile((char*)FILE_CONFIG, 0, (byte*)&Settings, sizeof( SettingsStruct));
  if (err.length())
    return(err);

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

  err=LoadFromFile((char*)FILE_SECURITY, 0, (byte*)&SecuritySettings, sizeof( SecurityStruct));
  md5.begin();
  md5.add((uint8_t *)&SecuritySettings, sizeof(SecuritySettings)-16);
  md5.calculate();
  md5.getBytes(calculatedMd5);
  if (memcmp (calculatedMd5, SecuritySettings.md5, 16)==0){
    addLog(LOG_LEVEL_INFO, F("CRC  : SecuritySettings CRC   ...OK "));
    if (memcmp(SecuritySettings.ProgmemMd5,CRCValues.runTimeMD5, 16)!=0)
      addLog(LOG_LEVEL_INFO, F("CRC  : binary has changed since last save of Settings"));
 }
  else{
    addLog(LOG_LEVEL_ERROR, F("CRC  : SecuritySettings CRC   ...FAIL"));
  }
  setUseStaticIP(useStaticIP());
  ExtraTaskSettings.clear(); // make sure these will not contain old settings.
  return(err);
}

/********************************************************************************************\
  Offsets in settings files
  \*********************************************************************************************/
bool getSettingsParameters(SettingsType settingsType, int index, int& max_index, int& offset, int& max_size, int& struct_size) {
  struct_size = 0;
  switch (settingsType) {
    case TaskSettings_Type:
      max_index = TASKS_MAX;
      offset = DAT_OFFSET_TASKS + (index * DAT_TASKS_DISTANCE);
      max_size = DAT_TASKS_SIZE;
      struct_size = sizeof(ExtraTaskSettingsStruct);
      break;
    case CustomTaskSettings_Type:
      max_index = TASKS_MAX;
      offset = DAT_OFFSET_TASKS + (index * DAT_TASKS_DISTANCE) + DAT_TASKS_CUSTOM_OFFSET;
      max_size = DAT_TASKS_CUSTOM_SIZE;
      // struct_size may differ.
      break;
    case ControllerSettings_Type:
      max_index = CONTROLLER_MAX;
      offset = DAT_OFFSET_CONTROLLER + (index * DAT_CONTROLLER_SIZE);
      max_size = DAT_CONTROLLER_SIZE;
      struct_size = sizeof(ControllerSettingsStruct);
      break;
    case CustomControllerSettings_Type:
      max_index = CONTROLLER_MAX;
      offset = DAT_OFFSET_CUSTOM_CONTROLLER + (index * DAT_CUSTOM_CONTROLLER_SIZE);
      max_size = DAT_CUSTOM_CONTROLLER_SIZE;
      // struct_size may differ.
      break;
    case NotificationSettings_Type:
      max_index = NOTIFICATION_MAX;
      offset = index * DAT_NOTIFICATION_SIZE;
      max_size = DAT_NOTIFICATION_SIZE;
      struct_size = sizeof(NotificationSettingsStruct);
      break;
    default:
      max_index = -1;
      offset = -1;
      return false;
  }
  return true;
}

bool getAndLogSettingsParameters(bool read, SettingsType settingsType, int index, int& offset, int& max_size) {
  if (loglevelActiveFor(LOG_LEVEL_DEBUG_DEV)) {
    String log = read ? F("Read") : F("Write");
    log += F(" settings: ");
    log += getSettingsTypeString(settingsType);
    log += F(" index: ");
    log += index;
    addLog(LOG_LEVEL_DEBUG_DEV, log);
  }
  return getSettingsParameters(settingsType, index, offset, max_size);
}

bool getSettingsParameters(SettingsType settingsType, int index, int& offset, int& max_size) {
  int max_index = -1;
  int struct_size;
  if (!getSettingsParameters(settingsType, index, max_index, offset, max_size, struct_size))
    return false;
  if (index >= 0 && index < max_index) return true;
  offset = -1;
  return false;
}

/********************************************************************************************\
  Save Task settings to SPIFFS
  \*********************************************************************************************/
String SaveTaskSettings(byte TaskIndex)
{
  checkRAM(F("SaveTaskSettings"));
  if (ExtraTaskSettings.TaskIndex != TaskIndex)
    return F("SaveTaskSettings taskIndex does not match");
  String err = SaveToFile(TaskSettings_Type, TaskIndex, (char*)FILE_CONFIG, (byte*)&ExtraTaskSettings, sizeof(struct ExtraTaskSettingsStruct));
  if (err.length() == 0)
    err = checkTaskSettings(TaskIndex);
  return err;
}


/********************************************************************************************\
  Load Task settings from SPIFFS
  \*********************************************************************************************/
String LoadTaskSettings(byte TaskIndex)
{
  checkRAM(F("LoadTaskSettings"));
  if (ExtraTaskSettings.TaskIndex == TaskIndex)
    return(String()); //already loaded
  ExtraTaskSettings.clear();
  String result = "";
  result = LoadFromFile(TaskSettings_Type, TaskIndex, (char*)FILE_CONFIG, (byte*)&ExtraTaskSettings, sizeof(struct ExtraTaskSettingsStruct));

  // After loading, some settings may need patching.
  ExtraTaskSettings.TaskIndex = TaskIndex; // Needed when an empty task was requested
  if (ExtraTaskSettings.TaskDeviceValueNames[0][0] == 0) {
    // if field set empty, reload defaults
    struct EventStruct TempEvent;
    TempEvent.TaskIndex = TaskIndex;
    String dummyString;
    //the plugin call should populate ExtraTaskSettings with its default values.
    PluginCall(PLUGIN_GET_DEVICEVALUENAMES, &TempEvent, dummyString);
  }

  return result;
}


/********************************************************************************************\
  Save Custom Task settings to SPIFFS
  \*********************************************************************************************/
String SaveCustomTaskSettings(int TaskIndex, byte* memAddress, int datasize)
{
  checkRAM(F("SaveCustomTaskSettings"));
  return(SaveToFile(CustomTaskSettings_Type, TaskIndex, (char*)FILE_CONFIG, memAddress, datasize));
}


/********************************************************************************************\
  Clear custom task settings
  \*********************************************************************************************/
String ClearCustomTaskSettings(int TaskIndex)
{
  // addLog(LOG_LEVEL_DEBUG, F("Clearing custom task settings"));
  return(ClearInFile(CustomTaskSettings_Type, TaskIndex, (char*)FILE_CONFIG));
}

/********************************************************************************************\
  Load Custom Task settings to SPIFFS
  \*********************************************************************************************/
String LoadCustomTaskSettings(int TaskIndex, byte* memAddress, int datasize)
{
  checkRAM(F("LoadCustomTaskSettings"));
  return(LoadFromFile(CustomTaskSettings_Type, TaskIndex, (char*)FILE_CONFIG, memAddress, datasize));
}

/********************************************************************************************\
  Save Controller settings to SPIFFS
  \*********************************************************************************************/
String SaveControllerSettings(int ControllerIndex, byte* memAddress, int datasize)
{
  checkRAM(F("SaveControllerSettings"));
  return SaveToFile(ControllerSettings_Type, ControllerIndex, (char*)FILE_CONFIG, memAddress, datasize);
}


/********************************************************************************************\
  Load Controller settings to SPIFFS
  \*********************************************************************************************/
String LoadControllerSettings(int ControllerIndex, byte* memAddress, int datasize)
{
  checkRAM(F("LoadControllerSettings"));
  return(LoadFromFile(ControllerSettings_Type, ControllerIndex, (char*)FILE_CONFIG, memAddress, datasize));
}


/********************************************************************************************\
  Clear Custom Controller settings
  \*********************************************************************************************/
String ClearCustomControllerSettings(int ControllerIndex)
{
  checkRAM(F("ClearCustomControllerSettings"));
  // addLog(LOG_LEVEL_DEBUG, F("Clearing custom controller settings"));
  return(ClearInFile(CustomControllerSettings_Type, ControllerIndex, (char*)FILE_CONFIG));
}


/********************************************************************************************\
  Save Custom Controller settings to SPIFFS
  \*********************************************************************************************/
String SaveCustomControllerSettings(int ControllerIndex,byte* memAddress, int datasize)
{
  checkRAM(F("SaveCustomControllerSettings"));
  return SaveToFile(CustomControllerSettings_Type, ControllerIndex, (char*)FILE_CONFIG, memAddress, datasize);
}


/********************************************************************************************\
  Load Custom Controller settings to SPIFFS
  \*********************************************************************************************/
String LoadCustomControllerSettings(int ControllerIndex,byte* memAddress, int datasize)
{
  checkRAM(F("LoadCustomControllerSettings"));
  return(LoadFromFile(CustomControllerSettings_Type, ControllerIndex, (char*)FILE_CONFIG, memAddress, datasize));
}

/********************************************************************************************\
  Save Controller settings to SPIFFS
  \*********************************************************************************************/
String SaveNotificationSettings(int NotificationIndex, byte* memAddress, int datasize)
{
  checkRAM(F("SaveNotificationSettings"));
  return SaveToFile(NotificationSettings_Type, NotificationIndex, (char*)FILE_NOTIFICATION, memAddress, datasize);
}


/********************************************************************************************\
  Load Controller settings to SPIFFS
  \*********************************************************************************************/
String LoadNotificationSettings(int NotificationIndex, byte* memAddress, int datasize)
{
  checkRAM(F("LoadNotificationSettings"));
  return(LoadFromFile(NotificationSettings_Type, NotificationIndex, (char*)FILE_NOTIFICATION, memAddress, datasize));
}




/********************************************************************************************\
  Init a file with zeros on SPIFFS
  \*********************************************************************************************/
String InitFile(const char* fname, int datasize)
{
  checkRAM(F("InitFile"));
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
  if (index < 0) {
    String log = F("SaveToFile: ");
    log += fname;
    log += F(" ERROR, invalid position in file");
    addLog(LOG_LEVEL_ERROR, log);
    return log;
  }

  checkRAM(F("SaveToFile"));
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
  if (index < 0) {
    String log = F("ClearInFile: ");
    log += fname;
    log += F(" ERROR, invalid position in file");
    addLog(LOG_LEVEL_ERROR, log);
    return log;
  }

  checkRAM(F("ClearInFile"));
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
String LoadFromFile(char* fname, int offset, byte* memAddress, int datasize)
{
  if (offset < 0) {
    String log = F("LoadFromFile: ");
    log += fname;
    log += F(" ERROR, invalid position in file");
    addLog(LOG_LEVEL_ERROR, log);
    return log;
  }
  START_TIMER;

  checkRAM(F("LoadFromFile"));

  fs::File f = SPIFFS.open(fname, "r+");
  SPIFFS_CHECK(f, fname);
  SPIFFS_CHECK(f.seek(offset, fs::SeekSet), fname);
  SPIFFS_CHECK(f.read(memAddress,datasize), fname);
  f.close();

  STOP_TIMER(LOADFILE_STATS);

  return(String());
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

String LoadFromFile(SettingsType settingsType, int index, char* fname, byte* memAddress, int datasize) {
  bool read = true;
  int offset, max_size;
  if (!getAndLogSettingsParameters(read, settingsType, index, offset, max_size)) {
    return getSettingsFileIndexRangeError(read, settingsType, index);
  }
  if (datasize > max_size)
    return getSettingsFileDatasizeError(read, settingsType, index, datasize, max_size);
  return(LoadFromFile(fname, offset, memAddress, datasize));
}

String SaveToFile(SettingsType settingsType, int index, char* fname, byte* memAddress, int datasize) {
  bool read = false;
  int offset, max_size;
  if (!getAndLogSettingsParameters(read, settingsType, index, offset, max_size)) {
    return getSettingsFileIndexRangeError(read, settingsType, index);
  }
  if (datasize > max_size)
    return getSettingsFileDatasizeError(read, settingsType, index, datasize, max_size);
  return(SaveToFile(fname, offset, memAddress, datasize));
}

String ClearInFile(SettingsType settingsType, int index, char* fname) {
  bool read = false;
  int offset, max_size;
  if (!getAndLogSettingsParameters(read, settingsType, index, offset, max_size)) {
    return getSettingsFileIndexRangeError(read, settingsType, index);
  }
  return(ClearInFile(fname, offset, max_size));
}

/********************************************************************************************\
  Check SPIFFS area settings
  \*********************************************************************************************/
int SpiffsSectors()
{
  checkRAM(F("SpiffsSectors"));
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
  Get partition table information
  \*********************************************************************************************/
#ifdef ESP32
String getPartitionType(byte pType, byte pSubType) {
  esp_partition_type_t partitionType = static_cast<esp_partition_type_t>(pType);
  esp_partition_subtype_t partitionSubType = static_cast<esp_partition_subtype_t>(pSubType);
  if (partitionType == ESP_PARTITION_TYPE_APP) {
    if (partitionSubType >= ESP_PARTITION_SUBTYPE_APP_OTA_MIN &&
        partitionSubType < ESP_PARTITION_SUBTYPE_APP_OTA_MAX) {
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
  const esp_partition_t * _mypart;
  esp_partition_iterator_t _mypartiterator = esp_partition_find(partitionType, ESP_PARTITION_SUBTYPE_ANY, NULL);
  if (_mypartiterator) {
    do {
      _mypart = esp_partition_get(_mypartiterator);
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
#endif
