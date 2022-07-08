#ifndef DATASTRUCTS_EXTRATASKSETTINGSSTRUCT_H
#define DATASTRUCTS_EXTRATASKSETTINGSSTRUCT_H

/*********************************************************************************************\
* ExtraTaskSettingsStruct
\*********************************************************************************************/

#include <Arduino.h>

#include "../CustomBuild/ESPEasyLimits.h"
#include "../Globals/Plugins.h"

// This is only used by some plugins to store extra settings like formula descriptions.
// These settings can only be active for one plugin, meaning they have to be loaded
// over and over again from flash when another active plugin uses these values.
// FIXME @TD-er: Should think of another mechanism to make this more efficient.
struct ExtraTaskSettingsStruct
{
  ExtraTaskSettingsStruct();

  void        clear();

  void        validate();

  bool        checkUniqueValueNames() const;

  void        clearUnusedValueNames(uint8_t usedVars);

  bool        checkInvalidCharInNames(const char *name) const;

  bool        checkInvalidCharInNames() const;

  static bool validCharForNames(char character);

  void        setAllowedRange(taskVarIndex_t taskVarIndex,
                              const float  & minValue,
                              const float  & maxValue);
  void        setIgnoreRangeCheck(taskVarIndex_t taskVarIndex);

  bool        ignoreRangeCheck(taskVarIndex_t taskVarIndex) const;

  float       checkAllowedRange(taskVarIndex_t taskVarIndex,
                                const float  & value) const;

  bool        valueInAllowedRange(taskVarIndex_t taskVarIndex,
                                  const float  & value) const;

  taskIndex_t TaskIndex; // Always < TASKS_MAX or INVALID_TASK_INDEX
  char        TaskDeviceName[NAME_FORMULA_LENGTH_MAX + 1];
  char        TaskDeviceFormula[VARS_PER_TASK][NAME_FORMULA_LENGTH_MAX + 1];
  char        TaskDeviceValueNames[VARS_PER_TASK][NAME_FORMULA_LENGTH_MAX + 1];
  long        TaskDevicePluginConfigLong[PLUGIN_EXTRACONFIGVAR_MAX] = {0};
  uint8_t     TaskDeviceValueDecimals[VARS_PER_TASK] = {0};
  int16_t     TaskDevicePluginConfig[PLUGIN_EXTRACONFIGVAR_MAX] = {0};
  float       TaskDeviceMinValue[VARS_PER_TASK];
  float       TaskDeviceMaxValue[VARS_PER_TASK];
  float       TaskDeviceErrorValue[VARS_PER_TASK];
};


#endif // DATASTRUCTS_EXTRATASKSETTINGSSTRUCT_H
