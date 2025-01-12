#include "../Helpers/ESPEasy_Storage.h"

#include "../../ESPEasy_common.h"

#include "../CustomBuild/CompiletimeDefines.h"
#include "../CustomBuild/StorageLayout.h"

#include "../DataStructs/TimingStats.h"

#include "../DataTypes/ESPEasyFileType.h"
#include "../DataTypes/ESPEasyTimeSource.h"
#include "../DataTypes/SPI_options.h"

#if FEATURE_MQTT
# include "../ESPEasyCore/Controller.h"
#endif // if FEATURE_MQTT
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
#include "../Globals/RuntimeData.h"
#include "../Globals/SecuritySettings.h"
#include "../Globals/Settings.h"
#include "../Globals/WiFi_AP_Candidates.h"

#include "../Helpers/ESPEasyRTC.h"
#include "../Helpers/ESPEasy_checks.h"
#include "../Helpers/ESPEasy_FactoryDefault.h"
#include "../Helpers/ESPEasy_time_calc.h"
#include "../Helpers/FS_Helper.h"
#include "../Helpers/Hardware.h"
#include "../Helpers/Hardware_device_info.h"
#include "../Helpers/Memory.h"
#include "../Helpers/Misc.h"
#include "../Helpers/Networking.h"
#include "../Helpers/Numerical.h"
#include "../Helpers/PeriodicalActions.h"
#include "../Helpers/StringConverter.h"
#include "../Helpers/StringParser.h"
#if FEATURE_MQTT
#include "../Helpers/_CPlugin_Helper.h"
#endif

#if FEATURE_RTC_CACHE_STORAGE
# include "../Globals/C016_ControllerCache.h"
#endif // if FEATURE_RTC_CACHE_STORAGE

#ifdef ESP32
# include <esp_partition.h>
#endif // ifdef ESP32

#ifdef ESP32
String patch_fname(const String& fname) {
  if (fname.startsWith(F("/"))) {
    return fname;
  }
  return String('/') + fname;
}

#endif // ifdef ESP32

/********************************************************************************************\
   file system error handling
   Look here for error # reference: https://github.com/pellepl/spiffs/blob/master/src/spiffs.h
 \*********************************************************************************************/
String FileError(int line, const char *fname)
{
  String log = strformat(F("FS   : Error while reading/writing %s in %d"), fname, line);

  addLog(LOG_LEVEL_ERROR, log);
  return log;
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
  #endif // ifndef BUILD_NO_RAM_TRACKER

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
  return EMPTY_STRING;
}

bool fileExists(const __FlashStringHelper *fname)
{
  return fileExists(String(fname));
}

bool fileExists(const String& fname) {
  #ifdef USE_SECOND_HEAP
  HeapSelectDram ephemeral;
  #endif // ifdef USE_SECOND_HEAP

  const String patched_fname = patch_fname(fname);
  auto search                = Cache.fileExistsMap.find(patched_fname);

  if (search != Cache.fileExistsMap.end()) {
    return search->second;
  }
  bool res = ESPEASY_FS.exists(patched_fname);
  #if FEATURE_SD

  if (!res) {
    res = SD.exists(patched_fname);
  }
  #endif // if FEATURE_SD

  // Only keep track of existing files or non-existing filenames that may be requested several times.
  // Not the non-existing files from the cache controller
  #if FEATURE_RTC_CACHE_STORAGE

  if (res || !isCacheFile(patched_fname))
  #endif // if FEATURE_RTC_CACHE_STORAGE
  {
    Cache.fileExistsMap.emplace(
      std::make_pair(
        patched_fname,
        res));
  }

  if (Cache.fileCacheClearMoment == 0) {
    if (node_time.getTimeSource() == timeSource_t::No_time_source) {
      // use some random value as we don't have a time yet
      Cache.fileCacheClearMoment = HwRandom();
    } else {
      Cache.fileCacheClearMoment = node_time.getLocalUnixTime();
    }
  }
  return res;
}

fs::File tryOpenFile(const String& fname, const String& mode, FileDestination_e destination) {
  START_TIMER;
  fs::File f;

  if (fname.isEmpty() || equals(fname, '/')) {
    return f;
  }

  bool exists = fileExists(fname);

  if (!exists) {
    if (equals(mode, 'r')) {
      return f;
    }
    clearFileCaches();
  }

  if ((destination == FileDestination_e::ANY) || (destination == FileDestination_e::FLASH)) {
    f = ESPEASY_FS.open(patch_fname(fname), mode.c_str());
  }
  #if FEATURE_SD

  if (!f && ((destination == FileDestination_e::ANY) || (destination == FileDestination_e::SD))) {
    f = SD.open(patch_fname(fname).c_str(), mode.c_str());
  }
  #endif // if FEATURE_SD


  STOP_TIMER(TRY_OPEN_FILE);
  return f;
}

bool fileMatchesTaskSettingsType(const String& fname) {
  const String config_dat_file = patch_fname(getFileName(FileType::CONFIG_DAT));

  return config_dat_file.equalsIgnoreCase(patch_fname(fname));
}

bool tryRenameFile(const String& fname_old, const String& fname_new, FileDestination_e destination) {
  clearFileCaches();

  if (fileExists(fname_old) && !fileExists(fname_new)) {
    if (fileMatchesTaskSettingsType(fname_old)) {
      clearAllCaches();
    } else {
      clearAllButTaskCaches();
    }
    bool res = false;

    if ((destination == FileDestination_e::ANY) || (destination == FileDestination_e::FLASH)) {
      res = ESPEASY_FS.rename(patch_fname(fname_old), patch_fname(fname_new));
    }
    #if FEATURE_SD && defined(ESP32) // FIXME ESP8266 SDClass doesn't support rename

    if (!res && ((destination == FileDestination_e::ANY) || (destination == FileDestination_e::SD))) {
      res = SD.rename(patch_fname(fname_old), patch_fname(fname_new));
    }
    #endif // if FEATURE_SD && defined(ESP32)
    return res;
  }
  return false;
}

