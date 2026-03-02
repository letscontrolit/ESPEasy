#include "../WebServer/NetworkPage.h"

#ifdef WEBSERVER_NETWORK


# include "../DataStructs/ESPEasy_EventStruct.h"
# include "../Globals/Settings.h"
# include "../Helpers/ESPEasy_Storage.h"
# include "../Helpers/ESPEasy_Storage.h"
# include "../Helpers/PrintToString.h"
# include "../Helpers/StringConverter.h"
# include "../WebServer/ESPEasy_WebServer.h"
# include "../WebServer/HTML_wrappers.h"
# include "../WebServer/KeyValueWriter_WebForm.h"
# include "../WebServer/Markup.h"
# include "../WebServer/Markup_Buttons.h"
# include "../WebServer/Markup_Forms.h"
# include "../../ESPEasy/net/Globals/NWPlugins.h"
# include "../../ESPEasy/net/Helpers/_NWPlugin_Helper_webform.h"
# include "../../ESPEasy/net/Helpers/_NWPlugin_init.h"
# include "../../ESPEasy/net/Helpers/NW_info_writer.h"
# include "../../ESPEasy/net/_NWPlugin_Helper.h"

# ifdef ESP8266
#  define MAX_NR_NETWORKS_IN_TABLE  2
# endif
# ifdef ESP32
#  define MAX_NR_NETWORKS_IN_TABLE  NETWORK_MAX
# endif


using namespace ESPEasy::net;

void handle_networks()
{
# ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_networks"));
# endif // ifndef BUILD_NO_RAM_TRACKER

  if (!isLoggedIn()) { return; }
  navMenuIndex = MENU_INDEX_NETWORK;
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate(_HEAD);


  // 'index' value in the URL
  uint8_t networkindex       = getFormItemInt(F("index"), 0);
  const bool networkIndexSet = networkindex != 0 && validNetworkIndex(networkindex - 1);
  --networkindex; // Index in URL is starting from 1, but starting from 0 in the array.

  const int networkDriver_webarg_value = getFormItemInt(F("networkDriver"), -1);

  // submitted data
  if ((networkDriver_webarg_value != -1) && networkIndexSet)
  {
    bool mustInit             = false;
    bool mustCallNWpluginSave = false;

    // TODO TD-er: Implement saving submitted settings
    const nwpluginID_t nwpluginID = nwpluginID_t::toPluginID(networkDriver_webarg_value);

    auto NetworkSettings = MakeNetworkSettings();

    if (Settings.getNWPluginID_for_network(networkindex) != nwpluginID)
    {
# ifndef LIMIT_BUILD_SIZE
      addLog(LOG_LEVEL_INFO, strformat(
               F("HandleNW: Driver changed from %d to %d"),
               Settings.getNWPluginID_for_network(networkindex).value,
               nwpluginID.value));
# endif // ifndef LIMIT_BUILD_SIZE

      // NetworkDriver has changed.
      Settings.setNWPluginID_for_network(networkindex, nwpluginID);

      // there is a networkDriverIndex selected?
      if (nwpluginID.isValid())
      {
        mustInit = true;

        handle_networks_clearLoadDefaults(networkindex, *NetworkSettings);
      }
    }

    // subitted same networkDriverIndex
    else
    {
      // there is a networkDriverIndex selected
# ifndef LIMIT_BUILD_SIZE
      addLog(LOG_LEVEL_INFO, concat(
               F("HandleNW: Driver selected: "),
               nwpluginID.value) + (nwpluginID.isValid() ? F("valid") : F("invalid")));
# endif // ifndef LIMIT_BUILD_SIZE

      if (nwpluginID.isValid())
      {
        mustInit = true;

        handle_networks_CopySubmittedSettings(networkindex, *NetworkSettings);
        mustCallNWpluginSave = true;
      }
    }

    if (mustCallNWpluginSave) {
      // Call NWPLUGIN_WEBFORM_SAVE after destructing NetworkSettings object to reduce RAM usage.
      // Network plugin almost only deals with custom network settings.
      // Even if they need to save things to the NetworkSettings, then the changes must
      // already be saved first as the NWPluginCall does not have the NetworkSettings as argument.
      handle_networks_CopySubmittedSettings_NWPluginCall(networkindex);
    }
    addHtmlError(SaveSettings());

    if (mustInit) {
      // Init network plugin using the new settings.
      NWPlugin_Exit_Init(networkindex);
    }

  }

  html_add_form();

  if (networkIndexSet)
  {
    handle_networks_NetworkSettingsPage(networkindex);
  }
  else
  {
    handle_networks_ShowAllNetworksTable();
  }

  sendHeadandTail_stdtemplate(_TAIL);
  TXBuffer.endStream();
}

