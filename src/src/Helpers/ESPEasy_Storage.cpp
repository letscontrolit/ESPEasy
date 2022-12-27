#include "../Helpers/ESPEasy_Storage.h"

#include "../../ESPEasy_common.h"

#include "../CustomBuild/CompiletimeDefines.h"
#include "../CustomBuild/StorageLayout.h"

#include "../DataStructs/TimingStats.h"

#include "../DataTypes/ESPEasyFileType.h"
#include "../DataTypes/ESPEasyTimeSource.h"
#include "../DataTypes/SPI_options.h"

#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../ESPEasyCore/ESPEasyNetwork.h"
#include "../ESPEasyCore/ESPEasyWifi.h"
#include "../ESPEasyCore/Serial.h"

#include "../Globals/CRCValues.h"
#include "../Globals/Cache.h"
#include "../Globals/Device.h"
#include "../Globals/ESPEasyWiFiEvent.h"
#include "../Globals/ESPEasy_Scheduler.h"
#include "../Globals/ESPEasy_time.h"
#include "../Globals/EventQueue.h"
#include "../Globals/ExtraTaskSettings.h"
#include "../Globals/NetworkState.h"
#include "../Globals/Plugins.h"
#include "../Globals/RTC.h"
#include "../Globals/ResetFactoryDefaultPref.h"
#include "../Globals/SecuritySettings.h"
#include "../Globals/Settings.h"
#include "../Globals/WiFi_AP_Candidates.h"

#include "../Helpers/ESPEasyRTC.h"
#include "../Helpers/ESPEasy_checks.h"
#include "../Helpers/ESPEasy_FactoryDefault.h"
#include "../Helpers/ESPEasy_time_calc.h"
#include "../Helpers/FS_Helper.h"
#include "../Helpers/Hardware.h"
#include "../Helpers/Memory.h"
#include "../Helpers/Misc.h"
#include "../Helpers/Networking.h"
#include "../Helpers/Numerical.h"
#include "../Helpers/PeriodicalActions.h"
#include "../Helpers/StringConverter.h"
#include "../Helpers/StringParser.h"


#ifdef ESP32
#include <MD5Builder.h>
#include <esp_partition.h>
#endif

#ifdef ESP32
String patch_fname(const String& fname) {
  if (fname.startsWith(F("/"))) {
    return fname;
  }
  return String('/') + fname;
}
#endif
#ifdef ESP8266
#define patch_fname(F) (F)
#endif

/********************************************************************************************\
   file system error handling
   Look here for error # reference: https://github.com/pellepl/spiffs/blob/master/src/spiffs.h
 \*********************************************************************************************/
String FileError(int line, const char *fname)
{
  String err = F("FS   : Error while reading/writing ");

  err += fname;
  err += F(" in ");
  err += line;
  addLogMove(LOG_LEVEL_ERROR, err);
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
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("flashGuard"));
  #endif

  if (RTC.flashDayCounter > MAX_FLASHWRITES_PER_DAY)
  {
    String log = F("FS   : Daily flash write rate exceeded! (powercycle or send command 'resetFlashWriteCounter' to reset this)");
    addLog(LOG_LEVEL_ERROR, log);
    return log;
  }
  flashCount();
  return EMPTY_STRING;
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

bool fileExists(const __FlashStringHelper * fname)
{
  return fileExists(String(fname));
}

bool fileExists(const String& fname) {
  #ifdef USE_SECOND_HEAP
  HeapSelectDram ephemeral;
  #endif

  const String patched_fname = patch_fname(fname);
  auto search = Cache.fileExistsMap.find(patched_fname);
  if (search != Cache.fileExistsMap.end()) {
    return search->second;
  }
  bool res = ESPEASY_FS.exists(patched_fname);
  #if FEATURE_SD
  if (!res) {
    res = SD.exists(patched_fname);
  }
  #endif
  Cache.fileExistsMap[patched_fname] = res;
  if (Cache.fileCacheClearMoment == 0) {
    if (node_time.timeSource == timeSource_t::No_time_source) {
      // use some random value as we don't have a time yet
      Cache.fileCacheClearMoment = HwRandom();
    } else {
      Cache.fileCacheClearMoment = node_time.now();
    }
  }
  return res;
}

fs::File tryOpenFile(const String& fname, const String& mode) {
  START_TIMER;
  fs::File f;
  if (fname.isEmpty() || fname.equals(F("/"))) {
    return f;
  }

  bool exists = fileExists(fname);

  if (!exists) {
    if (mode.equals(F("r"))) {
      return f;
    }
    clearFileCaches();
  }
  f = ESPEASY_FS.open(patch_fname(fname), mode.c_str());
  #  if FEATURE_SD

  if (!f) {
    // FIXME TD-er: Should this fallback to SD only be done on "r" mode?
    f = SD.open(fname.c_str(), mode.c_str());
  }
  #  endif // if FEATURE_SD


  STOP_TIMER(TRY_OPEN_FILE);
  return f;
}

bool tryRenameFile(const String& fname_old, const String& fname_new) {
  clearFileCaches();
  if (fileExists(fname_old) && !fileExists(fname_new)) {
    clearAllCaches();
    return ESPEASY_FS.rename(patch_fname(fname_old), patch_fname(fname_new));
  }
  return false;
}

