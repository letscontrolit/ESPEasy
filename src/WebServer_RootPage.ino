#include "src/Globals/Nodes.h"

// ********************************************************************************
// Web Interface root page
// ********************************************************************************
void handle_root() {
  checkRAM(F("handle_root"));

  // if Wifi setup, launch setup wizard
  if (wifiSetup)
  {
    WebServer.send(200, F("text/html"), F("<meta HTTP-EQUIV='REFRESH' content='0; url=/setup'>"));
    return;
  }

  if (!isLoggedIn()) { return; }
  navMenuIndex = 0;

  // if index.htm exists on SPIFFS serve that one (first check if gziped version exists)
  if (loadFromFS(true, F("/index.htm.gz"))) { return; }

  if (loadFromFS(false, F("/index.htm.gz"))) { return; }

  if (loadFromFS(true, F("/index.htm"))) { return; }

  if (loadFromFS(false, F("/index.htm"))) { return; }

  TXBuffer.startStream();
  String  sCommand  = WebServer.arg(F("cmd"));
  boolean rebootCmd = strcasecmp_P(sCommand.c_str(), PSTR("reboot")) == 0;
  sendHeadandTail_stdtemplate(_HEAD, rebootCmd);

  int freeMem = ESP.getFreeHeap();


  if ((strcasecmp_P(sCommand.c_str(),
                    PSTR("wifidisconnect")) != 0) && (rebootCmd == false) && (strcasecmp_P(sCommand.c_str(), PSTR("reset")) != 0))
  {
    printToWeb     = true;
    printWebString = "";

    if (sCommand.length() > 0) {
      ExecuteCommand(VALUE_SOURCE_HTTP, sCommand.c_str());
    }

    // IPAddress ip = WiFi.localIP();
    // IPAddress gw = WiFi.gatewayIP();

    TXBuffer += printWebString;
    TXBuffer += F("<form>");
    html_table_class_normal();
    addFormHeader(F("System Info"));

    addRowLabelValue(LabelType::UNIT_NR);
    addRowLabelValue(LabelType::GIT_BUILD);
    addRowLabel(getLabel(LabelType::LOCAL_TIME));

    if (systemTimePresent())
    {
      TXBuffer += getValue(LabelType::LOCAL_TIME);
    }
    else {
      TXBuffer += F("<font color='red'>No system time source</font>");
    }

    addRowLabel(getLabel(LabelType::UPTIME));
    {
      TXBuffer += getExtendedValue(LabelType::UPTIME);
    }
    addRowLabel(getLabel(LabelType::LOAD_PCT));

    if (wdcounter > 0)
    {
      TXBuffer += String(getCPUload());
      TXBuffer += F("% (LC=");
      TXBuffer += String(getLoopCountPerSec());
      TXBuffer += ')';
    }

    addRowLabel(F("Free Mem"));
    TXBuffer += String(freeMem);
    TXBuffer += " (";
    TXBuffer += String(lowestRAM);
    TXBuffer += F(" - ");
    TXBuffer += String(lowestRAMfunction);
    TXBuffer += ')';
    addRowLabel(F("Free Stack"));
    TXBuffer += String(getCurrentFreeStack());
    TXBuffer += " (";
    TXBuffer += String(lowestFreeStack);
    TXBuffer += F(" - ");
    TXBuffer += String(lowestFreeStackfunction);
    TXBuffer += ')';

    addRowLabelValue(LabelType::IP_ADDRESS);
    addRowLabel(getLabel(LabelType::WIFI_RSSI));

    if (WiFiConnected())
    {
      TXBuffer += String(WiFi.RSSI());
      TXBuffer += F(" dB (");
      TXBuffer += WiFi.SSID();
      TXBuffer += ')';
    }

    #ifdef FEATURE_MDNS
    html_TR_TD();
    TXBuffer += F("mDNS:<TD><a href='http://");
    TXBuffer += WifiGetHostname();
    TXBuffer += F(".local'>");
    TXBuffer += WifiGetHostname();
    TXBuffer += F(".local</a>");
    html_TD(3);
    #endif // ifdef FEATURE_MDNS
    html_TR_TD();
    html_TD();
    addButton(F("sysinfo"), F("More info"));

    html_end_table();
    html_BR();
    html_BR();
    html_table_class_multirow_noborder();
    html_TR();
    html_table_header(F("Node List"));
    html_table_header("Name");
    html_table_header(getLabel(LabelType::BUILD_DESC));
    html_table_header("Type");
    html_table_header("IP", 160); // Should fit "255.255.255.255"
    html_table_header("Age");

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

        TXBuffer += F("Unit ");
        TXBuffer += String(it->first);
        html_TD();

        if (isThisUnit) {
          TXBuffer += Settings.Name;
        }
        else {
          TXBuffer += it->second.nodeName;
        }
        html_TD();

        if (it->second.build) {
          TXBuffer += String(it->second.build);
        }
        html_TD();
        TXBuffer += getNodeTypeDisplayString(it->second.nodeType);
        html_TD();
        html_add_wide_button_prefix();
        TXBuffer += F("http://");
        TXBuffer += it->second.ip.toString();
        TXBuffer += "'>";
        TXBuffer += it->second.ip.toString();
        TXBuffer += "</a>";
        html_TD();
        TXBuffer += String(it->second.age);
      }
    }

    html_end_table();
    html_end_form();

    printWebString = "";
    printToWeb     = false;
    sendHeadandTail_stdtemplate(_TAIL);
    TXBuffer.endStream();
  }
  else
  {
    // TODO: move this to handle_tools, from where it is actually called?

    // have to disconnect or reboot from within the main loop
    // because the webconnection is still active at this point
    // disconnect here could result into a crash/reboot...
    if (strcasecmp_P(sCommand.c_str(), PSTR("wifidisconnect")) == 0)
    {
      addLog(LOG_LEVEL_INFO, F("WIFI : Disconnecting..."));
      cmd_within_mainloop = CMD_WIFI_DISCONNECT;
    }

    if (strcasecmp_P(sCommand.c_str(), PSTR("reboot")) == 0)
    {
      addLog(LOG_LEVEL_INFO, F("     : Rebooting..."));
      cmd_within_mainloop = CMD_REBOOT;
    }

    if (strcasecmp_P(sCommand.c_str(), PSTR("reset")) == 0)
    {
      addLog(LOG_LEVEL_INFO, F("     : factory reset..."));
      cmd_within_mainloop = CMD_REBOOT;
      TXBuffer           += F(
        "OK. Please wait > 1 min and connect to Acces point.<BR><BR>PW=configesp<BR>URL=<a href='http://192.168.4.1'>192.168.4.1</a>");
      TXBuffer.endStream();
      ExecuteCommand(VALUE_SOURCE_HTTP, sCommand.c_str());
    }

    TXBuffer += "OK";
    TXBuffer.endStream();
  }
}
