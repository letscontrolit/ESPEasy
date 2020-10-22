#include "../WebServer/ControlPage.h"


#ifdef WEBSERVER_CONTROL

#include "../WebServer/AccessControl.h"

#include "../WebServer/HTML_wrappers.h"

#include "../Commands/InternalCommands.h"
#include "../Helpers/StringConverter.h"
#include "../Helpers/StringParser.h"
#include "../Globals/EventQueue.h"

#include "../../ESPEasy-Globals.h"


// ********************************************************************************
// Web Interface control page (no password!)
// ********************************************************************************
void handle_control() {
  checkRAM(F("handle_control"));

  if (!clientIPallowed()) { return; }

  // TXBuffer.startStream(true); // true= json
  // sendHeadandTail_stdtemplate(_HEAD);
  String webrequest = web_server.arg(F("cmd"));
  addLog(LOG_LEVEL_INFO,  String(F("HTTP: ")) + webrequest);
  webrequest = parseTemplate(webrequest);
#ifndef BUILD_NO_DEBUG
  addLog(LOG_LEVEL_DEBUG, String(F("HTTP after parseTemplate: ")) + webrequest);
#endif // ifndef BUILD_NO_DEBUG

  bool handledCmd = false;
  bool sendOK = false;
  // in case of event, store to buffer and return...
  String command = parseString(webrequest, 1);

  if ((command == F("event")) || (command == F("asyncevent")))
  {
    eventQueue.add(parseStringToEnd(webrequest, 2));
    handledCmd  = true;
    sendOK = true;
//  }
//  else {
  } else if (command.equalsIgnoreCase(F("taskrun")) ||
           command.equalsIgnoreCase(F("taskvalueset")) ||
           command.equalsIgnoreCase(F("taskvaluetoggle")) ||
           command.equalsIgnoreCase(F("let")) ||
           command.equalsIgnoreCase(F("logPortStatus")) ||
           command.equalsIgnoreCase(F("jsonportstatus")) ||
           command.equalsIgnoreCase(F("rules"))) {
    handledCmd = ExecuteCommand_internal(EventValueSource::Enum::VALUE_SOURCE_HTTP, webrequest.c_str());
    sendOK = true;
    //handledCmd = true;
  } else {
    printToWeb     = true;
    printWebString = "";
    TXBuffer.startJsonStream();
    handledCmd = ExecuteCommand_internal(EventValueSource::Enum::VALUE_SOURCE_HTTP, webrequest.c_str());
    TXBuffer += printWebString;
    TXBuffer.endStream();
    printToWeb     = false;
    sendOK = false;
  }

  if (handledCmd) {
    if (sendOK) {
      TXBuffer.startStream("*");
      TXBuffer += "OK";
      TXBuffer.endStream();
    }
    return;
  }

  printToWeb     = true;
  printWebString = "";
  bool unknownCmd = !ExecuteCommand_plugin_config(EventValueSource::Enum::VALUE_SOURCE_HTTP, webrequest.c_str());

  if (printToWebJSON) { // it is setted in PLUGIN_WRITE (SendStatus)
    TXBuffer.startJsonStream();
  }
  else {
    TXBuffer.startStream();
  }

  if (unknownCmd) {
    addHtml(F("Unknown or restricted command!"));
  }
  else {
    addHtml(printWebString);
  }

  TXBuffer.endStream();

  printWebString = "";
  printToWeb     = false;
  printToWebJSON = false;
}

#endif // ifdef WEBSERVER_CONTROL
