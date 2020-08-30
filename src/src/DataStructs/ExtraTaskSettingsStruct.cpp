#include "../DataStructs/ExtraTaskSettingsStruct.h"

#include "../../ESPEasy_common.h"

ExtraTaskSettingsStruct::ExtraTaskSettingsStruct() : TaskIndex(INVALID_TASK_INDEX) {
  clear();
}

void ExtraTaskSettingsStruct::clear() {
  TaskIndex = INVALID_TASK_INDEX;
  ZERO_FILL(TaskDeviceName);

  for (byte i = 0; i < VARS_PER_TASK; ++i) {
    TaskDeviceValueDecimals[i] = 2;
    ZERO_FILL(TaskDeviceFormula[i]);
    ZERO_FILL(TaskDeviceValueNames[i]);
  }

  for (byte i = 0; i < PLUGIN_EXTRACONFIGVAR_MAX; ++i) {
    TaskDevicePluginConfigLong[i] = 0;
    TaskDevicePluginConfig[i]     = 0;
  }
}

void ExtraTaskSettingsStruct::validate() {
  ZERO_TERMINATE(TaskDeviceName);

  for (byte i = 0; i < VARS_PER_TASK; ++i) {
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

void ExtraTaskSettingsStruct::clearUnusedValueNames(byte usedVars) {
  for (byte i = usedVars; i < VARS_PER_TASK; ++i) {
    TaskDeviceValueDecimals[i] = 2;
    ZERO_FILL(TaskDeviceFormula[i]);
    ZERO_FILL(TaskDeviceValueNames[i]);
  }
}

bool ExtraTaskSettingsStruct::checkInvalidCharInNames(const char *name) const {
  int pos = 0;

  while (*(name + pos) != 0) {
    switch (*(name + pos)) {
      case ',':
      case ' ':
      case '#':
      case '-':
      case '+':
      case '/':
      case '*':
      case '=':
      case '^':
      case '%':
      case '!':
      case '[':
      case ']':
      case '{':
      case '}':
      case '(':
      case ')':
        return false;
    }
    ++pos;
  }
  return true;
}

bool ExtraTaskSettingsStruct::checkInvalidCharInNames() const {
  if (!checkInvalidCharInNames(&TaskDeviceName[0])) { return false; }

  for (int i = 0; i < (VARS_PER_TASK - 1); ++i) {
    if (!checkInvalidCharInNames(&TaskDeviceValueNames[i][0])) { return false; }
  }
  return true;
}
