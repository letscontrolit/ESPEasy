#include "../DataStructs/Caches.h"

#include "../Globals/Device.h"
#include "../Globals/ExtraTaskSettings.h"
#include "../Globals/Settings.h"
#include "../Globals/WiFi_AP_Candidates.h"

#include "../Helpers/ESPEasy_Storage.h"


#include <ESPeasySerial.h>


void Caches::clearAllCaches()
{
  clearFileCaches();
  clearTaskCaches();
  WiFi_AP_Candidates.clearCache();
  rulesHelper.closeAllFiles();
}

void Caches::clearTaskCaches() {
  taskIndexName.clear();
  taskIndexValueName.clear();
  extraTaskSettings_cache.clear();
  updateActiveTaskUseSerial0();
}

void Caches::clearFileCaches()
{
  fileExistsMap.clear();
  fileCacheClearMoment = 0;
}

void Caches::updateActiveTaskUseSerial0() {
  activeTaskUseSerial0 = false;

  // Check to see if a task is enabled and using the pins we also use for receiving commands.
  // We're now receiving only from Serial0, so check if an enabled task is also using it.
  for (taskIndex_t task = 0; validTaskIndex(task); ++task)
  {
    const deviceIndex_t DeviceIndex = getDeviceIndex_from_TaskIndex(task);

    if (Settings.TaskDeviceEnabled[task] && validDeviceIndex(DeviceIndex)) {
      if ((Device[DeviceIndex].Type == DEVICE_TYPE_SERIAL) ||
          (Device[DeviceIndex].Type == DEVICE_TYPE_SERIAL_PLUS1)) {
        switch (ESPeasySerialType::getSerialType(
                  static_cast<ESPEasySerialPort>(Settings.TaskDevicePort[task]),
                  Settings.TaskDevicePin1[task],
                  Settings.TaskDevicePin2[task]))
        {
          case ESPEasySerialPort::serial0_swap:
          case ESPEasySerialPort::serial0:
            activeTaskUseSerial0 = true;
            break;
          default:
            break;
        }
      }
    }
  }
}

uint8_t Caches::getTaskDeviceValueDecimals(taskIndex_t TaskIndex, uint8_t rel_index)
{
  if (validTaskIndex(TaskIndex) && (rel_index < VARS_PER_TASK)) {
    auto it = getExtraTaskSettings(TaskIndex);

    if (it != extraTaskSettings_cache.end()) {
      return it->second.decimals[rel_index];
    }
  }
  return 0;
}

String Caches::getTaskDeviceName(taskIndex_t TaskIndex)
{
  if (validTaskIndex(TaskIndex)) {
  #ifdef ESP8266

    // ESP8266 uses SPIFFS, which is fast enough to read the task settings
    // Also RAM is limited, so don't waste it on caching where it is not needed.
    LoadTaskSettings(TaskIndex);
    return ExtraTaskSettings.TaskDeviceName;
  #endif // ifdef ESP8266
  #ifdef ESP32

    auto it = getExtraTaskSettings(TaskIndex);

    if (it != extraTaskSettings_cache.end()) {
      return it->second.TaskDeviceName;
    }
  #endif // ifdef ESP32
  }
  return EMPTY_STRING;
}

String Caches::getTaskDeviceValueName(taskIndex_t TaskIndex, uint8_t rel_index)
{
  if (validTaskIndex(TaskIndex) && (rel_index < VARS_PER_TASK)) {
  #ifdef ESP8266

    // ESP8266 uses SPIFFS, which is fast enough to read the task settings
    // Also RAM is limited, so don't waste it on caching where it is not needed.
    LoadTaskSettings(TaskIndex);
    return ExtraTaskSettings.TaskDeviceValueNames[rel_index];
  #endif // ifdef ESP8266
  #ifdef ESP32

    auto it = getExtraTaskSettings(TaskIndex);

    if (it != extraTaskSettings_cache.end()) {
      return it->second.TaskDeviceValueNames[rel_index];
    }
    #endif // ifdef ESP32
  }

  return EMPTY_STRING;
}

String Caches::getTaskDeviceFormula(taskIndex_t TaskIndex, uint8_t rel_index)
{
  if (validTaskIndex(TaskIndex) && (rel_index < VARS_PER_TASK)) {
    {
      // Just a quick test to see if we do have a formula present.
      // Task Formula are not used very often, so we will probably almost always have to return an empty string.
      auto it = getExtraTaskSettings(TaskIndex);

      if (it != extraTaskSettings_cache.end()) {
        if (!it->second.hasFormula) { return EMPTY_STRING; }
      }
    }

    LoadTaskSettings(TaskIndex);
    return ExtraTaskSettings.TaskDeviceFormula[rel_index];
  }
  return EMPTY_STRING;
}

long Caches::getTaskDevicePluginConfigLong(taskIndex_t TaskIndex, uint8_t rel_index)
{
  if (validTaskIndex(TaskIndex) && (rel_index < PLUGIN_EXTRACONFIGVAR_MAX)) {
    #ifdef ESP8266
    LoadTaskSettings(TaskIndex);
    return ExtraTaskSettings.TaskDevicePluginConfigLong[rel_index];
    #endif // ifdef ESP8266
    #ifdef ESP32
    auto it = getExtraTaskSettings(TaskIndex);

    if (it != extraTaskSettings_cache.end()) {
      if (bitRead(it->second.TaskDevicePluginConfigLong_index_used, rel_index) == 0)
      {
        return 0;
      }
      auto it_long = it->second.long_values.find(ExtraTaskSettings_cache_t::make_Long_ValuesIndex(rel_index));

      if (it_long != it->second.long_values.end()) {
        return it_long->second;
      }

      // Should not get here, since the long_values map should be in sync with the index_used bitmap.
    }
    #endif // ifdef ESP32
  }

  return 0;
}

