#include "Plugins.h"
#include "../../ESPEasy_plugindefs.h"
#include "../DataStructs/ESPEasy_EventStruct.h"
#include "../Globals/Settings.h"


void updateTaskPluginCache() {
  ++countFindPluginId; // Used for statistics.
  Task_id_to_Plugin_id.resize(TASKS_MAX);
  for (byte y = 0; y < TASKS_MAX; ++y) {
    Task_id_to_Plugin_id[y] = -1;
    bool foundPlugin = false;
    for (byte x = 0; x < PLUGIN_MAX && !foundPlugin; ++x) {
      if (Plugin_id[x] != 0 && Plugin_id[x] == Settings.TaskDeviceNumber[y]) {
        foundPlugin = true;
        Task_id_to_Plugin_id[y] = x;
      }
    }
  }
}

int getPluginId_from_TaskIndex(byte taskIndex) {
  if (taskIndex < TASKS_MAX) {
    int retry = 1;
    while (retry >= 0) {
      int plugin = Task_id_to_Plugin_id[taskIndex];
      if (plugin >= 0 && plugin < PLUGIN_MAX) {
        if (Plugin_id[plugin] == Settings.TaskDeviceNumber[taskIndex])
          return plugin;
      }
      updateTaskPluginCache();
      --retry;
    }
  }
  return -1;
}



/********************************************************************************************\
   Find name of plugin given the plugin device index..
 \*********************************************************************************************/
String getPluginNameFromDeviceIndex(byte deviceIndex) {
  String deviceName = "";
  if (deviceIndex >= PLUGIN_MAX) {
    return deviceName;
  }

  Plugin_ptr[deviceIndex](PLUGIN_GET_DEVICENAME, 0, deviceName);
  return deviceName;
}


boolean (*Plugin_ptr[PLUGIN_MAX])(byte, struct EventStruct*, String&);

std::vector<byte> Plugin_id;
std::vector<int> Task_id_to_Plugin_id;
unsigned long countFindPluginId = 0;