#include "../WebServer/PluginListPage.h"


#if FEATURE_PLUGIN_LIST

# include "../WebServer/ESPEasy_WebServer.h"
# include "../WebServer/HTML_wrappers.h"

# include "../_Plugin_Helper.h"
# if FEATURE_NOTIFIER
#  include "../Globals/NPlugins.h"
#  include "../DataTypes/NPluginID.h"
# endif // if FEATURE_NOTIFIER
# ifdef WEBSERVER_NETWORK
#  include "../ESPEasy/net/DataTypes/NetworkDriverIndex.h"
# endif

void handle_pluginlist() {
  # ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_pluginlist"));
  # endif // ifndef BUILD_NO_RAM_TRACKER

  # if FEATURE_MQTT_TLS // TODO Add check for FEATURE_HTTP_TLS when https://github.com/letscontrolit/ESPEasy/pull/5402 is merged
  const int colspan = 6;
  # else // if FEATURE_MQTT_TLS
  const int colspan = 5;
  # endif // if FEATURE_MQTT_TLS

  TXBuffer.startStream();
  sendHeadandTail_stdtemplate(_HEAD);

  // the table header
  html_table_class_normal();

  for (uint8_t p = 0; p < 2; ++p) { // Sorted by Description and by PluginID
    addTableSeparator(concat(F("Plugins sorted by "), 0 == p ? F("Description") : F("Plugin ID")), colspan, 3);
    html_TR();
    html_table_header(F("Plugin"),      80);
    html_table_header(F(""),            25);
    html_table_header(F("Description"), 800);
    html_table_header(F(""),            50);
    # if FEATURE_MQTT_TLS
    html_table_header(F(""),            50);
    # endif // if FEATURE_MQTT_TLS
    html_table_header(F(""));

    deviceIndex_t x;
    bool done = false;

    while (!done) {
      const deviceIndex_t deviceIndex = (0 == p) ? getDeviceIndex_sorted(x) : x;

      if (!validDeviceIndex(deviceIndex)) {
        done = true;
      } else {
        const pluginID_t pluginID = getPluginID_from_DeviceIndex(deviceIndex);

        if (validPluginID(pluginID)) {
          html_TR_TD();
          addHtml(get_formatted_Plugin_number(pluginID));
          html_TD();
          addRTDPluginButton(pluginID);
          html_TD();
          addHtml(getPluginNameFromDeviceIndex(deviceIndex));
          html_TD();
        }
      }
      ++x;
    }
  }

  {
    addTableSeparator(F("Controllers"), colspan, 3);
    html_TR();
    html_table_header(F("Controller"),  80);
    html_table_header(F(""),            25);
    html_table_header(F("Description"), 800);
    html_table_header(F("MQTT"),        50);
    # if FEATURE_MQTT_TLS
    html_table_header(F("TLS"),         50);
    # endif // if FEATURE_MQTT_TLS
    html_table_header(F(""));
    protocolIndex_t x;
    bool done = false;

    while (!done) {
      if (!validProtocolIndex(x)) {
        done = true;
      } else {
        const cpluginID_t cpluginID = getCPluginID_from_ProtocolIndex(x);
        const ProtocolStruct& proto = getProtocolStruct(x);

        if (validCPluginID(cpluginID)) {
          html_TR_TD();
          addHtml(get_formatted_Controller_number(cpluginID));
          html_TD();
          # ifndef LIMIT_BUILD_SIZE
          addRTDControllerButton(cpluginID);
          # endif // ifndef LIMIT_BUILD_SIZE
          html_TD();
          addHtml(getCPluginNameFromProtocolIndex(x));

          html_TD();

          if (proto.usesMQTT) {
            addEnabled(true);
          }
          # if FEATURE_MQTT_TLS

          html_TD();

          if (proto.usesTLS) {
            addEnabled(true);
          }
          # endif // if FEATURE_MQTT_TLS
          html_TD();
        }
      }
      ++x;
    }
  }

  # if FEATURE_NOTIFIER
  {
    addTableSeparator(F("Notifications"), colspan, 3);
    html_TR();
    html_table_header(F("Notifier"),    80);
    html_table_header(F(""),            25);
    html_table_header(F("Description"), 800);
    html_table_header(F(""),            50);
    #  if FEATURE_MQTT_TLS
    html_table_header(F(""),            50);
    #  endif // if FEATURE_MQTT_TLS
    html_table_header(F(""));

    for (uint8_t x = 0; x <= notificationCount; x++)
    {
      html_TR_TD();
      const npluginID_t plugin(Notification[x].Number);
      addHtml(plugin.toDisplayString());
      html_TD();
      addRTDHelpButton(strformat(F("Notify/%s.html"), plugin.toDisplayString().c_str()));
      html_TD();
      String NotificationName;
      NPlugin_ptr[x](NPlugin::Function::NPLUGIN_GET_DEVICENAME, 0, NotificationName);
      addHtml(NotificationName);
      html_TD();
    }
  }
  # endif // if FEATURE_NOTIFIER

  # ifdef WEBSERVER_NETWORK
  {
    addTableSeparator(F("Networks"), colspan, 3);
    html_TR();
    html_table_header(F("Network"),     80);
    html_table_header(F(""),            25);
    html_table_header(F("Description"), 800);
    html_table_header(F(""),            50);
    #  if FEATURE_MQTT_TLS
    html_table_header(F(""),            50);
    #  endif // if FEATURE_MQTT_TLS
    html_table_header(F(""));

    ESPEasy::net::networkDriverIndex_t tmpNetworkDriverIndex{};

    while (validNetworkDriverIndex(tmpNetworkDriverIndex))
    {
      html_TR_TD();
      const ESPEasy::net::nwpluginID_t number = getNWPluginID_from_NetworkDriverIndex(tmpNetworkDriverIndex);
      addHtml(number.toDisplayString());
      html_TD();
      #  ifndef LIMIT_BUILD_SIZE
      addRTDNetworkDriverButton(number);
      #  endif // ifndef LIMIT_BUILD_SIZE
      html_TD();
      addHtml(getNWPluginNameFromNetworkDriverIndex(tmpNetworkDriverIndex));
      html_TD();
      ++tmpNetworkDriverIndex;
    }

  }
  # endif // ifdef WEBSERVER_NETWORK

  html_end_table();
  html_end_form();
  sendHeadandTail_stdtemplate(_TAIL);
  TXBuffer.endStream();
}

#endif // if FEATURE_PLUGIN_LIST