bool tryDeleteFile(const String& fname, FileDestination_e destination) {
  if (fname.length() > 0)
  {
    #if FEATURE_RTC_CACHE_STORAGE

    if (isCacheFile(fname)) {
      ControllerCache.closeOpenFiles();
    }
    #endif // if FEATURE_RTC_CACHE_STORAGE

    if (fileMatchesTaskSettingsType(fname)) {
      clearAllCaches();
    } else {
      clearAllButTaskCaches();
    }
    bool res = false;

    if ((destination == FileDestination_e::ANY) || (destination == FileDestination_e::FLASH)) {
      res = ESPEASY_FS.remove(patch_fname(fname));
    }
    #if FEATURE_SD

    if (!res && ((destination == FileDestination_e::ANY) || (destination == FileDestination_e::SD))) {
      res = SD.remove(patch_fname(fname));
    }
    #endif // if FEATURE_SD

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
bool BuildFixes()
{
  if (Settings.Build == get_build_nr()) {
    // Not changed
    return false;
  }
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("BuildFixes"));
  #endif // ifndef BUILD_NO_RAM_TRACKER
  serialPrintln(F("\nBuild changed!"));

  if (Settings.Build < 145)
  {
    InitFile(SettingsType::SettingsFileEnum::FILE_NOTIFICATION_type);
  }

  if (Settings.Build < 20101)
  {
    #ifdef LIMIT_BUILD_SIZE
    serialPrintln(F("Fix reset Pin"));
    #endif // ifdef LIMIT_BUILD_SIZE
    Settings.Pin_Reset = -1;
  }

  if (Settings.Build < 20102) {
    // Settings were 'mangled' by using older version
    // Have to patch settings to make sure no bogus data is being used.
    #ifdef LIMIT_BUILD_SIZE
    serialPrintln(F("Fix settings with uninitalized data or corrupted by switching between versions"));
    #endif // ifdef LIMIT_BUILD_SIZE
    Settings.UseRTOSMultitasking              = false;
    Settings.Pin_Reset                        = -1;
    Settings.SyslogFacility                   = DEFAULT_SYSLOG_FACILITY;
    Settings.MQTTUseUnitNameAsClientId_unused = DEFAULT_MQTT_USE_UNITNAME_AS_CLIENTID;
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
      MakeControllerSettings(ControllerSettings); // -V522

      if (AllocatedControllerSettings()) {
        LoadControllerSettings(controller_idx, *ControllerSettings);

        String clientid;

        if (Settings.MQTTUseUnitNameAsClientId_unused) {
          clientid = F("%sysname%");

          if (Settings.appendUnitToHostname()) {
            clientid += F("_%unit%");
          }
        }
        else {
          clientid = F("ESPClient_%mac%");
        }
        safe_strncpy(ControllerSettings->ClientID, clientid, sizeof(ControllerSettings->ClientID));

        ControllerSettings->mqtt_uniqueMQTTclientIdReconnect(Settings.uniqueMQTTclientIdReconnect_unused());
        ControllerSettings->mqtt_retainFlag(Settings.MQTTRetainFlag_unused);
        SaveControllerSettings(controller_idx, *ControllerSettings);
      }
    }
    #endif // if FEATURE_MQTT
  }

  if (Settings.Build < 20107) {
    Settings.WebserverPort = 80;
  }

  if (Settings.Build < 20108) {
#ifdef ESP32

    // Ethernet related settings are never used on ESP8266
    Settings.ETH_Phy_Addr      = DEFAULT_ETH_PHY_ADDR;
    Settings.ETH_Pin_mdc_cs    = DEFAULT_ETH_PIN_MDC;
    Settings.ETH_Pin_mdio_irq  = DEFAULT_ETH_PIN_MDIO;
    Settings.ETH_Pin_power_rst = DEFAULT_ETH_PIN_POWER;
    Settings.ETH_Phy_Type      = DEFAULT_ETH_PHY_TYPE;
    Settings.ETH_Clock_Mode    = DEFAULT_ETH_CLOCK_MODE;
#endif // ifdef ESP32
    Settings.NetworkMedium = DEFAULT_NETWORK_MEDIUM;
  }

  if (Settings.Build < 20109) {
    Settings.SyslogPort = 514;
  }

  if (Settings.Build < 20110) {
    Settings.I2C_clockSpeed_Slow  = DEFAULT_I2C_CLOCK_SPEED_SLOW;
    Settings.I2C_Multiplexer_Type = I2C_MULTIPLEXER_NONE;
    Settings.I2C_Multiplexer_Addr = -1;

    for (taskIndex_t x = 0; x < TASKS_MAX; x++) {
      Settings.I2C_Multiplexer_Channel[x] = -1;
    }
    Settings.I2C_Multiplexer_ResetPin = -1;
  }

  if (Settings.Build < 20111) {
    #ifdef ESP32
    constexpr uint8_t maxStatesesp32 = NR_ELEMENTS(Settings.PinBootStates_ESP32);

    for (uint8_t i = 0; i < maxStatesesp32; ++i) {
      Settings.PinBootStates_ESP32[i] = 0;
    }
    #endif // ifdef ESP32
  }

  if (Settings.Build < 20112) {
    Settings.WiFi_TX_power           = 70; // 70 = 17.5dBm. unit: 0.25 dBm
    Settings.WiFi_sensitivity_margin = 3;  // Margin in dBm on top of sensitivity.
  }

  if (Settings.Build < 20113) {
    Settings.NumberExtraWiFiScans = 0;
  }

  if (Settings.Build < 20114) {
    #ifdef USES_P003

    // P003_Pulse was always using the pull-up, now it is a setting.
    constexpr pluginID_t PLUGIN_ID_P003_PULSE(3);

    for (taskIndex_t taskIndex = 0; taskIndex < TASKS_MAX; ++taskIndex) {
      if (Settings.getPluginID_for_task(taskIndex) == PLUGIN_ID_P003_PULSE) {
        Settings.TaskDevicePin1PullUp[taskIndex] = true;
      }
    }
    #endif // ifdef USES_P003
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
    constexpr pluginID_t PLUGIN_ID_P053_PMSx003(53);

    for (taskIndex_t taskIndex = 0; taskIndex < TASKS_MAX; ++taskIndex) {
      if (Settings.getPluginID_for_task(taskIndex) == PLUGIN_ID_P053_PMSx003) {
        Settings.TaskDevicePluginConfig[taskIndex][3] = -1;
      }
    }

    // Remove PeriodicalScanWiFi
    // Reset to default 0 for future use.
    Settings.VariousBits_1.unused_15 = 0;
  }
  #endif // ifdef USES_P053

  if (Settings.Build <= 21066) { // 2024-12-31
    // (MQTT) Keep Alive time-out is now defined in the controller settings.
    #if FEATURE_MQTT
    for (controllerIndex_t controller_idx = 0; controller_idx < CONTROLLER_MAX; ++controller_idx) {
    // controllerIndex_t controller_idx = firstEnabledMQTT_ControllerIndex();
      const protocolIndex_t protocol_idx = getProtocolIndex_from_ControllerIndex(controller_idx);

    if (validProtocolIndex(protocol_idx) && getProtocolStruct(protocol_idx).usesMQTT) {
      MakeControllerSettings(ControllerSettings); // -V522

      if (AllocatedControllerSettings()) {
        LoadControllerSettings(controller_idx, *ControllerSettings);

        // Used to be hardcoded at 10 sec., now default 60 sec. and configurable
        ControllerSettings->KeepAliveTime = CONTROLLER_KEEP_ALIVE_TIME_DFLT;
        SaveControllerSettings(controller_idx, *ControllerSettings);
      }
    }
    }
    #endif // if FEATURE_MQTT
  }

  // Starting 2022/08/18
  // Use get_build_nr() value for settings transitions.
  // This value will also be shown when building using PlatformIO, when showing the  Compile time defines
  Settings.Build      = get_build_nr();
  Settings.StructSize = sizeof(Settings);

  // We may have changed the settings, so update checksum.
  // This way we save settings less often as these changes are always reproducible via this
  // settings transitions function.

  return !COMPUTE_STRUCT_CHECKSUM_UPDATE(SettingsStruct, Settings);
}

/********************************************************************************************\
   Mount FS and check config.dat
 \*********************************************************************************************/
void fileSystemCheck()
{
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("fileSystemCheck"));
  #endif // ifndef BUILD_NO_RAM_TRACKER
  addLog(LOG_LEVEL_INFO, F("FS   : Mounting..."));
#if defined(ESP32) && defined(USE_LITTLEFS)

  if ((getPartionCount(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_SPIFFS) != 0)
      && ESPEASY_FS.begin())
#else // if defined(ESP32) && defined(USE_LITTLEFS)

  if (ESPEASY_FS.begin())
#endif // if defined(ESP32) && defined(USE_LITTLEFS)
  {
    clearAllCaches();

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      addLogMove(LOG_LEVEL_INFO, strformat(
                   F("FS   : "
#ifdef USE_LITTLEFS
                     "LittleFS"
#else // ifdef USE_LITTLEFS
                     "SPIFFS"
#endif // ifdef USE_LITTLEFS
                     " mount successful, used %u bytes of %u"),
                   SpiffsUsedBytes(), SpiffsTotalBytes()));
    }

    // Run garbage collection before any file is open.
    uint8_t retries = 3;

    while (retries > 0 && GarbageCollection()) {
      --retries;
    }

    fs::File f = tryOpenFile(SettingsType::getSettingsFileName(SettingsType::Enum::BasicSettings_Type), "r");

    if (f) {
      f.close();
    } else {
      ResetFactory(false);
    }
  }
  else
  {
    const __FlashStringHelper *log = F("FS   : Mount failed");
    serialPrintln(log);
    addLog(LOG_LEVEL_ERROR, log);
    ResetFactory();
  }
}

bool FS_format() {
   #ifdef USE_LITTLEFS
     # ifdef ESP32
  const bool res = ESPEASY_FS.begin(true);
  ESPEASY_FS.end();
  return res;
     # else // ifdef ESP32
  return ESPEASY_FS.format();
     # endif // ifdef ESP32
   #else // ifdef USE_LITTLEFS
  return ESPEASY_FS.format();

   #endif // ifdef USE_LITTLEFS
}

