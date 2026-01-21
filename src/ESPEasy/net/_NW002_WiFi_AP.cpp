#include "../../ESPEasy_common.h"

#ifdef USES_NW002

// #######################################################################################################
// ########################### Network Plugin 002: WiFi Access Point #####################################
// #######################################################################################################

# define NWPLUGIN_002
# define NWPLUGIN_ID_002         2
# define NWPLUGIN_NAME_002       "WiFi AP"

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
# include "../net/ESPEasyNetwork.h"
# include "../net/Globals/NWPlugins.h"
# include "../net/Helpers/_NWPlugin_Helper_webform.h"
# include "../net/Helpers/_NWPlugin_init.h"
# include "../net/NWPluginStructs/NW002_data_struct_WiFi_AP.h"


// TODO TD-er: This code should be moved to this NW002 plugin
# include "../net/wifi/ESPEasyWifi.h"

# ifdef ESP32
#  include <esp_wifi.h>
#  include <esp_wifi_ap_get_sta_list.h>
# endif // ifdef ESP32

namespace ESPEasy {
namespace net {

bool NWPlugin_002(NWPlugin::Function function, EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {
    case NWPlugin::Function::NWPLUGIN_DRIVER_ADD:
    {
      NetworkDriverStruct& nw = getNetworkDriverStruct(networkDriverIndex_t::toNetworkDriverIndex(event->idx));
      nw.onlySingleInstance    = true;
      nw.alwaysPresent         = true;
      nw.enabledOnFactoryReset = true;
      nw.fixedNetworkIndex     = NWPLUGIN_ID_002 - 1; // Start counting at 0
      break;
    }

    case NWPlugin::Function::NWPLUGIN_LOAD_DEFAULTS:
    {
# ifdef ESP32
      Settings.setRoutePrio_for_network(event->NetworkIndex, 10);
# endif
      Settings.setNetworkInterfaceSubnetBlockClientIP(event->NetworkIndex, false);
      Settings.setNetworkInterfaceStartupDelayAtBoot(event->NetworkIndex, 10000);
      Settings.StartAP_on_NW002_init(false);
      Settings.StartAPfallback_NoCredentials(true);
      Settings.DoNotStartAPfallback_ConnectFail(false);
      Settings.APfallback_autostart_max_uptime_m(0);
      Settings.APfallback_minimal_on_time_sec(DEFAULT_AP_FALLBACK_MINIMAL_ON_TIME_SEC);
      break;
    }

    case NWPlugin::Function::NWPLUGIN_GET_DEVICENAME:
    {
      string = F(NWPLUGIN_NAME_002);
      break;
    }

# ifdef ESP32
    case NWPlugin::Function::NWPLUGIN_GET_INTERFACE:
    {
      event->networkInterface = &WiFi.AP;
      success                 = event->networkInterface != nullptr;
      break;
    }
# endif // ifdef ESP32

    case NWPlugin::Function::NWPLUGIN_WEBSERVER_SHOULD_RUN:
    {
      success = ESPEasy::net::wifi::wifiAPmodeActivelyUsed();

      break;
    }

    case NWPlugin::Function::NWPLUGIN_WEBFORM_SHOW_ACTIVE:
    {
      success = ESPEasy::net::wifi::WifiIsAP(WiFi.getMode());
      break;
    }

    case NWPlugin::Function::NWPLUGIN_WEBFORM_SHOW_CONNECTED:
    {
      if (event->kvWriter == nullptr) {
        break;
      }

      KeyValueStruct kv(F("Clients"));

# ifdef ESP32
      wifi_sta_list_t wifi_sta_list               = { 0 };
      wifi_sta_mac_ip_list_t wifi_sta_mac_ip_list = { 0 };

      esp_wifi_ap_get_sta_list(&wifi_sta_list);
      esp_wifi_ap_get_sta_list_with_ip(&wifi_sta_list, &wifi_sta_mac_ip_list);

      for (int i = 0; i < wifi_sta_mac_ip_list.num; i++)
      {
        const MAC_address mac(wifi_sta_mac_ip_list.sta[i].mac);
        const IPAddress   ip(wifi_sta_mac_ip_list.sta[i].ip.addr);

        kv.appendValue(strformat(
                         F("%s IP:%s (%d dBm)"),
                         mac.toString().c_str(),
                         ip.toString().c_str(),
                         wifi_sta_list.sta[i].rssi
                         ));
      }
# endif // ifdef ESP32

# ifdef ESP8266
#  ifndef LIMIT_BUILD_SIZE
      struct station_info *station = wifi_softap_get_station_info();

      while (station)
      {
        const MAC_address mac(station->bssid);
        const IPAddress   ip(station->ip.addr);

        kv.appendValue(strformat(
                         F("%s IP:%s"),
                         mac.toString().c_str(),
                         ip.toString().c_str()
                         ));

        station = STAILQ_NEXT(station, next);
      }
      wifi_softap_free_station_info();

#  else // ifndef LIMIT_BUILD_SIZE
      const uint8_t num = SOFTAP_STATION_COUNT;

      if (num > 0) {
        success = true;
        string  = num;
        string += F(" client");

        if (num > 1) { string += 's'; }
      }
#  endif // ifndef LIMIT_BUILD_SIZE
# endif // ifdef ESP8266

      if (kv._values.size() > 0) {
        success = true;
        event->kvWriter->write(kv);
      }
      break;
    }

    case NWPlugin::Function::NWPLUGIN_WEBFORM_SHOW_NAME:
    {
      if (event->kvWriter) {
        event->kvWriter->write({ F("Name"), F("ap") });
        success = true;
      }
      break;
    }

    case NWPlugin::Function::NWPLUGIN_WEBFORM_SHOW_HOSTNAME:
    {
      if (event->kvWriter) {
        event->kvWriter->write({ F("Hostname"),
# ifdef ESP32
                                 WiFi.AP.SSID()
# else
                                 WiFi.softAPSSID()
# endif // ifdef ESP32
                               });
        success = true;
      }
      break;
    }

# ifdef ESP8266
    case NWPlugin::Function::NWPLUGIN_WEBFORM_SHOW_HW_ADDRESS:
    {
      if (event->kvWriter) {
        event->kvWriter->write({ F("MAC"), WiFi.softAPmacAddress(), KeyValueStruct::Format::PreFormatted });
        success = true;
      }
      break;
    }

    case NWPlugin::Function::NWPLUGIN_WEBFORM_SHOW_IP:
    {
      if (event->kvWriter) {
        event->kvWriter->write({ F("IP"), WiFi.softAPIP().toString(), KeyValueStruct::Format::PreFormatted });
        success = true;
      }
      break;
    }

    case NWPlugin::Function::NWPLUGIN_CLIENT_IP_WEB_ACCESS_ALLOWED:
    {
      // FIXME TD-er: Shouldn't we just always allow access from AP?
      if (!Settings.getNetworkInterfaceSubnetBlockClientIP(event->NetworkIndex)) {
        IPAddress client_ip;
        client_ip.fromString(string);

        // FIXME TD-er: Do we allow to set the subnetmask for AP to anything else?
        const IPAddress subnet(255, 255, 255, 0);
        const IPAddress localIP = WiFi.softAPIP();
        bool success            = true;

        for (uint8_t i = 0; success && i < 4; ++i) {
          if ((localIP[i] & subnet[i]) != (client_ip[i] & subnet[i])) {
            success = false;
          }
        }
      }
      break;
    }
# endif // ifdef ESP8266

    case NWPlugin::Function::NWPLUGIN_WEBFORM_SAVE:
    {

      // Access point password.
      copyFormPassword(F("apkey"), SecuritySettings.WifiAPKey, sizeof(SecuritySettings.WifiAPKey));

      Settings.WiFiAP_channel = getFormItemInt(LabelType::WIFI_AP_CHANNEL);

      // When set, user will be redirected to /setup or root page when connecting to this AP
      Settings.ApCaptivePortal(isFormItemChecked(LabelType::WIFI_ENABLE_CAPTIVE_PORTAL));

      // Usually the AP will be started when no WiFi is defined, or the defined one cannot be found. This flag may prevent it.
      Settings.StartAPfallback_NoCredentials(isFormItemChecked(LabelType::WIFI_START_AP_NO_CREDENTIALS));
      Settings.DoNotStartAPfallback_ConnectFail(!isFormItemChecked(LabelType::WIFI_START_AP_ON_CONNECT_FAIL));
      Settings.StartAP_on_NW002_init(isFormItemChecked(LabelType::WIFI_START_AP_ON_NW002_INIT));
      Settings.APfallback_autostart_max_uptime_m(getFormItemInt(LabelType::WIFI_MAX_UPTIME_AUTO_START_AP));
      Settings.APfallback_minimal_on_time_sec(getFormItemInt(LabelType::WIFI_AP_MINIMAL_ON_TIME));


# ifdef ESP32
      Settings.WiFi_AP_enable_NAPT(isFormItemChecked(LabelType::WIFI_AP_ENABLE_NAPT));
# endif

      break;
    }

    case NWPlugin::Function::NWPLUGIN_WEBFORM_LOAD:
    {
      addFormCheckBox(LabelType::WIFI_START_AP_ON_NW002_INIT);

      addFormSubHeader(F("Wifi AP Settings"));
      addFormPasswordBox(F("WPA AP Mode Key"), F("apkey"), SecuritySettings.WifiAPKey, 63);
      addFormNote(F("WPA Key must be at least 8 characters long"));


      {
# if CONFIG_SOC_WIFI_SUPPORT_5G

        // See wifi_5g_channel_bit_t for all supported channels
        const int wifiChannels[] =
        { 1,   2,  3,   4,   5,   6,   7,   8,   9,   10,   11,  12,  13, 14 // 2.4 GHz
          ,36,   40,  44,  48                                                // 5 GHz U-NII-1
          ,52,   56,  60,  64,  68                                           // 5 GHz U-NII-2A
          // ,72, 76, 80, 84, 88 /* , 92 */                                  // 5 GHz U-NII-2B
          ,/* 96,*/ 100,104, 108, 112, 116, 120, 124, 128, 132,  136, 140    // 5 GHz U-NII-2C
          ,144                                                               // 5 GHz U-NII-2C/3
          ,149,  153, 157, 161, 165                                          // 5 GHz U-NII-3
          ,169                                                               // 5 GHz U-NII-3/4
          ,173,  177                                                         // 5 GHz U-NII-4
        };
        constexpr int nrwifiChannels = NR_ELEMENTS(wifiChannels);
        const FormSelectorOptions selector(
          nrwifiChannels,
          wifiChannels);
        selector.addFormSelector(
          LabelType::WIFI_AP_CHANNEL,
          Settings.WiFiAP_channel);
# else // if CONFIG_SOC_WIFI_SUPPORT_5G
        addFormNumericBox(LabelType::WIFI_AP_CHANNEL, 1, 14);
# endif // if CONFIG_SOC_WIFI_SUPPORT_5G

      }
# ifdef ESP32
      addFormCheckBox(LabelType::WIFI_AP_ENABLE_NAPT);
# endif // ifdef ESP32

      addFormSubHeader(F("Wifi AP Fallback"));
      {
        LabelType::Enum labels[]{
          LabelType::WIFI_ENABLE_CAPTIVE_PORTAL
          , LabelType::WIFI_START_AP_NO_CREDENTIALS
          , LabelType::WIFI_START_AP_ON_CONNECT_FAIL
        };

        addFormCheckBoxes(labels, NR_ELEMENTS(labels));
      }

      addFormNumericBox(LabelType::WIFI_MAX_UPTIME_AUTO_START_AP, 0, 255);
      addFormNumericBox(LabelType::WIFI_AP_MINIMAL_ON_TIME,       0, 255);
      break;
    }

    case NWPlugin::Function::NWPLUGIN_INIT:
    {
      initNWPluginData(event->NetworkIndex, new (std::nothrow) ESPEasy::net::wifi::NW002_data_struct_WiFi_AP(event->NetworkIndex));
      ESPEasy::net::wifi::NW002_data_struct_WiFi_AP *NW_data =
        static_cast<ESPEasy::net::wifi::NW002_data_struct_WiFi_AP *>(getNWPluginData(event->NetworkIndex));

      if (NW_data) {
        success = NW_data->init(event);
      }
      break;
    }

    case NWPlugin::Function::NWPLUGIN_EXIT:
    {
      ESPEasy::net::wifi::NW002_data_struct_WiFi_AP *NW_data =
        static_cast<ESPEasy::net::wifi::NW002_data_struct_WiFi_AP *>(getNWPluginData(event->NetworkIndex));

      if (NW_data) {
        NW_data->exit(event);
      }
      success = true;
      break;
    }

# ifdef ESP32
    case NWPlugin::Function::NWPLUGIN_PRIORITY_ROUTE_CHANGED:
    {
      ESPEasy::net::wifi::NW002_data_struct_WiFi_AP *NW_data =
        static_cast<ESPEasy::net::wifi::NW002_data_struct_WiFi_AP *>(getNWPluginData(event->NetworkIndex));

      if (NW_data) {
        success = NW_data->handle_priority_route_changed();
      }
      break;
    }
# endif // ifdef ESP32


    default:
      break;

  }
  return success;
}

} // namespace net
} // namespace ESPEasy

#endif // ifdef USES_NW002
