#include "_Plugin_Helper.h"

#include "ESPEasy_common.h"

#include "src/CustomBuild/ESPEasyLimits.h"
#include "src/DataStructs/SettingsStruct.h"
#include "src/Globals/Plugins.h"
#include "src/Globals/Settings.h"
#include "src/Globals/SecuritySettings.h"
#include "src/Helpers/Misc.h"
#include "src/Helpers/StringParser.h"


PluginTaskData_base *Plugin_task_data[TASKS_MAX] = { nullptr, };

String PCONFIG_LABEL(int n) {
  if (n < PLUGIN_CONFIGVAR_MAX) {
    String result = F("pconf_");
    result += n;
    return result;
  }
  return F("error");
}

void resetPluginTaskData() {
  for (taskIndex_t i = 0; i < TASKS_MAX; ++i) {
    Plugin_task_data[i] = nullptr;
  }
}

void clearPluginTaskData(taskIndex_t taskIndex) {
  if (validTaskIndex(taskIndex)) {
    if (Plugin_task_data[taskIndex] != nullptr) {
      delete Plugin_task_data[taskIndex];
      Plugin_task_data[taskIndex] = nullptr;
    }
  }
}

void initPluginTaskData(taskIndex_t taskIndex, PluginTaskData_base *data) {
  if (!validTaskIndex(taskIndex)) { 
    if (data != nullptr) {
      delete data;
    }
    return; 
  }

  clearPluginTaskData(taskIndex);

  if (data == nullptr) {
    return;
  }

  if (Settings.TaskDeviceEnabled[taskIndex]) {
    Plugin_task_data[taskIndex]                     = data;
    Plugin_task_data[taskIndex]->_taskdata_pluginID = Settings.TaskDeviceNumber[taskIndex];
  }
}

PluginTaskData_base* getPluginTaskData(taskIndex_t taskIndex) {
  if (pluginTaskData_initialized(taskIndex)) {
    return Plugin_task_data[taskIndex];
  }
  return nullptr;
}

bool pluginTaskData_initialized(taskIndex_t taskIndex) {
  if (!validTaskIndex(taskIndex)) {
    return false;
  }
  return Plugin_task_data[taskIndex] != nullptr &&
         (Plugin_task_data[taskIndex]->_taskdata_pluginID == Settings.TaskDeviceNumber[taskIndex]);
}

String getPluginCustomArgName(int varNr) {
  String argName = F("pc_arg");

  argName += varNr + 1;
  return argName;
}

// Helper function to create formatted custom values for display in the devices overview page.
// When called from PLUGIN_WEBFORM_SHOW_VALUES, the last item should add a traling div_br class
// if the regular values should also be displayed.
// The call to PLUGIN_WEBFORM_SHOW_VALUES should only return success = true when no regular values should be displayed
// Note that the varNr of the custom values should not conflict with the existing variable numbers (e.g. start at VARS_PER_TASK)
String pluginWebformShowValue(taskIndex_t taskIndex, byte varNr, const String& label, const String& value, bool addTrailingBreak) {
  String result;
  size_t length   = 96 + label.length() + value.length();
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

bool pluginOptionalTaskIndexArgumentMatch(taskIndex_t taskIndex, const String& string, byte paramNr) {
  if (!validTaskIndex(taskIndex)) {
    return false;
  }
  const taskIndex_t found_taskIndex = parseCommandArgumentTaskIndex(string, paramNr);

  if (!validTaskIndex(found_taskIndex)) {
    // Optional parameter not present
    return true;
  }
  return found_taskIndex == taskIndex;
}

int getValueCountForTask(taskIndex_t   taskIndex) {
  struct EventStruct TempEvent(taskIndex);
  String dummy;
  PluginCall(PLUGIN_GET_DEVICEVALUECOUNT, &TempEvent, dummy);
  return TempEvent.Par1;
}

int checkDeviceVTypeForTask(struct EventStruct* event) {
  if (event->sensorType == Sensor_VType::SENSOR_TYPE_NOT_SET) {
    if (validTaskIndex(event->TaskIndex)) {
      String dummy;
      if (PluginCall(PLUGIN_GET_DEVICEVTYPE, event, dummy)) {
        return event->idx; // pconfig_index
      }
    }
  }
  return -1;
}