void handle_networks_clearLoadDefaults(ESPEasy::net::networkIndex_t networkindex, NetworkSettingsStruct& NetworkSettings) {

  // TODO TD-er: Must also check NetworkDriverStruct to see if something else must be done
}

void handle_networks_CopySubmittedSettings(ESPEasy::net::networkIndex_t networkindex, NetworkSettingsStruct& NetworkSettings)
{
  // copy all settings to network settings struct
  for (int parameterIdx = 0; parameterIdx <= NetworkSettingsStruct::NETWORK_ENABLED; ++parameterIdx) {
    NetworkSettingsStruct::VarType varType = static_cast<NetworkSettingsStruct::VarType>(parameterIdx);
    saveNetworkParameterForm(NetworkSettings, networkindex, varType);
  }

}

void handle_networks_CopySubmittedSettings_NWPluginCall(ESPEasy::net::networkIndex_t networkindex) {
  networkDriverIndex_t NetworkDriverIndex = getNetworkDriverIndex_from_NetworkIndex(networkindex);

  if (validNetworkDriverIndex(NetworkDriverIndex)) {
    struct EventStruct TempEvent;
    TempEvent.NetworkIndex = networkindex;

    // Call network plugin to save CustomNetworkSettings
# ifndef LIMIT_BUILD_SIZE
    addLog(LOG_LEVEL_INFO, F("Call network plugin to save CustomNetworkSettings"));
# endif
# ifdef ESP32
    Settings.setRoutePrio_for_network(networkindex, getFormItemInt(F("routeprio"), 0));
    Settings.setNetworkInterfaceSubnetBlockClientIP(networkindex, isFormItemChecked(F("block_web_access")));
# endif // ifdef ESP32
# if FEATURE_USE_IPV6
    Settings.setNetworkEnabled_IPv6(networkindex, isFormItemChecked(F("en_ipv6")));
# endif
    Settings.setNetworkInterfaceStartupDelayAtBoot(networkindex, getFormItemInt(F("delay_start")));
    String dummy;
    ESPEasy::net::NWPluginCall(NWPlugin::Function::NWPLUGIN_WEBFORM_SAVE, &TempEvent, dummy);
  }

}