#ifdef ESP32

# include <esp_partition.h>

int getPartionCount(uint8_t pType, uint8_t pSubType) {
  esp_partition_type_t partitionType       = static_cast<esp_partition_type_t>(pType);
  esp_partition_subtype_t  subtype         = static_cast<esp_partition_subtype_t>(pSubType);
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

#endif // ifdef ESP32

 #ifdef ESP8266
bool clearPartition(ESP8266_partition_type ptype) {
  uint32_t address;
  int32_t  size;
  int32_t  sector = getPartitionInfo(ESP8266_partition_type::rf_cal, address, size);

  while (size > 0) {
    if (!ESP.flashEraseSector(sector)) { return false; }
    ++sector;
    size -= SPI_FLASH_SEC_SIZE;
  }
  return true;
}

bool clearRFcalPartition() {
  return clearPartition(ESP8266_partition_type::rf_cal);
}

bool clearWiFiSDKpartition() {
  return clearPartition(ESP8266_partition_type::wifi);
}

#endif // ifdef ESP8266


/********************************************************************************************\
   Garbage collection
 \*********************************************************************************************/
bool GarbageCollection() {
  #ifdef CORE_POST_2_6_0

  // Perform garbage collection
  START_TIMER;

  if (ESPEASY_FS.gc()) {
# ifndef BUILD_NO_DEBUG
    addLog(LOG_LEVEL_INFO, F("FS   : Success garbage collection"));
# endif // ifndef BUILD_NO_DEBUG
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

/********************************************************************************************\
   Save settings to file system
 \*********************************************************************************************/
String SaveSettings(bool forFactoryReset)
{
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("SaveSettings"));
  #endif // ifndef BUILD_NO_RAM_TRACKER
  String err;
  {
    Settings.StructSize = sizeof(Settings);

    // FIXME @TD-er: As discussed in #1292, the CRC for the settings is now disabled.

    Settings.validate();
    initSerial();

    if (forFactoryReset) {
      Settings.forceSave();
    }

    if (!COMPUTE_STRUCT_CHECKSUM_UPDATE(SettingsStruct, Settings)

        /*
           computeChecksum(
            Settings.md5,
            reinterpret_cast<uint8_t *>(&Settings),
            sizeof(SettingsStruct),
            offsetof(SettingsStruct, md5))
         */
        ) {
      err = SaveToFile(SettingsType::getSettingsFileName(SettingsType::Enum::BasicSettings_Type).c_str(),
                       0,
                       reinterpret_cast<const uint8_t *>(&Settings),
                       sizeof(Settings));
    }
#ifndef BUILD_NO_DEBUG
    else {
      addLog(LOG_LEVEL_INFO, F("Skip saving settings, not changed"));
    }
#endif // ifndef BUILD_NO_DEBUG
  }

  if (err.length()) {
    return err;
  }

#ifndef BUILD_MINIMAL_OTA

  // Must check this after saving, or else it is not possible to fix multiple
  // issues which can only corrected on different pages.
  if (!SettingsCheck(err)) { return err; }
#endif // ifndef BUILD_MINIMAL_OTA

  //  }

  err = SaveSecuritySettings(forFactoryReset);

  return err;
}

String SaveSecuritySettings(bool forFactoryReset) {
  String err;

  SecuritySettings.validate();
  memcpy(SecuritySettings.ProgmemMd5, CRCValues.runTimeMD5, 16);

  if (forFactoryReset) {
    SecuritySettings.forceSave();
  }

  if (SecuritySettings.updateChecksum()) {
    // Settings have changed, save to file.
    err = SaveToFile(SettingsType::getSettingsFileName(SettingsType::Enum::SecuritySettings_Type).c_str(),
                     0,
                     reinterpret_cast<const uint8_t *>(&SecuritySettings),
                     sizeof(SecuritySettings));

    // Security settings are saved, may be update of WiFi settings or hostname.
    if (!forFactoryReset && !NetworkConnected()) {
      if (SecuritySettings.hasWiFiCredentials() && (active_network_medium == NetworkMedium_t::WIFI)) {
        WiFiEventData.wifiConnectAttemptNeeded = true;
        WiFi_AP_Candidates.force_reload(); // Force reload of the credentials and found APs from the last scan
        resetWiFi();
        AttemptWiFiConnect();
      }
    }
  }
#ifndef BUILD_NO_DEBUG
  else {
    addLog(LOG_LEVEL_INFO, F("Skip saving SecuritySettings, not changed"));
  }
#endif // ifndef BUILD_NO_DEBUG

  // FIXME TD-er: How to check if these have changed?
  if (forFactoryReset) {
    ExtendedControllerCredentials.clear();
  }

  ExtendedControllerCredentials.save();

  if (!forFactoryReset) {
    afterloadSettings();
  }
  return err;
}

void afterloadSettings() {
  ExtraTaskSettings.clear(); // make sure these will not contain old settings.

  if ((Settings.Version != VERSION) || (Settings.PID != ESP_PROJECT_PID)) {
    // Not valid settings, so do not continue
    return;
  }

  // Load ResetFactoryDefaultPreference from provisioning.dat if available.
  // FIXME TD-er: Must actually move content of Provisioning.dat to NVS and then delete file
  uint32_t pref_temp = Settings.ResetFactoryDefaultPreference;

  #ifdef ESP32

  if (pref_temp == 0) {
    if (ResetFactoryDefaultPreference.getPreference() == 0) {
      // Try loading from NVS
      ESPEasy_NVS_Helper preferences;
      ResetFactoryDefaultPreference.init(preferences);
      pref_temp = ResetFactoryDefaultPreference.getPreference();
    }
  }
  #endif // ifdef ESP32
  #if FEATURE_CUSTOM_PROVISIONING

  if (fileExists(getFileName(FileType::PROVISIONING_DAT))) {
    MakeProvisioningSettings(ProvisioningSettings);

    if (ProvisioningSettings.get()) {
      loadProvisioningSettings(*ProvisioningSettings);

      if (ProvisioningSettings->matchingFlashSize()) {
        if ((pref_temp == 0) && (ProvisioningSettings->ResetFactoryDefaultPreference.getPreference() != 0)) {
          pref_temp = ProvisioningSettings->ResetFactoryDefaultPreference.getPreference();
        }
      }
    }
  }
  #endif // if FEATURE_CUSTOM_PROVISIONING

  // TODO TD-er: Try to get the information from more locations to make it more persistent
  // Maybe EEPROM location?


  ResetFactoryDefaultPreference_struct pref(pref_temp);

  if (modelMatchingFlashSize(pref.getDeviceModel())) {
    ResetFactoryDefaultPreference = pref_temp;
  }
  applyFactoryDefaultPref();
  Scheduler.setEcoMode(Settings.EcoPowerMode());
  #ifdef ESP32
  setCpuFrequencyMhz(Settings.EcoPowerMode() ? getCPU_MinFreqMHz() : getCPU_MaxFreqMHz());
  #endif // ifdef ESP32

  if (!Settings.UseRules) {
    eventQueue.clear();
  }
  node_time.applyTimeZone();
  CheckRunningServices(); // To update changes in hostname.
}

/********************************************************************************************\
   Load settings from file system
 \*********************************************************************************************/
String LoadSettings()
{
  clearAllButTaskCaches();
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("LoadSettings"));
  #endif // ifndef BUILD_NO_RAM_TRACKER

  uint8_t oldSettingsChecksum[16] = { 0 };
  memcpy(oldSettingsChecksum, Settings.md5, 16);


  String err;

  err =
    LoadFromFile(SettingsType::getSettingsFileName(SettingsType::Enum::BasicSettings_Type).c_str(),
                 0,
                 reinterpret_cast<uint8_t *>(&Settings),
                 sizeof(SettingsStruct));

  if (memcmp(oldSettingsChecksum, Settings.md5, 16) != 0) {
    // File has changed, so need to flush all task caches.
    Cache.clearAllTaskCaches();
  }

  if (err.length()) {
    return err;
  }

  if (!BuildFixes()) {
    #ifndef BUILD_NO_DEBUG

    if (COMPUTE_STRUCT_CHECKSUM(SettingsStruct, Settings)) {
      addLog(LOG_LEVEL_INFO, concat(F("CRC  : Settings CRC"), F("...OK")));
    } else {
      addLog(LOG_LEVEL_ERROR, concat(F("CRC  : Settings CRC"), F("...FAIL")));
    }
    #endif // ifndef BUILD_NO_DEBUG
  }

  Settings.validate();
  initSerial();

  err =
    LoadFromFile(SettingsType::getSettingsFileName(SettingsType::Enum::SecuritySettings_Type).c_str(),
                 0,
                 reinterpret_cast<uint8_t *>(&SecuritySettings),
                 sizeof(SecurityStruct));

#ifndef BUILD_NO_DEBUG

  if (SecuritySettings.checksumMatch()) {
    addLog(LOG_LEVEL_INFO, concat(F("CRC  : SecuritySettings CRC"), F("...OK ")));

    if (memcmp(SecuritySettings.ProgmemMd5, CRCValues.runTimeMD5, 16) != 0) {
      addLog(LOG_LEVEL_INFO, F("CRC  : binary has changed since last save of Settings"));
    }
  }
  else {
    addLog(LOG_LEVEL_ERROR, concat(F("CRC  : SecuritySettings CRC"), F("...FAIL")));
  }
#endif // ifndef BUILD_NO_DEBUG

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
        // Disable temporarily as unit crashed
        // FIXME TD-er: Should this be stored?
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
      // Disable temporarily as unit crashed
      // FIXME TD-er: Should this be stored?
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

#endif // if FEATURE_NOTIFIER

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
    log += concat(F(" settings: "), SettingsType::getSettingsTypeString(settingsType));
    log += concat(F(" index: "), index);
    addLogMove(LOG_LEVEL_DEBUG_DEV, log);
  }
#endif // ifndef BUILD_NO_DEBUG
  return SettingsType::getSettingsParameters(settingsType, index, offset, max_size);
}

