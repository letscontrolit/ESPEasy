#ifndef DATASTRUCTS_EXTRATASKSETTINGSSTRUCT_H
#define DATASTRUCTS_EXTRATASKSETTINGSSTRUCT_H

/*********************************************************************************************\
* ExtraTaskSettingsStruct
\*********************************************************************************************/

#include "../../ESPEasy_common.h"

#include "../CustomBuild/ESPEasyLimits.h"
#include "../DataStructs/ChecksumType.h"
#if FEATURE_PLUGIN_STATS
#include "../DataStructs/PluginStats_Config.h"
#endif
#include "../Globals/Plugins.h"

// This is only used by some plugins to store extra settings like formula descriptions.
// These settings can only be active for one plugin, meaning they have to be loaded
// over and over again from flash when another active plugin uses these values.
// FIXME @TD-er: Should think of another mechanism to make this more efficient.
struct ExtraTaskSettingsStruct
{
  ExtraTaskSettingsStruct();

  void          clear();

  void          validate();
 
  ChecksumType  computeChecksum() const;

  bool          checkUniqueValueNames() const;

  void          clearUnusedValueNames(uint8_t usedVars);

  bool          checkInvalidCharInNames(const char *name) const;

  bool          checkInvalidCharInNames() const;

  static bool   validCharForNames(char character);
  static String getInvalidCharsForNames();

  void          setTaskDeviceValueName(taskVarIndex_t taskVarIndex, const String& str);
  void          setTaskDeviceValueName(taskVarIndex_t taskVarIndex, const __FlashStringHelper * str);

  void          clearTaskDeviceValueName(taskVarIndex_t taskVarIndex);
  void          clearDefaultTaskDeviceValueNames();

  void          setAllowedRange(taskVarIndex_t taskVarIndex,
                                const float  & minValue,
                                const float  & maxValue);
  void          setIgnoreRangeCheck(taskVarIndex_t taskVarIndex);

  bool          ignoreRangeCheck(taskVarIndex_t taskVarIndex) const;

  float         checkAllowedRange(taskVarIndex_t taskVarIndex,
                                  const float  & value) const;

  bool          valueInAllowedRange(taskVarIndex_t taskVarIndex,
                                    const float  & value) const;
#if FEATURE_PLUGIN_STATS
  bool          enabledPluginStats(taskVarIndex_t taskVarIndex) const;
  void          enablePluginStats(taskVarIndex_t taskVarIndex,
                                  bool           enabled);
  bool          anyEnabledPluginStats() const;

  PluginStats_Config_t getPluginStatsConfig(taskVarIndex_t taskVarIndex) const;
  void setPluginStatsConfig(taskVarIndex_t taskVarIndex, PluginStats_Config_t config);

#endif // if FEATURE_PLUGIN_STATS

  bool          isDefaultTaskVarName(taskVarIndex_t taskVarIndex) const;
  void          isDefaultTaskVarName(taskVarIndex_t taskVarIndex,
                                     bool           isDefault);

  #if FEATURE_TASKVALUE_UNIT_OF_MEASURE
  uint8_t       getTaskVarUnitOfMeasure(taskVarIndex_t taskVarIndex) const;
  void          setTaskVarUnitOfMeasure(taskVarIndex_t taskVarIndex,
  uint8_t        unitOfMeasure);
  #endif // if FEATURE_TASKVALUE_UNIT_OF_MEASURE
                                      
  #if FEATURE_CUSTOM_TASKVAR_VTYPE
  uint8_t       getTaskVarCustomVType(taskVarIndex_t taskVarIndex) const;
  void          setTaskVarCustomVType(taskVarIndex_t taskVarIndex,
                                      uint8_t        customVType);
  #endif // if FEATURE_CUSTOM_TASKVAR_VTYPE

  #if FEATURE_MQTT_STATE_CLASS
  uint8_t       getTaskVarStateClass(taskVarIndex_t taskVarIndex) const;
  void          setTaskVarStateClass(taskVarIndex_t taskVarIndex,
                                     uint8_t        stateClass);
  #endif // if FEATURE_MQTT_STATE_CLASS

  void          populateDeviceValueNamesSeq(const __FlashStringHelper *valuename,
                                            size_t                     nrValues,
                                            uint8_t                    defaultDecimals,
                                            bool                       displayString);

  taskIndex_t TaskIndex; // Always < TASKS_MAX or INVALID_TASK_INDEX
  char        TaskDeviceName[NAME_FORMULA_LENGTH_MAX + 1];
  char        TaskDeviceFormula[VARS_PER_TASK][NAME_FORMULA_LENGTH_MAX + 1];
  char        TaskDeviceValueNames[VARS_PER_TASK][NAME_FORMULA_LENGTH_MAX + 1];
  uint8_t     dummy1;
  uint8_t     version;
  long        TaskDevicePluginConfigLong[PLUGIN_EXTRACONFIGVAR_MAX];
  uint8_t     TaskDeviceValueDecimals[VARS_PER_TASK];
  int16_t     TaskDevicePluginConfig[PLUGIN_EXTRACONFIGVAR_MAX];
  float       TaskDeviceMinValue[VARS_PER_TASK];
  float       TaskDeviceMaxValue[VARS_PER_TASK];
  float       TaskDeviceErrorValue[VARS_PER_TASK];
  uint32_t    VariousBits[VARS_PER_TASK];
};


#endif // DATASTRUCTS_EXTRATASKSETTINGSSTRUCT_H