bool tryDeleteFile(const String& fname) {
  if (fname.length() > 0)
  {
    clearAllCaches();
    bool res = ESPEASY_FS.remove(patch_fname(fname));
    #if FEATURE_SD
    if (!res) {
      res = SD.remove(patch_fname(fname));
    }
    #endif

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
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("BuildFixes"));
  #endif
  serialPrintln(F("\nBuild changed!"));

  if (Settings.Build < 145)
  {
    InitFile(SettingsType::SettingsFileEnum::FILE_NOTIFICATION_type);
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
    Settings.MQTTUseUnitNameAsClientId_unused = DEFAULT_MQTT_USE_UNITNAME_AS_CLIENTID;
    Settings.StructSize                = sizeof(Settings);
  }

  if (Settings.Build < 20103) {
    Settings.ResetFactoryDefaultPreference = 0;
    Settings.OldRulesEngine(DEFAULT_RULES_OLDENGINE);
  }
  if (Settings.Build < 20105) {
    Settings.I2C_clockSpeed = DEFAULT_I2C_CLOCK_SPEED;
  }
  if (Settings.Build <= 20106) {
    // ClientID is now defined in the controller settings.
    #if FEATURE_MQTT
    controllerIndex_t controller_idx = firstEnabledMQTT_ControllerIndex();
    if (validControllerIndex(controller_idx)) {
      MakeControllerSettings(ControllerSettings); //-V522
      if (AllocatedControllerSettings()) {
        LoadControllerSettings(controller_idx, ControllerSettings);

        String clientid;
        if (Settings.MQTTUseUnitNameAsClientId_unused) {
          clientid = F("%sysname%");
          if (Settings.appendUnitToHostname()) {
            clientid += F("_%unit%");
          }
        }
        else {
          clientid  = F("ESPClient_%mac%");
        }
        safe_strncpy(ControllerSettings.ClientID, clientid, sizeof(ControllerSettings.ClientID));

        ControllerSettings.mqtt_uniqueMQTTclientIdReconnect(Settings.uniqueMQTTclientIdReconnect_unused());
        ControllerSettings.mqtt_retainFlag(Settings.MQTTRetainFlag_unused);
        SaveControllerSettings(controller_idx, ControllerSettings);
      }
    }
    #endif // if FEATURE_MQTT
  }
  if (Settings.Build < 20107) {
    Settings.WebserverPort = 80;
  }
  if (Settings.Build < 20108) {
    Settings.ETH_Phy_Addr   = DEFAULT_ETH_PHY_ADDR;
    Settings.ETH_Pin_mdc    = DEFAULT_ETH_PIN_MDC;
    Settings.ETH_Pin_mdio   = DEFAULT_ETH_PIN_MDIO;
    Settings.ETH_Pin_power  = DEFAULT_ETH_PIN_POWER;
    Settings.ETH_Phy_Type   = DEFAULT_ETH_PHY_TYPE;
    Settings.ETH_Clock_Mode = DEFAULT_ETH_CLOCK_MODE;
    Settings.NetworkMedium  = DEFAULT_NETWORK_MEDIUM;
  }
  if (Settings.Build < 20109) {
    Settings.SyslogPort = 514;
  }
  if (Settings.Build < 20110) {
    Settings.I2C_clockSpeed_Slow = DEFAULT_I2C_CLOCK_SPEED_SLOW;
    Settings.I2C_Multiplexer_Type = I2C_MULTIPLEXER_NONE;
    Settings.I2C_Multiplexer_Addr = -1;
    for (taskIndex_t x = 0; x < TASKS_MAX; x++) {
      Settings.I2C_Multiplexer_Channel[x] = -1;
    }
    Settings.I2C_Multiplexer_ResetPin = -1;
  }
  if (Settings.Build < 20111) {
    #ifdef ESP32
    constexpr uint8_t maxStatesesp32 = sizeof(Settings.PinBootStates_ESP32) / sizeof(Settings.PinBootStates_ESP32[0]);
    for (uint8_t i = 0; i < maxStatesesp32; ++i) {
      Settings.PinBootStates_ESP32[i] = 0;
    }
    #endif
  }
  if (Settings.Build < 20112) {
    Settings.WiFi_TX_power = 70; // 70 = 17.5dBm. unit: 0.25 dBm
    Settings.WiFi_sensitivity_margin = 3; // Margin in dBm on top of sensitivity.
  }
  if (Settings.Build < 20113) {
    Settings.NumberExtraWiFiScans = 0;
  }
  if (Settings.Build < 20114) {
    #ifdef USES_P003
    // P003_Pulse was always using the pull-up, now it is a setting.
    for (taskIndex_t taskIndex = 0; taskIndex < TASKS_MAX; ++taskIndex) {
      if (Settings.TaskDeviceNumber[taskIndex] == 3) {
        Settings.TaskDevicePin1PullUp[taskIndex] = true;
      }
    }
    #endif
  }
  if (Settings.Build < 20115) {
    if (Settings.InitSPI != static_cast<int>(SPI_Options_e::UserDefined)) { // User-defined SPI pins set to None
      Settings.SPI_SCLK_pin = -1;
      Settings.SPI_MISO_pin = -1;
      Settings.SPI_MOSI_pin = -1;
    }
  }
  #ifdef USES_P053
  if (Settings.Build < 20116) {
    // Added PWR button, init to "-none-"
    for (taskIndex_t taskIndex = 0; taskIndex < TASKS_MAX; ++taskIndex) {
      if (Settings.TaskDeviceNumber[taskIndex] == 53) {
        Settings.TaskDevicePluginConfig[taskIndex][3] = -1;
      }
    }
    // Remove PeriodicalScanWiFi
    // Reset to default 0 for future use.
    bitWrite(Settings.VariousBits1, 15, 0);
  }
  #endif

  // Starting 2022/08/18
  // Use get_build_nr() value for settings transitions.
  // This value will also be shown when building using PlatformIO, when showing the  Compile time defines 


  Settings.Build = get_build_nr();
  return SaveSettings();
}

/********************************************************************************************\
   Mount FS and check config.dat
 \*********************************************************************************************/
void fileSystemCheck()
{
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("fileSystemCheck"));
  #endif
  addLog(LOG_LEVEL_INFO, F("FS   : Mounting..."));
#if defined(ESP32) && defined(USE_LITTLEFS)
  if (getPartionCount(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_SPIFFS) != 0 
      && ESPEASY_FS.begin())
#else
  if (ESPEASY_FS.begin())
#endif
  {
    clearAllCaches();
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = F("FS   : Mount successful, used ");
      log += SpiffsUsedBytes();
      log += F(" bytes of ");
      log += SpiffsTotalBytes();
      addLogMove(LOG_LEVEL_INFO, log);
    }

    // Run garbage collection before any file is open.
    uint8_t retries = 3;

    while (retries > 0 && GarbageCollection()) {
      --retries;
    }

    fs::File f = tryOpenFile(SettingsType::getSettingsFileName(SettingsType::Enum::BasicSettings_Type).c_str(), "r");
    if (f) { 
      f.close(); 
    } else {
      ResetFactory();
    }
  }
  else
  {
    const __FlashStringHelper * log = F("FS   : Mount failed");
    serialPrintln(log);
    addLogMove(LOG_LEVEL_ERROR, log);
    ResetFactory();
  }
}

bool FS_format() {
  #ifdef USE_LITTLEFS
    #ifdef ESP32
    const bool res = ESPEASY_FS.begin(true);
    ESPEASY_FS.end();
    return res;
    #else
    return ESPEASY_FS.format();
    #endif
  #else
  return ESPEASY_FS.format();
  #endif
}

#ifdef ESP32

# include <esp_partition.h>

int getPartionCount(uint8_t pType, uint8_t pSubType) {
  esp_partition_type_t partitionType       = static_cast<esp_partition_type_t>(pType);
  esp_partition_subtype_t subtype          = static_cast<esp_partition_subtype_t>(pSubType);
  esp_partition_iterator_t _mypartiterator = esp_partition_find(partitionType, subtype, NULL);
  int nrPartitions                         = 0;

  if (_mypartiterator) {
    do {
      ++nrPartitions;
    } while ((_mypartiterator = esp_partition_next(_mypartiterator)) != NULL);
  }
  esp_partition_iterator_release(_mypartiterator);
  return nrPartitions;
}


#endif

/********************************************************************************************\
   Garbage collection
 \*********************************************************************************************/
bool GarbageCollection() {
  #ifdef CORE_POST_2_6_0

  // Perform garbage collection
  START_TIMER;

  if (ESPEASY_FS.gc()) {
    addLog(LOG_LEVEL_INFO, F("FS   : Success garbage collection"));
    STOP_TIMER(FS_GC_SUCCESS);
    return true;
  }
  STOP_TIMER(FS_GC_FAIL);
  return false;
  #else // ifdef CORE_POST_2_6_0

  // Not supported, so nothing was removed.
  return false;
  #endif // ifdef CORE_POST_2_6_0
}

