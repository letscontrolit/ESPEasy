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

  addLogMove(LOG_LEVEL_INFO,  concat(F("HTTP: "), webrequest));
  webrequest = parseTemplate(webrequest);
#ifndef BUILD_NO_DEBUG
  addLogMove(LOG_LEVEL_DEBUG, concat(F("HTTP after parseTemplate: "), webrequest));
#endif // ifndef BUILD_NO_DEBUG

  bool handledCmd = false;
  bool sendOK     = false;
  printWebString = String();
  printToWeb     = false;
  printToWebJSON = false;

  // in case of event, store to buffer and return...
  const String command = parseString(webrequest, 1);

  if ((equals(command, F("event"))) || (equals(command, F("asyncevent"))))
  {
    eventQueue.addMove(parseStringToEndKeepCase(webrequest, 2));
    handledCmd = true;
    sendOK     = true;
  } else if (equals(command, F("taskrun")) ||
             equals(command, F("taskrunat")) ||
             equals(command, F("scheduletaskrun")) ||
             equals(command, F("taskvalueset")) ||
             equals(command, F("taskvaluesetandrun")) ||
             equals(command, F("taskvaluetoggle")) ||
             equals(command, F("let")) ||
             equals(command, F("logportstatus")) ||
             equals(command, F("jsonportstatus")) ||
             equals(command, F("rules"))) {
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
