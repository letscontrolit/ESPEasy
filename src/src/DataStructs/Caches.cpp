#include "../DataStructs/Caches.h"

#include "../Globals/Device.h"
#include "../Globals/ExtraTaskSettings.h"
#include "../Globals/Settings.h"
#include "../Globals/WiFi_AP_Candidates.h"

#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/StringConverter.h"

#ifdef PLUGIN_USES_SERIAL
# include <ESPeasySerial.h>
#endif // ifdef PLUGIN_USES_SERIAL

void Caches::clearAllCaches()
{
  clearAllButTaskCaches();
  clearAllTaskCaches();
}

void Caches::clearAllButTaskCaches() {
  clearFileCaches();
  WiFi_AP_Candidates.clearCache();
  rulesHelper.closeAllFiles();
}

void Caches::clearAllTaskCaches() {
  taskIndexName.clear();
  taskIndexValueName.clear();
  extraTaskSettings_cache.clear();
  updateActiveTaskUseSerial0();
}

void Caches::clearTaskCache(taskIndex_t TaskIndex) {
  clearTaskIndexFromMaps(TaskIndex);

  auto it = extraTaskSettings_cache.find(TaskIndex);

  if (it != extraTaskSettings_cache.end()) {
    extraTaskSettings_cache.erase(it);
  }
  updateActiveTaskUseSerial0();
}

void Caches::clearFileCaches()
{
  fileExistsMap.clear();
  fileCacheClearMoment = 0;
}

bool Caches::matchChecksumExtraTaskSettings(taskIndex_t TaskIndex, const ChecksumType& checksum) const
{
  if (validTaskIndex(TaskIndex)) {
    auto it = extraTaskSettings_cache.find(TaskIndex);

    if (it != extraTaskSettings_cache.end()) {
      return checksum == it->second.md5checksum;
    }
  }
  return false;
}

void Caches::updateActiveTaskUseSerial0() {
  activeTaskUseSerial0 = false;
#ifdef PLUGIN_USES_SERIAL

  if (getDeviceCount() <= 0) {
    return;
  }

  // Check to see if a task is enabled and using the pins we also use for receiving commands.
  // We're now receiving only from Serial0, so check if an enabled task is also using it.
  for (taskIndex_t task = 0; task < TASKS_MAX; ++task)
  {
    const deviceIndex_t DeviceIndex = getDeviceIndex_from_TaskIndex(task);

    if (Settings.TaskDeviceEnabled[task] && validDeviceIndex(DeviceIndex)) {
      if ((Device[DeviceIndex].Type == DEVICE_TYPE_SERIAL) ||
          (Device[DeviceIndex].Type == DEVICE_TYPE_SERIAL_PLUS1)) {
        const ESPEasySerialPort port = ESPeasySerialType::getSerialType(
          static_cast<ESPEasySerialPort>(Settings.TaskDevicePort[task]),
          Settings.TaskDevicePin1[task],
          Settings.TaskDevicePin2[task]);

        // FIXME TD-er: Must not check for conflict with serial0, but for conflict with ESPEasy_Console.
        # ifdef ESP32

        if (port == ESPEasySerialPort::serial0)
        {
          activeTaskUseSerial0 = true;
        }
        # endif // ifdef ESP32
        # ifdef ESP8266

        if ((port == ESPEasySerialPort::serial0_swap) ||
            (port == ESPEasySerialPort::serial0))
        {
          activeTaskUseSerial0 = true;
        }
        # endif // ifdef ESP8266
      }
    }
  }
#endif // ifdef PLUGIN_USES_SERIAL
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
    auto it = getExtraTaskSettings(TaskIndex);

    if (it != extraTaskSettings_cache.end()) {
      return it->second.TaskDeviceName;
    }
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

bool Caches::hasFormula(taskIndex_t TaskIndex, uint8_t rel_index)
{
  if (validTaskIndex(TaskIndex) && (rel_index < VARS_PER_TASK)) {
    // Just a quick test to see if we do have a formula present.
    // Task Formula are not used very often, so we will probably almost always have to return an empty string.
    auto it = getExtraTaskSettings(TaskIndex);

    if (it != extraTaskSettings_cache.end()) {
      return bitRead(it->second.hasFormula, 2 * rel_index);
    }
  }
  return false;
}

bool Caches::hasFormula(taskIndex_t TaskIndex)
{
  if (validTaskIndex(TaskIndex)) {
    // Just a quick test to see if we do have a formula present.
    // Task Formula are not used very often, so we will probably almost always have to return an empty string.
    auto it = getExtraTaskSettings(TaskIndex);

    if (it != extraTaskSettings_cache.end()) {
      return it->second.hasFormula != 0;
    }
  }
  return false;
}

bool Caches::hasFormula_with_prevValue(taskIndex_t TaskIndex, uint8_t rel_index)
{
  if (validTaskIndex(TaskIndex) && (rel_index < VARS_PER_TASK)) {
    // Just a quick test to see if we do have a formula present which requires %pvalue%.
    auto it = getExtraTaskSettings(TaskIndex);

    if (it != extraTaskSettings_cache.end()) {
      return bitRead(it->second.hasFormula, 2 * rel_index + 1);
    }
  }
  return false;
}

String Caches::getTaskDeviceFormula(taskIndex_t TaskIndex, uint8_t rel_index)
{
  if ((rel_index < VARS_PER_TASK) && hasFormula(TaskIndex, rel_index)) {
    #ifdef ESP32
    auto it = getExtraTaskSettings(TaskIndex);

    if (it != extraTaskSettings_cache.end()) {
      return it->second.TaskDeviceFormula[rel_index];
    }
    #else // ifdef ESP32
    LoadTaskSettings(TaskIndex);
    return ExtraTaskSettings.TaskDeviceFormula[rel_index];
    #endif // ifdef ESP32
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
      return it->second.pluginStatsConfig[rel_index].isEnabled();
    }
  }
  return false;
}

