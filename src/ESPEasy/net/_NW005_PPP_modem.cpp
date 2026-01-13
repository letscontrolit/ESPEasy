#include "../../ESPEasy_common.h"

#ifdef USES_NW005

// #######################################################################################################
// ########################### Network Plugin 005: PPP modem #############################################
// #######################################################################################################

# define NWPLUGIN_005
# define NWPLUGIN_ID_005         5
# define NWPLUGIN_NAME_005       "PPP modem"

# include "../../src/DataStructs/ESPEasy_EventStruct.h"
# include "../net/ESPEasyNetwork.h"
# include "../../src/Globals/SecuritySettings.h"
# include "../../src/Globals/Settings.h"
# include "../../src/Helpers/ESPEasy_Storage.h"
# include "../../src/Helpers/PrintToString.h"
# include "../../src/Helpers/StringConverter.h"
# include "../../src/Helpers/_Plugin_Helper_serial.h"
# include "../../src/WebServer/ESPEasy_WebServer.h"
# include "../../src/WebServer/HTML_Print.h"
# include "../../src/WebServer/HTML_wrappers.h"
# include "../../src/WebServer/Markup.h"
# include "../../src/WebServer/Markup_Forms.h"
# include "../../src/WebServer/common.h"
# include "../net/Globals/NWPlugins.h"
# include "../net/Helpers/_NWPlugin_Helper_webform.h"
# include "../net/Helpers/_NWPlugin_init.h"
# include "../net/NWPluginStructs/NW005_data_struct_PPP_modem.h"


# include <ESPEasySerialPort.h>

# include <PPP.h>

