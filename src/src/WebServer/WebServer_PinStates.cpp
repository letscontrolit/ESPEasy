#include "src/Helpers/PortStatus.h"

#ifdef WEBSERVER_NEW_UI


// ********************************************************************************
// Web Interface pin state list
// ********************************************************************************
void handle_pinstates_json() {
  checkRAM(F("handle_pinstates"));

  if (!isLoggedIn()) { return; }
  navMenuIndex = MENU_INDEX_TOOLS;
  TXBuffer.startJsonStream();

  bool first = true;
  addHtml(F("["));

  for (std::map<uint32_t, portStatusStruct>::iterator it = globalMapPortStatus.begin(); it != globalMapPortStatus.end(); ++it)
  {
    if (!first) {
      addHtml(",");
    } else {
      first = false;
    }
    addHtml("{");


    const uint16_t plugin = getPluginFromKey(it->first);
    const uint16_t port   = getPortFromKey(it->first);

    stream_next_json_object_value(F("plugin"),  String(plugin));
    stream_next_json_object_value(F("port"),    String(port));
    stream_next_json_object_value(F("state"),   String(it->second.state));
    stream_next_json_object_value(F("task"),    String(it->second.task));
    stream_next_json_object_value(F("monitor"), String(it->second.monitor));
    stream_next_json_object_value(F("command"), String(it->second.command));
    stream_last_json_object_value(F("init"), String(it->second.init));
  }

  addHtml(F("]"));


  /*
     html_table_header(F("Plugin"), F("Official_plugin_list"), 0);
     html_table_header("GPIO");
     html_table_header("Mode");
     html_table_header(F("Value/State"));
     for (byte x = 0; x < PINSTATE_TABLE_MAX; x++)
      if (pinStates[x].plugin != 0)
      {
        html_TR_TD(); TXBuffer += "P";
        if (pinStates[x].plugin < 100)
        {
          addHtml("0");
        }
        if (pinStates[x].plugin < 10)
        {
          addHtml("0");
        }
        TXBuffer += pinStates[x].plugin;
        html_TD();
        TXBuffer += pinStates[x].index;
        html_TD();
        byte mode = pinStates[x].mode;
        TXBuffer += getPinModeString(mode);
        html_TD();
        TXBuffer += pinStates[x].value;
      }
   */

  TXBuffer.endStream();
}

#endif // WEBSERVER_NEW_UI

#ifdef WEBSERVER_PINSTATES

void handle_pinstates() {
  checkRAM(F("handle_pinstates"));

  if (!isLoggedIn()) { return; }
  navMenuIndex = MENU_INDEX_TOOLS;
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate(_HEAD);

  // addFormSubHeader(F("Pin state table<TR>"));

  html_table_class_multirow();
  html_TR();
  html_table_header(F("Plugin"), F("Official_plugin_list"), 0);
  html_table_header("GPIO");
  html_table_header("Mode");
  html_table_header(F("Value/State"));
  html_table_header(F("Task"));
  html_table_header(F("Monitor"));
  html_table_header(F("Command"));
  html_table_header("Init");

  for (std::map<uint32_t, portStatusStruct>::iterator it = globalMapPortStatus.begin(); it != globalMapPortStatus.end(); ++it)
  {
    html_TR_TD();
    addHtml("P");
    const uint16_t plugin = getPluginFromKey(it->first);
    const uint16_t port   = getPortFromKey(it->first);

    if (plugin < 100)
    {
      addHtml("0");
    }

    if (plugin < 10)
    {
      addHtml("0");
    }
    addHtml(String(plugin));
    html_TD();
    addHtml(String(port));
    html_TD();
    addHtml(getPinModeString(it->second.mode));
    html_TD();
    addHtml(String(it->second.state));
    html_TD();
    addHtml(String(it->second.task));
    html_TD();
    addHtml(String(it->second.monitor));
    html_TD();
    addHtml(String(it->second.command));
    html_TD();
    addHtml(String(it->second.init));
  }


  /*
     html_table_header(F("Plugin"), F("Official_plugin_list"), 0);
     html_table_header("GPIO");
     html_table_header("Mode");
     html_table_header(F("Value/State"));
     for (byte x = 0; x < PINSTATE_TABLE_MAX; x++)
      if (pinStates[x].plugin != 0)
      {
        html_TR_TD(); TXBuffer += "P";
        if (pinStates[x].plugin < 100)
        {
          addHtml("0");
        }
        if (pinStates[x].plugin < 10)
        {
          addHtml("0");
        }
        TXBuffer += pinStates[x].plugin;
        html_TD();
        TXBuffer += pinStates[x].index;
        html_TD();
        byte mode = pinStates[x].mode;
        TXBuffer += getPinModeString(mode);
        html_TD();
        TXBuffer += pinStates[x].value;
      }
   */
  html_end_table();
  sendHeadandTail_stdtemplate(_TAIL);
  TXBuffer.endStream();
}

#endif // ifdef WEBSERVER_PINSTATES
