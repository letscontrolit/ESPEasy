#include "../Helpers/WebServer_commandHelper.h"

#include "../../ESPEasy-Globals.h"
#include "../Commands/InternalCommands.h"
#include "../Globals/EventQueue.h"
#include "../Helpers/StringConverter.h"
#include "../Helpers/StringParser.h"
#include "../WebServer/AccessControl.h"


HandledWebCommand_result handle_command_from_web(EventValueSource::Enum source, String& webrequest)
{
  if (!clientIPallowed()) { return HandledWebCommand_result::IP_not_allowed; }
  webrequest.trim();
  if (webrequest.isEmpty()) { return HandledWebCommand_result::NoCommand; }

  addLog(LOG_LEVEL_INFO,  String(F("HTTP: ")) + webrequest);
  webrequest = parseTemplate(webrequest);
#ifndef BUILD_NO_DEBUG
  addLog(LOG_LEVEL_DEBUG, String(F("HTTP after parseTemplate: ")) + webrequest);
#endif // ifndef BUILD_NO_DEBUG

  bool handledCmd = false;
  bool sendOK     = false;
  printWebString = String();
  printToWeb     = false;
  printToWebJSON = false;

  // in case of event, store to buffer and return...
  String command = parseString(webrequest, 1);

  if ((command.equals(F("event"))) || (command.equals(F("asyncevent"))))
  {
    eventQueue.addMove(parseStringToEndKeepCase(webrequest, 2));
    handledCmd = true;
    sendOK     = true;
  } else if (command.equals(F("taskrun")) ||
             command.equals(F("taskvalueset")) ||
             command.equals(F("taskvaluesetandrun")) ||
             command.equals(F("taskvaluetoggle")) ||
             command.equals(F("let")) ||
             command.equals(F("logportstatus")) ||
             command.equals(F("jsonportstatus")) ||
             command.equals(F("rules"))) {
    printToWeb = true;
    handledCmd = ExecuteCommand_internal(source, webrequest.c_str());
    sendOK     = true;

    // handledCmd = true;
  } else {
    printToWeb     = true;
    handledCmd     = ExecuteCommand_all_config(source, webrequest.c_str());
    sendOK         = false;
  }

  if (handledCmd) {
    if (sendOK) {
      String reply = printWebString.isEmpty() ? F("OK") : printWebString;
      removeChar(reply, '\n'); // Don't use newline in JSON.
      if (printToWebJSON) {
        // Format "OK" to JSON format
        printWebString = F("{\"return\": \"");
        printWebString += reply;
        printWebString += F("\",\"command\": \"");
        printWebString += webrequest;
        printWebString += F("\"}");
      } else {
        printWebString = reply;
      }
    }
    return HandledWebCommand_result::CommandHandled;
  }

  if (printToWebJSON) {
    // Format error to JSON format
    printWebString = F("{\"return\": \"");
    printWebString += F("Unknown or restricted command");
    printWebString += F("\",\"command\": \"");
    printWebString += webrequest;
    printWebString += F("\"}");
  }
  return HandledWebCommand_result::Unknown_or_restricted_command;
}
