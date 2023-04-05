#ifndef DATASTRUCTS_CACHES_H
#define DATASTRUCTS_CACHES_H

#include <map>

#include "../../ESPEasy_common.h"
#include "../CustomBuild/ESPEasyLimits.h"
#include "../DataStructs/ChecksumType.h"
#ifdef ESP32
# include "../DataStructs/ControllerSettingsStruct.h"
# include "../DataTypes/ControllerIndex.h"
#endif // ifdef ESP32
#include "../Globals/Plugins.h"

#include "../Helpers/RulesHelper.h"

// Key is combination of array index + some offset reflecting the used array
// Store those sparingly used TaskDevicePluginConfigLong and TaskDevicePluginConfig
// from the ExtraTaskSettings
typedef std::map<uint8_t, long> ExtraTaskSettings_long_Map;


struct ExtraTaskSettings_cache_t {
  #ifdef ESP32
  static uint8_t make_Long_ValuesIndex(uint8_t arrayIndex)    {
    return arrayIndex;
  }

  static uint8_t make_Uint16_ValuesIndex(uint8_t arrayIndex)  {
    return arrayIndex + PLUGIN_EXTRACONFIGVAR_MAX;
  }

  ExtraTaskSettings_long_Map long_values;

  uint16_t TaskDevicePluginConfigLong_index_used = 0;
  uint16_t TaskDevicePluginConfig_index_used     = 0;

  String TaskDeviceValueNames[VARS_PER_TASK];
  #endif // ifdef ESP32
  String       TaskDeviceName;
  ChecksumType md5checksum;
  uint8_t      decimals[VARS_PER_TASK] = { 0 };
  #if FEATURE_PLUGIN_STATS
  uint8_t enabledPluginStats = 0;
  #endif // if FEATURE_PLUGIN_STATS
  bool hasFormula = false;
};

typedef std::map<String, taskIndex_t>                    TaskIndexNameMap;
typedef std::map<String, uint8_t>                        TaskIndexValueNameMap;
typedef std::map<String, uint8_t>                        FilePresenceMap;
typedef std::map<taskIndex_t, ExtraTaskSettings_cache_t> ExtraTaskSettingsMap;

#ifdef ESP32
typedef std::map<controllerIndex_t, ControllerSettingsStruct> ControllerSettingsMap;
#endif // ifdef ESP32

struct Caches {
  void    clearAllCaches();
  void    clearAllButTaskCaches();

  void    clearAllTaskCaches();
  void    clearTaskCache(taskIndex_t TaskIndex);

  void    clearFileCaches();

  bool    matchChecksumExtraTaskSettings(taskIndex_t         TaskIndex,
                                         const ChecksumType& checksum) const;

  void    updateActiveTaskUseSerial0();

  uint8_t getTaskDeviceValueDecimals(taskIndex_t TaskIndex,
                                     uint8_t     rel_index);

  String  getTaskDeviceName(taskIndex_t TaskIndex);

  String  getTaskDeviceValueName(taskIndex_t TaskIndex,
                                 uint8_t     rel_index);

  // Check to see if at least one of the taskvalues has a non-empty formula field.
  bool    hasFormula(taskIndex_t TaskIndex);


  String  getTaskDeviceFormula(taskIndex_t TaskIndex,
                               uint8_t     rel_index);

  long    getTaskDevicePluginConfigLong(taskIndex_t TaskIndex,
                                        uint8_t     rel_index);

  int16_t getTaskDevicePluginConfig(taskIndex_t TaskIndex,
                                    uint8_t     rel_index);

  #if FEATURE_PLUGIN_STATS
  bool enabledPluginStats(taskIndex_t TaskIndex,
                          uint8_t     rel_index);
  #endif // if FEATURE_PLUGIN_STATS


  // Update all cached values, except the checksum.
  void updateExtraTaskSettingsCache();

  // Update the cached value.
  // Only to be called from LoadTaskSettings() or SaveTaskSettings()
  // since only those functions know the checksum of what has been stored.
  void updateExtraTaskSettingsCache_afterLoad_Save();

  #ifdef ESP32
  bool getControllerSettings(controllerIndex_t         index,
                             ControllerSettingsStruct& ControllerSettings) const;

  void setControllerSettings(controllerIndex_t               index,
                             const ControllerSettingsStruct& ControllerSettings);

  void clearControllerSettings(controllerIndex_t index);
  #endif // ifdef ESP32

private:

  ExtraTaskSettingsMap::const_iterator getExtraTaskSettings(taskIndex_t TaskIndex);

  void                                 clearTaskIndexFromMaps(taskIndex_t TaskIndex);

public:

  TaskIndexNameMap      taskIndexName;
  TaskIndexValueNameMap taskIndexValueName;
  FilePresenceMap       fileExistsMap;
  RulesHelperClass      rulesHelper;

private:

  ExtraTaskSettingsMap extraTaskSettings_cache;

  #ifdef ESP32

  // Only cache Controller Settings on ESP32 due to memory restrictions on ESP8266
  ControllerSettingsMap controllerSetings_cache;
  #endif // ifdef ESP32

public:

  ChecksumType controllerSettings_checksums[CONTROLLER_MAX] = {};
  uint32_t     fileCacheClearMoment                         = 0;


  bool activeTaskUseSerial0 = false;
};


#endif // DATASTRUCTS_CACHES_H