bool computeChecksum(
  uint8_t checksum[16], 
  uint8_t * data, 
  size_t struct_size, 
  size_t len_upto_md5,
  bool updateChecksum)
{
  if (len_upto_md5 > struct_size) len_upto_md5 = struct_size;
  MD5Builder md5;
  md5.begin();

  if (len_upto_md5 > 0) {
    md5.add(data, len_upto_md5);
  }
  if ((len_upto_md5 + 16) < struct_size) {
    data += len_upto_md5 + 16;
    const int len_after_md5 = struct_size - 16 - len_upto_md5;
    if (len_after_md5 > 0) {
      md5.add(data, len_after_md5);
    }
  }
  md5.calculate();
  uint8_t    tmp_md5[16] = { 0 };
  md5.getBytes(tmp_md5);
  if (memcmp(tmp_md5, checksum, 16) != 0) {
    // Data has changed, copy computed checksum
    if (updateChecksum) {
      memcpy(checksum, tmp_md5, 16);
    }
    return false;
  }
  return true;
}

/********************************************************************************************\
   Save settings to file system
 \*********************************************************************************************/
String SaveSettings()
{
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("SaveSettings"));
  #endif
  String     err;
  {
    Settings.StructSize = sizeof(Settings);

    // FIXME @TD-er: As discussed in #1292, the CRC for the settings is now disabled.

    Settings.validate();

    if (!COMPUTE_STRUCT_CHECKSUM_UPDATE(SettingsStruct, Settings)
    /*
    computeChecksum(
        Settings.md5,
        reinterpret_cast<uint8_t *>(&Settings),
        sizeof(SettingsStruct),
        offsetof(SettingsStruct, md5))
    */
        ) {
      err = SaveToFile(SettingsType::getSettingsFileName(SettingsType::Enum::BasicSettings_Type).c_str(), 0, reinterpret_cast<const uint8_t *>(&Settings), sizeof(Settings));
    } else {
      addLog(LOG_LEVEL_INFO, F("Skip saving settings, not changed"));
    }
  }

  if (err.length()) {
    return err;
  }

#ifndef BUILD_MINIMAL_OTA
  // Must check this after saving, or else it is not possible to fix multiple
  // issues which can only corrected on different pages.
  if (!SettingsCheck(err)) { return err; }
#endif

  //  }

  err = SaveSecuritySettings();

  return err;
}

String SaveSecuritySettings() {
  String     err;

  SecuritySettings.validate();
  memcpy(SecuritySettings.ProgmemMd5, CRCValues.runTimeMD5, 16);

  if (!COMPUTE_STRUCT_CHECKSUM_UPDATE(SecurityStruct, SecuritySettings)) {
    // Settings have changed, save to file.
    err = SaveToFile(SettingsType::getSettingsFileName(SettingsType::Enum::SecuritySettings_Type).c_str(), 0, reinterpret_cast<const uint8_t *>(&SecuritySettings), sizeof(SecuritySettings));

    // Security settings are saved, may be update of WiFi settings or hostname.
    if (!NetworkConnected()) {
      if (SecuritySettings.hasWiFiCredentials() && active_network_medium == NetworkMedium_t::WIFI) {
        WiFiEventData.wifiConnectAttemptNeeded = true;
        WiFi_AP_Candidates.force_reload(); // Force reload of the credentials and found APs from the last scan
        resetWiFi();
        AttemptWiFiConnect();
      }
    }
  } else {
    addLog(LOG_LEVEL_INFO, F("Skip saving SecuritySettings, not changed"));

  }

  // FIXME TD-er: How to check if these have changed?
  ExtendedControllerCredentials.save();
  afterloadSettings();
  return err;
}

void afterloadSettings() {
  ExtraTaskSettings.clear(); // make sure these will not contain old settings.

  // Load ResetFactoryDefaultPreference from provisioning.dat if available.
  uint32_t pref_temp = Settings.ResetFactoryDefaultPreference;
  #if FEATURE_CUSTOM_PROVISIONING
  if (fileExists(getFileName(FileType::PROVISIONING_DAT))) {
    MakeProvisioningSettings(ProvisioningSettings);
    if (AllocatedProvisioningSettings()) {
      loadProvisioningSettings(ProvisioningSettings);
      if (ProvisioningSettings.matchingFlashSize()) {
        pref_temp = ProvisioningSettings.ResetFactoryDefaultPreference.getPreference();
      }
    }
  }
  #endif

  // TODO TD-er: Try to get the information from more locations to make it more persistent
  // Maybe EEPROM location?


  ResetFactoryDefaultPreference_struct pref(pref_temp);
  if (modelMatchingFlashSize(pref.getDeviceModel())) {
    ResetFactoryDefaultPreference = pref_temp;
  }
  Scheduler.setEcoMode(Settings.EcoPowerMode());
  #ifdef ESP32
  setCpuFrequencyMhz(Settings.EcoPowerMode() ? 80 : 240);
  #endif

  if (!Settings.UseRules) {
    eventQueue.clear();
  }
  CheckRunningServices(); // To update changes in hostname.
}

/********************************************************************************************\
   Load settings from file system
 \*********************************************************************************************/
String LoadSettings()
{
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("LoadSettings"));
  #endif
  String  err;

  err = LoadFromFile(SettingsType::getSettingsFileName(SettingsType::Enum::BasicSettings_Type).c_str(), 0, reinterpret_cast<uint8_t *>(&Settings), sizeof(SettingsStruct));

  if (err.length()) {
    return err;
  }
  Settings.validate();

  if (COMPUTE_STRUCT_CHECKSUM(SettingsStruct, Settings)) {
    addLog(LOG_LEVEL_INFO,  F("CRC  : Settings CRC           ...OK"));
  } else{
    addLog(LOG_LEVEL_ERROR, F("CRC  : Settings CRC           ...FAIL"));
  }

  err = LoadFromFile(SettingsType::getSettingsFileName(SettingsType::Enum::SecuritySettings_Type).c_str(), 0, reinterpret_cast<uint8_t *>(&SecuritySettings), sizeof(SecurityStruct));

  if (COMPUTE_STRUCT_CHECKSUM(SecurityStruct, SecuritySettings)) {
    addLog(LOG_LEVEL_INFO, F("CRC  : SecuritySettings CRC   ...OK "));

    if (memcmp(SecuritySettings.ProgmemMd5, CRCValues.runTimeMD5, 16) != 0) {
      addLog(LOG_LEVEL_INFO, F("CRC  : binary has changed since last save of Settings"));
    }
  }
  else {
    addLog(LOG_LEVEL_ERROR, F("CRC  : SecuritySettings CRC   ...FAIL"));
  }

  ExtendedControllerCredentials.load();

  //  setupStaticIPconfig();
  // FIXME TD-er: Must check if static/dynamic IP was changed and trigger a reconnect? Or is a reboot better when changing those settings?
  afterloadSettings();
  SecuritySettings.validate();
  return err;
}



