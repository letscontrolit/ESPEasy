#pragma once

#include "../../ESPEasy_common.h"

#include "../../src/CustomBuild/ESPEasyLimits.h"
#include "../../src/DataStructs/ESPEasy_EventStruct.h"
#include "../../src/Helpers/LongTermTimer.h"
#include "../../src/WebServer/KeyValueWriter_WebForm.h"
#include "../net/DataStructs/NWPluginData_base.h"
#include "../net/ESPEasyNetwork.h"

#if FEATURE_STORE_NETWORK_INTERFACE_SETTINGS
#include "../../src/Helpers/_ESPEasy_key_value_store.h"
#endif

namespace ESPEasy {
namespace net {


extern NWPluginData_base *NWPlugin_task_data[NETWORK_MAX];

// Try to allocate in PSRAM or 2nd heap if possible
#define special_initNWPluginData(I, T) void *ptr = special_calloc(1, sizeof(T)); \
        if (ptr) { initNWPluginData(I, new (ptr) T()); }


// ==============================================
// Data used by instances of NWPlugins.
// =============================================

void resetNWPluginData();

void clearNWPluginData(ESPEasy::net::networkIndex_t networkIndex);

bool initNWPluginData(ESPEasy::net::networkIndex_t     networkIndex,
                      NWPluginData_base * data);

NWPluginData_base* getNWPluginData(ESPEasy::net::networkIndex_t networkIndex);

bool nwpluginTaskData_initialized(ESPEasy::net::networkIndex_t networkIndex);
#if FEATURE_STORE_NETWORK_INTERFACE_SETTINGS
bool load_nwpluginTaskData_KVS(ESPEasy_key_value_store *kvs, ESPEasy::net::networkIndex_t networkIndex, ESPEasy::net::nwpluginID_t nwPluginID);
bool store_nwpluginTaskData_KVS(ESPEasy_key_value_store *kvs, ESPEasy::net::networkIndex_t networkIndex, ESPEasy::net::nwpluginID_t nwPluginID);
#endif
} // namespace net
} // namespace ESPEasy
