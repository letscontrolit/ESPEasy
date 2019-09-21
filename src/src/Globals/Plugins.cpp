#include "Plugins.h"
#include "src/DataStructs/ESPEasy_EventStruct.h"
#include "ESPEasy_plugindefs.h"

int getPluginId_from_TaskIndex(byte taskIndex) {
  if (taskIndex >= Task_id_to_Plugin_id.size())
  {
    return 0;
  }
  return Task_id_to_Plugin_id[taskIndex]; 
}

/*
// TODO TD-er: Move from _Plugin.ino as soon as Settings is global defined.
int getPluginId(byte taskId) {
  if (taskId < TASKS_MAX) {
    int retry = 1;
    while (retry >= 0) {
      int plugin = Task_id_to_Plugin_id[taskId];
      if (plugin >= 0 && plugin < PLUGIN_MAX) {
        if (Plugin_id[plugin] == Settings.TaskDeviceNumber[taskId])
          return plugin;
      }
      updateTaskPluginCache();
      --retry;
    }
  }
  return -1;
}

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
*/


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