namespace ESPEasy {
namespace net {

bool NWPlugin_005(NWPlugin::Function function, EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {
    case NWPlugin::Function::NWPLUGIN_DRIVER_ADD:
    {
      NetworkDriverStruct& nw = getNetworkDriverStruct(networkDriverIndex_t::toNetworkDriverIndex(event->idx));
      nw.onlySingleInstance = true;
      nw.alwaysPresent      = false;
      break;
    }

    case NWPlugin::Function::NWPLUGIN_LOAD_DEFAULTS:
    {
      Settings.setRoutePrio_for_network(event->NetworkIndex, 20);
      Settings.setNetworkInterfaceSubnetBlockClientIP(event->NetworkIndex, true);
      Settings.setNetworkInterfaceStartupDelayAtBoot(event->NetworkIndex, 500 * event->NetworkIndex);
      break;
    }

    case NWPlugin::Function::NWPLUGIN_GET_DEVICENAME:
    {
      string = F(NWPLUGIN_NAME_005);
      break;
    }

    case NWPlugin::Function::NWPLUGIN_GET_INTERFACE:
    {
      ESPEasy::net::ppp::NW005_data_struct_PPP_modem *NW_data =
        static_cast<ESPEasy::net::ppp::NW005_data_struct_PPP_modem *>(getNWPluginData(event->NetworkIndex));

      if (NW_data && NW_data->attached()) {
        event->networkInterface = &PPP;
        success                 = event->networkInterface != nullptr;
      }
      break;
    }

    case NWPlugin::Function::NWPLUGIN_WEBSERVER_SHOULD_RUN:
    {
      // Webserver should not be accessed from PPP modem
      success = false;
      break;
    }

    case NWPlugin::Function::NWPLUGIN_WEBFORM_SHOW_CONNECTED:
    {
      if (event->kvWriter) {
        ESPEasy::net::ppp::NW005_data_struct_PPP_modem *NW_data =
          static_cast<ESPEasy::net::ppp::NW005_data_struct_PPP_modem *>(getNWPluginData(event->NetworkIndex));

        if (NW_data) {
          success = NW_data->attached();

          if (success) {
            if (event->kvWriter->summaryValueOnly()) {
              event->kvWriter->write({
                    EMPTY_STRING,
                    strformat(
                      F("%s (%s dBm)"),
                      NW_data->operatorName().c_str(),
                      NW_data->getRSSI().c_str())
                  });
            } else {
              NW_data->webform_load_UE_system_information(event->kvWriter);
              event->kvWriter->write({ F("BER"), NW_data->getBER() });
            }
          }
        }
      }

      break;
    }

    case NWPlugin::Function::NWPLUGIN_WEBFORM_SHOW_HW_ADDRESS:
    {
      if (event->kvWriter) {
        ESPEasy::net::ppp::NW005_data_struct_PPP_modem *NW_data =
          static_cast<ESPEasy::net::ppp::NW005_data_struct_PPP_modem *>(getNWPluginData(event->NetworkIndex));

        if (NW_data) {
          const String imei(NW_data->IMEI());

          if (imei.length()) {
            if (event->kvWriter->summaryValueOnly()) {
              event->kvWriter->write({ EMPTY_STRING, concat(F("IMEI: "), imei) });
            } else {
              NW_data->write_ModemState(event->kvWriter);
            }
          }
        }
      }

      // Still mark success = true to prevent a call to get the MAC address
      success = true;
      break;
    }

# ifndef LIMIT_BUILD_SIZE
    case NWPlugin::Function::NWPLUGIN_WEBFORM_SHOW_PORT:
    {
      if (event->kvWriter) {
        ESPEasy::net::ppp::NW005_data_struct_PPP_modem *NW_data =
          static_cast<ESPEasy::net::ppp::NW005_data_struct_PPP_modem *>(getNWPluginData(event->NetworkIndex));

        if (NW_data) {
          success = NW_data->webform_getPort(event->kvWriter);
        }
      }
      break;
    }
# endif // ifndef LIMIT_BUILD_SIZE

    case NWPlugin::Function::NWPLUGIN_WEBFORM_SAVE:
    {
      ESPEasy::net::ppp::NW005_data_struct_PPP_modem *NW_data =
        static_cast<ESPEasy::net::ppp::NW005_data_struct_PPP_modem *>(getNWPluginData(event->NetworkIndex));
      bool mustCleanup = NW_data == nullptr;

      if (mustCleanup) {
        NW_data = new (std::nothrow) ESPEasy::net::ppp::NW005_data_struct_PPP_modem(event->NetworkIndex);
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
      ESPEasy::net::ppp::NW005_data_struct_PPP_modem *NW_data =
        static_cast<ESPEasy::net::ppp::NW005_data_struct_PPP_modem *>(getNWPluginData(event->NetworkIndex));
      bool mustCleanup = NW_data == nullptr;

      if (mustCleanup) {
        NW_data = new (std::nothrow) ESPEasy::net::ppp::NW005_data_struct_PPP_modem(event->NetworkIndex);
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
      initNWPluginData(event->NetworkIndex, new (std::nothrow) ESPEasy::net::ppp::NW005_data_struct_PPP_modem(event->NetworkIndex));
      ESPEasy::net::ppp::NW005_data_struct_PPP_modem *NW_data =
        static_cast<ESPEasy::net::ppp::NW005_data_struct_PPP_modem *>(getNWPluginData(event->NetworkIndex));

      if (NW_data) {
        success = NW_data->init(event);
      }
      break;
    }

    case NWPlugin::Function::NWPLUGIN_EXIT:
    {
      ESPEasy::net::ppp::NW005_data_struct_PPP_modem *NW_data =
        static_cast<ESPEasy::net::ppp::NW005_data_struct_PPP_modem *>(getNWPluginData(event->NetworkIndex));

      if (NW_data) {
        NW_data->exit(event);
      }
      success = true;
      break;
    }


    case NWPlugin::Function::NWPLUGIN_FIFTY_PER_SECOND:
    {
      break;
    }

    case NWPlugin::Function::NWPLUGIN_PRIORITY_ROUTE_CHANGED:
    {
      ESPEasy::net::ppp::NW005_data_struct_PPP_modem *NW_data =
        static_cast<ESPEasy::net::ppp::NW005_data_struct_PPP_modem *>(getNWPluginData(event->NetworkIndex));

      if (NW_data) {
        success = NW_data->handle_priority_route_changed();
      }
      break;
    }

    case NWPlugin::Function::NWPLUGIN_WRITE:
    {
      ESPEasy::net::ppp::NW005_data_struct_PPP_modem *NW_data =
        static_cast<ESPEasy::net::ppp::NW005_data_struct_PPP_modem *>(getNWPluginData(event->NetworkIndex));

      if (NW_data) {
        success = NW_data->handle_nwplugin_write(event, string);
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

#endif // ifdef USES_NW005
