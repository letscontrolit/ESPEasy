
// ********************************************************************************
// Web Interface control page (no password!)
// ********************************************************************************
void handle_control() {
  checkRAM(F("handle_control"));

  if (!clientIPallowed()) { return; }

  // TXBuffer.startStream(true); // true= json
  // sendHeadandTail_stdtemplate(_HEAD);
  String webrequest = WebServer.arg(F("cmd"));

  // in case of event, store to buffer and return...
  String command = parseString(webrequest, 1);
  addLog(LOG_LEVEL_INFO,  String(F("HTTP: ")) + webrequest);
  webrequest = parseTemplate(webrequest, webrequest.length());
#ifndef BUILD_NO_DEBUG
  addLog(LOG_LEVEL_DEBUG, String(F("HTTP after parseTemplate: ")) + webrequest);
#endif // ifndef BUILD_NO_DEBUG

  bool handledCmd = false;

  if (command == F("event"))
  {
    eventBuffer = webrequest.substring(6);
    handledCmd  = true;
  }
  else if (command.equalsIgnoreCase(F("taskrun")) ||
           command.equalsIgnoreCase(F("taskvalueset")) ||
           command.equalsIgnoreCase(F("taskvaluetoggle")) ||
           command.equalsIgnoreCase(F("let")) ||
           command.equalsIgnoreCase(F("logPortStatus")) ||
           command.equalsIgnoreCase(F("jsonportstatus")) ||
           command.equalsIgnoreCase(F("rules"))) {
    ExecuteCommand(VALUE_SOURCE_HTTP, webrequest.c_str());
    handledCmd = true;
  }

  if (handledCmd) {
    TXBuffer.startStream("*");
    TXBuffer += "OK";
    TXBuffer.endStream();
    return;
  }

  struct EventStruct TempEvent;
  parseCommandString(&TempEvent, webrequest);
  TempEvent.Source = VALUE_SOURCE_HTTP;

  printToWeb     = true;
  printWebString = "";

  bool unknownCmd = false;

  if (PluginCall(PLUGIN_WRITE, &TempEvent, webrequest)) {}
  else if (remoteConfig(&TempEvent, webrequest)) {}
  else { unknownCmd = true; }

  if (printToWebJSON) { // it is setted in PLUGIN_WRITE (SendStatus)
    TXBuffer.startJsonStream();
  }
  else {
    TXBuffer.startStream();
  }

  if (unknownCmd) {
    TXBuffer += F("Unknown or restricted command!");
  }
  else {
    TXBuffer += printWebString;
  }

  TXBuffer.endStream();

  printWebString = "";
  printToWeb     = false;
  printToWebJSON = false;
}