/********************************************************************************************\
   Disable Plugin, based on bootFailedCount
 \*********************************************************************************************/
uint8_t disablePlugin(uint8_t bootFailedCount) {
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

uint8_t disableAllPlugins(uint8_t bootFailedCount) {
  if (bootFailedCount > 0) {
    --bootFailedCount;
    for (taskIndex_t i = 0; i < TASKS_MAX; ++i) {
        Settings.TaskDeviceEnabled[i] = false;
    }
  }
  return bootFailedCount;
}


/********************************************************************************************\
   Disable Controller, based on bootFailedCount
 \*********************************************************************************************/
uint8_t disableController(uint8_t bootFailedCount) {
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

uint8_t disableAllControllers(uint8_t bootFailedCount) {
  if (bootFailedCount > 0) {
    --bootFailedCount;
    for (controllerIndex_t i = 0; i < CONTROLLER_MAX; ++i) {
      Settings.ControllerEnabled[i] = false;
    }
  }
  return bootFailedCount;
}


/********************************************************************************************\
   Disable Notification, based on bootFailedCount
 \*********************************************************************************************/
#if FEATURE_NOTIFIER
uint8_t disableNotification(uint8_t bootFailedCount) {
  for (uint8_t i = 0; i < NOTIFICATION_MAX && bootFailedCount > 0; ++i) {
    if (Settings.NotificationEnabled[i]) {
      --bootFailedCount;

      if (bootFailedCount == 0) {
        Settings.NotificationEnabled[i] = false;
      }
    }
  }
  return bootFailedCount;
}

uint8_t disableAllNotifications(uint8_t bootFailedCount) {
  if (bootFailedCount > 0) {
    --bootFailedCount;
    for (uint8_t i = 0; i < NOTIFICATION_MAX; ++i) {
        Settings.NotificationEnabled[i] = false;
    }
  }
  return bootFailedCount;
}
#endif

/********************************************************************************************\
   Disable Rules, based on bootFailedCount
 \*********************************************************************************************/
uint8_t disableRules(uint8_t bootFailedCount) {
  if (bootFailedCount > 0) {
    --bootFailedCount;
    Settings.UseRules = false;
  }
  return bootFailedCount;
}


bool getAndLogSettingsParameters(bool read, SettingsType::Enum settingsType, int index, int& offset, int& max_size) {
#ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG_DEV)) {
    String log = read ? F("Read") : F("Write");
    log += F(" settings: ");
    log += SettingsType::getSettingsTypeString(settingsType);
    log += F(" index: ");
    log += index;
    addLogMove(LOG_LEVEL_DEBUG_DEV, log);
  }
#endif // ifndef BUILD_NO_DEBUG
  return SettingsType::getSettingsParameters(settingsType, index, offset, max_size);
}


/********************************************************************************************\
   Load array of Strings from Custom settings
   Use maxStringLength = 0 to optimize for size (strings will be concatenated)
 \*********************************************************************************************/
String LoadStringArray(SettingsType::Enum settingsType, int index, String strings[], uint16_t nrStrings, uint16_t maxStringLength, uint32_t offset_in_block)
{
  int offset, max_size;
  if (!SettingsType::getSettingsParameters(settingsType, index, offset, max_size))
  {
    #ifndef BUILD_NO_DEBUG
    return F("Invalid index for custom settings");
    #else
    return F("Save error");
    #endif
  }

  const uint32_t bufferSize = 128;

  // FIXME TD-er: For now stack allocated, may need to be heap allocated?
  if (maxStringLength >= bufferSize) { return F("Max 128 chars allowed"); }

  char buffer[bufferSize] = {0};

  String   result;
  uint32_t readPos       = offset_in_block;
  uint32_t nextStringPos = readPos;
  uint32_t stringCount   = 0;

  const uint16_t estimatedStringSize = maxStringLength > 0 ? maxStringLength : bufferSize;
  String   tmpString;
  {
    #ifdef USE_SECOND_HEAP
    // Store each string in 2nd heap
    HeapSelectIram ephemeral;
    #endif
    tmpString.reserve(estimatedStringSize);
  }
  {
    while (stringCount < nrStrings && static_cast<int>(readPos) < max_size) {
      const uint32_t readSize = std::min(bufferSize, max_size - readPos);
      result += LoadFromFile(settingsType,
                            index,
                            reinterpret_cast<uint8_t *>(&buffer),
                            readSize,
                            readPos);

      for (uint32_t i = 0; i < readSize && stringCount < nrStrings; ++i) {
        const uint32_t curPos = readPos + i;

        if (curPos >= nextStringPos) {
          if (buffer[i] == 0) {
            if (maxStringLength != 0) {
              // Specific string length, so we have to set the next string position.
              nextStringPos += maxStringLength;
            }
            #ifdef USE_SECOND_HEAP
            // Store each string in 2nd heap
            HeapSelectIram ephemeral;
            #endif

            strings[stringCount] = tmpString;
            tmpString = String();
            tmpString.reserve(estimatedStringSize);
            ++stringCount;
          } else {
            tmpString += buffer[i];
          }
        }
      }
      readPos += bufferSize;
    }
  }

  if ((!tmpString.isEmpty()) && (stringCount < nrStrings)) {
    result              += F("Incomplete custom settings for index ");
    result              += (index + 1);
    strings[stringCount] = tmpString;
  }
  return result;
}

/********************************************************************************************\
   Save array of Strings from Custom settings
   Use maxStringLength = 0 to optimize for size (strings will be concatenated)
 \*********************************************************************************************/
String SaveStringArray(SettingsType::Enum settingsType, int index, const String strings[], uint16_t nrStrings, uint16_t maxStringLength, uint32_t posInBlock)
{
  int offset, max_size;
  if (!SettingsType::getSettingsParameters(settingsType, index, offset, max_size))
  {
    #ifndef BUILD_NO_DEBUG
    return F("Invalid index for custom settings");
    #else
    return F("Save error");
    #endif
  }

  #ifdef ESP8266
  uint16_t bufferSize = 256;
  #endif
  #ifdef ESP32
  uint16_t bufferSize = 1024;
  #endif
  if (bufferSize > max_size) {
    bufferSize = max_size;
  }

  std::vector<uint8_t> buffer;
  buffer.resize(bufferSize);

  String   result;
  int      writePos        = posInBlock;
  uint16_t stringCount     = 0;
  uint16_t stringReadPos   = 0;
  uint16_t nextStringPos   = writePos;
  uint16_t curStringLength = 0;

  if (maxStringLength != 0) {
    // Specified string length, check given strings
    for (uint16_t i = 0; i < nrStrings; ++i) {
      if (strings[i].length() >= maxStringLength) {
        result += getCustomTaskSettingsError(i);
      }
    }
  }

  while (stringCount < nrStrings && writePos < max_size) {
    for (size_t i = 0; i < buffer.size(); ++i) {
      buffer[i] = 0;
    }

    int bufpos = 0;
    for ( ; bufpos < bufferSize && stringCount < nrStrings; ++bufpos) {
      if (stringReadPos == 0) {
        // We're at the start of a string
        curStringLength = strings[stringCount].length();

        if (maxStringLength != 0) {
          if (curStringLength >= maxStringLength) {
            curStringLength = maxStringLength - 1;
          }
        }
      }

      const uint16_t curPos = writePos + bufpos;

      if (curPos >= nextStringPos) {
        if (stringReadPos < curStringLength) {
          buffer[bufpos] = strings[stringCount][stringReadPos];
          ++stringReadPos;
        } else {
          buffer[bufpos] = 0;
          stringReadPos  = 0;
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
    result   += SaveToFile(settingsType, index, &(buffer[0]), bufpos, writePos);
    writePos += bufpos;
  }

  if ((writePos >= max_size) && (stringCount < nrStrings)) {
    result += F("Error: Not all strings fit in custom settings.");
  }
  return result;
}



/********************************************************************************************\
   Save Task settings to file system
 \*********************************************************************************************/
String SaveTaskSettings(taskIndex_t TaskIndex)
{
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("SaveTaskSettings"));
  #endif

  if (ExtraTaskSettings.TaskIndex != TaskIndex) {
    #ifndef BUILD_NO_DEBUG
    return F("SaveTaskSettings taskIndex does not match");
    #else
    return F("Save error");
    #endif
  }
  String err = SaveToFile(SettingsType::Enum::TaskSettings_Type,
                          TaskIndex,
                          reinterpret_cast<const uint8_t *>(&ExtraTaskSettings),
                          sizeof(struct ExtraTaskSettingsStruct));
#ifndef BUILD_MINIMAL_OTA
  if (err.isEmpty()) {
    err = checkTaskSettings(TaskIndex);
  }
#endif
  return err;
}

/********************************************************************************************\
   Load Task settings from file system
 \*********************************************************************************************/
String LoadTaskSettings(taskIndex_t TaskIndex)
{
  if (ExtraTaskSettings.TaskIndex == TaskIndex) {
    return EMPTY_STRING; // already loaded
  }
  if (!validTaskIndex(TaskIndex)) {
    return EMPTY_STRING; // Un-initialized task index.
  }
  ExtraTaskSettings.clear();
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("LoadTaskSettings"));
  #endif

  START_TIMER
  const String result = LoadFromFile(SettingsType::Enum::TaskSettings_Type, TaskIndex, reinterpret_cast<uint8_t *>(&ExtraTaskSettings), sizeof(struct ExtraTaskSettingsStruct));

  // After loading, some settings may need patching.
  ExtraTaskSettings.TaskIndex = TaskIndex; // Needed when an empty task was requested

  const deviceIndex_t DeviceIndex = getDeviceIndex_from_TaskIndex(TaskIndex);
  if (validDeviceIndex(DeviceIndex)) {
    if (!Device[DeviceIndex].configurableDecimals()) {
      // Nr of decimals cannot be configured, so set them to 0 just to be sure.
      for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
        ExtraTaskSettings.TaskDeviceValueDecimals[i] = 0;
      }      
    }
  }

  if (ExtraTaskSettings.TaskDeviceValueNames[0][0] == 0) {
    // if field set empty, reload defaults
    struct EventStruct TempEvent(TaskIndex);
    String tmp;

    // the plugin call should populate ExtraTaskSettings with its default values.
    PluginCall(PLUGIN_GET_DEVICEVALUENAMES, &TempEvent, tmp);
  }
  ExtraTaskSettings.validate();
  Cache.updateExtraTaskSettingsCache();
  STOP_TIMER(LOAD_TASK_SETTINGS);

  return result;
}

/********************************************************************************************\
   Save Custom Task settings to file system
 \*********************************************************************************************/
String SaveCustomTaskSettings(taskIndex_t TaskIndex, const uint8_t *memAddress, int datasize, uint32_t posInBlock)
{
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("SaveCustomTaskSettings"));
  #endif
  return SaveToFile(SettingsType::Enum::CustomTaskSettings_Type, TaskIndex, memAddress, datasize, posInBlock);
}

