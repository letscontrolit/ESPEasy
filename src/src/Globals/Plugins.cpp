#include "Plugins.h"
#include "../../ESPEasy_plugindefs.h"
#include "../DataStructs/ESPEasy_EventStruct.h"
#include "../Globals/Device.h"
#include "../Globals/Settings.h"
#include "../../ESPEasy_Log.h"

#define USERVAR_MAX_INDEX    (VARS_PER_TASK * TASKS_MAX)


deviceIndex_t  INVALID_DEVICE_INDEX  = PLUGIN_MAX;
taskIndex_t    INVALID_TASK_INDEX    = TASKS_MAX;
pluginID_t     INVALID_PLUGIN_ID     = 0;
userVarIndex_t INVALID_USERVAR_INDEX = USERVAR_MAX_INDEX;
taskVarIndex_t INVALID_TASKVAR_INDEX = VARS_PER_TASK;

std::map<pluginID_t, deviceIndex_t> Plugin_id_to_DeviceIndex;
std::vector<pluginID_t>    DeviceIndex_to_Plugin_id;
std::vector<deviceIndex_t> DeviceIndex_sorted;

float customFloatVar[CUSTOM_VARS_MAX];

float UserVar[VARS_PER_TASK * TASKS_MAX];

int deviceCount = -1;

boolean (*Plugin_ptr[PLUGIN_MAX])(byte,
                                  struct EventStruct *,
                                  String&);

void clearUserVar() {
  for (size_t i = 0; i < sizeof(UserVar) / sizeof(float); ++i) {
    UserVar[i] = 0.0f;
  }
}



bool validDeviceIndex(deviceIndex_t index) {
  if (index < PLUGIN_MAX) {
    const pluginID_t pluginID = DeviceIndex_to_Plugin_id[index];
    return pluginID != INVALID_PLUGIN_ID;
  }
  return false;
}

bool validTaskIndex(taskIndex_t index) {
  return index < TASKS_MAX;
}

bool validPluginID(pluginID_t pluginID) {
  return (pluginID != INVALID_PLUGIN_ID);
}

bool validPluginID_fullcheck(pluginID_t pluginID) {
  if (!validPluginID(pluginID)) {
    return false;
  }
  auto it = Plugin_id_to_DeviceIndex.find(pluginID);
  return (it != Plugin_id_to_DeviceIndex.end());
}

bool validUserVarIndex(userVarIndex_t index) {
  return index < USERVAR_MAX_INDEX;
}

bool validTaskVarIndex(taskVarIndex_t index) {
  return index < VARS_PER_TASK;
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
  if (pluginID != INVALID_PLUGIN_ID) {
    auto it = Plugin_id_to_DeviceIndex.find(pluginID);

    if (it != Plugin_id_to_DeviceIndex.end())
    {
      if (!validDeviceIndex(it->second)) { return INVALID_DEVICE_INDEX; }

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
    Plugin_ptr[deviceIndex](PLUGIN_GET_DEVICENAME, nullptr, deviceName);
  }
  return deviceName;
}

String getPluginNameFromPluginID(pluginID_t pluginID) {
  deviceIndex_t deviceIndex = getDeviceIndex(pluginID);

  if (!validDeviceIndex(deviceIndex)) {
    String name = F("Plugin ");
    name += String(static_cast<int>(pluginID));
    name += F(" not included in build");
    return name;
  }
  return getPluginNameFromDeviceIndex(deviceIndex);
}

// ********************************************************************************
// Device Sort routine, compare two array entries
// ********************************************************************************
bool arrayLessThan(const String& ptr_1, const String& ptr_2)
{
  unsigned int i = 0;

  while (i < ptr_1.length()) // For each character in string 1, starting with the first:
  {
    if (ptr_2.length() < i)  // If string 2 is shorter, then switch them
    {
      return true;
    }
    const char check1 = static_cast<char>(ptr_1[i]); // get the same char from string 1 and string 2
    const char check2 = static_cast<char>(ptr_2[i]);

    if (check1 == check2) {
      // they're equal so far; check the next char !!
      i++;
    } else {
      return check2 > check1;
    }
  }
  return false;
}

// ********************************************************************************
// Device Sort routine, actual sorting alfabetically by plugin name.
// Sorting does happen case sensitive.
// ********************************************************************************
void sortDeviceIndexArray() {
  // First fill the existing number of the DeviceIndex.
  DeviceIndex_sorted.resize(deviceCount + 1);

  for (deviceIndex_t x = 0; x <= deviceCount; x++) {
    if (validPluginID(DeviceIndex_to_Plugin_id[x])) {
      DeviceIndex_sorted[x] = x;
    } else {
      DeviceIndex_sorted[x] = INVALID_DEVICE_INDEX;
    }
  }

  // Do the sorting.
  int innerLoop;
  int mainLoop;

  for (mainLoop = 1; mainLoop <= deviceCount; mainLoop++)
  {
    innerLoop = mainLoop;

    while (innerLoop  >= 1)
    {
      if (arrayLessThan(
            getPluginNameFromDeviceIndex(DeviceIndex_sorted[innerLoop]),
            getPluginNameFromDeviceIndex(DeviceIndex_sorted[innerLoop - 1])))
      {
        deviceIndex_t temp = DeviceIndex_sorted[innerLoop - 1];
        DeviceIndex_sorted[innerLoop - 1] = DeviceIndex_sorted[innerLoop];
        DeviceIndex_sorted[innerLoop]     = temp;
      }
      innerLoop--;
    }
  }
}
