#include "Plugins.h"
#include "../../ESPEasy_plugindefs.h"
#include "../DataStructs/ESPEasy_EventStruct.h"
#include "../Globals/Device.h"
#include "../Globals/Settings.h"
#include "../../ESPEasy_Log.h"



deviceIndex_t INVALID_DEVICE_INDEX = PLUGIN_MAX;
taskIndex_t   INVALID_TASK_INDEX = TASKS_MAX;
pluginID_t    INVALID_PLUGIN_ID  = 0;

std::map<pluginID_t, deviceIndex_t> Plugin_id_to_DeviceIndex;
std::vector<pluginID_t> DeviceIndex_to_Plugin_id;

boolean (*Plugin_ptr[PLUGIN_MAX])(byte, struct EventStruct*, String&);


bool validDeviceIndex(deviceIndex_t index) {
  return index < PLUGIN_MAX;
}

bool validTaskIndex(taskIndex_t index) {
  return index < TASKS_MAX;
}

bool validPluginID(pluginID_t pluginID) {
  return pluginID != 0;
}

bool supportedPluginID(pluginID_t pluginID) {
  return validDeviceIndex(getDeviceIndex(pluginID));
}

deviceIndex_t getDeviceIndex_from_TaskIndex(taskIndex_t taskIndex) {
  if (validTaskIndex(taskIndex)) {
    return getDeviceIndex(Settings.TaskDeviceNumber[taskIndex]);
  }
  return INVALID_DEVICE_INDEX;
}


deviceIndex_t getDeviceIndex(pluginID_t pluginID)
{
  if (validPluginID(pluginID)) {
    auto it = Plugin_id_to_DeviceIndex.find(pluginID);
    if (it != Plugin_id_to_DeviceIndex.end())
    {
      if (Device[it->second].Number != pluginID) {
        // FIXME TD-er: Just a check for now, can be removed later when it does not occur.
        addLog(LOG_LEVEL_ERROR, F("getDeviceIndex error in Device Vector"));
      }
      return it->second;
    }
  }
  return INVALID_DEVICE_INDEX;
}

/********************************************************************************************\
   Find name of plugin given the plugin device index..
 \*********************************************************************************************/
String getPluginNameFromDeviceIndex(deviceIndex_t deviceIndex) {
  String deviceName = "";
  if (validDeviceIndex(deviceIndex)) {
    Plugin_ptr[deviceIndex](PLUGIN_GET_DEVICENAME, 0, deviceName);
  }  
  return deviceName;
}
