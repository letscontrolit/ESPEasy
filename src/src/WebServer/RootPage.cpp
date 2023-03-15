#include "../WebServer/RootPage.h"


#ifdef WEBSERVER_ROOT

# include "../WebServer/ESPEasy_WebServer.h"
# include "../WebServer/HTML_wrappers.h"
# include "../WebServer/LoadFromFS.h"
# include "../WebServer/Markup.h"
# include "../WebServer/Markup_Buttons.h"
# include "../WebServer/Markup_Forms.h"

# include "../Commands/InternalCommands.h"
# include "../ESPEasyCore/ESPEasyNetwork.h"
# include "../Globals/ESPEasy_time.h"
# include "../Globals/ESPEasyWiFiEvent.h"
# include "../Globals/MainLoopCommand.h"
# include "../Globals/NetworkState.h"
# include "../Globals/Nodes.h"
# include "../Globals/Settings.h"
# include "../Globals/Statistics.h"
# include "../Helpers/ESPEasy_Storage.h"
# include "../Helpers/Memory.h"
# include "../Helpers/Misc.h"
# include "../Helpers/StringGenerator_System.h"
# include "../Helpers/WebServer_commandHelper.h"


# include "../../ESPEasy-Globals.h"

# if FEATURE_MQTT
#  include "../Globals/MQTT.h"
#  include "../ESPEasyCore/Controller.h" // For finding enabled MQTT controller
# endif // if FEATURE_MQTT


# ifndef MAIN_PAGE_SHOW_BASIC_INFO_NOT_LOGGED_IN
  #  define MAIN_PAGE_SHOW_BASIC_INFO_NOT_LOGGED_IN false
# endif // ifndef MAIN_PAGE_SHOW_BASIC_INFO_NOT_LOGGED_IN

// Define main page elements present
# ifndef MAIN_PAGE_SHOW_SYSINFO_BUTTON
  #  define MAIN_PAGE_SHOW_SYSINFO_BUTTON    true
# endif // ifndef MAIN_PAGE_SHOW_SYSINFO_BUTTON

# ifndef MAIN_PAGE_SHOW_WiFi_SETUP_BUTTON
  #  define MAIN_PAGE_SHOW_WiFi_SETUP_BUTTON   false
# endif // ifndef MAIN_PAGE_SHOW_WiFi_SETUP_BUTTON

# ifndef MAIN_PAGE_SHOW_NODE_LIST_BUILD
  #  define MAIN_PAGE_SHOW_NODE_LIST_BUILD   true
# endif // ifndef MAIN_PAGE_SHOW_NODE_LIST_BUILD
# ifndef MAIN_PAGE_SHOW_NODE_LIST_TYPE
  #  define MAIN_PAGE_SHOW_NODE_LIST_TYPE    true
# endif // ifndef MAIN_PAGE_SHOW_NODE_LIST_TYPE


