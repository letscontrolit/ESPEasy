#ifndef PLUGIN_HELPER_H
#define PLUGIN_HELPER_H

// Defines to make plugins more readable.

#ifndef PCONFIG
  #define PCONFIG(n) (Settings.TaskDevicePluginConfig[event->TaskIndex][(n)])
#endif
#ifndef PCONFIG_FLOAT
  #define PCONFIG_FLOAT(n) (Settings.TaskDevicePluginConfigFloat[event->TaskIndex][(n)])
#endif
#ifndef PCONFIG_LONG
  #define PCONFIG_LONG(n) (Settings.TaskDevicePluginConfigLong[event->TaskIndex][(n)])
#endif
#ifndef PIN
  // Please note the 'offset' of N compared to normal pin numbering.
  #define PIN(n) (Settings.TaskDevicePin[n][event->TaskIndex])
#endif
#ifndef CONFIG_PIN1
  #define CONFIG_PIN1 (Settings.TaskDevicePin1[event->TaskIndex])
#endif
#ifndef CONFIG_PIN2
  #define CONFIG_PIN2 (Settings.TaskDevicePin2[event->TaskIndex])
#endif
#ifndef CONFIG_PIN3
  #define CONFIG_PIN3 (Settings.TaskDevicePin3[event->TaskIndex])
#endif
#ifndef CONFIG_PORT
  #define CONFIG_PORT (Settings.TaskDevicePort[event->TaskIndex])
#endif




//==============================================
// Data used by instances of plugins.
// =============================================

// base class to be able to delete a data object from the array.
// N.B. in order to use this, a data object must inherit from this base class.
//      This is a compile time check.
struct PluginTaskData_base {
  virtual ~PluginTaskData_base() {}

  // We cannot use dynamic_cast, so we must keep track of the plugin ID to
  // perform checks on the casting.
  // This is also a check to only use these functions and not to insert pointers
  // at random in the Plugin_task_data array.
  int _taskdata_plugin_id = -1;
};

PluginTaskData_base* Plugin_task_data[TASKS_MAX] = { NULL, };

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

void initPluginTaskData(byte taskIndex, PluginTaskData_base* data) {
  clearPluginTaskData(taskIndex);
  if (taskIndex < TASKS_MAX && Settings.TaskDeviceEnabled[taskIndex]) {
    Plugin_task_data[taskIndex] = data;
    Plugin_task_data[taskIndex]->_taskdata_plugin_id = Task_id_to_Plugin_id[taskIndex];
  }
}


PluginTaskData_base* getPluginTaskData(byte taskIndex) {
  if (taskIndex >= TASKS_MAX) {
    return nullptr;
  }
  if (Plugin_task_data[taskIndex] != nullptr && Plugin_task_data[taskIndex]->_taskdata_plugin_id == Task_id_to_Plugin_id[taskIndex]) {
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

#endif // PLUGIN_HELPER_H
