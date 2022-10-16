#ifndef DATASTRUCTS_CACHES_H
#define DATASTRUCTS_CACHES_H

#include <map>
#include "../../ESPEasy_common.h"
#include "../CustomBuild/ESPEasyLimits.h"
#include "../Globals/Plugins.h"
#include "../Helpers/RulesHelper.h"


struct ExtraTaskSettings_cache_t {
  #ifdef ESP32
  String TaskDeviceValueNames[VARS_PER_TASK];
  String TaskDeviceName;
  #endif // ifdef ESP32
  uint8_t decimals[VARS_PER_TASK] = { 0 };
  bool    hasFormula              = false;
};

typedef std::map<String, taskIndex_t>                   TaskIndexNameMap;
typedef std::map<String, uint8_t>                       TaskIndexValueNameMap;
typedef std::map<String, int>                          FilePresenceMap;
typedef std::map<taskIndex_t, ExtraTaskSettings_cache_t>ExtraTaskSettingsMap;

struct Caches {
  void    clearAllCaches();

  void    clearTaskCaches();

  void    clearFileCaches();

  void    updateActiveTaskUseSerial0();

  uint8_t getTaskDeviceValueDecimals(taskIndex_t TaskIndex,
                                     uint8_t     rel_index);

  String  getTaskDeviceName(taskIndex_t TaskIndex);

  String  getTaskDeviceValueName(taskIndex_t TaskIndex,
                                 uint8_t     rel_index);

  String  getTaskDeviceFormula(taskIndex_t TaskIndex,
                               uint8_t     rel_index);

  // Update the cached value, called after LoadTaskSettings()
  void updateExtraTaskSettingsCache();

private:

  ExtraTaskSettingsMap::const_iterator getExtraTaskSettings(taskIndex_t TaskIndex);

public:

  TaskIndexNameMap      taskIndexName;
  TaskIndexValueNameMap taskIndexValueName;
  FilePresenceMap       fileExistsMap;  // Filesize. -1 if not present
  RulesHelperClass      rulesHelper;

private:

  ExtraTaskSettingsMap extraTaskSettings_cache;

public:

  uint32_t fileCacheClearMoment = 0;

  bool activeTaskUseSerial0 = false;
};


#endif // DATASTRUCTS_CACHES_H
