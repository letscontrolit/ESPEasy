#ifndef GLOBALS_PLUGIN_H
#define GLOBALS_PLUGIN_H

#include <map>
#include <vector>
#include "../DataStructs/ESPEasyLimits.h"


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


typedef byte     deviceIndex_t;
typedef byte     taskIndex_t;
typedef uint8_t  pluginID_t;
typedef uint16_t userVarIndex_t;
typedef uint16_t taskVarIndex_t;

extern deviceIndex_t  INVALID_DEVICE_INDEX;
extern taskIndex_t    INVALID_TASK_INDEX;
extern pluginID_t     INVALID_PLUGIN_ID;
extern userVarIndex_t INVALID_USERVAR_INDEX;
extern taskVarIndex_t INVALID_TASKVAR_INDEX;


/*********************************************************************************************\
* Custom Variables for usage in rules and http.
* Syntax: %vX%
* usage:
* let,1,10
* if %v1%=10 do ...
\*********************************************************************************************/
extern float customFloatVar[CUSTOM_VARS_MAX];
extern float UserVar[VARS_PER_TASK * TASKS_MAX];

extern int deviceCount;

// Array of function pointers to call plugins.
extern boolean (*Plugin_ptr[PLUGIN_MAX])(byte,
                                         struct EventStruct *,
                                         String&);


// Map to match a plugin ID to a "DeviceIndex"
extern std::map<pluginID_t, deviceIndex_t> Plugin_id_to_DeviceIndex;

// Vector to match a "DeviceIndex" to a plugin ID.
extern std::vector<pluginID_t> DeviceIndex_to_Plugin_id;

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


/********************************************************************************************\
   Find Device Index given a plugin ID
 \*********************************************************************************************/
deviceIndex_t getDeviceIndex(pluginID_t Number);

String        getPluginNameFromDeviceIndex(deviceIndex_t deviceIndex);
String        getPluginNameFromPluginID(pluginID_t pluginID);

void          sortDeviceIndexArray();


void prepare_I2C_by_taskIndex(taskIndex_t taskIndex, deviceIndex_t DeviceIndex);
void post_I2C_by_taskIndex(taskIndex_t taskIndex, deviceIndex_t DeviceIndex);

/*********************************************************************************************\
* Function call to all or specific plugins
\*********************************************************************************************/
byte PluginCall(byte Function, struct EventStruct *event, String& str);


#endif // GLOBALS_PLUGIN_H