/********************************************************************************************\
   Save array of Strings to Custom Task settings
   Use maxStringLength = 0 to optimize for size (strings will be concatenated)
 \*********************************************************************************************/
String SaveCustomTaskSettings(taskIndex_t TaskIndex, String strings[], uint16_t nrStrings, uint16_t maxStringLength, uint32_t posInBlock)
{
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("SaveCustomTaskSettings"));
  #endif
  return SaveStringArray(
    SettingsType::Enum::CustomTaskSettings_Type, TaskIndex,
    strings, nrStrings, maxStringLength, posInBlock);
}

String getCustomTaskSettingsError(uint8_t varNr) {
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
  return ClearInFile(SettingsType::Enum::CustomTaskSettings_Type, TaskIndex);
}

/********************************************************************************************\
   Load Custom Task settings from file system
 \*********************************************************************************************/
String LoadCustomTaskSettings(taskIndex_t TaskIndex, uint8_t *memAddress, int datasize, int offset_in_block)
{
  START_TIMER;
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("LoadCustomTaskSettings"));
  #endif
  String result = LoadFromFile(SettingsType::Enum::CustomTaskSettings_Type, TaskIndex, memAddress, datasize, offset_in_block);
  STOP_TIMER(LOAD_CUSTOM_TASK_STATS);
  return result;
}

/********************************************************************************************\
   Load array of Strings from Custom Task settings
   Use maxStringLength = 0 to optimize for size (strings will be concatenated)
 \*********************************************************************************************/
String LoadCustomTaskSettings(taskIndex_t TaskIndex, String strings[], uint16_t nrStrings, uint16_t maxStringLength, uint32_t offset_in_block)
{
  START_TIMER;
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("LoadCustomTaskSettings"));
  #endif
  String result = LoadStringArray(SettingsType::Enum::CustomTaskSettings_Type,
                           TaskIndex,
                           strings, nrStrings, maxStringLength, offset_in_block);
  STOP_TIMER(LOAD_CUSTOM_TASK_STATS);
  return result;
}

/********************************************************************************************\
   Save Controller settings to file system
 \*********************************************************************************************/
String SaveControllerSettings(controllerIndex_t ControllerIndex, ControllerSettingsStruct& controller_settings)
{
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("SaveControllerSettings"));
  #endif
  controller_settings.validate(); // Make sure the saved controller settings have proper values.
  return SaveToFile(SettingsType::Enum::ControllerSettings_Type, ControllerIndex,
                    reinterpret_cast<const uint8_t *>(&controller_settings), sizeof(controller_settings));
}

/********************************************************************************************\
   Load Controller settings to file system
 \*********************************************************************************************/