void handle_networks_ShowAllNetworksTable()
{
  html_table_class_multirow();
  html_TR();
  html_table_header(F(""),           70);
  html_table_header(F("Nr"),         50);
  html_table_header(F("Enabled"),    100);
  html_table_header(F("Network Adapter"));
  html_table_header(F("Active"),     100);
  html_table_header(F("Name"),       50);
  # ifdef ESP32
  html_table_header(F("Route Prio"), 50);
  # endif
  html_table_header(F("Connected"));
  html_table_header(F("Hostname/SSID"));
  html_table_header(F("HW Address"));
  html_table_header(F("IP"));
# ifndef LIMIT_BUILD_SIZE
  html_table_header(F("Port"));
# endif

  for (ESPEasy::net::networkIndex_t x = 0; x < MAX_NR_NETWORKS_IN_TABLE; x++)
  {
    const nwpluginID_t nwpluginID = Settings.getNWPluginID_for_network(x);
    const bool nwplugin_set       = nwpluginID.isValid();

    html_TR_TD();

    addPlugin_Add_Edit_Button(F("network"), x, nwplugin_set, supportedNWPluginID(nwpluginID));

    if (nwplugin_set)
    {
      addEnabled(Settings.getNetworkEnabled(x));
      html_TD();

      // Network Adapter
      addHtml(getNWPluginNameFromNWPluginID(Settings.getNWPluginID_for_network(x)));

      const NWPlugin::Function functions[] {
        NWPlugin::Function::NWPLUGIN_WEBFORM_SHOW_ACTIVE,
        NWPlugin::Function::NWPLUGIN_WEBFORM_SHOW_NAME,
# ifdef ESP32
        NWPlugin::Function::NWPLUGIN_WEBFORM_SHOW_ROUTE_PRIO,
# endif
        NWPlugin::Function::NWPLUGIN_WEBFORM_SHOW_CONNECTED,
        NWPlugin::Function::NWPLUGIN_WEBFORM_SHOW_HOSTNAME,
        NWPlugin::Function::NWPLUGIN_WEBFORM_SHOW_HW_ADDRESS,
        NWPlugin::Function::NWPLUGIN_WEBFORM_SHOW_IP
# ifndef LIMIT_BUILD_SIZE
        , NWPlugin::Function::NWPLUGIN_WEBFORM_SHOW_PORT
# endif
      };

      //      const networkDriverIndex_t NetworkDriverIndex = getNetworkDriverIndex_from_NetworkIndex(x);

      for (uint8_t i = 0; i < NR_ELEMENTS(functions); ++i) {
        html_TD();

        if (Settings.getNetworkEnabled(x) 
# ifndef LIMIT_BUILD_SIZE
        || (functions[i] == NWPlugin::Function::NWPLUGIN_WEBFORM_SHOW_PORT)
# endif
      ) {
          KeyValueWriter_WebForm webFormWriter;
          webFormWriter.setSummaryValueOnly();
          struct EventStruct TempEvent;
          TempEvent.kvWriter = &webFormWriter;

          TempEvent.NetworkIndex = x;

          String str;

          const bool res = ESPEasy::net::NWPluginCall(functions[i], &TempEvent);

          switch (functions[i])
          {
# ifdef ESP32

            case NWPlugin::Function::NWPLUGIN_WEBFORM_SHOW_ROUTE_PRIO:

              if (TempEvent.Par1 > 0) {
                addHtmlInt(TempEvent.Par1);

                if (TempEvent.Par2) {
                  addHtml(F("(*)"));
                }
              }
              break;
# endif // ifdef ESP32

            case NWPlugin::Function::NWPLUGIN_WEBFORM_SHOW_ACTIVE:
              addEnabled(res);
              break;
            case NWPlugin::Function::NWPLUGIN_WEBFORM_SHOW_CONNECTED:

              ESPEasy::net::NWPluginCall(NWPlugin::Function::NWPLUGIN_GET_CONNECTED_DURATION, &TempEvent);

              if (!res) {
                addEnabled(res);
              }
              break;
            default: break;
          }
        }
      }


      /*
         const NetworkDriverStruct& driver = ESPEasy::net::getNetworkDriverStruct(NetworkDriverIndex);

         if ((INVALID_NETWORKDRIVER_INDEX == NetworkDriverIndex) || driver.usesPort) {
         addHtmlInt(13 == nwpluginID ? Settings.UDPPort : NetworkSettings->Port); // P2P/C013 exception
         }
       */

    }
    else {
# ifndef LIMIT_BUILD_SIZE
      html_TD(9);
# else
      html_TD(8);
# endif // ifndef LIMIT_BUILD_SIZE
    }
  }

  html_end_table();
  html_end_form();
}

