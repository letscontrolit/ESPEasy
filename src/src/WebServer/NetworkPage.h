#ifndef WEBSERVER_WEBSERVER_NETWORKPAGE_H
#define WEBSERVER_WEBSERVER_NETWORKPAGE_H

#include "../WebServer/common.h"

#ifdef WEBSERVER_NETWORK

#include "../Helpers/KeyValueWriter.h"

#include "../../ESPEasy/net/DataTypes/NetworkIndex.h"
#include "../../ESPEasy/net/DataStructs/NetworkSettingsStruct.h"


// ********************************************************************************
// Web Interface Network page
// ********************************************************************************
void handle_networks();

// ********************************************************************************
// Selected network has changed.
// Clear all Network settings and load some defaults
// ********************************************************************************
void handle_networks_clearLoadDefaults(ESPEasy::net::networkIndex_t networkindex, ESPEasy::net::NetworkSettingsStruct& NetworkSettings);

// ********************************************************************************
// Collect all submitted form data and store in the NetworkSettings
// ********************************************************************************
void handle_networks_CopySubmittedSettings(ESPEasy::net::networkIndex_t networkindex, ESPEasy::net::NetworkSettingsStruct& NetworkSettings);

void handle_networks_CopySubmittedSettings_NWPluginCall(ESPEasy::net::networkIndex_t networkindex);

// ********************************************************************************
// Show table with all selected networks
// ********************************************************************************
void handle_networks_ShowAllNetworksTable();

// ********************************************************************************
// Show the network settings page
// ********************************************************************************
void handle_networks_NetworkSettingsPage(ESPEasy::net::networkIndex_t networkindex);


#endif // ifdef WEBSERVER_NETWORK
#endif // ifndef WEBSERVER_WEBSERVER_NETWORKPAGE_H
