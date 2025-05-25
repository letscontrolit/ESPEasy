#ifndef HELPERS_ESPEASY_STORAGE_H
#define HELPERS_ESPEASY_STORAGE_H


#include "../../ESPEasy_common.h"

#include "../Helpers/FS_Helper.h"

#include "../CustomBuild/StorageLayout.h"

#include "../DataStructs/ChecksumType.h"
#include "../DataStructs/ProvisioningStruct.h"
#include "../DataTypes/ESPEasyFileType.h"
#include "../DataTypes/SettingsType.h"
#include "../Globals/Plugins.h"
#include "../Globals/CPlugins.h"


/********************************************************************************************\
   file system error handling
   Look here for error # reference: https://github.com/pellepl/spiffs/blob/master/src/spiffs.h
 \*********************************************************************************************/
String FileError(int line, const char *fname);

/********************************************************************************************\
   Keep track of number of flash writes.
 \*********************************************************************************************/
void flashCount();

String flashGuard();

String appendLineToFile(const String& fname, const String& line);

String appendToFile(const String& fname, const uint8_t *data, unsigned int size);

enum class FileDestination_e : uint8_t {
  ANY   = 0,
  FLASH = 1,
  SD    = 2,
};

bool fileExists(const __FlashStringHelper * fname);
bool fileExists(const __FlashStringHelper *fname, FileDestination_e& destination);
bool fileExists(const String& fname, FileDestination_e& destination);
bool fileExists(const String& fname);

int fileSize(const String& fname);

fs::File tryOpenFile(const String& fname, const String& mode, FileDestination_e destination = FileDestination_e::ANY);

bool tryRenameFile(const String& fname_old, const String& fname_new, FileDestination_e destination = FileDestination_e::ANY);

bool tryDeleteFile(const String& fname, FileDestination_e destination = FileDestination_e::ANY);

/********************************************************************************************\
   Fix stuff to clear out differences between releases
   Return true when settings were changed/patched
 \*********************************************************************************************/
bool BuildFixes();

/********************************************************************************************\
   Mount FS and check config.dat
 \*********************************************************************************************/
void fileSystemCheck();

bool FS_format();

#ifdef ESP32
uint32_t getWiFi_CalibrationVersion();

bool check_and_update_WiFi_Calibration();
#endif
bool Erase_WiFi_Calibration();

#ifdef ESP32

int  getPartionCount(uint8_t pType, uint8_t pSubType = 0xFF);
String patch_fname(const String& fname);

#endif
#ifdef ESP8266
#define patch_fname(F) (F)
#endif

/********************************************************************************************\
   Low level clear RFcal and SDK WiFi parameters.
 \*********************************************************************************************/
 #ifdef ESP8266
bool clearRFcalPartition();

bool clearWiFiSDKpartition();

#endif


/********************************************************************************************\
   Garbage collection
 \*********************************************************************************************/
bool GarbageCollection();


// Macros needed for template class types, like SettingsStruct
#define COMPUTE_STRUCT_CHECKSUM_UPDATE(STRUCT,OBJECT) \
   ChecksumType::computeChecksum(OBJECT.md5,\
                   reinterpret_cast<uint8_t *>(&OBJECT),\
                   sizeof(STRUCT),\
                   offsetof(STRUCT, md5),\
                   true)

#define COMPUTE_STRUCT_CHECKSUM(STRUCT,OBJECT) \
   ChecksumType::computeChecksum(OBJECT.md5,\
                   reinterpret_cast<uint8_t *>(&OBJECT),\
                   sizeof(STRUCT),\
                   offsetof(STRUCT, md5),\
                   false)

/********************************************************************************************\
   Save settings to file system
 \*********************************************************************************************/
String SaveSettings(bool forFactoryReset = false);

String SaveSecuritySettings(bool forFactoryReset = false);

void afterloadSettings();

/********************************************************************************************\
   Load settings from file system
 \*********************************************************************************************/
String LoadSettings();

/********************************************************************************************\
   Disable Plugin, based on bootFailedCount
 \*********************************************************************************************/
uint8_t disablePlugin(uint8_t bootFailedCount);
uint8_t disableAllPlugins(uint8_t bootFailedCount);

/********************************************************************************************\
   Disable Controller, based on bootFailedCount
 \*********************************************************************************************/
uint8_t disableController(uint8_t bootFailedCount);
uint8_t disableAllControllers(uint8_t bootFailedCount);

/********************************************************************************************\
   Disable Notification, based on bootFailedCount
 \*********************************************************************************************/
#if FEATURE_NOTIFIER
uint8_t disableNotification(uint8_t bootFailedCount);
uint8_t disableAllNotifications(uint8_t bootFailedCount);
#endif

