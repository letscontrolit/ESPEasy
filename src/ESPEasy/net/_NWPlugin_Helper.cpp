#include "../net/_NWPlugin_Helper.h"

#include "../../src/CustomBuild/ESPEasyLimits.h"

#include "../net/Globals/NWPlugins.h"
#include "../../src/Globals/Settings.h"

namespace ESPEasy {
namespace net {


NWPluginData_base *NWPlugin_task_data[NETWORK_MAX]{ 0 };

void resetNWPluginData() {
  for (ESPEasy::net::networkIndex_t i = 0; i < NR_ELEMENTS(NWPlugin_task_data); ++i) {
    NWPlugin_task_data[i] = nullptr;
  }
}

void clearNWPluginData(ESPEasy::net::networkIndex_t networkIndex) {
  if (validNetworkIndex(networkIndex)) {
    if (NWPlugin_task_data[networkIndex] != nullptr) {
      delete NWPlugin_task_data[networkIndex];
      NWPlugin_task_data[networkIndex] = nullptr;
    }
  }
}

bool initNWPluginData(ESPEasy::net::networkIndex_t networkIndex, NWPluginData_base *data) {
  if (!validNetworkIndex(networkIndex)) {
    if (data != nullptr) {
      delete data;
      data = nullptr;
    }
    return false;
  }

  // 2nd heap may have been active to allocate the NWPluginData, but here we need to keep the default heap active
  #ifdef USE_SECOND_HEAP
  HeapSelectDram ephemeral;
  #endif // ifdef USE_SECOND_HEAP


  clearNWPluginData(networkIndex);

  if (data != nullptr) {
    if (Settings.getNetworkEnabled(networkIndex)) {
#if FEATURE_STORE_NETWORK_INTERFACE_SETTINGS

      if (!data->init_KVS()) {
        delete data;
        data = nullptr;
      } else {
        NWPlugin_task_data[networkIndex] = data;
      }
#else // if FEATURE_STORE_NETWORK_INTERFACE_SETTINGS
      NWPlugin_task_data[networkIndex] = data;
#endif // if FEATURE_STORE_NETWORK_INTERFACE_SETTINGS
#if FEATURE_NETWORK_STATS

      NWPlugin_task_data[networkIndex]->initPluginStats();
#endif // if FEATURE_NETWORK_STATS

    } else {
      delete data;
      data = nullptr;
    }
  }
  return getNWPluginData(networkIndex) != nullptr;
}

NWPluginData_base* getNWPluginData(ESPEasy::net::networkIndex_t networkIndex) {
  if (nwpluginTaskData_initialized(networkIndex)) {
    return NWPlugin_task_data[networkIndex];
  }
  return nullptr;
}

bool nwpluginTaskData_initialized(ESPEasy::net::networkIndex_t networkIndex) {
  if (!validNetworkIndex(networkIndex)) {
    return false;
  }
  return NWPlugin_task_data[networkIndex] != nullptr &&
         (NWPlugin_task_data[networkIndex]->getNWPluginID() == Settings.getNWPluginID_for_network(networkIndex));
}

#if FEATURE_STORE_NETWORK_INTERFACE_SETTINGS

bool load_nwpluginTaskData_KVS(ESPEasy_key_value_store *kvs, ESPEasy::net::networkIndex_t networkIndex, ESPEasy::net::nwpluginID_t nwPluginID)
{
  if (!kvs) { return false; }
  return kvs->load(
    SettingsType::Enum::NetworkInterfaceSettings_Type,
    networkIndex,
    0,
    nwPluginID.value);
}

bool store_nwpluginTaskData_KVS(ESPEasy_key_value_store *kvs, ESPEasy::net::networkIndex_t networkIndex,
                                ESPEasy::net::nwpluginID_t nwPluginID)
{
  if (!kvs) { return false; }

# if defined(USES_NW003) || defined(USES_NW004)

  if (!kvs->isEmpty()) {
    // Check if NW003_KEY_ETH_INDEX or NW004_KEY_ETH_INDEX is set.
    // If not, then make sure there is no conflict with existing other network interfaces.
    int8_t value = -1;

    if (kvs->getValue(1, value)) {
      if (value == -1) {
        // Value has not been set. Check if there are other Ethernet adapters set.
        ++value;

        for (uint8_t i = 2; i < NETWORK_MAX; ++i) {
          if (i != networkIndex) {
            auto id = Settings.getNWPluginID_for_network(i);

            if ((id == ESPEasy::net::nwpluginID_t(3)) ||
                (id == ESPEasy::net::nwpluginID_t(4))) {
              ++value;
            }
          }
        }
        kvs->setValue(1, value);
      }
    }

  }
# endif // if defined(USES_NW003) || defined(USES_NW004)

  return kvs->store(
    SettingsType::Enum::NetworkInterfaceSettings_Type,
    networkIndex,
    0,
    nwPluginID.value);
}

#endif // if FEATURE_ESPEASY_KEY_VALUE_STORE

} // namespace net
} // namespace ESPEasy
