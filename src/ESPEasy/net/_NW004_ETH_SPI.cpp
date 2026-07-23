#include "../../ESPEasy_common.h"

#ifdef USES_NW004

// #######################################################################################################
// ########################### Network Plugin 004: Ethernet SPI ##########################################
// #######################################################################################################

# define NWPLUGIN_004
# define NWPLUGIN_ID_004         4
# define NWPLUGIN_NAME_004       "Ethernet (SPI)"

# include "../../src/DataStructs/ESPEasy_EventStruct.h"
# include "../../src/Globals/Settings.h"
# include "../net/Helpers/_NWPlugin_init.h"
# include "../net/Helpers/NW_info_writer.h"

# include "../net/NWPluginStructs/NW004_data_struct_ETH_SPI.h"


# include <pins_arduino.h>

namespace ESPEasy {
namespace net {

bool NWPlugin_004(NWPlugin::Function function, EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {
    case NWPlugin::Function::NWPLUGIN_DRIVER_ADD:
    {
      NetworkDriverStruct& nw = getNetworkDriverStruct(networkDriverIndex_t::toNetworkDriverIndex(event->idx));
      nw.onlySingleInstance = true;
      nw.alwaysPresent      = false;
      # if DEFAULT_ENABLED_NW004
      nw.enabledOnFactoryReset = true;
      # endif
      break;
    }

    case NWPlugin::Function::NWPLUGIN_LOAD_DEFAULTS:
    {
      Settings.setAppendNetworkAdapterNameToHostname(event->NetworkIndex, DEFAULT_ETH_APPEND_NW_NAME_TO_HOSTNAME);
      Settings.setRoutePrio_for_network(
        event->NetworkIndex, 
        DEFAULT_ETH_ROUTE_PRIO - (5 * event->NetworkIndex));
      Settings.setNetworkInterfaceSubnetBlockClientIP(event->NetworkIndex, false);
      Settings.setNetworkInterfaceStartupDelay(event->NetworkIndex, 500 * event->NetworkIndex);

      ESPEasy_key_value_store kvs;
      ESPEasy::net::eth::NW004_data_struct_ETH_SPI::loadDefaults(
        &kvs,
        event->NetworkIndex,
        ESPEasy::net::nwpluginID_t(NWPLUGIN_ID_004));

      break;
    }

    case NWPlugin::Function::NWPLUGIN_GET_DEVICENAME:
    {
      string = F(NWPLUGIN_NAME_004);
      break;
    }

    # ifdef ESP32
    case NWPlugin::Function::NWPLUGIN_GET_INTERFACE:
    {
      event->networkInterface = ESPEasy::net::eth::ETH_NWPluginData_static_runtime::getInterface(event->NetworkIndex);
      success                 = event->networkInterface != nullptr;
      break;
    }
# endif // ifdef ESP32

    case NWPlugin::Function::NWPLUGIN_WEBSERVER_SHOULD_RUN:
    {
      auto iface = ESPEasy::net::eth::ETH_NWPluginData_static_runtime::getInterface(event->NetworkIndex);

      if (iface) {
        success = iface->connected();
      }
      break;
    }

    case NWPlugin::Function::NWPLUGIN_FALLBACK_INTERFACE_SHOULD_START:
    {
      success = true;
      break;
    }

    case NWPlugin::Function::NWPLUGIN_WEBFORM_SHOW_CONNECTED:
    {
      auto iface = ESPEasy::net::eth::ETH_NWPluginData_static_runtime::getInterface(event->NetworkIndex);

      if (iface) {
        success = write_Eth_Show_Connected(*iface, event->kvWriter);
      }
      break;
    }

    case NWPlugin::Function::NWPLUGIN_WEBFORM_SHOW_HW_ADDRESS:
    {
      ESPEasy::net::eth::NW004_data_struct_ETH_SPI *NW_data =
        static_cast<ESPEasy::net::eth::NW004_data_struct_ETH_SPI *>(getNWPluginData(event->NetworkIndex));

      if (NW_data) {
        success = NW_data->write_Eth_HW_Address(event->kvWriter);
      }
      break;
    }
    case NWPlugin::Function::NWPLUGIN_WEBFORM_SHOW_PORT:
    {
      ESPEasy::net::eth::NW004_data_struct_ETH_SPI *NW_data =
        static_cast<ESPEasy::net::eth::NW004_data_struct_ETH_SPI *>(getNWPluginData(event->NetworkIndex));

      if (NW_data) {
        success = NW_data->write_Eth_port(event->kvWriter);
      }
      break;
    }
    case NWPlugin::Function::NWPLUGIN_WEBFORM_SAVE:
    {
      ESPEasy::net::eth::NW004_data_struct_ETH_SPI *NW_data =
        static_cast<ESPEasy::net::eth::NW004_data_struct_ETH_SPI *>(getNWPluginData(event->NetworkIndex));
      bool mustCleanup = NW_data == nullptr;

      if (mustCleanup) {
        NW_data = new (std::nothrow) ESPEasy::net::eth::NW004_data_struct_ETH_SPI(event->NetworkIndex, nullptr);
# if FEATURE_STORE_NETWORK_INTERFACE_SETTINGS

        if (NW_data) {
          NW_data->init_KVS();
        }
# endif // if FEATURE_STORE_NETWORK_INTERFACE_SETTINGS
      }

      if (NW_data) {
        NW_data->webform_save(event);

        if (mustCleanup) { delete NW_data; }
        success = true;
      }
      break;
    }

    case NWPlugin::Function::NWPLUGIN_WEBFORM_LOAD:
    {
      ESPEasy::net::eth::NW004_data_struct_ETH_SPI *NW_data =
        static_cast<ESPEasy::net::eth::NW004_data_struct_ETH_SPI *>(getNWPluginData(event->NetworkIndex));
      bool mustCleanup = NW_data == nullptr;

      if (mustCleanup) {
        NW_data = new (std::nothrow) ESPEasy::net::eth::NW004_data_struct_ETH_SPI(event->NetworkIndex, nullptr);
# if FEATURE_STORE_NETWORK_INTERFACE_SETTINGS

        if (NW_data) {
          NW_data->init_KVS();
        }
# endif // if FEATURE_STORE_NETWORK_INTERFACE_SETTINGS
      }

      if (NW_data) {
        NW_data->webform_load(event);
        success = true;

        if (mustCleanup) { delete NW_data; }

      }
      break;
    }


    case NWPlugin::Function::NWPLUGIN_INIT:
    {
      auto iface = ESPEasy::net::eth::ETH_NWPluginData_static_runtime::init(event->NetworkIndex);

      if (iface) {
        initNWPluginData(event->NetworkIndex, new (std::nothrow) ESPEasy::net::eth::NW004_data_struct_ETH_SPI(event->NetworkIndex, iface));
        auto *NW_data = getNWPluginData(event->NetworkIndex);

        if (NW_data) {
          success = NW_data->init(event);
        }
      }
      break;
    }

    case NWPlugin::Function::NWPLUGIN_EXIT:
    {
      auto *NW_data = getNWPluginData(event->NetworkIndex);

      if (NW_data) {
        NW_data->exit(event);
      }
      success = true;
      break;
    }

    case NWPlugin::Function::NWPLUGIN_PRIORITY_ROUTE_CHANGED:
    {
      ESPEasy::net::eth::NW004_data_struct_ETH_SPI *NW_data =
        static_cast<ESPEasy::net::eth::NW004_data_struct_ETH_SPI *>(getNWPluginData(event->NetworkIndex));

      if (NW_data) {
        success = NW_data->handle_priority_route_changed();
      }
      break;
    }

    default:
      break;

  }

  return success;
}

} // namespace net
} // namespace ESPEasy

#endif // ifdef USES_NW004