/********************************************************************************************\
   Load array of Strings from Custom settings
   Use maxStringLength = 0 to optimize for size (strings will be concatenated)
 \*********************************************************************************************/
String LoadStringArray(SettingsType::Enum settingsType,
                       int                index,
                       String             strings[],
                       uint16_t           nrStrings,
                       uint16_t           maxStringLength,
                       uint32_t           offset_in_block)
{
  int offset, max_size;

  if (!SettingsType::getSettingsParameters(settingsType, index, offset, max_size))
  {
    #ifndef BUILD_NO_DEBUG
    return F("Invalid index for custom settings");
    #else // ifndef BUILD_NO_DEBUG
    return F("Save error");
    #endif // ifndef BUILD_NO_DEBUG
  }

  const uint32_t bufferSize = 128;

  // FIXME TD-er: For now stack allocated, may need to be heap allocated?
  if (maxStringLength >= bufferSize) { return F("Max 128 chars allowed"); }

  char buffer[bufferSize] = { 0 };

  String   result;
  uint32_t readPos       = offset_in_block;
  uint32_t nextStringPos = readPos;
  uint32_t stringCount   = 0;

  const uint16_t estimatedStringSize = maxStringLength > 0 ? maxStringLength : bufferSize;
  String tmpString;
  tmpString.reserve(estimatedStringSize);
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
            move_special(strings[stringCount], std::move(tmpString));

            // Do not allocate tmpString on 2nd heap as byte access on 2nd heap is much slower
            // We're appending per byte, so better prefer speed for short lived objects
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
    result += concat(F("Incomplete custom settings for index "), index + 1);
    move_special(strings[stringCount], std::move(tmpString));
  }
  return result;
}

/********************************************************************************************\
   Save array of Strings from Custom settings
   Use maxStringLength = 0 to optimize for size (strings will be concatenated)
 \*********************************************************************************************/
String SaveStringArray(SettingsType::Enum settingsType,
                       int                index,
                       const String       strings[],
                       uint16_t           nrStrings,
                       uint16_t           maxStringLength,
                       uint32_t           posInBlock)
{
  // FIXME TD-er: Must add some check to see if the existing data has changed before saving.
  int offset, max_size;

  if (!SettingsType::getSettingsParameters(settingsType, index, offset, max_size))
  {
    #ifndef BUILD_NO_DEBUG
    return F("Invalid index for custom settings");
    #else // ifndef BUILD_NO_DEBUG
    return F("Save error");
    #endif // ifndef BUILD_NO_DEBUG
  }

  #ifdef ESP8266
  uint16_t bufferSize = 256;
  #endif // ifdef ESP8266
  #ifdef ESP32
  uint16_t bufferSize = 1024;
  #endif // ifdef ESP32

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

    for (; bufpos < bufferSize && stringCount < nrStrings; ++bufpos) {
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

  #if FEATURE_EXTENDED_CUSTOM_SETTINGS

  if ((SettingsType::Enum::CustomTaskSettings_Type == settingsType) &&
      ((writePos - posInBlock) <= DAT_TASKS_CUSTOM_SIZE)) { // Not needed, so can be deleted
    DeleteExtendedCustomTaskSettingsFile(settingsType, index);
  }
  #endif // if FEATURE_EXTENDED_CUSTOM_SETTINGS

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
  #endif // ifndef BUILD_NO_RAM_TRACKER

  if (ExtraTaskSettings.TaskIndex != TaskIndex) {
    #ifndef BUILD_NO_DEBUG
    return F("SaveTaskSettings taskIndex does not match");
    #else // ifndef BUILD_NO_DEBUG
    return F("Save error");
    #endif // ifndef BUILD_NO_DEBUG
  }

  START_TIMER
  String err;

  if (!Cache.matchChecksumExtraTaskSettings(TaskIndex, ExtraTaskSettings.computeChecksum())) {
    // Clear task device value names before saving, will generate again when loading them later.
    ExtraTaskSettings.clearDefaultTaskDeviceValueNames();
    ExtraTaskSettings.validate(); // Validate before saving will reduce nr of saves as it is more likely to not have changed the next time
                                  // it will be saved.

    // Call to validate() may have changed the content, so re-compute the checksum.
    // This is how it is now stored, so we can now also update the
    // ExtraTaskSettings cache. This may prevent a reload.
    Cache.updateExtraTaskSettingsCache_afterLoad_Save();

    err = SaveToFile(SettingsType::Enum::TaskSettings_Type,
                     TaskIndex,
                     reinterpret_cast<const uint8_t *>(&ExtraTaskSettings),
                     sizeof(struct ExtraTaskSettingsStruct));

#if !defined(PLUGIN_BUILD_MINIMAL_OTA) && !defined(ESP8266_1M)

    if (err.isEmpty()) {
      err = checkTaskSettings(TaskIndex);
    }
#endif // if !defined(PLUGIN_BUILD_MINIMAL_OTA) && !defined(ESP8266_1M)

    // FIXME TD-er: Is this still needed as it is also cleared on PLUGIN_INIT and PLUGIN_EXIT?
    UserVar.clear_computed(ExtraTaskSettings.TaskIndex);
  }
#ifndef LIMIT_BUILD_SIZE
  else {
    addLog(LOG_LEVEL_INFO, F("Skip saving task settings, not changed"));
  }
#endif // ifndef LIMIT_BUILD_SIZE
  STOP_TIMER(SAVE_TASK_SETTINGS);
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
  START_TIMER

  ExtraTaskSettings.clear();
  const deviceIndex_t DeviceIndex = getDeviceIndex_from_TaskIndex(TaskIndex);

  if (!validDeviceIndex(DeviceIndex)) {
    // No need to load from storage, as there is no plugin assigned to this task.
    ExtraTaskSettings.TaskIndex = TaskIndex; // Needed when an empty task was requested

    // FIXME TD-er: Do we need to keep a cache of an empty task?
    // Maybe better to do this?
    Cache.clearTaskCache(TaskIndex);

    //    Cache.updateExtraTaskSettingsCache_afterLoad_Save();
    return EMPTY_STRING;
  }
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("LoadTaskSettings"));
  #endif // ifndef BUILD_NO_RAM_TRACKER

  const String result = LoadFromFile(
    SettingsType::Enum::TaskSettings_Type,
    TaskIndex,
    reinterpret_cast<uint8_t *>(&ExtraTaskSettings),
    sizeof(struct ExtraTaskSettingsStruct));

  // After loading, some settings may need patching.
  ExtraTaskSettings.TaskIndex = TaskIndex; // Needed when an empty task was requested

  if (!Device[DeviceIndex].configurableDecimals()) {
    // Nr of decimals cannot be configured, so set them to 0 just to be sure.
    for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
      ExtraTaskSettings.TaskDeviceValueDecimals[i] = 0;
    }
  }
  loadDefaultTaskValueNames_ifEmpty(TaskIndex);

  ExtraTaskSettings.validate();
  Cache.updateExtraTaskSettingsCache_afterLoad_Save();
  STOP_TIMER(LOAD_TASK_SETTINGS);

  return result;
}