int16_t Caches::getTaskDevicePluginConfig(taskIndex_t TaskIndex, uint8_t rel_index)
{
  if (validTaskIndex(TaskIndex) && (rel_index < PLUGIN_EXTRACONFIGVAR_MAX)) {
    #ifdef ESP8266
    LoadTaskSettings(TaskIndex);
    return ExtraTaskSettings.TaskDevicePluginConfig[rel_index];
    #endif // ifdef ESP8266
    #ifdef ESP32
    auto it = getExtraTaskSettings(TaskIndex);

    if (it != extraTaskSettings_cache.end()) {
      if (bitRead(it->second.TaskDevicePluginConfig_index_used, rel_index) == 0)
      {
        return 0;
      }
      auto it_long = it->second.long_values.find(ExtraTaskSettings_cache_t::make_Uint16_ValuesIndex(rel_index));

      if (it_long != it->second.long_values.end()) {
        return it_long->second;
      }

      // Should not get here, since the long_values map should be in sync with the index_used bitmap.
    }
    #endif // ifdef ESP32
  }

  return 0;
}

#if FEATURE_PLUGIN_STATS
bool Caches::enabledPluginStats(taskIndex_t TaskIndex, uint8_t rel_index)
{
  if (validTaskIndex(TaskIndex) && (rel_index < VARS_PER_TASK)) {
    auto it = getExtraTaskSettings(TaskIndex);

    if (it != extraTaskSettings_cache.end()) {
      return bitRead(it->second.enabledPluginStats, rel_index);
    }
  }
  return false;
}

#endif // if FEATURE_PLUGIN_STATS


void Caches::updateExtraTaskSettingsCache()
{
  const taskIndex_t TaskIndex = ExtraTaskSettings.TaskIndex;

  if (validTaskIndex(TaskIndex)) {
    auto it = extraTaskSettings_cache.find(TaskIndex);

    if (it != extraTaskSettings_cache.end()) {
      extraTaskSettings_cache.erase(it);
    }

    ExtraTaskSettings_cache_t tmp;
      #ifdef ESP32
    tmp.TaskDeviceName = ExtraTaskSettings.TaskDeviceName;
      #endif // ifdef ESP32

    #if FEATURE_PLUGIN_STATS
    tmp.enabledPluginStats = 0;
    #endif // if FEATURE_PLUGIN_STATS

    for (size_t i = 0; i < VARS_PER_TASK; ++i) {
        #ifdef ESP32
      tmp.TaskDeviceValueNames[i] = ExtraTaskSettings.TaskDeviceValueNames[i];
        #endif // ifdef ESP32

      if (ExtraTaskSettings.TaskDeviceFormula[i][0] != 0) {
        tmp.hasFormula = true;
      }
      tmp.decimals[i] = ExtraTaskSettings.TaskDeviceValueDecimals[i];
      #if FEATURE_PLUGIN_STATS

      if (ExtraTaskSettings.enabledPluginStats(i)) {
        bitSet(tmp.enabledPluginStats, i);
      }
      #endif // if FEATURE_PLUGIN_STATS
    }
    #ifdef ESP32
    tmp.TaskDevicePluginConfigLong_index_used = 0;
    tmp.TaskDevicePluginConfig_index_used     = 0;
    tmp.long_values.clear();

    for (size_t i = 0; i < PLUGIN_EXTRACONFIGVAR_MAX; ++i) {
      if (ExtraTaskSettings.TaskDevicePluginConfigLong[i] != 0) {
        bitSet(tmp.TaskDevicePluginConfigLong_index_used, i);
        tmp.long_values[ExtraTaskSettings_cache_t::make_Long_ValuesIndex(i)] = ExtraTaskSettings.TaskDevicePluginConfigLong[i];
      }

      if (ExtraTaskSettings.TaskDevicePluginConfig[i] != 0) {
        bitSet(tmp.TaskDevicePluginConfig_index_used, i);
        tmp.long_values[ExtraTaskSettings_cache_t::make_Uint16_ValuesIndex(i)] = ExtraTaskSettings.TaskDevicePluginConfig[i];
      }
    }
    #endif // ifdef ESP32

    extraTaskSettings_cache[TaskIndex] = tmp;
  }
}

ExtraTaskSettingsMap::const_iterator Caches::getExtraTaskSettings(taskIndex_t TaskIndex)
{
  if (validTaskIndex(TaskIndex)) {
    auto it = extraTaskSettings_cache.find(TaskIndex);

    if (it == extraTaskSettings_cache.end()) {
      ExtraTaskSettings.clear(); // Force reload so the cache is filled
      LoadTaskSettings(TaskIndex);
      it = extraTaskSettings_cache.find(TaskIndex);
    }
    return it;
  }
  return extraTaskSettings_cache.end();
}