PluginStats_Config_t Caches::getPluginStatsConfig(taskIndex_t TaskIndex, taskVarIndex_t taskVarIndex)
{
  if (validTaskIndex(TaskIndex) && (taskVarIndex < VARS_PER_TASK)) {
    auto it = getExtraTaskSettings(TaskIndex);

    if (it != extraTaskSettings_cache.end()) {
      return it->second.pluginStatsConfig[taskVarIndex];
    }
  }
  return PluginStats_Config_t(0);
}

#endif // if FEATURE_PLUGIN_STATS

#if FEATURE_TASKVALUE_UNIT_OF_MEASURE
uint8_t Caches::getTaskVarUnitOfMeasure(taskIndex_t    TaskIndex,
                                        taskVarIndex_t taskVarIndex) {
  if (validTaskIndex(TaskIndex) && (validTaskVarIndex(taskVarIndex))) {
    auto it = getExtraTaskSettings(TaskIndex);

    if (it != extraTaskSettings_cache.end()) {
      return it->second.unitOfMeasure[taskVarIndex];
    }
  }
  return 0;
}

#endif // if FEATURE_TASKVALUE_UNIT_OF_MEASURE

#if FEATURE_CUSTOM_TASKVAR_VTYPE
uint8_t Caches::getTaskVarCustomVType(taskIndex_t    TaskIndex,
                                      taskVarIndex_t taskVarIndex) {
  if (validTaskIndex(TaskIndex) && (validTaskVarIndex(taskVarIndex))) {
    auto it = getExtraTaskSettings(TaskIndex);

    if (it != extraTaskSettings_cache.end()) {
      return it->second.customVType[taskVarIndex];
    }
  }
  return 0;
}

#endif // if FEATURE_CUSTOM_TASKVAR_VTYPE

#if FEATURE_MQTT_STATE_CLASS
uint8_t Caches::getTaskVarStateClass(taskIndex_t    TaskIndex,
                                     taskVarIndex_t taskVarIndex) {
  if (validTaskIndex(TaskIndex) && (validTaskVarIndex(taskVarIndex))) {
    auto it = getExtraTaskSettings(TaskIndex);

    if (it != extraTaskSettings_cache.end()) {
      return it->second.mqttStateClass[taskVarIndex];
    }
  }
  return 0;
}

#endif // if FEATURE_MQTT_STATE_CLASS