#if FEATURE_ALTERNATIVE_CDN_URL
String _CDN_url_cache;
bool   _CDN_url_loaded = false;

String get_CDN_url_custom() {
  if (!_CDN_url_loaded) {
    String strings[] = { EMPTY_STRING };

    LoadStringArray(
      SettingsType::Enum::CdnSettings_Type, 0,
      strings, NR_ELEMENTS(strings), 255, 0);
    _CDN_url_cache  = strings[0];
    _CDN_url_loaded = true;
  }
  return _CDN_url_cache;
}

void set_CDN_url_custom(const String& url) {
  _CDN_url_cache = url;
  _CDN_url_cache.trim();

  if (!_CDN_url_cache.isEmpty() && !_CDN_url_cache.endsWith(F("/"))) {
    _CDN_url_cache.concat('/');
  }
  _CDN_url_loaded = true;

  String strings[] = { EMPTY_STRING };

  LoadStringArray(
    SettingsType::Enum::CdnSettings_Type, 0,
    strings, NR_ELEMENTS(strings), 255, 0);

  if (url.equals(strings[0])) {
    // No need to save, is already the same
    return;
  }

  strings[0] = url;

  SaveStringArray(
    SettingsType::Enum::CdnSettings_Type, 0,
    strings, NR_ELEMENTS(strings), 255, 0);
}

#endif // if FEATURE_ALTERNATIVE_CDN_URL

/********************************************************************************************\
   Save Custom Task settings to file system
 \*********************************************************************************************/
String SaveCustomTaskSettings(taskIndex_t TaskIndex, const uint8_t *memAddress, int datasize, uint32_t posInBlock)
{
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("SaveCustomTaskSettings"));
  #endif // ifndef BUILD_NO_RAM_TRACKER
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
  #endif // ifndef BUILD_NO_RAM_TRACKER
  return SaveStringArray(
    SettingsType::Enum::CustomTaskSettings_Type, TaskIndex,
    strings, nrStrings, maxStringLength, posInBlock);
}

String getCustomTaskSettingsError(uint8_t varNr) {
  return strformat(F("Error: Text too long for line %d\n"), varNr + 1);
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
  #endif // ifndef BUILD_NO_RAM_TRACKER
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
  #endif // ifndef BUILD_NO_RAM_TRACKER
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
  #endif // ifndef BUILD_NO_RAM_TRACKER

  START_TIMER;

  controller_settings.validate(); // Make sure the saved controller settings have proper values.

  const ChecksumType checksum(reinterpret_cast<const uint8_t *>(&controller_settings), sizeof(ControllerSettingsStruct));

  if (checksum == (Cache.controllerSettings_checksums[ControllerIndex])) {
#ifndef BUILD_NO_DEBUG
    addLog(LOG_LEVEL_INFO, concat(F("Skip saving ControllerSettings: "), checksum.toString()));
#endif // ifndef BUILD_NO_DEBUG
    return EMPTY_STRING;
  }
  const String res = SaveToFile(SettingsType::Enum::ControllerSettings_Type, ControllerIndex,
                                reinterpret_cast<const uint8_t *>(&controller_settings), sizeof(controller_settings));

  Cache.controllerSettings_checksums[ControllerIndex] = checksum;
  #ifdef ESP32
  Cache.setControllerSettings(ControllerIndex, controller_settings);
  #endif // ifdef ESP32
  STOP_TIMER(SAVE_CONTROLLER_SETTINGS);

  return res;
}

/********************************************************************************************\
   Load Controller settings to file system
 \*********************************************************************************************/
String LoadControllerSettings(controllerIndex_t ControllerIndex, ControllerSettingsStruct& controller_settings) {
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("LoadControllerSettings"));
  #endif // ifndef BUILD_NO_RAM_TRACKER
  START_TIMER
  #ifdef ESP32

  if (Cache.getControllerSettings(ControllerIndex, controller_settings)) {
    STOP_TIMER(LOAD_CONTROLLER_SETTINGS_C);
    return EMPTY_STRING;
  }
  #endif // ifdef ESP32
  String result =
    LoadFromFile(SettingsType::Enum::ControllerSettings_Type, ControllerIndex,
                 reinterpret_cast<uint8_t *>(&controller_settings), sizeof(controller_settings));

  controller_settings.validate(); // Make sure the loaded controller settings have proper values.

  Cache.controllerSettings_checksums[ControllerIndex] = controller_settings.computeChecksum();
  #ifdef ESP32
  Cache.setControllerSettings(ControllerIndex, controller_settings);
  #endif // ifdef ESP32
  STOP_TIMER(LOAD_CONTROLLER_SETTINGS);
  return result;
}

/********************************************************************************************\
   Clear Custom Controller settings
 \*********************************************************************************************/
String ClearCustomControllerSettings(controllerIndex_t ControllerIndex)
{
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("ClearCustomControllerSettings"));
  #endif // ifndef BUILD_NO_RAM_TRACKER

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
  #endif // ifndef BUILD_NO_RAM_TRACKER
  return SaveToFile(SettingsType::Enum::CustomControllerSettings_Type, ControllerIndex, memAddress, datasize);
}

/********************************************************************************************\
   Load Custom Controller settings to file system
 \*********************************************************************************************/
String LoadCustomControllerSettings(controllerIndex_t ControllerIndex, uint8_t *memAddress, int datasize)
{
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("LoadCustomControllerSettings"));
  #endif // ifndef BUILD_NO_RAM_TRACKER
  return LoadFromFile(SettingsType::Enum::CustomControllerSettings_Type, ControllerIndex, memAddress, datasize);
}

#if FEATURE_CUSTOM_PROVISIONING

/********************************************************************************************\
   Save Provisioning Settings
 \*********************************************************************************************/
String saveProvisioningSettings(ProvisioningStruct& ProvisioningSettings)
{
  String err;

  ProvisioningSettings.validate();
  memcpy(ProvisioningSettings.ProgmemMd5, CRCValues.runTimeMD5, 16);

  if (!COMPUTE_STRUCT_CHECKSUM_UPDATE(ProvisioningStruct, ProvisioningSettings))
  {
    // Settings have changed, save to file.
    err =
      SaveToFile_trunc(getFileName(FileType::PROVISIONING_DAT, 0).c_str(), 0, (uint8_t *)&ProvisioningSettings, sizeof(ProvisioningStruct));
  }
  return err;
}

/********************************************************************************************\
   Load Provisioning Settings
 \*********************************************************************************************/
String loadProvisioningSettings(ProvisioningStruct& ProvisioningSettings)
{
  String err = LoadFromFile(getFileName(FileType::PROVISIONING_DAT, 0).c_str(),
                            0,
                            (uint8_t *)&ProvisioningSettings,
                            sizeof(ProvisioningStruct));

# ifndef BUILD_NO_DEBUG

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
# endif // ifndef BUILD_NO_DEBUG
  ProvisioningSettings.validate();
  return err;
}

#endif // if FEATURE_CUSTOM_PROVISIONING

#if FEATURE_NOTIFIER

/********************************************************************************************\
   Save Controller settings to file system
 \*********************************************************************************************/
