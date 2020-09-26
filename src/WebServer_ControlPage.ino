
#ifdef WEBSERVER_CONTROL

#include "src/Commands/InternalCommands.h"
#include "src/Helpers/StringParser.h"


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

  // in case of event, store to buffer and return...
  String command = parseString(webrequest, 1);

  if ((command == F("event")) || (command == F("asyncevent")))
  {
    eventQueue.add(parseStringToEnd(webrequest, 2));
    handledCmd = true;
  }
  else if (command.equalsIgnoreCase(F("taskrun")) ||
           command.equalsIgnoreCase(F("taskvalueset")) ||
           command.equalsIgnoreCase(F("taskvaluetoggle")) ||
           command.equalsIgnoreCase(F("let")) ||
           command.equalsIgnoreCase(F("logPortStatus")) ||
           command.equalsIgnoreCase(F("jsonportstatus")) ||
           command.equalsIgnoreCase(F("rules"))) {
    ExecuteCommand_internal(EventValueSource::Enum::VALUE_SOURCE_HTTP, webrequest.c_str());
    handledCmd = true;
  }

  if (handledCmd) {
    TXBuffer.startStream("*");
    addHtml(F("OK"));
    TXBuffer.endStream();
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
