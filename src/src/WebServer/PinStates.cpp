#include "../WebServer/PinStates.h"

#include "../WebServer/WebServer.h"
#include "../WebServer/HTML_wrappers.h"

#include "../DataStructs/PinMode.h"
#include "../Globals/GlobalMapPortStatus.h"
#include "../Helpers/PortStatus.h"

#ifdef WEBSERVER_NEW_UI


// ********************************************************************************
// Web Interface pin state list
// ********************************************************************************
void handle_pinstates_json() {
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_pinstates"));
  #endif

  if (!isLoggedIn()) { return; }
  navMenuIndex = MENU_INDEX_TOOLS;
  TXBuffer.startJsonStream();

  bool first = true;
  addHtml('[');

  for (std::map<uint32_t, portStatusStruct>::iterator it = globalMapPortStatus.begin(); it != globalMapPortStatus.end(); ++it)
  {
    if (!first) {
      addHtml(',');
    } else {
      first = false;
    }
    addHtml('{');


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

  addHtml(']');

  TXBuffer.endStream();
}

#endif // WEBSERVER_NEW_UI

#ifdef WEBSERVER_PINSTATES

void handle_pinstates() {
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_pinstates"));
  #endif

  if (!isLoggedIn()) { return; }
  navMenuIndex = MENU_INDEX_TOOLS;
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate(_HEAD);

  // addFormSubHeader(F("Pin state table<TR>"));

  html_table_class_multirow();
  html_TR();
  html_table_header(F("Plugin"), F("Official_plugin_list"), 0);
  html_table_header(F("GPIO"));
  html_table_header(F("Mode"));
  html_table_header(F("Value/State"));
  html_table_header(F("Task"));
  html_table_header(F("Monitor"));
  html_table_header(F("Command"));
  html_table_header(F("Init"));

  for (std::map<uint32_t, portStatusStruct>::iterator it = globalMapPortStatus.begin(); it != globalMapPortStatus.end(); ++it)
  {
    html_TR_TD();
    addHtml('P');
    const uint16_t plugin = getPluginFromKey(it->first);
    const uint16_t port   = getPortFromKey(it->first);

    if (plugin < 100)
    {
      addHtml('0');
    }

    if (plugin < 10)
    {
      addHtml('0');
    }
    addHtmlInt(plugin);
    html_TD();
    addHtmlInt(port);
    html_TD();
    addHtml(getPinModeString(it->second.mode));
    html_TD();
    addHtmlInt(it->second.getValue());
    html_TD();
    addHtmlInt(it->second.task);
    html_TD();
    addHtmlInt(it->second.monitor);
    html_TD();
    addHtmlInt(it->second.command);
    html_TD();
    addHtmlInt(it->second.init);
  }

  html_end_table();
  sendHeadandTail_stdtemplate(_TAIL);
  TXBuffer.endStream();
}

#endif // ifdef WEBSERVER_PINSTATES