/********************************************************************************************\
   Disable Rules, based on bootFailedCount
 \*********************************************************************************************/
uint8_t disableRules(uint8_t bootFailedCount);


bool getAndLogSettingsParameters(bool read, SettingsType::Enum settingsType, int index, int& offset, int& max_size);

/********************************************************************************************\
   Load array of Strings from Custom settings
   Use maxStringLength = 0 to optimize for size (strings will be concatenated)
 \*********************************************************************************************/
String LoadStringArray(SettingsType::Enum settingsType, int index, String strings[], uint16_t nrStrings, uint16_t maxStringLength, uint32_t offset_in_block = 0);


/********************************************************************************************\
   Save array of Strings from Custom settings
   Use maxStringLength = 0 to optimize for size (strings will be concatenated)
 \*********************************************************************************************/
String SaveStringArray(SettingsType::Enum settingsType, int index, const String strings[], uint16_t nrStrings, uint16_t maxStringLength, uint32_t posInBlock = 0);


/********************************************************************************************\
   Save Task settings to file system
 \*********************************************************************************************/
String SaveTaskSettings(taskIndex_t TaskIndex);

/********************************************************************************************\
   Load Task settings from file system
 \*********************************************************************************************/
String LoadTaskSettings(taskIndex_t TaskIndex);

/********************************************************************************************\
   Load/Save CDN custom setting from file system
 \*********************************************************************************************/
#if FEATURE_ALTERNATIVE_CDN_URL
String get_CDN_url_custom();
void set_CDN_url_custom(const String &url);
#endif // if FEATURE_ALTERNATIVE_CDN_URL

/********************************************************************************************\
   Save Custom Task settings to file system
 \*********************************************************************************************/
String SaveCustomTaskSettings(taskIndex_t TaskIndex, const uint8_t *memAddress, int datasize, uint32_t posInBlock = 0);

/********************************************************************************************\
   Save array of Strings to Custom Task settings
   Use maxStringLength = 0 to optimize for size (strings will be concatenated)
 \*********************************************************************************************/
String SaveCustomTaskSettings(taskIndex_t TaskIndex, String strings[], uint16_t nrStrings, uint16_t maxStringLength, uint32_t posInBlock = 0);

String getCustomTaskSettingsError(uint8_t varNr);

/********************************************************************************************\
   Clear custom task settings
 \*********************************************************************************************/
String ClearCustomTaskSettings(taskIndex_t TaskIndex);

/********************************************************************************************\
   Delete Extended custom task settings file if it exists, with validity checks
 \*********************************************************************************************/
#if FEATURE_EXTENDED_CUSTOM_SETTINGS
bool DeleteExtendedCustomTaskSettingsFile(SettingsType::Enum settingsType, int index);
#endif // if FEATURE_EXTENDED_CUSTOM_SETTINGS

/********************************************************************************************\
   Load Custom Task settings from file system
 \*********************************************************************************************/
String LoadCustomTaskSettings(taskIndex_t TaskIndex, uint8_t *memAddress, int datasize, int offset_in_block = 0);

/********************************************************************************************\
   Load array of Strings from Custom Task settings
   Use maxStringLength = 0 to optimize for size (strings will be concatenated)
 \*********************************************************************************************/
String LoadCustomTaskSettings(taskIndex_t TaskIndex, String strings[], uint16_t nrStrings, uint16_t maxStringLength, uint32_t offset_in_block = 0);

/********************************************************************************************\
   Save Controller settings to file system
 \*********************************************************************************************/
String SaveControllerSettings(controllerIndex_t ControllerIndex, ControllerSettingsStruct& controller_settings);

/********************************************************************************************\
   Load Controller settings to file system
 \*********************************************************************************************/
String LoadControllerSettings(controllerIndex_t ControllerIndex, ControllerSettingsStruct& controller_settings);

/********************************************************************************************\
   Clear Custom Controller settings
 \*********************************************************************************************/
String ClearCustomControllerSettings(controllerIndex_t ControllerIndex);

/********************************************************************************************\
   Save Custom Controller settings to file system
 \*********************************************************************************************/
String SaveCustomControllerSettings(controllerIndex_t ControllerIndex, const uint8_t *memAddress, int datasize);

/********************************************************************************************\
   Load Custom Controller settings to file system
 \*********************************************************************************************/
String LoadCustomControllerSettings(controllerIndex_t ControllerIndex, uint8_t *memAddress, int datasize);


#if FEATURE_CUSTOM_PROVISIONING
/********************************************************************************************\
   Save Provisioning Settings
 \*********************************************************************************************/
String saveProvisioningSettings(ProvisioningStruct& ProvisioningSettings);

/********************************************************************************************\
   Load Provisioning Settings
 \*********************************************************************************************/
String loadProvisioningSettings(ProvisioningStruct& ProvisioningSettings);
#endif