String LoadControllerSettings(controllerIndex_t ControllerIndex, ControllerSettingsStruct& controller_settings) {
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("LoadControllerSettings"));
  #endif
  String result =
    LoadFromFile(SettingsType::Enum::ControllerSettings_Type, ControllerIndex,
                 reinterpret_cast<uint8_t *>(&controller_settings), sizeof(controller_settings));
  controller_settings.validate(); // Make sure the loaded controller settings have proper values.
  return result;
}

/********************************************************************************************\
   Clear Custom Controller settings
 \*********************************************************************************************/
String ClearCustomControllerSettings(controllerIndex_t ControllerIndex)
{
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("ClearCustomControllerSettings"));
  #endif

  // addLog(LOG_LEVEL_DEBUG, F("Clearing custom controller settings"));
  return ClearInFile(SettingsType::Enum::CustomControllerSettings_Type, ControllerIndex);
}

/********************************************************************************************\
   Save Custom Controller settings to file system
 \*********************************************************************************************/
String SaveCustomControllerSettings(controllerIndex_t ControllerIndex, const uint8_t *memAddress, int datasize)
{
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("SaveCustomControllerSettings"));
  #endif
  return SaveToFile(SettingsType::Enum::CustomControllerSettings_Type, ControllerIndex, memAddress, datasize);
}

/********************************************************************************************\
   Load Custom Controller settings to file system
 \*********************************************************************************************/
String LoadCustomControllerSettings(controllerIndex_t ControllerIndex, uint8_t *memAddress, int datasize)
{
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("LoadCustomControllerSettings"));
  #endif
  return LoadFromFile(SettingsType::Enum::CustomControllerSettings_Type, ControllerIndex, memAddress, datasize);
}


#if FEATURE_CUSTOM_PROVISIONING
/********************************************************************************************\
   Save Provisioning Settings
 \*********************************************************************************************/
String saveProvisioningSettings(ProvisioningStruct& ProvisioningSettings)
{
  String     err;

  ProvisioningSettings.validate();
  memcpy(ProvisioningSettings.ProgmemMd5, CRCValues.runTimeMD5, 16);
  if (!COMPUTE_STRUCT_CHECKSUM_UPDATE(ProvisioningStruct, ProvisioningSettings))
  {
    // Settings have changed, save to file.
    err = SaveToFile_trunc(getFileName(FileType::PROVISIONING_DAT, 0).c_str(), 0, (uint8_t *)&ProvisioningSettings, sizeof(ProvisioningStruct));
  }
  return err;
}

/********************************************************************************************\
   Load Provisioning Settings
 \*********************************************************************************************/
String loadProvisioningSettings(ProvisioningStruct& ProvisioningSettings)
{
  String err = LoadFromFile(getFileName(FileType::PROVISIONING_DAT, 0).c_str(), 0, (uint8_t *)&ProvisioningSettings, sizeof(ProvisioningStruct));

  if (COMPUTE_STRUCT_CHECKSUM(ProvisioningStruct, ProvisioningSettings))
  {
    addLog(LOG_LEVEL_INFO, F("CRC  : ProvisioningSettings CRC   ...OK "));

    if (memcmp(ProvisioningSettings.ProgmemMd5, CRCValues.runTimeMD5, 16) != 0) {
      addLog(LOG_LEVEL_INFO, F("CRC  : binary has changed since last save of Settings"));
    }
  }
  else {
    addLog(LOG_LEVEL_ERROR, F("CRC  : ProvisioningSettings CRC   ...FAIL"));
  }
  ProvisioningSettings.validate();
  return err;
}

#endif

#if FEATURE_NOTIFIER
/********************************************************************************************\
   Save Controller settings to file system
 \*********************************************************************************************/
String SaveNotificationSettings(int NotificationIndex, const uint8_t *memAddress, int datasize)
{
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("SaveNotificationSettings"));
  #endif
  return SaveToFile(SettingsType::Enum::NotificationSettings_Type, NotificationIndex, memAddress, datasize);
}

/********************************************************************************************\
   Load Controller settings to file system
 \*********************************************************************************************/
String LoadNotificationSettings(int NotificationIndex, uint8_t *memAddress, int datasize)
{
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("LoadNotificationSettings"));
  #endif
  return LoadFromFile(SettingsType::Enum::NotificationSettings_Type, NotificationIndex, memAddress, datasize);
}
#endif
/********************************************************************************************\
   Init a file with zeros on file system
 \*********************************************************************************************/
String InitFile(const String& fname, int datasize)
{
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("InitFile"));
  #endif
  FLASH_GUARD();

  fs::File f = tryOpenFile(fname, "w");

  if (f) {
    for (int x = 0; x < datasize; x++)
    {
      // See https://github.com/esp8266/Arduino/commit/b1da9eda467cc935307d553692fdde2e670db258#r32622483
      uint8_t zero_value = 0;
      SPIFFS_CHECK(f.write(&zero_value, 1), fname.c_str());
    }
    f.close();
  }

  // OK
  return EMPTY_STRING;
}

String InitFile(SettingsType::Enum settingsType)
{
  return InitFile(SettingsType::getSettingsFile(settingsType));
}

String InitFile(SettingsType::SettingsFileEnum file_type)
{
  return InitFile(SettingsType::getSettingsFileName(file_type), 
                  SettingsType::getInitFileSize(file_type));
}

/********************************************************************************************\
   Save data into config file on file system
 \*********************************************************************************************/
String SaveToFile(const char *fname, int index, const uint8_t *memAddress, int datasize)
{
  return doSaveToFile(fname, index, memAddress, datasize, "r+");
}

String SaveToFile_trunc(const char *fname, int index, const uint8_t *memAddress, int datasize)
{
  return doSaveToFile(fname, index, memAddress, datasize, "w+");
}

// See for mode description: https://github.com/esp8266/Arduino/blob/master/doc/filesystem.rst
String doSaveToFile(const char *fname, int index, const uint8_t *memAddress, int datasize, const char *mode)
{
#ifndef BUILD_NO_DEBUG
#ifndef ESP32

  if (allocatedOnStack(memAddress)) {
    String log = F("SaveToFile: ");
    log += fname;
    log += F(" ERROR, Data allocated on stack");
    addLog(LOG_LEVEL_ERROR, log);

    //    return log;  // FIXME TD-er: Should this be considered a breaking error?
  }
#endif // ifndef ESP32
#endif

  if (index < 0) {
    #ifndef BUILD_NO_DEBUG
    String log = F("SaveToFile: ");
    log += fname;
    log += F(" ERROR, invalid position in file");
    #else
    String log = F("Save error");
    #endif
    addLog(LOG_LEVEL_ERROR, log);
    return log;
  }
  START_TIMER;
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("SaveToFile"));
  #endif
  FLASH_GUARD();
  
  #ifndef BUILD_NO_DEBUG
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("SaveToFile: free stack: ");
    log += getCurrentFreeStack();
    addLogMove(LOG_LEVEL_INFO, log);
  }
  #endif
  delay(1);
  unsigned long timer = millis() + 50;
  fs::File f          = tryOpenFile(fname, mode);

  if (f) {
    clearAllCaches();
    SPIFFS_CHECK(f,                          fname);
    SPIFFS_CHECK(f.seek(index, fs::SeekSet), fname);
    const uint8_t *pointerToByteToSave = memAddress;

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
    #ifndef BUILD_NO_DEBUG
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log;
      log.reserve(48);
      log += F("FILE : Saved ");
      log += fname;
      log += F(" offset: ");
      log += index;
      log += F(" size: ");
      log += datasize;
      addLogMove(LOG_LEVEL_INFO, log);
    }
    #endif
  } else {
    #ifndef BUILD_NO_DEBUG
    String log = F("SaveToFile: ");
    log += fname;
    log += F(" ERROR, Cannot save to file");
    #else
    String log = F("Save error");
    #endif

    addLog(LOG_LEVEL_ERROR, log);
    return log;
  }
  STOP_TIMER(SAVEFILE_STATS);
  #ifndef BUILD_NO_DEBUG
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("SaveToFile: free stack after: ");
    log += getCurrentFreeStack();
    addLogMove(LOG_LEVEL_INFO, log);
  }
  #endif

  // OK
  return EMPTY_STRING;
}

