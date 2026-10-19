#pragma once

#include "../WebServer/common.h"

#ifdef WEBSERVER_NETWORK

#include "../../ESPEasy/net/DataTypes/NetworkIndex.h"
#include "../../ESPEasy/net/DataStructs/NetworkSettingsStruct.h"

// ********************************************************************************
// Web Interface Network page
// ********************************************************************************
void handle_networks();

#endif // ifdef WEBSERVER_NETWORK
