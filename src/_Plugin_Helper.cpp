#include "_Plugin_Helper.h"

#include "ESPEasy_common.h"
#include "ESPEasy_fdwdecl.h"

#include "src/DataStructs/ESPEasyLimits.h"
#include "src/DataStructs/SettingsStruct.h"
#include "src/Globals/Plugins.h"
#include "src/Globals/Settings.h"
#include "src/Globals/SecuritySettings.h"


PluginTaskData_base *Plugin_task_data[TASKS_MAX] = { NULL, };

String PCONFIG_LABEL(int n) {
  if (n < PLUGIN_CONFIGVAR_MAX) {
    String result = F("pconf_");
    result += n;
    return result;
  }
  return F("error");
}


void resetPluginTaskData() {
  for (byte i = 0; i < TASKS_MAX; ++i) {
    Plugin_task_data[i] = nullptr;
  }
}

void clearPluginTaskData(byte taskIndex) {
  if (taskIndex < TASKS_MAX) {
    if (Plugin_task_data[taskIndex] != nullptr) {
      delete Plugin_task_data[taskIndex];
      Plugin_task_data[taskIndex] = nullptr;
    }
  }
}

void initPluginTaskData(byte taskIndex, PluginTaskData_base *data) {
  clearPluginTaskData(taskIndex);

  if ((taskIndex < TASKS_MAX) && Settings.TaskDeviceEnabled[taskIndex]) {
    Plugin_task_data[taskIndex]                      = data;
    Plugin_task_data[taskIndex]->_taskdata_plugin_id = getPluginId_from_TaskIndex(taskIndex);
  }
}

PluginTaskData_base* getPluginTaskData(byte taskIndex) {
  if (taskIndex >= TASKS_MAX) {
    return nullptr;
  }

  if ((Plugin_task_data[taskIndex] != nullptr) && (Plugin_task_data[taskIndex]->_taskdata_plugin_id == getPluginId_from_TaskIndex(taskIndex))) {
    return Plugin_task_data[taskIndex];
  }
  return nullptr;
}

bool pluginTaskData_initialized(byte taskIndex) {
  // FIXME TD-er: Must check for type also.
  if (taskIndex < TASKS_MAX) {
    return Plugin_task_data[taskIndex] != nullptr;
  }
  return false;
}

String getPluginCustomArgName(int varNr) {
  String argName = F("plugin_custom_arg");
  argName += varNr + 1;
  return argName;
}

// Helper function to create formatted custom values for display in the devices overview page.
// When called from PLUGIN_WEBFORM_SHOW_VALUES, the last item should add a traling div_br class
// if the regular values should also be displayed.
// The call to PLUGIN_WEBFORM_SHOW_VALUES should only return success = true when no regular values should be displayed
// Note that the varNr of the custom values should not conflict with the existing variable numbers (e.g. start at VARS_PER_TASK)
String pluginWebformShowValue(byte taskIndex, byte varNr, const String& label, const String& value, bool addTrailingBreak) {
  String result;
  size_t length = 96 + label.length() + value.length();
  String breakStr = F("<div class='div_br'></div>");
  if (addTrailingBreak) {
    length += breakStr.length();
  }
  result.reserve(length);
  if (varNr > 0) {
    result += breakStr;
  }
  result += F("<div class='div_l' id='valuename_");
  result += String(taskIndex);
  result += '_';
  result += String(varNr);
  result += "'>";
  result += label;
  result += F(":</div><div class='div_r' id='value_");
  result += String(taskIndex);
  result += '_';
  result += String(varNr);
  result += "'>";
  result += value;
  result += "</div>";
  if (addTrailingBreak) {
    result += breakStr;
  }
  return result;
}