void handle_networks_NetworkSettingsPage(ESPEasy::net::networkIndex_t networkindex)
{
  if (!validNetworkIndex(networkindex)) { return; }

  const networkDriverIndex_t networkDriverIndex =
    getNetworkDriverIndex_from_NWPluginID(
      Settings.getNWPluginID_for_network(networkindex));
  const NetworkDriverStruct& cur_driver = ESPEasy::net::getNetworkDriverStruct(networkDriverIndex);


  // Show network settings page
  {
    html_table_class_normal();
    addFormHeader(F("Network Settings"));
    addRowLabel(F("Network Driver"));
    const nwpluginID_t choice = Settings.getNWPluginID_for_network(networkindex);

    const bool networkDriverSelectorDisabled = cur_driver.alwaysPresent && validNetworkIndex(cur_driver.fixedNetworkIndex) &&
                                               networkindex == cur_driver.fixedNetworkIndex;

    if (networkDriverSelectorDisabled) {
      addSelector_Head(F("networkDriver"));

      // Must add the fixed network driver label here.

      addSelector_Item(getNWPluginNameFromNetworkDriverIndex(networkDriverIndex),
                       choice.value,
                       true);
    } else {
      addSelector_Head_reloadOnChange(F("networkDriver"));
      addSelector_Item(F("- Standalone -"), 0, false, false, EMPTY_STRING);
      networkDriverIndex_t tmpNetworkDriverIndex{};

      while (validNetworkDriverIndex(tmpNetworkDriverIndex))
      {
        const NetworkDriverStruct& driver = getNetworkDriverStruct(tmpNetworkDriverIndex);
        const nwpluginID_t number         = getNWPluginID_from_NetworkDriverIndex(tmpNetworkDriverIndex);
        const bool disabled               = driver.alwaysPresent &&
                                            validNetworkIndex(driver.fixedNetworkIndex) &&
                                            networkindex != driver.fixedNetworkIndex;
        addSelector_Item(getNWPluginNameFromNetworkDriverIndex(tmpNetworkDriverIndex),
                         number.value,
                         choice == number,
                         disabled);
        ++tmpNetworkDriverIndex;
      }
    }
    addSelector_Foot();

  }

  # ifndef LIMIT_BUILD_SIZE
  addRTDNetworkDriverButton(getNWPluginID_from_NetworkDriverIndex(networkDriverIndex));
  # endif // ifndef LIMIT_BUILD_SIZE

  if (Settings.getNWPluginID_for_network(networkindex)) {
    // Separate enabled checkbox as it doesn't need to use the NetworkSettings.
    // So NetworkSettings object can be destructed before network specific settings are loaded.
    addNetworkEnabledForm(networkindex);
    addFormSeparator(2);

    // TODO TD-er: Add driver specifics from NetworkDriverStruct

    // Load network specific settings
    struct EventStruct TempEvent;
    TempEvent.NetworkIndex = networkindex;

# ifdef ESP32
    addFormNumericBox(
      F("Route Priority"),
      F("routeprio"),
      Settings.getRoutePrio_for_network(networkindex),
      0, 255);
    addFormNote(F("The active interface with highest priority will be used for default route (gateway)."));
# endif // ifdef ESP32
    addFormCheckBox(F("Block Web Access"), F("block_web_access"), Settings.getNetworkInterfaceSubnetBlockClientIP(networkindex));
    addFormNote(F("When checked, any host from a subnet on this network interface will be blocked"));
    addFormNumericBox(F("Delay Startup At Boot"), F("delay_start"), Settings.getNetworkInterfaceStartupDelayAtBoot(networkindex), 0, 60000);
    addUnit(F("ms"));

# if FEATURE_USE_IPV6
    addFormCheckBox(F("Enable IPv6"), F("en_ipv6"), Settings.getNetworkEnabled_IPv6(networkindex));

    if (!Settings.EnableIPv6()) {
      addFormNote(F("IPv6 is disabled on tools->Advanced page"));
    }
# endif // if FEATURE_USE_IPV6

    String str;
    ESPEasy::net::NWPluginCall(NWPlugin::Function::NWPLUGIN_WEBFORM_LOAD, &TempEvent, str);

# if FEATURE_NETWORK_STATS

    if (Settings.getNetworkEnabled(TempEvent.NetworkIndex))
    {
      // Task statistics and historic data in a chart
      auto *NW_data = ESPEasy::net::getNWPluginData(TempEvent.NetworkIndex);

      if (NW_data && NW_data->hasPluginStats()) {
        addFormSubHeader(F("Statistics"));
#  if FEATURE_CHART_JS

        if (NW_data->nrSamplesPresent() > 0) {
          addRowColspan(2);

          //        addRowLabel(F("Historic data"));
          NW_data->plot_ChartJS();
          addHtml(F("</td></tr>"));
        }
#  endif // if FEATURE_CHART_JS
        String dummy;

        // bool   somethingAdded = false;

        if (!ESPEasy::net::NWPluginCall(NWPlugin::Function::NWPLUGIN_WEBFORM_LOAD_SHOW_STATS, &TempEvent, dummy)) {
          /*somethingAdded =*/ NW_data->webformLoad_show_stats(&TempEvent);

          //        } else {
          //          somethingAdded = true;
        }
      }
    }

# endif // if FEATURE_NETWORK_STATS

# ifdef ESP32

    if (ESPEasy::net::NWPluginCall(NWPlugin::Function::NWPLUGIN_GET_INTERFACE, &TempEvent, str))
    {
      KeyValueWriter_WebForm writer(true);

      ESPEasy::net::write_NetworkAdapterFlags(networkindex, writer.createChild(F("Network Interface")).get());
#  ifndef LIMIT_BUILD_SIZE
      ESPEasy::net::write_NetworkAdapterPort(networkindex, writer.createChild(F("Port")).get());
#  endif
      ESPEasy::net::write_IP_config(networkindex, writer.createChild(F("IP Config")).get());
      ESPEasy::net::write_NetworkConnectionInfo(networkindex, writer.createChild(F("Connection Information")).get());
    }
# endif // ifdef ESP32

  }

  addFormSeparator(2);
  html_TR_TD();
  html_TD();
  addButton(F("network"), F("Close"));
  addSubmitButton();
  html_end_table();
  html_end_form();

}

#endif // ifdef WEBSERVER_NETWORK
