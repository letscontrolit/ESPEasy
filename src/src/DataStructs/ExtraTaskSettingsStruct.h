#ifndef DATASTRUCTS_EXTRATASKSETTINGSSTRUCT_H
#define DATASTRUCTS_EXTRATASKSETTINGSSTRUCT_H

/*********************************************************************************************\
* ExtraTaskSettingsStruct
\*********************************************************************************************/

#include <Arduino.h>

#include "../CustomBuild/ESPEasyLimits.h"
#include "../DataStructs/ChecksumType.h"
#include "../Globals/Plugins.h"

// This is only used by some plugins to store extra settings like formula descriptions.
// These settings can only be active for one plugin, meaning they have to be loaded
// over and over again from flash when another active plugin uses these values.
// FIXME @TD-er: Should think of another mechanism to make this more efficient.
struct ExtraTaskSettingsStruct
{
  ExtraTaskSettingsStruct() = default;

  void          clear();

  void          validate();
 
  ChecksumType  computeChecksum() const;

  bool          checkUniqueValueNames() const;

  void          clearUnusedValueNames(uint8_t usedVars);

  bool          checkInvalidCharInNames(const char *name) const;

  bool          checkInvalidCharInNames() const;

  static bool   validCharForNames(char character);
  static String getInvalidCharsForNames();

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
#endif // if FEATURE_PLUGIN_STATS

  void          populateDeviceValueNamesSeq(const __FlashStringHelper *valuename,
                                            size_t                     nrValues,
                                            uint8_t                    defaultDecimals,
                                            bool                       displayString);

  taskIndex_t TaskIndex = INVALID_TASK_INDEX; // Always < TASKS_MAX or INVALID_TASK_INDEX
  char        TaskDeviceName[NAME_FORMULA_LENGTH_MAX + 1];
  char        TaskDeviceFormula[VARS_PER_TASK][NAME_FORMULA_LENGTH_MAX + 1];
  char        TaskDeviceValueNames[VARS_PER_TASK][NAME_FORMULA_LENGTH_MAX + 1];
  uint8_t     dummy1                                                           = 0;
  uint8_t     version                                                          = 1;
  long        TaskDevicePluginConfigLong[PLUGIN_EXTRACONFIGVAR_MAX];
  uint8_t     TaskDeviceValueDecimals[VARS_PER_TASK];
  int16_t     TaskDevicePluginConfig[PLUGIN_EXTRACONFIGVAR_MAX];
  float       TaskDeviceMinValue[VARS_PER_TASK];
  float       TaskDeviceMaxValue[VARS_PER_TASK];
  float       TaskDeviceErrorValue[VARS_PER_TASK];
  uint32_t    VariousBits[VARS_PER_TASK];
};


#endif // DATASTRUCTS_EXTRATASKSETTINGSSTRUCT_H