void Caches::updateExtraTaskSettingsCache()
{
  const taskIndex_t TaskIndex = ExtraTaskSettings.TaskIndex;

  if (validTaskIndex(TaskIndex)) {
    ExtraTaskSettings_cache_t tmp;

    auto it = extraTaskSettings_cache.find(TaskIndex);

    if (it != extraTaskSettings_cache.end()) {
      // We need to keep the original checksum, from when loaded from storage
      tmp.md5checksum                = it->second.md5checksum;
      tmp.defaultTaskDeviceValueName = it->second.defaultTaskDeviceValueName;

      // Now clear it so we can create a fresh copy.
      extraTaskSettings_cache.erase(it);
      clearTaskIndexFromMaps(TaskIndex);
    }
    move_special(tmp.TaskDeviceName, String(ExtraTaskSettings.TaskDeviceName));

    for (size_t i = 0; i < VARS_PER_TASK; ++i) {
      #ifdef ESP32
      move_special(tmp.TaskDeviceValueNames[i], String(ExtraTaskSettings.TaskDeviceValueNames[i]));
      #endif // ifdef ESP32

      if (ExtraTaskSettings.TaskDeviceFormula[i][0] != 0) {
        bitSet(tmp.hasFormula, 2 * i);
        String formula(ExtraTaskSettings.TaskDeviceFormula[i]);

        if (formula.indexOf(F("%pvalue%")) != -1) {
          bitSet(tmp.hasFormula, 2 * i + 1);
        }


        #ifdef ESP32
        tmp.TaskDeviceFormula[i] = std::move(formula);
        #endif // ifdef ESP32
      }
      tmp.decimals[i] = ExtraTaskSettings.TaskDeviceValueDecimals[i];
      #if FEATURE_PLUGIN_STATS

      tmp.pluginStatsConfig[i] = ExtraTaskSettings.getPluginStatsConfig(i);
      #endif // if FEATURE_PLUGIN_STATS

      #if FEATURE_TASKVALUE_UNIT_OF_MEASURE
      tmp.unitOfMeasure[i] = ExtraTaskSettings.getTaskVarUnitOfMeasure(i);
      #endif // if FEATURE_TASKVALUE_UNIT_OF_MEASURE
      #if FEATURE_CUSTOM_TASKVAR_VTYPE
      tmp.customVType[i] = ExtraTaskSettings.getTaskVarCustomVType(i);
      #endif // if FEATURE_CUSTOM_TASKVAR_VTYPE
      #if FEATURE_MQTT_STATE_CLASS
      tmp.mqttStateClass[i] = ExtraTaskSettings.getTaskVarStateClass(i);
      #endif // if FEATURE_MQTT_STATE_CLASS
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

    extraTaskSettings_cache.emplace(std::make_pair(TaskIndex, std::move(tmp)));
  }
}

void Caches::updateExtraTaskSettingsCache_afterLoad_Save()
{
  if (!validTaskIndex(ExtraTaskSettings.TaskIndex)) {
    return;
  }

  // First update all other values
  updateExtraTaskSettingsCache();

  // Iterator has changed
  auto it = extraTaskSettings_cache.find(ExtraTaskSettings.TaskIndex);

  if (it != extraTaskSettings_cache.end()) {
    for (size_t i = 0; i < VARS_PER_TASK; ++i) {
      bitWrite(it->second.defaultTaskDeviceValueName, i, ExtraTaskSettings.isDefaultTaskVarName(i));
    }
    it->second.md5checksum = ExtraTaskSettings.computeChecksum();
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

void Caches::clearTaskIndexFromMaps(taskIndex_t TaskIndex)
{
  {
    auto it = taskIndexName.begin();

    for (; it != taskIndexName.end();) {
      if (it->second == TaskIndex) {
        it = taskIndexName.erase(it);
      } else {
        ++it;
      }
    }
  }
  {
    const String searchstr = String('#') + TaskIndex;
    auto it                = taskIndexValueName.begin();

    for (; it != taskIndexValueName.end();) {
      if (it->first.endsWith(searchstr)) {
        it = taskIndexValueName.erase(it);
      } else {
        ++it;
      }
    }
  }
}

  #ifdef ESP32
bool Caches::getControllerSettings(controllerIndex_t index,  ControllerSettingsStruct& ControllerSettings) const
{
  if (!validControllerIndex(index)) { return false; }

  auto it = controllerSetings_cache.find(index);

  if (it == controllerSetings_cache.end()) {
    return false;
  }

  ControllerSettings = it->second;
  return true;
}

void Caches::setControllerSettings(controllerIndex_t index, const ControllerSettingsStruct& ControllerSettings)
{
  auto it = controllerSetings_cache.find(index);

  if (it != controllerSetings_cache.end()) {
    if (it->second.computeChecksum() == ControllerSettings.computeChecksum()) {
      return;
    }
    controllerSetings_cache.erase(it);
  }
  controllerSetings_cache.emplace(index, ControllerSettings);
}

void Caches::clearControllerSettings(controllerIndex_t index)
{
  auto it = controllerSetings_cache.find(index);

  if (it != controllerSetings_cache.end()) {
    controllerSetings_cache.erase(it);
  }
}

  #endif // ifdef ESP32
