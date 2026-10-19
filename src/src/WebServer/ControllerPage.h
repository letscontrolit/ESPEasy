#ifndef WEBSERVER_WEBSERVER_CONTROLLERPAGE_H
#define WEBSERVER_WEBSERVER_CONTROLLERPAGE_H

#include "../WebServer/common.h"

#ifdef WEBSERVER_CONTROLLERS

#include "../DataStructs/ControllerSettingsStruct.h"

#include "../Globals/CPlugins.h"

// ********************************************************************************
// Web Interface controller page
// ********************************************************************************
void handle_controllers();


#endif // ifdef WEBSERVER_CONTROLLERS
#endif