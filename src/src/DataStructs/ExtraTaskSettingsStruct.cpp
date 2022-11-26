#include "../DataStructs/ExtraTaskSettingsStruct.h"

#include "../../ESPEasy_common.h"

#define EXTRA_TASK_SETTINGS_VERSION 1

ExtraTaskSettingsStruct::ExtraTaskSettingsStruct() : TaskIndex(INVALID_TASK_INDEX) {
  ZERO_FILL(TaskDeviceName);

  clearUnusedValueNames(0);

  for (uint8_t i = 0; i < PLUGIN_EXTRACONFIGVAR_MAX; ++i) {
    TaskDevicePluginConfigLong[i] = 0;
    TaskDevicePluginConfig[i]     = 0;
  }
}

void ExtraTaskSettingsStruct::clear() {
  *this = ExtraTaskSettingsStruct();
}

void ExtraTaskSettingsStruct::validate() {
  ZERO_TERMINATE(TaskDeviceName);

  for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
    ZERO_TERMINATE(TaskDeviceFormula[i]);
    ZERO_TERMINATE(TaskDeviceValueNames[i]);
  }

  if (dummy1 != 0) {
    // FIXME TD-er: This check was added to add the version for allowing to make transitions on the data.
    // If we've been using this for a while, we no longer need to check for the value of this dummy and we can re-use it for something else.
    dummy1  = 0;
    version = 0;
  }

  if (version != EXTRA_TASK_SETTINGS_VERSION) {
    if (version < 1) {
      // Need to initialize the newly added fields
      for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
        setIgnoreRangeCheck(i);
        TaskDeviceErrorValue[i] = 0.0f;
        VariousBits[i]          = 0u;
      }
    }
    version = EXTRA_TASK_SETTINGS_VERSION;
  }
}

bool ExtraTaskSettingsStruct::checkUniqueValueNames() const {
  for (int i = 0; i < (VARS_PER_TASK - 1); ++i) {
    for (int j = i; j < VARS_PER_TASK; ++j) {
      if ((i != j) && (TaskDeviceValueNames[i][0] != 0)) {
        if (strcasecmp(TaskDeviceValueNames[i], TaskDeviceValueNames[j]) == 0) {
          return false;
        }
      }
    }
  }
  return true;
}

void ExtraTaskSettingsStruct::clearUnusedValueNames(uint8_t usedVars) {
  for (uint8_t i = usedVars; i < VARS_PER_TASK; ++i) {
    ZERO_FILL(TaskDeviceFormula[i]);
    ZERO_FILL(TaskDeviceValueNames[i]);
    TaskDeviceValueDecimals[i] = 2;
    setIgnoreRangeCheck(i);
    TaskDeviceErrorValue[i] = 0.0f;
    VariousBits[i] = 0;
  }
}

bool ExtraTaskSettingsStruct::checkInvalidCharInNames(const char *name) const {
  int pos = 0;

  while (*(name + pos) != 0) {
    if (!validCharForNames(*(name + pos))) { return false; }
    ++pos;
  }
  return true;
}

bool ExtraTaskSettingsStruct::checkInvalidCharInNames() const {
  if (!checkInvalidCharInNames(&TaskDeviceName[0])) { return false; }

  for (int i = 0; i < VARS_PER_TASK; ++i) {
    if (!checkInvalidCharInNames(&TaskDeviceValueNames[i][0])) { return false; }
  }
  return true;
}

bool ExtraTaskSettingsStruct::validCharForNames(char c) {
  // Smal optimization to check these chars as they are in sequence in the ASCII table

  /*
     case '(': // 40
     case ')': // 41
     case '*': // 42
     case '+': // 43
     case ',': // 44
     case '-': // 45
   */

  if ((c >= '(') && (c <= '-')) { return false; }

  if (
    (c == ' ') ||
    (c == '!') ||
    (c == '#') ||
    (c == '%') ||
    (c == '/') ||
    (c == '=') ||
    (c == '[') ||
    (c == ']') ||
    (c == '^') ||
    (c == '{') ||
    (c == '}')) {
    return false;
  }
  return true;
}

void ExtraTaskSettingsStruct::setAllowedRange(taskVarIndex_t taskVarIndex, const float& minValue, const float& maxValue)
{
  if (validTaskVarIndex(taskVarIndex)) {
    if (minValue > maxValue) {
      TaskDeviceMinValue[taskVarIndex] = maxValue;
      TaskDeviceMaxValue[taskVarIndex] = minValue;
    } else {
      TaskDeviceMinValue[taskVarIndex] = minValue;
      TaskDeviceMaxValue[taskVarIndex] = maxValue;
    }
  }
}

void ExtraTaskSettingsStruct::setIgnoreRangeCheck(taskVarIndex_t taskVarIndex)
{
  if (validTaskVarIndex(taskVarIndex)) {
    // Clear range to indicate no range check should be done.
    TaskDeviceMinValue[taskVarIndex] = 0.0f;
    TaskDeviceMaxValue[taskVarIndex] = 0.0f;
  }
}

bool ExtraTaskSettingsStruct::ignoreRangeCheck(taskVarIndex_t taskVarIndex) const
{
  if (validTaskVarIndex(taskVarIndex)) {
    return essentiallyEqual(TaskDeviceMinValue[taskVarIndex], TaskDeviceMaxValue[taskVarIndex]);
  }
  return true;
}

bool ExtraTaskSettingsStruct::valueInAllowedRange(taskVarIndex_t taskVarIndex, const float& value) const
{
  if (ignoreRangeCheck(taskVarIndex)) { return true; }

  if (validTaskVarIndex(taskVarIndex)) {
    return definitelyLessThan(value, TaskDeviceMaxValue[taskVarIndex]) ||
           definitelyGreaterThan(value, TaskDeviceMinValue[taskVarIndex]);
  }
  #ifndef BUILD_NO_DEBUG
  addLog(LOG_LEVEL_ERROR, F("Programming error: invalid taskVarIndex"));
  #endif // ifndef BUILD_NO_DEBUG
  return false;
}

float ExtraTaskSettingsStruct::checkAllowedRange(taskVarIndex_t taskVarIndex, const float& value) const
{
  if (!valueInAllowedRange(taskVarIndex, value)) {
    if (validTaskVarIndex(taskVarIndex)) {
      return TaskDeviceErrorValue[taskVarIndex];
    }
  }
  return value;
}

#if FEATURE_PLUGIN_STATS
// Plugin Stats is now only a single bit, but this may later changed into a combobox with some options.
// Thus leave 8 bits for the plugin stats options.

bool ExtraTaskSettingsStruct::enabledPluginStats(taskVarIndex_t taskVarIndex) const
{
  if (!validTaskVarIndex(taskVarIndex)) { return false; }
  return bitRead(VariousBits[taskVarIndex], 0);
}

void ExtraTaskSettingsStruct::enablePluginStats(taskVarIndex_t taskVarIndex, bool enabled)
{
  if (validTaskVarIndex(taskVarIndex)) {
    bitWrite(VariousBits[taskVarIndex], 0, enabled);
  }
}

bool ExtraTaskSettingsStruct::anyEnabledPluginStats() const
{
  for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
    if (enabledPluginStats(i)) return true;
  }
  return false;
}
#endif