String SaveNotificationSettings(int NotificationIndex, const uint8_t *memAddress, int datasize)
{
  # ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("SaveNotificationSettings"));
  # endif // ifndef BUILD_NO_RAM_TRACKER
  return SaveToFile(SettingsType::Enum::NotificationSettings_Type, NotificationIndex, memAddress, datasize);
}

/********************************************************************************************\
   Load Controller settings to file system
 \*********************************************************************************************/
String LoadNotificationSettings(int NotificationIndex, uint8_t *memAddress, int datasize)
{
  # ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("LoadNotificationSettings"));
  # endif // ifndef BUILD_NO_RAM_TRACKER
  return LoadFromFile(SettingsType::Enum::NotificationSettings_Type, NotificationIndex, memAddress, datasize);
}

#endif


/********************************************************************************************\
   Handle certificate files on the file system.
   The content will be stripped from unusable character like quotes, spaces etc.
 \*********************************************************************************************/
#if FEATURE_TLS
static inline bool is_base64(char c) {
  return (isalnum(c) || (c == '+') || (c == '/'));
}

bool cleanupCertificate(String & certificate, bool &changed)
{
  changed = false;
  // "-----BEGIN CERTIFICATE-----" positions in dash_pos[0] and dash_pos[1]
  // "-----END CERTIFICATE-----"   positions in dash_pos[2] and dash_pos[3]
  int dash_pos[4] = { 0 };
  int last_pos = 0;
  for (int i = 0; i < 4 && last_pos != -1; ++i) {
    dash_pos[i] = certificate.indexOf(F("-----"), last_pos);
    last_pos = dash_pos[i] + 5;
//    addLog(LOG_LEVEL_INFO, String(F(" dash_pos: ")) + String(dash_pos[i]));
  }
  if (last_pos == -1) return false;

  int read_pos = dash_pos[1] + 5; // next char after "-----BEGIN CERTIFICATE-----"
  String newCert;
  newCert.reserve((dash_pos[3] + 6) - dash_pos[0]);

  // "-----BEGIN CERTIFICATE-----" 
  newCert += certificate.substring(dash_pos[0], read_pos); 

  char last_char = certificate[read_pos - 1];
  for (; read_pos < dash_pos[2]; ++read_pos) {
    const char c = certificate[read_pos];
    if ((c == 'n' && last_char == '\\') || (c == '\n')) {
      if (!newCert.endsWith(String('\n'))) {
        newCert += '\n';
      }
    } else if (is_base64(c) || c == '=') {
      newCert += c;
    }
    last_char = c;
  }

  // "-----END CERTIFICATE-----" 
  newCert += certificate.substring(dash_pos[2], dash_pos[3] + 5);
  newCert += '\n';

  changed = !certificate.equals(newCert);
  certificate = std::move(newCert);
  return true;
}

String SaveCertificate(const String& fname, const String& certificate)
{
  return SaveToFile(fname.c_str(), 0, (const uint8_t *)certificate.c_str(), certificate.length() + 1);
}

String LoadCertificate(const String& fname, String& certificate, bool cleanup)
{
  bool changed = false;
  if (fileExists(fname)) {
    fs::File f = tryOpenFile(fname, "r");
    SPIFFS_CHECK(f, fname.c_str());
    #ifndef BUILD_NO_DEBUG
    String log = F("LoadCertificate: ");
    log += fname;
    #else
    String log = F("LoadCertificate error");
    #endif

    certificate.clear();

    if (!certificate.reserve(f.size())) {
      #ifndef BUILD_NO_DEBUG
      log += F(" ERROR, Out of memory");
      #endif
      addLog(LOG_LEVEL_ERROR, log);
      f.close();
      return log;
    }
    bool done = false;
    while (f.available() && !done) { 
      const char c = (char)f.read(); 
      if (c == '\0') {
        done = true;
      } else {
        certificate += c;
      }
    }
    f.close();

    if (cleanup) {
      if (!cleanupCertificate(certificate, changed)) {
        certificate.clear();
        #ifndef BUILD_NO_DEBUG
        log += F(" ERROR, Invalid certificate format");
        #endif
        addLog(LOG_LEVEL_ERROR, log);
        return log;
      } else if (changed) {
        //return SaveCertificate(fname, certificate);
      }
    }
  }

  return EMPTY_STRING;
}
#endif

/********************************************************************************************\
   Init a file with zeros on file system
 \*********************************************************************************************/
String InitFile(const String& fname, int datasize)
{
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("InitFile"));
  #endif // ifndef BUILD_NO_RAM_TRACKER
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
  return doSaveToFile(
    fname, index, memAddress, datasize,
    fileExists(fname) ? "r+" : "w+");
}

String SaveToFile_trunc(const char *fname, int index, const uint8_t *memAddress, int datasize)
{
  return doSaveToFile(fname, index, memAddress, datasize, "w+");
}

// See for mode description: https://github.com/esp8266/Arduino/blob/master/doc/filesystem.rst
String doSaveToFile(const char *fname, int index, const uint8_t *memAddress, int datasize, const char *mode)
{
#ifndef BUILD_NO_DEBUG
# ifndef ESP32

  if (allocatedOnStack(memAddress)) {
    addLog(LOG_LEVEL_ERROR, strformat(F("SaveToFile: %s ERROR, Data allocated on stack"), fname));

    //    return log;  // FIXME TD-er: Should this be considered a breaking error?
  }
# endif // ifndef ESP32
#endif  // ifndef BUILD_NO_DEBUG

  if (index < 0) {
    #ifndef BUILD_NO_DEBUG
    const String log = strformat(F("SaveToFile: %s ERROR, invalid position in file"), fname);
    #else // ifndef BUILD_NO_DEBUG
    const String log = F("Save error");
    #endif // ifndef BUILD_NO_DEBUG
    addLog(LOG_LEVEL_ERROR, log);
    return log;
  }
  START_TIMER;
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("SaveToFile"));
  #endif // ifndef BUILD_NO_RAM_TRACKER
  FLASH_GUARD();

  #ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLog(LOG_LEVEL_INFO, concat(F("SaveToFile: free stack: "),  getCurrentFreeStack()));
  }
  #endif // ifndef BUILD_NO_DEBUG
  delay(1);
  unsigned long timer = millis() + 50;
  fs::File f          = tryOpenFile(fname, mode);

  if (f) {
    clearAllButTaskCaches();
    SPIFFS_CHECK(f,                          fname);
    if (index > 0) {
      SPIFFS_CHECK(f.seek(index, fs::SeekSet), fname);
    }
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
      addLogMove(LOG_LEVEL_INFO, strformat(F("FILE : Saved %s offset: %d size: %d"), fname, index, datasize));
    }
    #endif // ifndef BUILD_NO_DEBUG
  } else {
    #ifndef BUILD_NO_DEBUG
    const String log = strformat(F("SaveToFile: %s ERROR, Cannot save to file"), fname);
    #else // ifndef BUILD_NO_DEBUG
    const String log = F("Save error");
    #endif // ifndef BUILD_NO_DEBUG

    addLog(LOG_LEVEL_ERROR, log);
    return log;
  }
  STOP_TIMER(SAVEFILE_STATS);
  #ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLogMove(LOG_LEVEL_INFO, concat(F("SaveToFile: free stack after: "), getCurrentFreeStack()));
  }
  #endif // ifndef BUILD_NO_DEBUG

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
    const String log = strformat(F("ClearInFile: %s ERROR, invalid position in file"), fname);
    #else // ifndef BUILD_NO_DEBUG
    const String log = F("Save error");
    #endif // ifndef BUILD_NO_DEBUG

    addLog(LOG_LEVEL_ERROR, log);
    return log;
  }

  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("ClearInFile"));
  #endif // ifndef BUILD_NO_RAM_TRACKER
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
    const String log = strformat(F("ClearInFile: %s ERROR, Cannot save to file"), fname);
    #else // ifndef BUILD_NO_DEBUG
    const String log = F("Save error");
    #endif // ifndef BUILD_NO_DEBUG
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
    const String log = strformat(F("LoadFromFile: %s ERROR, invalid position in file"), fname);
    #else // ifndef BUILD_NO_DEBUG
    const String log = F("Load error");
    #endif // ifndef BUILD_NO_DEBUG
    addLog(LOG_LEVEL_ERROR, log);
    return log;
  }
  delay(0);
  START_TIMER;
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("LoadFromFile"));
  #endif // ifndef BUILD_NO_RAM_TRACKER

  fs::File f = tryOpenFile(fname, "r");
  SPIFFS_CHECK(f, fname);
  const int fileSize = f.size();

  if (fileSize > offset) {
    SPIFFS_CHECK(f.seek(offset, fs::SeekSet), fname);

    if (fileSize < (offset + datasize)) {
      const int newdatasize = datasize + offset - fileSize;

      // File is smaller, make sure to set excess memory to 0.
      memset(memAddress + newdatasize, 0u, (datasize - newdatasize));

      datasize = newdatasize;
    }
    SPIFFS_CHECK(f.read(memAddress, datasize), fname);
  }
  f.close();

  STOP_TIMER(LOADFILE_STATS);
  delay(0);

  return EMPTY_STRING;
}

