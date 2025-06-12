#include "../Helpers/WebServer_commandHelper.h"

#include "../../ESPEasy-Globals.h"
#include "../Commands/ExecuteCommand.h"
#include "../Commands/InternalCommands_decoder.h"
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
  free_string(printWebString);
  printToWeb     = false;
  printToWebJSON = false;

  // in case of event, store to buffer and return...
  const String command = parseString(webrequest, 1);

  const ESPEasy_cmd_e command_e = match_ESPEasy_internal_command(command);

  if (command_e == ESPEasy_cmd_e::NotMatched) {
    // For sure not an internal command, try plugin or remote config
    printToWeb = true;
    handledCmd = ExecuteCommand_plugin_config({source, webrequest.c_str()});
    sendOK     = false;
  } else {
    if ((command_e == ESPEasy_cmd_e::event) || (command_e == ESPEasy_cmd_e::asyncevent))
    {
      eventQueue.addMove(parseStringToEndKeepCase(webrequest, 2));
      handledCmd = true;
      sendOK     = true;
    } else {
      switch (command_e) {
        case ESPEasy_cmd_e::taskrun:
        case ESPEasy_cmd_e::taskrunat:
        case ESPEasy_cmd_e::scheduletaskrun:
        case ESPEasy_cmd_e::taskvalueset:
        case ESPEasy_cmd_e::taskvaluesetandrun:
        case ESPEasy_cmd_e::taskvaluetoggle:
        case ESPEasy_cmd_e::let:
        #if FEATURE_STRING_VARIABLES
        case ESPEasy_cmd_e::letstr:
        #endif // if FEATURE_STRING_VARIABLES
#ifndef BUILD_NO_DIAGNOSTIC_COMMANDS
        case ESPEasy_cmd_e::logportstatus:
#endif
        case ESPEasy_cmd_e::logentry:
        case ESPEasy_cmd_e::rules:
          sendOK = true;
          break;
#ifndef BUILD_NO_DIAGNOSTIC_COMMANDS
        case ESPEasy_cmd_e::jsonportstatus:
          sendOK         = true;
          printToWebJSON = true;
          break;
#endif
#if FEATURE_USE_IPV6
        case ESPEasy_cmd_e::ip6:
          sendOK         = true;
          printToWebJSON = true;
          break;
#endif
        default:
          sendOK = false;
          break;
      }

      // handledCmd = true;
    } 
    if (!handledCmd) {
      printToWeb = true;
      handledCmd = ExecuteCommand_internal({source, webrequest.c_str()});
    }
  }

  if (handledCmd) {
    if (sendOK) {
      String reply = printWebString.isEmpty() ? F("OK") : printWebString;
      if (printToWebJSON) {
        removeChar(reply, '\n'); // Don't use newline in JSON.
        // Format return string of command to JSON format
        printWebString = strformat(
          F("{\"return\": %s,\"command\": %s}"),
          to_json_value(reply).c_str(),
          to_json_value(webrequest).c_str());
      } else {
        printWebString = reply;
      }
    }
    return HandledWebCommand_result::CommandHandled;
  }

  if (printToWebJSON) {
    // Format error to JSON format
    printWebString = strformat(
      F("{\"return\": \"Unknown or restricted command\",\"command\": \"%s\"}"),
      webrequest.c_str());
  }
  return HandledWebCommand_result::Unknown_or_restricted_command;
}
