#include "../WebServer/RootPage.h"


#ifdef WEBSERVER_ROOT

#include "../WebServer/WebServer.h"
#include "../WebServer/HTML_wrappers.h"
#include "../WebServer/LoadFromFS.h"
#include "../WebServer/Markup.h"
#include "../WebServer/Markup_Buttons.h"

#include "../Commands/InternalCommands.h"
#include "../ESPEasyCore/ESPEasyNetwork.h"
#include "../Globals/ESPEasy_time.h"
#include "../Globals/ESPEasyWiFiEvent.h"
#include "../Globals/MainLoopCommand.h"
#include "../Globals/NetworkState.h"
#include "../Globals/Nodes.h"
#include "../Globals/Settings.h"
#include "../Globals/Statistics.h"
#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/Memory.h"
#include "../Helpers/Misc.h"
#include "../Helpers/WebServer_commandHelper.h"


#include "../../ESPEasy-Globals.h"

#ifdef USES_MQTT
# include "../Globals/MQTT.h"
# include "../Helpers/PeriodicalActions.h" // For finding enabled MQTT controller
#endif


#ifndef MAIN_PAGE_SHOW_BASIC_INFO_NOT_LOGGED_IN
  #define MAIN_PAGE_SHOW_BASIC_INFO_NOT_LOGGED_IN false
#endif

// Define main page elements present
#ifndef MAIN_PAGE_SHOW_SYSINFO_BUTTON
  #define MAIN_PAGE_SHOW_SYSINFO_BUTTON    true
#endif

#ifndef MAIN_PAGE_SHOW_WiFi_SETUP_BUTTON
  #define MAIN_PAGE_SHOW_WiFi_SETUP_BUTTON   false
#endif

#ifndef MAIN_PAGE_SHOW_NODE_LIST_BUILD
  #define MAIN_PAGE_SHOW_NODE_LIST_BUILD   true
#endif
#ifndef MAIN_PAGE_SHOW_NODE_LIST_TYPE
  #define MAIN_PAGE_SHOW_NODE_LIST_TYPE    true
#endif


// ********************************************************************************
// Web Interface root page
// ********************************************************************************
void handle_root() {
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_root"));
  #endif

 if (captivePortal()) { // If captive portal redirect instead of displaying the page.
   return;
 }

  // if Wifi setup, launch setup wizard if AP_DONT_FORCE_SETUP is not set.
 if (WiFiEventData.wifiSetup && !Settings.ApDontForceSetup())
  {
    web_server.send(200, F("text/html"), F("<meta HTTP-EQUIV='REFRESH' content='0; url=/setup'>"));
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
  String sCommand  = webArg(F("cmd"));
  rebootCmd = strcasecmp_P(sCommand.c_str(), PSTR("reboot")) == 0;
  sendHeadandTail_stdtemplate(_HEAD, rebootCmd);
  {
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
        addHtml(String(getCPUload()));
        addHtml(F("% (LC="));
        addHtmlInt(getLoopCountPerSec());
        addHtml(')');
      }
      {
        addRowLabel(LabelType::FREE_MEM);
        addHtmlInt(freeMem);
        #ifndef BUILD_NO_RAM_TRACKER
        addHtml(F(" ("));
        addHtmlInt(lowestRAM);
        addHtml(F(" - "));
        addHtml(lowestRAMfunction);
        addHtml(')');
        #endif
      }
      {
        #ifdef USE_SECOND_HEAP
        addRowLabelValue(LabelType::FREE_HEAP_IRAM);
        #endif
      }
      {
        addRowLabel(LabelType::FREE_STACK);
        addHtmlInt(getCurrentFreeStack());
        #ifndef BUILD_NO_RAM_TRACKER
        addHtml(F(" ("));
        addHtmlInt(lowestFreeStack);
        addHtml(F(" - "));
        addHtml(lowestFreeStackfunction);
        addHtml(')');
        #endif
      }

  #ifdef HAS_ETHERNET
      addRowLabelValue(LabelType::ETH_WIFI_MODE);
  #endif

      if (!WiFiEventData.WiFiDisconnected())
      {
        addRowLabelValue(LabelType::IP_ADDRESS);
        addRowLabel(LabelType::WIFI_RSSI);
        addHtmlInt(WiFi.RSSI());
        addHtml(F(" dBm ("));
        addHtml(WiFi.SSID());
        addHtml(')');
      }

  #ifdef HAS_ETHERNET
      if(active_network_medium == NetworkMedium_t::Ethernet) {
        addRowLabelValue(LabelType::ETH_SPEED_STATE);
        addRowLabelValue(LabelType::ETH_IP_ADDRESS);
      }
  #endif

      #ifdef FEATURE_MDNS
      {
        addRowLabel(LabelType::M_DNS);
        addHtml(F("<a href='http://"));
        addHtml(getValue(LabelType::M_DNS));
        addHtml(F("'>"));
        addHtml(getValue(LabelType::M_DNS));
        addHtml(F("</a>"));
      }
      #endif // ifdef FEATURE_MDNS

      #ifdef USES_MQTT
      {
        if (validControllerIndex(firstEnabledMQTT_ControllerIndex())) {
          addRowLabel(F("MQTT Client Connected"));
          addEnabled(MQTTclient_connected);
        }
      }
      #endif


      #if MAIN_PAGE_SHOW_SYSINFO_BUTTON
      html_TR_TD();
      html_TD();
      addButton(F("sysinfo"), F("More info"));
      #endif
      #if MAIN_PAGE_SHOW_WiFi_SETUP_BUTTON
      html_TR_TD();
      html_TD();
      addButton(F("setup"), F("WiFi Setup"));
      #endif

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

      html_BR();
      if (Settings.Unit == 0 && Settings.UDPPort != 0) addFormNote(F("Warning: Unit number is 0, please change it if you want to send data to other units."));
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
      html_table_header(F("Age"));

      for (NodesMap::iterator it = Nodes.begin(); it != Nodes.end(); ++it)
      {
        if (it->second.ip[0] != 0)
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
            addHtml(Settings.Name);
          }
          else {
            addHtml(it->second.nodeName);
          }
          html_TD();

          if (MAIN_PAGE_SHOW_NODE_LIST_BUILD) {
            if (it->second.build) {
              addHtmlInt(it->second.build);
            }
            html_TD();
          }
          if (MAIN_PAGE_SHOW_NODE_LIST_TYPE) {
            addHtml(getNodeTypeDisplayString(it->second.nodeType));
            html_TD();
          }
          html_add_wide_button_prefix();
          {
            addHtml(F("http://"));
            addHtml(it->second.ip.toString());
            uint16_t port = it->second.webgui_portnumber;
            if (port !=0 && port != 80) {
              addHtml(':');
              addHtmlInt(port);
            }
            addHtml('\'', '>');
            addHtml(it->second.ip.toString());
            addHtml(F("</a>"));
          }
          html_TD();
          addHtmlInt(it->second.age);
        }
      }

      html_end_table();
      html_end_form();
    }

    printWebString = String();
    printToWeb     = false;
    sendHeadandTail_stdtemplate(_TAIL);
  }
  TXBuffer.endStream();
}

#endif // ifdef WEBSERVER_ROOT