String LoadFromFile(const char *fname, String& data, int offset)
{
  fs::File f = tryOpenFile(fname, "r");
  SPIFFS_CHECK(f, fname);
  #ifndef BUILD_NO_DEBUG
  String log = F("LoadFromFile: ");
  log += fname;
  #else
  String log = F("Load error");
  #endif

  if (!f || offset < 0 || (offset >= static_cast<int>(f.size()))) {
    #ifndef BUILD_NO_DEBUG
    log += F(" ERROR, invalid position in file");
    #endif
    addLog(LOG_LEVEL_ERROR, log);
    return log;
  }
  delay(0);
  START_TIMER;
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("LoadFromFile"));
  #endif
  
  SPIFFS_CHECK(f.seek(offset, fs::SeekSet),  fname);
  if (f) {
    if (!data.reserve(f.size() - offset)) {
      #ifndef BUILD_NO_DEBUG
      log += F(" ERROR, Out of memory");
      #endif
      addLog(LOG_LEVEL_ERROR, log);
      f.close();
      return log;
    }

    while (f.available()) { data += (char)f.read(); }
    f.close();
  }
  

  STOP_TIMER(LOADFILE_STATS);
  delay(0);

  return String();
}

/********************************************************************************************\
   Wrapper functions to handle errors in accessing settings
 \*********************************************************************************************/
String getSettingsFileIndexRangeError(bool read, SettingsType::Enum settingsType, int index) {
  if (settingsType >= SettingsType::Enum::SettingsType_MAX) {
    return concat(F("Unknown settingsType: "), static_cast<int>(settingsType));
  }
  String error = read ? F("Load") : F("Save");

  #ifndef BUILD_NO_DEBUG
  error += SettingsType::getSettingsTypeString(settingsType);
  error += concat(F(" index out of range: "), index);
  #else // ifndef BUILD_NO_DEBUG
  error += F(" error");
  #endif // ifndef BUILD_NO_DEBUG
  return error;
}

String getSettingsFileDatasizeError(bool read, SettingsType::Enum settingsType, int index, int datasize, int max_size) {
  String error = read ? F("Load") : F("Save");

  #ifndef BUILD_NO_DEBUG
  error += SettingsType::getSettingsTypeString(settingsType);
  error += strformat(F("(%d) datasize(%d) > max_size(%d)"), index, datasize, max_size);
  #else // ifndef BUILD_NO_DEBUG
  error += F(" error");
  #endif // ifndef BUILD_NO_DEBUG

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

  int dataOffset = 0;

  #if FEATURE_EXTENDED_CUSTOM_SETTINGS
  int taskIndex = INVALID_TASK_INDEX; // Use base filename

  if ((SettingsType::Enum::CustomTaskSettings_Type == settingsType) &&
      ((offset_in_block + datasize) > DAT_TASKS_CUSTOM_SIZE)) {
    if (offset_in_block < DAT_TASKS_CUSTOM_SIZE) { // block starts in regular Custom config: Load first part
      const String fname = SettingsType::getSettingsFileName(settingsType);
      dataOffset = DAT_TASKS_CUSTOM_SIZE - offset_in_block;
      const String res = LoadFromFile(fname.c_str(), offset + offset_in_block, memAddress, dataOffset);

      if (!res.isEmpty()) { return res; } // Error occurred?

      datasize       -= dataOffset;
      offset_in_block = DAT_TASKS_CUSTOM_SIZE;
    }
    const String fname = SettingsType::getSettingsFileName(settingsType, index);

    if (fileExists(fname)) { // Do we have a task-specific extension stored?
      if (offset_in_block >= DAT_TASKS_CUSTOM_SIZE) {
        offset_in_block -= DAT_TASKS_CUSTOM_SIZE;
      }
      offset    = 0;
      taskIndex = index; // Use task-specific filename
    }
  }
  #endif // if FEATURE_EXTENDED_CUSTOM_SETTINGS

  const String fname = SettingsType::getSettingsFileName(settingsType
                                                         #if FEATURE_EXTENDED_CUSTOM_SETTINGS
                                                         , taskIndex
                                                         #endif // if FEATURE_EXTENDED_CUSTOM_SETTINGS
                                                         );
  return LoadFromFile(fname.c_str(), (offset + offset_in_block), memAddress + dataOffset, datasize);
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

  int dataOffset = 0;

  #if FEATURE_EXTENDED_CUSTOM_SETTINGS
  int taskIndex = INVALID_TASK_INDEX;                      // Use base filename

  if ((SettingsType::Enum::CustomTaskSettings_Type == settingsType) &&
      (posInBlock + datasize > (DAT_TASKS_CUSTOM_SIZE))) { // max_size already handled above
    if (posInBlock < DAT_TASKS_CUSTOM_SIZE) {              // Partial in regular config.dat, save that part first
      const String fname = SettingsType::getSettingsFileName(settingsType);
      dataOffset = (DAT_TASKS_CUSTOM_SIZE - posInBlock);   // Bytes to keep 'local'
      # ifndef BUILD_NO_DEBUG
      const String styp = SettingsType::getSettingsTypeString(settingsType);

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        addLog(LOG_LEVEL_INFO, strformat(F("ExtraSaveToFile: %s file: %s size: %d pos: %d"),
                                         styp.c_str(), fname.c_str(), dataOffset, posInBlock));
      }
      # endif // ifndef BUILD_NO_DEBUG
      const String res = SaveToFile(fname.c_str(), offset + posInBlock, memAddress, dataOffset);

      if (!res.isEmpty()) { return res; } // Error occurred

      datasize  -= dataOffset;
      posInBlock = 0;
    } else {
      posInBlock -= DAT_TASKS_CUSTOM_SIZE;
    }
    offset    = 0;     // Start of the extension file
    taskIndex = index; // Use task-specific filename
  }
  #endif // if FEATURE_EXTENDED_CUSTOM_SETTINGS

  const String fname = SettingsType::getSettingsFileName(settingsType
                                                         #if FEATURE_EXTENDED_CUSTOM_SETTINGS
                                                         , taskIndex
                                                         #endif // if FEATURE_EXTENDED_CUSTOM_SETTINGS
                                                         );

  if (!fileExists(fname)) {
    #if FEATURE_EXTENDED_CUSTOM_SETTINGS

    if (!validTaskIndex(taskIndex)) {
    #endif // if FEATURE_EXTENDED_CUSTOM_SETTINGS
    InitFile(settingsType);
    #if FEATURE_EXTENDED_CUSTOM_SETTINGS
  } else {
    InitFile(fname, DAT_TASKS_CUSTOM_EXTENSION_SIZE); // Initialize task-specific file
  }
    #endif // if FEATURE_EXTENDED_CUSTOM_SETTINGS
  }
  #ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLog(LOG_LEVEL_INFO, concat(F("SaveToFile: "), SettingsType::getSettingsTypeString(settingsType)) +
           strformat(F(" file: %s task: %d"), fname.c_str(), index + 1));
  }
  #endif // ifndef BUILD_NO_DEBUG
  return SaveToFile(fname.c_str(), offset + posInBlock, memAddress + dataOffset, datasize);
}

