#include "../../ESPEasy_common.h"

#ifdef USES_NW003

// #######################################################################################################
// ########################### Network Plugin 003: Ethernet RMII #########################################
// #######################################################################################################

# define NWPLUGIN_003
# define NWPLUGIN_ID_003         3
# define NWPLUGIN_NAME_003       "Ethernet (RMII)"

# include "../../src/DataStructs/ESPEasy_EventStruct.h"
# include "../../src/Globals/SecuritySettings.h"
# include "../../src/Globals/Settings.h"
# include "../../src/Helpers/ESPEasy_Storage.h"
# include "../../src/Helpers/PrintToString.h"
# include "../../src/Helpers/StringConverter.h"
# include "../../src/WebServer/ESPEasy_WebServer.h"
# include "../../src/WebServer/HTML_Print.h"
# include "../../src/WebServer/HTML_wrappers.h"
# include "../../src/WebServer/Markup.h"
# include "../../src/WebServer/Markup_Forms.h"
# include "../../src/WebServer/common.h"
# include "../net/eth/ESPEasyEth.h"
# include "../net/Globals/NWPlugins.h"
# include "../net/Globals/NetworkState.h"
# include "../net/Helpers/_NWPlugin_Helper_webform.h"
# include "../net/Helpers/_NWPlugin_init.h"
# include "../net/Helpers/NW_info_writer.h"

# include "../net/NWPluginStructs/NW003_data_struct_ETH_RMII.h"

# include <pins_arduino.h>

namespace ESPEasy {
namespace net {

bool NWPlugin_003(NWPlugin::Function function, EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {
    case NWPlugin::Function::NWPLUGIN_DRIVER_ADD:
    {
      NetworkDriverStruct& nw = getNetworkDriverStruct(networkDriverIndex_t::toNetworkDriverIndex(event->idx));
      nw.onlySingleInstance = true;
# ifdef ESP32P4
      nw.alwaysPresent         = true;
      nw.enabledOnFactoryReset = true;
      nw.fixedNetworkIndex     = NWPLUGIN_ID_003 - 1; // Start counting at 0
# else // ifdef ESP32P4
      nw.alwaysPresent = false;
# endif // ifdef ESP32P4
      break;
    }

    case NWPlugin::Function::NWPLUGIN_LOAD_DEFAULTS:
    {
      Settings.setRoutePrio_for_network(event->NetworkIndex, 50);
      Settings.setNetworkInterfaceSubnetBlockClientIP(event->NetworkIndex, false);
      Settings.setNetworkInterfaceStartupDelayAtBoot(event->NetworkIndex, 500 * event->NetworkIndex);

      ESPEasy_key_value_store kvs;
      ESPEasy::net::eth::NW003_data_struct_ETH_RMII::loadDefaults(
        &kvs,
        event->NetworkIndex,
        ESPEasy::net::nwpluginID_t(NWPLUGIN_ID_003));

      break;
    }

    case NWPlugin::Function::NWPLUGIN_GET_DEVICENAME:
    {
      string = F(NWPLUGIN_NAME_003);
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
      ESPEasy::net::eth::NW003_data_struct_ETH_RMII *NW_data =
        static_cast<ESPEasy::net::eth::NW003_data_struct_ETH_RMII *>(getNWPluginData(event->NetworkIndex));

      if (NW_data) {
        success = NW_data->write_Eth_HW_Address(event->kvWriter);
      }
      break;
    }
# ifndef LIMIT_BUILD_SIZE
    case NWPlugin::Function::NWPLUGIN_WEBFORM_SHOW_PORT:
    {
      ESPEasy::net::eth::NW003_data_struct_ETH_RMII *NW_data =
        static_cast<ESPEasy::net::eth::NW003_data_struct_ETH_RMII *>(getNWPluginData(event->NetworkIndex));

      if (NW_data) {
        success = NW_data->write_Eth_port(event->kvWriter);
      }
      break;
    }
# endif // ifndef LIMIT_BUILD_SIZE

    case NWPlugin::Function::NWPLUGIN_WEBFORM_SAVE:
    {
      ESPEasy::net::eth::NW003_data_struct_ETH_RMII *NW_data =
        static_cast<ESPEasy::net::eth::NW003_data_struct_ETH_RMII *>(getNWPluginData(event->NetworkIndex));
      bool mustCleanup = NW_data == nullptr;

      if (mustCleanup) {
        NW_data = new (std::nothrow) ESPEasy::net::eth::NW003_data_struct_ETH_RMII(event->NetworkIndex, nullptr);
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
      ESPEasy::net::eth::NW003_data_struct_ETH_RMII *NW_data =
        static_cast<ESPEasy::net::eth::NW003_data_struct_ETH_RMII *>(getNWPluginData(event->NetworkIndex));
      bool mustCleanup = NW_data == nullptr;

      if (mustCleanup) {
        NW_data = new (std::nothrow) ESPEasy::net::eth::NW003_data_struct_ETH_RMII(event->NetworkIndex, nullptr);
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
        initNWPluginData(event->NetworkIndex, new (std::nothrow) ESPEasy::net::eth::NW003_data_struct_ETH_RMII(event->NetworkIndex, iface));
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

    case NWPlugin::Function::NWPLUGIN_TEN_PER_SECOND:
    // FIXME TD-er: Must make this act on DNS updates from other interfaces
    // Fall through
    case NWPlugin::Function::NWPLUGIN_PRIORITY_ROUTE_CHANGED:
    {
      ESPEasy::net::eth::NW003_data_struct_ETH_RMII *NW_data =
        static_cast<ESPEasy::net::eth::NW003_data_struct_ETH_RMII *>(getNWPluginData(event->NetworkIndex));

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


#endif // ifdef USES_NW003
