#ifndef GLOBALS_CPLUGIN_H
#define GLOBALS_CPLUGIN_H

#include "../../ESPEasy_common.h"

#include "../CustomBuild/ESPEasyLimits.h"
#include "../DataStructs/ControllerSettingsStruct.h"
#include "../DataTypes/ESPEasy_plugin_functions.h"
#include "../DataTypes/CPluginID.h"
#include "../DataTypes/ControllerIndex.h"
#include "../DataTypes/ProtocolIndex.h"


/********************************************************************************************\
   Structures to address the Cplugins (controllers) and their configurations.

   A build of ESPeasy may not have all Cplugins included.
   So there has to be some administration to keep track of what Cplugin is present
   and how to address it.

   We have:
   - CPlugin, like _C001.ino.
   - Controller -> A selected instance of a CPlugin (analog to "task" for plugins, shown in the Controllers tab in the web interface)
   - Protocol   -> A CPlugin included in the build.

   We have the following one-to-one relations:
   - CPlugin_id_to_ProtocolIndex - Map from CPlugin ID to Protocol Index.
   - ProtocolIndex_to_CPlugin_id - Vector from ProtocolIndex to CPlugin ID.
   - CPlugin_ptr - Array of function pointers to call Cplugins.
   - Protocol    - Vector of ProtocolStruct containing Cplugin specific information.
 \*********************************************************************************************/


bool CPluginCall(CPlugin::Function   Function,
                 struct EventStruct *event);
bool CPluginCall(CPlugin::Function   Function,
                 struct EventStruct *event,
                 String            & str);
bool CPluginCall(protocolIndex_t     protocolIndex,
                 CPlugin::Function   Function,
                 struct EventStruct *event,
                 String            & str);

bool              anyControllerEnabled();
controllerIndex_t findFirstEnabledControllerWithId(cpluginID_t cpluginid);

bool              validProtocolIndex(protocolIndex_t index);


// bool              validControllerIndex(controllerIndex_t index);
#define validControllerIndex(C_X)  ((C_X) < CONTROLLER_MAX)

// Check whether CPlugin is included in build.
bool              validCPluginID(cpluginID_t cpluginID);

// Check if cplugin is included in build.
// N.B. Invalid cplugin is also not considered supported.
// This is essentially (validCPluginID && validProtocolIndex)
bool            supportedCPluginID(cpluginID_t cpluginID);
protocolIndex_t getProtocolIndex_from_ControllerIndex(controllerIndex_t index);
protocolIndex_t getProtocolIndex_from_CPluginID(cpluginID_t cpluginID);
cpluginID_t     getCPluginID_from_ProtocolIndex(protocolIndex_t index);
cpluginID_t     getCPluginID_from_ControllerIndex(controllerIndex_t index);

String          getCPluginNameFromProtocolIndex(protocolIndex_t ProtocolIndex);
String          getCPluginNameFromCPluginID(cpluginID_t cpluginID);


#endif // GLOBALS_CPLUGIN_H