String ClearInFile(SettingsType::Enum settingsType, int index) {
  bool read = false;
  int  offset, max_size;

  if (!getAndLogSettingsParameters(read, settingsType, index, offset, max_size)) {
    return getSettingsFileIndexRangeError(read, settingsType, index);
  }
  #if FEATURE_EXTENDED_CUSTOM_SETTINGS

  if (SettingsType::Enum::CustomTaskSettings_Type == settingsType) {
    max_size = DAT_TASKS_CUSTOM_SIZE; // Don't also wipe the external size inside the config.dat file...
    DeleteExtendedCustomTaskSettingsFile(settingsType, index);
  }
  #endif // if FEATURE_EXTENDED_CUSTOM_SETTINGS

  const String fname = SettingsType::getSettingsFileName(settingsType);
  return ClearInFile(fname.c_str(), offset, max_size);
}

#if FEATURE_EXTENDED_CUSTOM_SETTINGS
bool DeleteExtendedCustomTaskSettingsFile(SettingsType::Enum settingsType, int index) {
  if ((SettingsType::Enum::CustomTaskSettings_Type == settingsType) && validTaskIndex(index)) {
    const String fname = SettingsType::getSettingsFileName(settingsType, index);

    if (fileExists(fname)) {
      const bool deleted = tryDeleteFile(fname); // Don't need the extension file anymore, so delete it
      # ifndef BUILD_NO_DEBUG

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        addLog(LOG_LEVEL_INFO, concat(F("CustomTaskSettings: Removing no longer needed file: "), fname));
      }
      # endif // ifndef BUILD_NO_DEBUG
      return deleted;
    }
  }
  return false;
}

#endif // if FEATURE_EXTENDED_CUSTOM_SETTINGS

/********************************************************************************************\
   Check file system area settings
 \*********************************************************************************************/
int SpiffsSectors()
{
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("SpiffsSectors"));
  #endif // ifndef BUILD_NO_RAM_TRACKER
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
    result = 8192; // Just assume 8k, since we cannot query it
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
    result = 256; // Just assume 256, since we cannot query it
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
  int freeSpace          = SpiffsTotalBytes() - SpiffsUsedBytes();
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

#if FEATURE_RTC_CACHE_STORAGE

/********************************************************************************************\
   Handling cached data
 \*********************************************************************************************/
String createCacheFilename(unsigned int count) {
  String fname;

  fname.reserve(16);
  # ifdef ESP32
  fname = '/';
  # endif // ifdef ESP32
  fname += strformat(F("cache_%d.bin"), count);
  return fname;
}

// Match string with an integer between '_' and ".bin"
int getCacheFileCountFromFilename(const String& fname) {
  if (!isCacheFile(fname)) { return -1; }
  int startpos = fname.indexOf('_');

  if (startpos < 0) { return -1; }
  int endpos = fname.indexOf(F(".bin"));

  if (endpos < 0) { return -1; }

  //  String digits = fname.substring(startpos + 1, endpos);
  int32_t result;

  if (validIntFromString(fname.substring(startpos + 1, endpos), result)) {
    return result;
  }
  return -1;
}

bool isCacheFile(const String& fname) {
  return fname.indexOf(F("cache_")) != -1;
}

// Look into the filesystem to see if there are any cache files present on the filesystem
// Return true if any found.
bool getCacheFileCounters(uint16_t& lowest, uint16_t& highest, size_t& filesizeHighest) {
  lowest          = 65535;
  highest         = 0;
  filesizeHighest = 0;
# ifdef ESP8266
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
# endif // ESP8266
# ifdef ESP32
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
#  ifndef BUILD_NO_DEBUG
        } else {
          if (loglevelActiveFor(LOG_LEVEL_INFO)) {
            addLog(LOG_LEVEL_INFO, concat(F("RTC  : Cannot get count from: "), fname));
          }
#  endif // ifndef BUILD_NO_DEBUG
        }
      }
    }
    file = root.openNextFile();
  }
# endif // ESP32

  if (lowest <= highest) {
    return true;
  }
  lowest  = 0;
  highest = 0;
  return false;
}

#endif // if FEATURE_RTC_CACHE_STORAGE

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
      return concat(F("OTA partition "), partitionSubType - ESP_PARTITION_SUBTYPE_APP_OTA_MIN);
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
        # ifdef USE_LITTLEFS
        return F("LittleFS");
        # else // ifdef USE_LITTLEFS
        return F("SPIFFS");
        # endif // ifdef USE_LITTLEFS
      default: break;
    }
  }
  return strformat(F("Unknown(%d)"), partitionSubType);
}

String getPartitionTableHeader(const String& itemSep, const String& lineEnd) {
  const char *itemSep_str = itemSep.c_str();

  return strformat(F("Address%sSize%sLabel%sPartition Type%sEncrypted%s"),
                   itemSep_str, itemSep_str, itemSep_str, itemSep_str, lineEnd.c_str());
}

String getPartitionTable(uint8_t pType, const String& itemSep, const String& lineEnd) {
  esp_partition_type_t partitionType = static_cast<esp_partition_type_t>(pType);
  String result;
  esp_partition_iterator_t _mypartiterator = esp_partition_find(partitionType, ESP_PARTITION_SUBTYPE_ANY, nullptr);

  if (_mypartiterator) {
    do {
      const esp_partition_t *_mypart = esp_partition_get(_mypartiterator);
      const char *itemSep_str        = itemSep.c_str();
      result += strformat(F("%x%s%s%s%s%s%s%s%s%s"),
                          _mypart->address,
                          itemSep_str,
                          formatToHex_decimal(_mypart->size, 1024).c_str(),
                          itemSep_str,
                          _mypart->label,
                          itemSep_str,
                          getPartitionType(_mypart->type, _mypart->subtype).c_str(),
                          itemSep_str,
                          String(_mypart->encrypted ? F("Yes") : F("-")).c_str(),
                          lineEnd.c_str());
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
  String fullUrl  = joinUrlFilename(url, filename);
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
      const String filename_bak = strformat(F("%s_bak"), filename.c_str());

      if (fileExists(filename_bak)) {
        if (!ResetFactoryDefaultPreference.delete_Bak_Files() || !tryDeleteFile(filename_bak)) {
          return F("Could not rename to _bak");
        }
      }

      // Must download it to a tmp file.
      const String tmpfile = strformat(F("%s_tmp"), filename.c_str());

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

bool validateUploadConfigDat(const uint8_t *buf) {
  bool result = false;
  struct TempStruct {
    unsigned long PID;
    int           Version;
  } Temp;

  for (unsigned int x = 0; x < sizeof(struct TempStruct); x++) {
    memcpy(reinterpret_cast<uint8_t *>(&Temp) + x, &buf[x], 1);
  }
  #ifndef BUILD_NO_DEBUG
  addLog(LOG_LEVEL_INFO, strformat(F("Validate config.dat, Version: %d = %d, PID: %d = %d"),
                                   Temp.Version, VERSION, Temp.PID, ESP_PROJECT_PID));
  #endif // ifndef BUILD_NO_DEBUG

  if ((Temp.Version == VERSION) && (Temp.PID == ESP_PROJECT_PID)) {
    result = true;
  }
  return result;
}

#if FEATURE_CUSTOM_PROVISIONING

String downloadFileType(FileType::Enum filetype, unsigned int filenr)
{
  String url, user, pass;

  {
    MakeProvisioningSettings(ProvisioningSettings);

    if (ProvisioningSettings.get()) {
      loadProvisioningSettings(*ProvisioningSettings);

      if (!ProvisioningSettings->fetchFileTypeAllowed(filetype, filenr)) {
        return F("Not Allowed");
      }

      if (!ProvisioningSettings->url[0]) {
        return F("Provision Config incomplete");
      }

      url  = ProvisioningSettings->url;
      user = ProvisioningSettings->user;
      pass = ProvisioningSettings->pass;
    }
  }
  String res = downloadFileType(url, user, pass, filetype, filenr);

  clearAllCaches();
  return res;
}

#endif // if FEATURE_CUSTOM_PROVISIONING