/********************************************************************************************\
   Clear a certain area in a file (set to 0)
 \*********************************************************************************************/
String ClearInFile(const char *fname, int index, int datasize)
{
  if (index < 0) {
    #ifndef BUILD_NO_DEBUG
    String log = F("ClearInFile: ");
    log += fname;
    log += F(" ERROR, invalid position in file");
    #else
    String log = F("Save error");
    #endif

    addLog(LOG_LEVEL_ERROR, log);
    return log;
  }

  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("ClearInFile"));
  #endif
  FLASH_GUARD();

  fs::File f = tryOpenFile(fname, "r+");

  if (f) {
    SPIFFS_CHECK(f.seek(index, fs::SeekSet), fname);

    for (int x = 0; x < datasize; x++)
    {
      // See https://github.com/esp8266/Arduino/commit/b1da9eda467cc935307d553692fdde2e670db258#r32622483
      uint8_t zero_value = 0;
      SPIFFS_CHECK(f.write(&zero_value, 1), fname);
    }
    f.close();
  } else {
    #ifndef BUILD_NO_DEBUG
    String log = F("ClearInFile: ");
    log += fname;
    log += F(" ERROR, Cannot save to file");
    #else
    String log = F("Save error");
    #endif
    addLog(LOG_LEVEL_ERROR, log);
    return log;
  }

  // OK
  return EMPTY_STRING;
}

/********************************************************************************************\
   Load data from config file on file system
 \*********************************************************************************************/
String LoadFromFile(const char *fname, int offset, uint8_t *memAddress, int datasize)
{
  if (offset < 0) {
    #ifndef BUILD_NO_DEBUG
    String log = F("LoadFromFile: ");
    log += fname;
    log += F(" ERROR, invalid position in file");
    #else
    String log = F("Load error");
    #endif
    addLog(LOG_LEVEL_ERROR, log);
    return log;
  }
  delay(0);
  START_TIMER;
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("LoadFromFile"));
  #endif
  fs::File f = tryOpenFile(fname, "r");
  SPIFFS_CHECK(f,                            fname);
  SPIFFS_CHECK(f.seek(offset, fs::SeekSet),  fname);
  SPIFFS_CHECK(f.read(memAddress, datasize), fname);
  f.close();

  STOP_TIMER(LOADFILE_STATS);
  delay(0);

  return EMPTY_STRING;
}

/********************************************************************************************\
   Wrapper functions to handle errors in accessing settings
 \*********************************************************************************************/
String getSettingsFileIndexRangeError(bool read, SettingsType::Enum settingsType, int index) {
  if (settingsType >= SettingsType::Enum::SettingsType_MAX) {
    String error = F("Unknown settingsType: ");
    error += static_cast<int>(settingsType);
    return error;
  }
  String error = read ? F("Load") : F("Save");
  #ifndef BUILD_NO_DEBUG
  error += SettingsType::getSettingsTypeString(settingsType);
  error += F(" index out of range: ");
  error += index;
  #else
  error += F(" error");
  #endif
  return error;
}

String getSettingsFileDatasizeError(bool read, SettingsType::Enum settingsType, int index, int datasize, int max_size) {
  String error = read ? F("Load") : F("Save");
  #ifndef BUILD_NO_DEBUG
  error += SettingsType::getSettingsTypeString(settingsType);
  error += '(';
  error += index;
  error += F(") datasize(");
  error += datasize;
  error += F(") > max_size(");
  error += max_size;
  error += ')';
  #else
  error += F(" error");
  #endif
  
  return error;
}

String LoadFromFile(SettingsType::Enum settingsType, int index, uint8_t *memAddress, int datasize, int offset_in_block) {
  bool read = true;
  int  offset, max_size;

  if (!getAndLogSettingsParameters(read, settingsType, index, offset, max_size)) {
    return getSettingsFileIndexRangeError(read, settingsType, index);
  }

  if ((datasize + offset_in_block) > max_size) {
    return getSettingsFileDatasizeError(read, settingsType, index, datasize, max_size);
  }
  const String fname = SettingsType::getSettingsFileName(settingsType);
  return LoadFromFile(fname.c_str(), (offset + offset_in_block), memAddress, datasize);
}

String SaveToFile(SettingsType::Enum settingsType, int index, const uint8_t *memAddress, int datasize, int posInBlock) {
  bool read = false;
  int  offset, max_size;

  if (!getAndLogSettingsParameters(read, settingsType, index, offset, max_size)) {
    return getSettingsFileIndexRangeError(read, settingsType, index);
  }

  if ((datasize > max_size) || ((posInBlock + datasize) > max_size)) {
    return getSettingsFileDatasizeError(read, settingsType, index, datasize, max_size);
  }
  const String fname = SettingsType::getSettingsFileName(settingsType);
  if (!fileExists(fname)) {
    InitFile(settingsType);
  }
  return SaveToFile(fname.c_str(), offset + posInBlock, memAddress, datasize);
}

String ClearInFile(SettingsType::Enum settingsType, int index) {
  bool read = false;
  int  offset, max_size;

  if (!getAndLogSettingsParameters(read, settingsType, index, offset, max_size)) {
    return getSettingsFileIndexRangeError(read, settingsType, index);
  }
  const String fname = SettingsType::getSettingsFileName(settingsType);
  return ClearInFile(fname.c_str(), offset, max_size);
}

/********************************************************************************************\
   Check file system area settings
 \*********************************************************************************************/
int SpiffsSectors()
{
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("SpiffsSectors"));
  #endif
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
  result = ESPEASY_FS.usedBytes();
  #endif // ifdef ESP32
  #ifdef ESP8266
  fs::FSInfo fs_info;
  ESPEASY_FS.info(fs_info);
  result = fs_info.usedBytes;
  #endif // ifdef ESP8266
  return result;
}

