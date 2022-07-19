#include "../Commands/HTTP.h"

#include "../../ESPEasy_common.h"

#include "../Commands/Common.h"

#include "../DataStructs/ControllerSettingsStruct.h"
#include "../DataStructs/SettingsStruct.h"

#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../ESPEasyCore/ESPEasyNetwork.h"

#include "../Globals/Settings.h"

#include "../Helpers/_CPlugin_Helper.h"
#include "../Helpers/Misc.h"
#include "../Helpers/Networking.h"
#include "../Helpers/StringParser.h"


const __FlashStringHelper* Command_HTTP_SendToHTTP(struct EventStruct *event, const char *Line)
{
  bool success = false;

  if (NetworkConnected()) {
    String user, pass;
    String host      = parseStringKeepCase(Line, 2);
    const int pos_at = host.indexOf('@');

    if (pos_at != -1) {
      user = host.substring(0, pos_at);
      host = host.substring(pos_at + 1);
      const int pos_colon = user.indexOf(':');

      if (pos_colon != -1) {
        pass = user.substring(pos_colon + 1);
        user = user.substring(0, pos_colon);
      }
    }

    const int port = parseCommandArgumentInt(Line, 2);
#ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      String log = F("SendToHTTP: Host: ");
      log += host;
      log += F(" port: ");
      log += port;
      addLogMove(LOG_LEVEL_DEBUG, log);
    }
#endif // ifndef BUILD_NO_DEBUG

    if ((port < 0) || (port > 65535)) { return return_command_failed(); }

    // FIXME TD-er: This is not using the tolerant settings option.
    // String path = tolerantParseStringKeepCase(Line, 4);
    const String path = parseStringToEndKeepCase(Line, 4);
#ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      String log = F("SendToHTTP: Path: ");
      log += path;
      addLogMove(LOG_LEVEL_DEBUG, log);
    }
#endif // ifndef BUILD_NO_DEBUG


    int httpCode = -1;
    WiFiClient   client;
    const String res = send_via_http(
      F("Command_HTTP_SendToHTTP"),
      client,
      CONTROLLER_CLIENTTIMEOUT_MAX,
      user,
      pass,
      host,
      port,
      path,
      F("GET"),
      EMPTY_STRING, // header
      EMPTY_STRING, // poststr
      httpCode);

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String logstr;
      logstr += F("SendToHTTP: ");
      logstr += httpCode;
	  if (!res.isEmpty()) {
		logstr += F(" Received reply: ");
		logstr += res;
	  }
      addLog(LOG_LEVEL_INFO, logstr);
    }

    if ((httpCode >= 100) && (httpCode < 300)) {
      return return_command_success();
    }
    addLog(LOG_LEVEL_ERROR, String(F("SendToHTTP: HTTP code: ")) + httpCode);
  } else {
    addLog(LOG_LEVEL_ERROR, F("SendToHTTP Not connected to network"));
  }
  return return_command_failed();
}
