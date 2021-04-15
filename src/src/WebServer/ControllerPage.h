#ifndef WEBSERVER_WEBSERVER_CONTROLLERPAGE_H
#define WEBSERVER_WEBSERVER_CONTROLLERPAGE_H

#include "../WebServer/common.h"

#include "../DataStructs/ControllerSettingsStruct.h"



#ifdef WEBSERVER_CONTROLLERS

#include "../Globals/CPlugins.h"

// ********************************************************************************
// Web Interface controller page
// ********************************************************************************
void handle_controllers();

// ********************************************************************************
// Selected controller has changed.
// Clear all Controller settings and load some defaults
// ********************************************************************************
void handle_controllers_clearLoadDefaults(byte controllerindex, ControllerSettingsStruct& ControllerSettings);

// ********************************************************************************
// Collect all submitted form data and store in the ControllerSettings
// ********************************************************************************
void handle_controllers_CopySubmittedSettings(byte controllerindex, ControllerSettingsStruct& ControllerSettings);

void handle_controllers_CopySubmittedSettings_CPluginCall(byte controllerindex);

// ********************************************************************************
// Show table with all selected controllers
// ********************************************************************************
void handle_controllers_ShowAllControllersTable();

// ********************************************************************************
// Show the controller settings page
// ********************************************************************************
void handle_controllers_ControllerSettingsPage(controllerIndex_t controllerindex);

#endif // ifdef WEBSERVER_CONTROLLERS




#endif