// ********************************************************************************
// Web Interface root page
// ********************************************************************************
void handle_root() {
  # ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_root"));
  # endif // ifndef BUILD_NO_RAM_TRACKER

  if (captivePortal()) { // If captive portal redirect instead of displaying the page.
    return;
  }

  // if Wifi setup, launch setup wizard if AP_DONT_FORCE_SETUP is not set.
  if (WiFiEventData.wifiSetup && !Settings.ApDontForceSetup())
  {
    web_server.send_P(200, (PGM_P)F("text/html"), (PGM_P)F("<meta HTTP-EQUIV='REFRESH' content='0; url=/setup'>"));
    return;
  }

  if (!MAIN_PAGE_SHOW_BASIC_INFO_NOT_LOGGED_IN) {
    if (!isLoggedIn()) { return; }
  }

  const bool loggedIn = isLoggedIn(false);

  navMenuIndex = 0;

  // if index.htm exists on FS serve that one (first check if gziped version exists)
  if (loadFromFS(F("/index.htm.gz"))) { return; }

  if (loadFromFS(F("/index.htm"))) { return; }

  TXBuffer.startStream();

  boolean rebootCmd = false;
  String  sCommand  = webArg(F("cmd"));
  rebootCmd = strcasecmp_P(sCommand.c_str(), PSTR("reboot")) == 0;
  sendHeadandTail_stdtemplate(_HEAD, rebootCmd);

  int freeMem = ESP.getFreeHeap();

  // TODO: move this to handle_tools, from where it is actually called?

  // have to disconnect or reboot from within the main loop
  // because the webconnection is still active at this point
  // disconnect here could result into a crash/reboot...
  if (strcasecmp_P(sCommand.c_str(), PSTR("wifidisconnect")) == 0)
  {
    addLog(LOG_LEVEL_INFO, F("WIFI : Disconnecting..."));
    cmd_within_mainloop = CMD_WIFI_DISCONNECT;
    addHtml(F("OK"));
  } else if (strcasecmp_P(sCommand.c_str(), PSTR("reboot")) == 0)
  {
    addLog(LOG_LEVEL_INFO, F("     : Rebooting..."));
    cmd_within_mainloop = CMD_REBOOT;
    addHtml(F("OK"));
  } else if (strcasecmp_P(sCommand.c_str(), PSTR("reset")) == 0)
  {
    if (loggedIn) {
      addLog(LOG_LEVEL_INFO, F("     : factory reset..."));
      cmd_within_mainloop = CMD_REBOOT;
      addHtml(F(
                "OK. Please wait > 1 min and connect to Access point.<BR><BR>PW=configesp<BR>URL=<a href='http://192.168.4.1'>192.168.4.1</a>"));
      TXBuffer.endStream();
      ExecuteCommand_internal(EventValueSource::Enum::VALUE_SOURCE_HTTP, sCommand.c_str());
      return;
    }
  } else {
    if (loggedIn) {
      handle_command_from_web(EventValueSource::Enum::VALUE_SOURCE_HTTP, sCommand);
      printToWeb     = false;
      printToWebJSON = false;
    }

    addHtml(F("<form>"));
    html_table_class_normal();
    addFormHeader(F("System Info"));

    addRowLabelValue(LabelType::UNIT_NR);
    addRowLabelValue(LabelType::GIT_BUILD);
    addRowLabel(LabelType::LOCAL_TIME);

    if (node_time.systemTimePresent())
    {
      addHtml(getValue(LabelType::LOCAL_TIME));
    }
    else {
      addHtml(F("<font color='red'>No system time source</font>"));
    }
    addRowLabelValue(LabelType::TIME_SOURCE);

    addRowLabel(LabelType::UPTIME);
    {
      addHtml(getExtendedValue(LabelType::UPTIME));
    }
    addRowLabel(LabelType::LOAD_PCT);

    if (wdcounter > 0)
    {
      addHtmlFloat(getCPUload());
      addHtml(F("% (LC="));
      addHtmlInt(getLoopCountPerSec());
      addHtml(')');
    }
    {
      addRowLabel(LabelType::FREE_MEM);
      addHtmlInt(freeMem);
        # ifndef BUILD_NO_RAM_TRACKER
      addHtml(F(" ("));
      addHtmlInt(lowestRAM);
      addHtml(F(" - "));
      addHtml(lowestRAMfunction);
      addHtml(')');
        # endif // ifndef BUILD_NO_RAM_TRACKER
    }
    {
        # ifdef USE_SECOND_HEAP
      addRowLabelValue(LabelType::FREE_HEAP_IRAM);
      # endif // ifdef USE_SECOND_HEAP
    }
    {
      addRowLabel(LabelType::FREE_STACK);
      addHtmlInt(getCurrentFreeStack());
        # ifndef BUILD_NO_RAM_TRACKER
      addHtml(F(" ("));
      addHtmlInt(lowestFreeStack);
      addHtml(F(" - "));
      addHtml(lowestFreeStackfunction);
      addHtml(')');
        # endif // ifndef BUILD_NO_RAM_TRACKER
    }

  # if FEATURE_ETHERNET
    addRowLabelValue(LabelType::ETH_WIFI_MODE);
  # endif // if FEATURE_ETHERNET

    if (!WiFiEventData.WiFiDisconnected())
    {
      addRowLabelValue(LabelType::IP_ADDRESS);
      addRowLabel(LabelType::WIFI_RSSI);
      addHtmlInt(WiFi.RSSI());
      addHtml(F(" dBm ("));
      addHtml(WiFi.SSID());
      addHtml(')');
    }

  # if FEATURE_ETHERNET

    if (active_network_medium == NetworkMedium_t::Ethernet) {
      addRowLabelValue(LabelType::ETH_SPEED_STATE);
      addRowLabelValue(LabelType::ETH_IP_ADDRESS);
    }
  # endif // if FEATURE_ETHERNET

      # if FEATURE_MDNS
    {
      addRowLabel(LabelType::M_DNS);
      addHtml(F("<a href='http://"));
      addHtml(getValue(LabelType::M_DNS));
      addHtml(F("'>"));
      addHtml(getValue(LabelType::M_DNS));
      addHtml(F("</a>"));
    }
      # endif // if FEATURE_MDNS

    # if FEATURE_MQTT
    {
      if (validControllerIndex(firstEnabledMQTT_ControllerIndex())) {
        addRowLabel(F("MQTT Client Connected"));
        addEnabled(MQTTclient_connected);
      }
    }
    # endif // if FEATURE_MQTT


    # if MAIN_PAGE_SHOW_SYSINFO_BUTTON
    html_TR_TD();
    html_TD();
    addButton(F("sysinfo"), F("More info"));
    # endif // if MAIN_PAGE_SHOW_SYSINFO_BUTTON
    # if MAIN_PAGE_SHOW_WiFi_SETUP_BUTTON
    html_TR_TD();
    html_TD();
    addButton(F("setup"), F("WiFi Setup"));
    # endif // if MAIN_PAGE_SHOW_WiFi_SETUP_BUTTON

    if (loggedIn) {
      if (printWebString.length() > 0)
      {
        html_BR();
        html_BR();
        addFormHeader(F("Command Argument"));
        addRowLabel(F("Command"));
        addHtml(sCommand);

        addHtml(F("<TR><TD colspan='2'>Command Output<BR><textarea readonly rows='10' wrap='on'>"));
        addHtml(printWebString);
        addHtml(F("</textarea>"));
        printWebString = String();
      }
    }
    html_end_table();

    # if FEATURE_ESPEASY_P2P
    html_BR();

    if ((Settings.Unit == 0) &&
        (Settings.UDPPort != 0)) { addFormNote(F("Warning: Unit number is 0, please change it if you want to send data to other units.")); }
    html_BR();
    html_table_class_multirow_noborder();
    html_TR();
    html_table_header(F("Node List"));
    html_table_header(F("Name"));

    if (MAIN_PAGE_SHOW_NODE_LIST_BUILD) {
      html_table_header(getLabel(LabelType::BUILD_DESC));
    }

    if (MAIN_PAGE_SHOW_NODE_LIST_TYPE) {
      html_table_header(F("Type"));
    }
    html_table_header(F("IP"), 160); // Should fit "255.255.255.255"
    html_table_header(F("Load"));
    html_table_header(F("Age (s)"));
    #  ifdef USES_ESPEASY_NOW

    if (Settings.UseESPEasyNow()) {
      html_table_header(F("Dist"));
      html_table_header(F("Peer Info"), 160);
    }
    #  endif // ifdef USES_ESPEASY_NOW

    for (auto it = Nodes.begin(); it != Nodes.end(); ++it)
    {
      if (it->second.valid())
      {
        bool isThisUnit = it->first == Settings.Unit;

        if (isThisUnit) {
          html_TR_TD_highlight();
        }
        else {
          html_TR_TD();
        }

        addHtml(F("Unit "));
        addHtmlInt(it->first);
        html_TD();

        if (isThisUnit) {
          addHtml(Settings.getName());
        }
        else {
          addHtml(it->second.getNodeName());
        }
        html_TD();

        if (MAIN_PAGE_SHOW_NODE_LIST_BUILD) {
          if (it->second.build) {
            addHtml(formatSystemBuildNr(it->second.build));
          }
          html_TD();
        }

        if (MAIN_PAGE_SHOW_NODE_LIST_TYPE) {
          addHtml(it->second.getNodeTypeDisplayString());
          html_TD();
        }

        if (it->second.ip[0] != 0)
        {
          html_add_wide_button_prefix();

          addHtml(F("http://"));
          addHtml(it->second.IP().toString());
          uint16_t port = it->second.webgui_portnumber;

          if ((port != 0) && (port != 80)) {
            addHtml(':');
            addHtmlInt(port);
          }
          addHtml('\'', '>');
          addHtml(it->second.IP().toString());
          addHtml(F("</a>"));
        }
        html_TD();
        const float load = it->second.getLoad();

        if (load > 0.1) {
          addHtmlFloat(load);
        }
        html_TD();
        addHtmlInt(static_cast<uint32_t>(it->second.getAge() / 1000)); // time in seconds
        #  ifdef USES_ESPEASY_NOW

        if (Settings.UseESPEasyNow()) {
          html_TD();

          if (it->second.distance != 255) {
            addHtmlInt(it->second.distance);
          }
          html_TD();

          if (it->second.ESPEasyNowPeer) {
            addHtml(F(ESPEASY_NOW_NAME));
            addHtml(' ');
            addHtml(it->second.ESPEasy_Now_MAC().toString());
            addHtml(F(" (ch: "));
            addHtmlInt(it->second.channel);
            int8_t rssi = it->second.getRSSI();

            if (rssi < 0) {
              addHtml(' ');
              addHtmlInt(rssi);
            }
            addHtml(')');
            const ESPEasy_now_traceroute_struct *trace = Nodes.getDiscoveryRoute(it->second.unit);

            if (trace != nullptr) {
              addHtml(' ');
              addHtml(trace->toString());
            }
          }
        }
        #  endif // ifdef USES_ESPEASY_NOW
      }
    }

    html_end_table();
  # endif // if FEATURE_ESPEASY_P2P
    html_end_form();

    printWebString = String();
    printToWeb     = false;
    sendHeadandTail_stdtemplate(_TAIL);
  }
  TXBuffer.endStream();
}

#endif // ifdef WEBSERVER_ROOT
