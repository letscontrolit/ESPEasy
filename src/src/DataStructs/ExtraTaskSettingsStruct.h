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
//FIXME @TD-er: Should think of another mechanism to make this more efficient.
struct ExtraTaskSettingsStruct
{
  ExtraTaskSettingsStruct();

  void clear();

  void validate();

  bool checkUniqueValueNames() const;

  void clearUnusedValueNames(byte usedVars);

  bool checkInvalidCharInNames(const char* name) const;

  bool checkInvalidCharInNames() const;

  taskIndex_t  TaskIndex;  // Always < TASKS_MAX or INVALID_TASK_INDEX
  char    TaskDeviceName[NAME_FORMULA_LENGTH_MAX + 1];
  char    TaskDeviceFormula[VARS_PER_TASK][NAME_FORMULA_LENGTH_MAX + 1];
  char    TaskDeviceValueNames[VARS_PER_TASK][NAME_FORMULA_LENGTH_MAX + 1];
  long    TaskDevicePluginConfigLong[PLUGIN_EXTRACONFIGVAR_MAX];
  byte    TaskDeviceValueDecimals[VARS_PER_TASK];
  int16_t TaskDevicePluginConfig[PLUGIN_EXTRACONFIGVAR_MAX];
};


#endif // DATASTRUCTS_EXTRATASKSETTINGSSTRUCT_H