#if FEATURE_NOTIFIER
/********************************************************************************************\
   Save Controller settings to file system
 \*********************************************************************************************/
String SaveNotificationSettings(int NotificationIndex, const uint8_t *memAddress, int datasize);


/********************************************************************************************\
   Load Controller settings to file system
 \*********************************************************************************************/
String LoadNotificationSettings(int NotificationIndex, uint8_t *memAddress, int datasize);
#endif


/********************************************************************************************\
   Handle certificate files on the file system.
   The content will be stripped from unusable character like quotes, spaces etc.
 \*********************************************************************************************/
#if FEATURE_TLS
String SaveCertificate(const String& fname, const String& certificate);
String LoadCertificate(const String& fname, String& certificate, bool cleanup = true);
#endif

/********************************************************************************************\
   Init a file with zeros on file system
 \*********************************************************************************************/
String InitFile(const String& fname, int datasize);

String InitFile(SettingsType::Enum settingsType);
String InitFile(SettingsType::SettingsFileEnum file_type);

/********************************************************************************************\
   Save data into config file on file system
 \*********************************************************************************************/
// Save to file in r+ mode
// Open for reading and writing.  
// The stream is positioned at the beginning of the file.
String SaveToFile(const char *fname, int index, const uint8_t *memAddress, int datasize);

// Save to file in w+ mode
// Open for reading and writing.  
// The file is created if it does not exist, otherwise it is truncated.
// The stream is positioned at the beginning of the file.

String SaveToFile_trunc(const char *fname, int index, const uint8_t *memAddress, int datasize);

// See for mode description: https://github.com/esp8266/Arduino/blob/master/doc/filesystem.rst
String doSaveToFile(const char *fname, int index, const uint8_t *memAddress, int datasize, const char *mode);


/********************************************************************************************\
   Clear a certain area in a file (set to 0)
 \*********************************************************************************************/
String ClearInFile(const char *fname, int index, int datasize);

/********************************************************************************************\
   Load data from config file on file system
 \*********************************************************************************************/
String LoadFromFile(const char *fname, int offset, uint8_t *memAddress, int datasize);

String LoadFromFile(const char *fname, String& data, int offset = 0);

/********************************************************************************************\
   Wrapper functions to handle errors in accessing settings
 \*********************************************************************************************/
String getSettingsFileIndexRangeError(bool read, SettingsType::Enum settingsType, int index);

String getSettingsFileDatasizeError(bool read, SettingsType::Enum settingsType, int index, int datasize, int max_size);

String LoadFromFile(SettingsType::Enum settingsType, int index, uint8_t *memAddress, int datasize, int offset_in_block = 0);

String SaveToFile(SettingsType::Enum settingsType, int index, const uint8_t *memAddress, int datasize, int posInBlock = 0);

String ClearInFile(SettingsType::Enum settingsType, int index);

/********************************************************************************************\
   Check file system area settings
 \*********************************************************************************************/
int SpiffsSectors();

size_t SpiffsUsedBytes();

size_t SpiffsTotalBytes();

size_t SpiffsBlocksize();

size_t SpiffsPagesize();

size_t SpiffsFreeSpace();

bool SpiffsFull();

#if FEATURE_RTC_CACHE_STORAGE
/********************************************************************************************\
   Handling cached data
 \*********************************************************************************************/
String createCacheFilename(unsigned int count);

bool isCacheFile(const String& fname);

// Match string with an integer between '_' and ".bin"
int getCacheFileCountFromFilename(const String& fname);

// Look into the filesystem to see if there are any cache files present on the filesystem
// Return true if any found.
bool getCacheFileCounters(uint16_t& lowest, uint16_t& highest, size_t& filesizeHighest);
#endif

/********************************************************************************************\
   Get partition table information
 \*********************************************************************************************/
#ifdef ESP32

String getPartitionType(uint8_t pType, uint8_t pSubType);

String getPartitionTableHeader(const String& itemSep, const String& lineEnd);

String getPartitionTable(uint8_t pType, const String& itemSep, const String& lineEnd);

#endif // ifdef ESP32

bool validateUploadConfigDat(const uint8_t *buf);

/********************************************************************************************\
   Download ESPEasy file types from HTTP server
 \*********************************************************************************************/
#if FEATURE_DOWNLOAD
String downloadFileType(const String& url, const String& user, const String& pass, FileType::Enum filetype, unsigned int filenr = 0);

#endif // if FEATURE_DOWNLOAD
#if FEATURE_CUSTOM_PROVISIONING
// Download file type based on settings stored in provisioning.dat file.
String downloadFileType(FileType::Enum filetype, unsigned int filenr = 0);

#endif




#endif // HELPERS_ESPEASY_STORAGE_H
