#include "../DataStructs/ExtraTaskSettingsStruct.h"

#include "../../ESPEasy_common.h"

ExtraTaskSettingsStruct::ExtraTaskSettingsStruct() : TaskIndex(INVALID_TASK_INDEX) {
  clear();
}

void ExtraTaskSettingsStruct::clear() {
  TaskIndex = INVALID_TASK_INDEX;
  ZERO_FILL(TaskDeviceName);

  for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
    TaskDeviceValueDecimals[i] = 2;
    ZERO_FILL(TaskDeviceFormula[i]);
    ZERO_FILL(TaskDeviceValueNames[i]);
  }

  for (uint8_t i = 0; i < PLUGIN_EXTRACONFIGVAR_MAX; ++i) {
    TaskDevicePluginConfigLong[i] = 0;
    TaskDevicePluginConfig[i]     = 0;
  }
}

void ExtraTaskSettingsStruct::validate() {
  ZERO_TERMINATE(TaskDeviceName);

  for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
    ZERO_TERMINATE(TaskDeviceFormula[i]);
    ZERO_TERMINATE(TaskDeviceValueNames[i]);
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
    TaskDeviceValueDecimals[i] = 2;
    ZERO_FILL(TaskDeviceFormula[i]);
    ZERO_FILL(TaskDeviceValueNames[i]);
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

  if (c >= '(' && c <= '-') return false;
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
    (c == '}'))
      return false;
  return true;
}