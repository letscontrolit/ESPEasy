#include "src/Globals/Nodes.h"
#include "src/Globals/Device.h"

// ********************************************************************************
// Web Interface custom page handler
// ********************************************************************************
boolean handle_custom(String path) {
  // path is a deepcopy, since it will be changed.
  checkRAM(F("handle_custom"));

  if (!clientIPallowed()) { return false; }

#if !defined(ESP32)
  path = path.substring(1);
#endif // if !defined(ESP32)

  // create a dynamic custom page, parsing task values into [<taskname>#<taskvalue>] placeholders and parsing %xx% system variables
  fs::File   dataFile      = tryOpenFile(path.c_str(), "r");
  const bool dashboardPage = path.startsWith(F("dashboard"));

  if (!dataFile && !dashboardPage) {
    return false;    // unknown file that does not exist...
  }

  if (dashboardPage) // for the dashboard page, create a default unit dropdown selector
  {
    // handle page redirects to other unit's as requested by the unit dropdown selector
    byte unit    = getFormItemInt(F("unit"));
    byte btnunit = getFormItemInt(F("btnunit"));

    if (!unit) { unit = btnunit; // unit element prevails, if not used then set to btnunit
    }

    if (unit && (unit != Settings.Unit))
    {
      NodesMap::iterator it = Nodes.find(unit);

      if (it != Nodes.end()) {
        TXBuffer.startStream();
        sendHeadandTail(F("TmplDsh"), _HEAD);
        TXBuffer += F("<meta http-equiv=\"refresh\" content=\"0; URL=http://");
        TXBuffer += it->second.ip.toString();
        TXBuffer += F("/dashboard.esp\">");
        sendHeadandTail(F("TmplDsh"), _TAIL);
        TXBuffer.endStream();
        return true;
      }
    }

    TXBuffer.startStream();
    sendHeadandTail(F("TmplDsh"), _HEAD);
    html_add_autosubmit_form();
    html_add_form();

    // create unit selector dropdown
    addSelector_Head(F("unit"), true);
    byte choice = Settings.Unit;

    for (NodesMap::iterator it = Nodes.begin(); it != Nodes.end(); ++it)
    {
      if ((it->second.ip[0] != 0) || (it->first == Settings.Unit))
      {
        String name = String(it->first) + F(" - ");

        if (it->first != Settings.Unit) {
          name += it->second.nodeName;
        }
        else {
          name += Settings.Name;
        }
        addSelector_Item(name, it->first, choice == it->first, false, "");
      }
    }
    addSelector_Foot();

    // create <> navigation buttons
    byte prev = Settings.Unit;
    byte next = Settings.Unit;
    NodesMap::iterator it;

    for (byte x = Settings.Unit - 1; x > 0; x--) {
      it = Nodes.find(x);

      if (it != Nodes.end()) {
        if (it->second.ip[0] != 0) { prev = x; break; }
      }
    }

    for (byte x = Settings.Unit + 1; x < UNIT_NUMBER_MAX; x++) {
      it = Nodes.find(x);

      if (it != Nodes.end()) {
        if (it->second.ip[0] != 0) { next = x; break; }
      }
    }

    html_add_button_prefix();
    TXBuffer += path;
    TXBuffer += F("?btnunit=");
    TXBuffer += prev;
    TXBuffer += F("'>&lt;</a>");
    html_add_button_prefix();
    TXBuffer += path;
    TXBuffer += F("?btnunit=");
    TXBuffer += next;
    TXBuffer += F("'>&gt;</a>");
  }

  // handle commands from a custom page
  String webrequest = WebServer.arg(F("cmd"));

  if (webrequest.length() > 0) {
    struct EventStruct TempEvent;
    parseCommandString(&TempEvent, webrequest);
    TempEvent.Source = VALUE_SOURCE_HTTP;

    if (PluginCall(PLUGIN_WRITE, &TempEvent, webrequest)) {}
    else if (remoteConfig(&TempEvent, webrequest)) {}
    else if (webrequest.startsWith(F("event"))) {
      ExecuteCommand(VALUE_SOURCE_HTTP, webrequest.c_str());
    }

    // handle some update processes first, before returning page update...
    String dummy;
    PluginCall(PLUGIN_TEN_PER_SECOND, 0, dummy);
  }


  if (dataFile)
  {
    String page = "";
    page.reserve(dataFile.size());

    while (dataFile.available()) {
      page += ((char)dataFile.read());
    }

    TXBuffer += parseTemplate(page, 0);
    dataFile.close();
  }
  else // if the requestef file does not exist, create a default action in case the page is named "dashboard*"
  {
    if (dashboardPage)
    {
      // if the custom page does not exist, create a basic task value overview page in case of dashboard request...
      TXBuffer += F(
        "<meta name='viewport' content='width=width=device-width, initial-scale=1'><STYLE>* {font-family:sans-serif; font-size:16pt;}.button {margin:4px; padding:4px 16px; background-color:#07D; color:#FFF; text-decoration:none; border-radius:4px}</STYLE>");
      html_table_class_normal();

      for (byte x = 0; x < TASKS_MAX; x++)
      {
        if (Settings.TaskDeviceNumber[x] != 0)
        {
          LoadTaskSettings(x);
          byte DeviceIndex = getDeviceIndex(Settings.TaskDeviceNumber[x]);
          html_TR_TD();
          TXBuffer += ExtraTaskSettings.TaskDeviceName;

          for (byte varNr = 0; varNr < VARS_PER_TASK; varNr++)
          {
            if ((Settings.TaskDeviceNumber[x] != 0) && (varNr < Device[DeviceIndex].ValueCount) &&
                (ExtraTaskSettings.TaskDeviceValueNames[varNr][0] != 0))
            {
              if (varNr > 0) {
                html_TR_TD();
              }
              html_TD();
              TXBuffer += ExtraTaskSettings.TaskDeviceValueNames[varNr];
              html_TD();
              TXBuffer += String(UserVar[x * VARS_PER_TASK + varNr], ExtraTaskSettings.TaskDeviceValueDecimals[varNr]);
            }
          }
        }
      }
    }
  }
  sendHeadandTail(F("TmplDsh"), _TAIL);
  TXBuffer.endStream();
  return true;
}
