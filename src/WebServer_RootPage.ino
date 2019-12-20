#ifdef WEBSERVER_ROOT

#include "src/Globals/Nodes.h"

// ********************************************************************************
// Web Interface root page
// ********************************************************************************
void handle_root(void) {
  checkRAM(F("handle_root"));

  // if Wifi setup, launch setup wizard
  if (wifiSetup)
  {
    WebServer.send(200, F("text/html"), F("<meta HTTP-EQUIV='REFRESH' content='0; url=/setup'>"));
    return;
  }

  if (!isLoggedIn(void)) { return; }
  navMenuIndex = 0;

  // if index.htm exists on SPIFFS serve that one (first check if gziped version exists)
  if (loadFromFS(true, F("/index.htm.gz"))) { return; }

  if (loadFromFS(false, F("/index.htm.gz"))) { return; }

  if (loadFromFS(true, F("/index.htm"))) { return; }

  if (loadFromFS(false, F("/index.htm"))) { return; }

  TXBuffer.startStream(void);
  String  sCommand  = WebServer.arg(F("cmd"));
  boolean rebootCmd = strcasecmp_P(sCommand.c_str(void), PSTR("reboot")) == 0;
  sendHeadandTail_stdtemplate(_HEAD, rebootCmd);

  int freeMem = ESP.getFreeHeap(void);


  if ((strcasecmp_P(sCommand.c_str(void),
                    PSTR("wifidisconnect")) != 0) && (rebootCmd == false) && (strcasecmp_P(sCommand.c_str(void), PSTR("reset")) != 0))
  {
    printToWeb     = true;
    printWebString = "";

    if (sCommand.length(void) > 0) {
      ExecuteCommand_internal(VALUE_SOURCE_HTTP, sCommand.c_str(void));
    }

    // IPAddress ip = WiFi.localIP(void);
    // IPAddress gw = WiFi.gatewayIP(void);

    TXBuffer += printWebString;
    TXBuffer += F("<form>");
    html_table_class_normal(void);
    addFormHeader(F("System Info"));

    addRowLabelValue(LabelType::UNIT_NR);
    addRowLabelValue(LabelType::GIT_BUILD);
    addRowLabel(getLabel(LabelType::LOCAL_TIME));

    if (systemTimePresent(void))
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
      TXBuffer += String(getCPUload(void));
      TXBuffer += F("% (LC=");
      TXBuffer += String(getLoopCountPerSec(void));
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
    TXBuffer += String(getCurrentFreeStack(void));
    TXBuffer += " (";
    TXBuffer += String(lowestFreeStack);
    TXBuffer += F(" - ");
    TXBuffer += String(lowestFreeStackfunction);
    TXBuffer += ')';

    addRowLabelValue(LabelType::IP_ADDRESS);
    addRowLabel(getLabel(LabelType::WIFI_RSSI));

    if (WiFiConnected(void))
    {
      TXBuffer += String(WiFi.RSSI(void));
      TXBuffer += F(" dB (");
      TXBuffer += WiFi.SSID(void);
      TXBuffer += ')';
    }

    #ifdef FEATURE_MDNS
    addRowLabel(getLabel(LabelType::M_DNS));
    TXBuffer += F("<a href='http://");
    TXBuffer += getValue(LabelType::M_DNS);
    TXBuffer += F("'>");
    TXBuffer += getValue(LabelType::M_DNS);
    TXBuffer += F("</a>");
    #endif // ifdef FEATURE_MDNS
    html_TR_TD(void);
    html_TD(void);
    addButton(F("sysinfo"), F("More info"));

    html_end_table(void);
    html_BR(void);
    html_BR(void);
    html_table_class_multirow_noborder(void);
    html_TR(void);
    html_table_header(F("Node List"));
    html_table_header("Name");
    html_table_header(getLabel(LabelType::BUILD_DESC));
    html_table_header("Type");
    html_table_header("IP", 160); // Should fit "255.255.255.255"
    html_table_header("Age");

    for (NodesMap::iterator it = Nodes.begin(void); it != Nodes.end(void); ++it)
    {
      if (it->second.ip[0] != 0)
      {
        bool isThisUnit = it->first == Settings.Unit;

        if (isThisUnit) {
          html_TR_TD_highlight(void);
        }
        else {
          html_TR_TD(void);
        }

        TXBuffer += F("Unit ");
        TXBuffer += String(it->first);
        html_TD(void);

        if (isThisUnit) {
          TXBuffer += Settings.Name;
        }
        else {
          TXBuffer += it->second.nodeName;
        }
        html_TD(void);

        if (it->second.build) {
          TXBuffer += String(it->second.build);
        }
        html_TD(void);
        TXBuffer += getNodeTypeDisplayString(it->second.nodeType);
        html_TD(void);
        html_add_wide_button_prefix(void);
        TXBuffer += F("http://");
        TXBuffer += it->second.ip.toString(void);
        TXBuffer += "'>";
        TXBuffer += it->second.ip.toString(void);
        TXBuffer += "</a>";
        html_TD(void);
        TXBuffer += String(it->second.age);
      }
    }

    html_end_table(void);
    html_end_form(void);

    printWebString = "";
    printToWeb     = false;
    sendHeadandTail_stdtemplate(_TAIL);
    TXBuffer.endStream(void);
  }
  else
  {
    // TODO: move this to handle_tools, from where it is actually called?

    // have to disconnect or reboot from within the main loop
    // because the webconnection is still active at this point
    // disconnect here could result into a crash/reboot...
    if (strcasecmp_P(sCommand.c_str(void), PSTR("wifidisconnect")) == 0)
    {
      addLog(LOG_LEVEL_INFO, F("WIFI : Disconnecting..."));
      cmd_within_mainloop = CMD_WIFI_DISCONNECT;
    }

    if (strcasecmp_P(sCommand.c_str(void), PSTR("reboot")) == 0)
    {
      addLog(LOG_LEVEL_INFO, F("     : Rebooting..."));
      cmd_within_mainloop = CMD_REBOOT;
    }

    if (strcasecmp_P(sCommand.c_str(void), PSTR("reset")) == 0)
    {
      addLog(LOG_LEVEL_INFO, F("     : factory reset..."));
      cmd_within_mainloop = CMD_REBOOT;
      TXBuffer           += F(
        "OK. Please wait > 1 min and connect to Acces point.<BR><BR>PW=configesp<BR>URL=<a href='http://192.168.4.1'>192.168.4.1</a>");
      TXBuffer.endStream(void);
      ExecuteCommand_internal(VALUE_SOURCE_HTTP, sCommand.c_str(void));
    }

    TXBuffer += "OK";
    TXBuffer.endStream(void);
  }
}

#endif // ifdef WEBSERVER_ROOT