size_t SpiffsTotalBytes() {
  static size_t result = 1; // Do not output 0, this may be used in divisions.
  if (result == 1) {
    #ifdef ESP32
    result = ESPEASY_FS.totalBytes();
    #endif // ifdef ESP32
    #ifdef ESP8266
    fs::FSInfo fs_info;
    ESPEASY_FS.info(fs_info);
    result = fs_info.totalBytes;
    #endif // ifdef ESP8266
  }
  return result;
}

size_t SpiffsBlocksize() {
  static size_t result = 1;
  if (result == 1) {
    #ifdef ESP32
    result = 8192;        // Just assume 8k, since we cannot query it
    #endif // ifdef ESP32
    #ifdef ESP8266
    fs::FSInfo fs_info;
    ESPEASY_FS.info(fs_info);
    result = fs_info.blockSize;
    #endif // ifdef ESP8266
  }
  return result;
}

size_t SpiffsPagesize() {
  static size_t result = 1;
  if (result == 1) {
    #ifdef ESP32
    result = 256;        // Just assume 256, since we cannot query it
    #endif // ifdef ESP32
    #ifdef ESP8266
    fs::FSInfo fs_info;
    ESPEASY_FS.info(fs_info);
    result = fs_info.pageSize;
    #endif // ifdef ESP8266
  }
  return result;
}

size_t SpiffsFreeSpace() {
  int freeSpace = SpiffsTotalBytes() - SpiffsUsedBytes();
  const size_t blocksize = SpiffsBlocksize();

  if (freeSpace < static_cast<int>(2 * blocksize)) {
    // Not enough free space left to store anything
    // There needs to be minimum of 2 free blocks.
    return 0;
  }
  return freeSpace - 2 * blocksize;
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
  fname += F("cache_");
  fname += String(count);
  fname += F(".bin");
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
  fs::Dir dir = ESPEASY_FS.openDir(F("cache"));

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
  fs::File root = ESPEASY_FS.open(F("/"));
  fs::File file = root.openNextFile();

  while (file)
  {
    if (!file.isDirectory()) {
      const String fname(file.name());
      if (fname.startsWith(F("/cache")) || fname.startsWith(F("cache"))) {
        int count = getCacheFileCountFromFilename(fname);

        if (count >= 0) {
          if (lowest > count) {
            lowest = count;
          }

          if (highest < count) {
            highest         = count;
            filesizeHighest = file.size();
          }
        } else {
          addLog(LOG_LEVEL_INFO, String(F("RTC  : Cannot get count from: ")) + fname);
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
String getPartitionType(uint8_t pType, uint8_t pSubType) {
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

  if (partitionSubType == 0x99) {
    return F("EEPROM"); // Not defined in esp_partition_subtype_t
  }

  if (partitionType == ESP_PARTITION_TYPE_DATA) {
    switch (partitionSubType) {
      case ESP_PARTITION_SUBTYPE_DATA_OTA:      return F("OTA selection");
      case ESP_PARTITION_SUBTYPE_DATA_PHY:      return F("PHY init data");
      case ESP_PARTITION_SUBTYPE_DATA_NVS:      return F("NVS");
      case ESP_PARTITION_SUBTYPE_DATA_COREDUMP: return F("COREDUMP");
      case ESP_PARTITION_SUBTYPE_DATA_ESPHTTPD: return F("ESPHTTPD");
      case ESP_PARTITION_SUBTYPE_DATA_FAT:      return F("FAT");
      case ESP_PARTITION_SUBTYPE_DATA_SPIFFS:   
        #ifdef USE_LITTLEFS
        return F("LittleFS");
        #else
        return F("SPIFFS");
        #endif
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

String getPartitionTable(uint8_t pType, const String& itemSep, const String& lineEnd) {
  esp_partition_type_t partitionType = static_cast<esp_partition_type_t>(pType);
  String result;
  esp_partition_iterator_t _mypartiterator = esp_partition_find(partitionType, ESP_PARTITION_SUBTYPE_ANY, nullptr);

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
    } while ((_mypartiterator = esp_partition_next(_mypartiterator)) != nullptr);
  }
  esp_partition_iterator_release(_mypartiterator);
  return result;
}

#endif // ifdef ESP32

#if FEATURE_DOWNLOAD
String downloadFileType(const String& url, const String& user, const String& pass, FileType::Enum filetype, unsigned int filenr)
{
  if (!getDownloadFiletypeChecked(filetype, filenr)) {
    // Not selected, so not downloaded
    return F("Not Allowed");
  }

  String filename = getFileName(filetype, filenr);
  String fullUrl;

  fullUrl.reserve(url.length() + filename.length() + 1); // May need to add an extra slash
  fullUrl = url;
  fullUrl = parseTemplate(fullUrl, true);                // URL encode

  // URLEncode may also encode the '/' into "%2f"
  // FIXME TD-er: Can this really occur?
  fullUrl.replace(F("%2f"), F("/"));

  while (filename.startsWith(F("/"))) {
    filename = filename.substring(1);
  }

  if (!fullUrl.endsWith(F("/"))) {
    fullUrl += F("/");
  }
  fullUrl += filename;

  String error;

  if (ResetFactoryDefaultPreference.deleteFirst()) {
    if (fileExists(filename) && !tryDeleteFile(filename)) {
      return F("Could not delete existing file");
    }

    if (!downloadFile(fullUrl, filename, user, pass, error)) {
      return error;
    }
  } else {
    if (fileExists(filename)) {
      String filename_bak = filename;
      filename_bak += F("_bak");
      if (fileExists(filename_bak)) {
        return F("Could not rename to _bak");
      }

      // Must download it to a tmp file.
      String tmpfile = filename;
      tmpfile += F("_tmp");

      if (!downloadFile(fullUrl, tmpfile, user, pass, error)) {
        return error;
      }

      if (fileExists(filename) && !tryRenameFile(filename, filename_bak)) {
        return F("Could not rename to _bak");
      } else {
        // File does not exist (anymore)
        if (!tryRenameFile(tmpfile, filename)) {
          error = F("Could not rename tmp file");

          if (tryRenameFile(filename_bak, filename)) {
            error += F("... reverted");
          } else {
            error += F(" Not reverted!");
          }
          return error;
        }
      }
    } else {
      if (!downloadFile(fullUrl, filename, user, pass, error)) {
        return error;
      }
    }
  }
  return error;
}

#endif // if FEATURE_DOWNLOAD

#if FEATURE_CUSTOM_PROVISIONING

String downloadFileType(FileType::Enum filetype, unsigned int filenr)
{
  if (!ResetFactoryDefaultPreference.allowFetchByCommand()) {
    return F("Not Allowed");
  }
  String url, user, pass;

  {
    MakeProvisioningSettings(ProvisioningSettings);

    if (AllocatedProvisioningSettings()) {
      loadProvisioningSettings(ProvisioningSettings);
      url  = ProvisioningSettings.url;
      user = ProvisioningSettings.user;
      pass = ProvisioningSettings.pass;
    }
  }
  return downloadFileType(url, user, pass, filetype, filenr);
}

#endif // if FEATURE_CUSTOM_PROVISIONING
