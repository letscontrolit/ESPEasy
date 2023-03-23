#ifndef GLOBALS_PLUGIN_H
#define GLOBALS_PLUGIN_H

#include "../../ESPEasy_common.h"

#include <map>
#include <vector>
#include "../CustomBuild/ESPEasyLimits.h"

#include "../DataTypes/PluginID.h"
#include "../DataTypes/DeviceIndex.h"
#include "../DataTypes/TaskIndex.h"



/********************************************************************************************\
   Structures to address the plugins and device configurations.

   A build of ESPeasy may not have all plugins included.
   So there has to be some administration to keep track of what plugin is present
   and how to address this plugin.
   The data structures containing the available plugins are addressed via a DeviceIndex.

   We have:
   - Plugin, like _P001_Switch.ino.
   - Task   -> A selected instance of a Plugin (Tasks are shown in the web interface)
   - Device -> A Plugin included in the build.
   

   We have the following one-to-one relations:
   - Plugin_id_to_DeviceIndex  - Map from Plugin ID to Device Index.
   - DeviceIndex_to_Plugin_id  - Vector from DeviceIndex to Plugin ID.
   - Plugin_ptr                - Array of function pointers to call plugins.
   - Device                    - Vector of DeviceStruct containing plugin specific information.


   UserVar has the output values for a task.
   - BaseVarIndex = taskIndex * VARS_PER_TASK
   - taskVarIndex = 0 ... (VARS_PER_TASK - 1)
   - userVarIndex = BaseVarIndex + taskVarIndex  => 0 ... USERVAR_MAX_INDEX
   - USERVAR_MAX_INDEX = (TASKS_MAX * VARS_PER_TASK)
 \*********************************************************************************************/

struct EventStruct;

extern int deviceCount;

// Array of function pointers to call plugins.
extern boolean (*Plugin_ptr[PLUGIN_MAX])(uint8_t,
                                         struct EventStruct *,
                                         String&);

// Vector to match a "DeviceIndex" to a plugin ID.
// INVALID_DEVICE_INDEX may be used as index for this array, thus one larger
extern pluginID_t DeviceIndex_to_Plugin_id[PLUGIN_MAX + 1];

// Map to match a plugin ID to a "DeviceIndex"
extern std::map<pluginID_t, deviceIndex_t> Plugin_id_to_DeviceIndex;

// Vector containing "DeviceIndex" alfabetically sorted.
extern std::vector<deviceIndex_t> DeviceIndex_sorted;


bool validDeviceIndex(deviceIndex_t index);
bool validTaskIndex(taskIndex_t index);
bool validPluginID(pluginID_t pluginID);
bool validPluginID_fullcheck(pluginID_t pluginID);
bool validUserVarIndex(userVarIndex_t index);
bool validTaskVarIndex(taskVarIndex_t index);

// Check if plugin is included in build.
// N.B. Invalid plugin is also not considered supported.
// This is essentially (validPluginID && validDeviceIndex)
bool          supportedPluginID(pluginID_t pluginID);

deviceIndex_t getDeviceIndex_from_TaskIndex(taskIndex_t taskIndex);
/*********************************************************************************************
 * get the taskPluginID with required checks, INVALID_PLUGIN_ID when invalid
 ********************************************************************************************/
pluginID_t getPluginID_from_TaskIndex(taskIndex_t taskIndex);

#if FEATURE_PLUGIN_PRIORITY
bool       isPluginI2CPowerManager_from_TaskIndex(taskIndex_t taskIndex);
#endif // if FEATURE_PLUGIN_PRIORITY

/********************************************************************************************\
   Find Device Index given a plugin ID
 \*********************************************************************************************/
deviceIndex_t getDeviceIndex(pluginID_t Number);

String        getPluginNameFromDeviceIndex(deviceIndex_t deviceIndex);
#if FEATURE_I2C_DEVICE_SCAN
bool          checkPluginI2CAddressFromDeviceIndex(deviceIndex_t deviceIndex, uint8_t i2cAddress);
#endif // if FEATURE_I2C_DEVICE_SCAN
#if FEATURE_I2C_GET_ADDRESS
uint8_t getTaskI2CAddress(taskIndex_t taskIndex);
#endif // if FEATURE_I2C_GET_ADDRESS

String        getPluginNameFromPluginID(pluginID_t pluginID);

void          sortDeviceIndexArray();


// Prepare I2C bus for next call to task
// Return false if task is I2C, but I2C bus is not ready
bool prepare_I2C_by_taskIndex(taskIndex_t taskIndex, deviceIndex_t DeviceIndex);
void post_I2C_by_taskIndex(taskIndex_t taskIndex, deviceIndex_t DeviceIndex);

/*********************************************************************************************\
* Function call to all or specific plugins
\*********************************************************************************************/
bool PluginCall(uint8_t Function, struct EventStruct *event, String& str);


/*********************************************************************************************\
* Adding plugins at boot
\*********************************************************************************************/
bool addPlugin(pluginID_t pluginID, deviceIndex_t x);


#endif // GLOBALS_PLUGIN_H