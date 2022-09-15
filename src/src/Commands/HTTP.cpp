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


#if FEATURE_SEND_TO_HTTP
const __FlashStringHelper* Command_HTTP_SendToHTTP(struct EventStruct *event, const char *Line)
{
  if (NetworkConnected()) {
    String   user, pass, host, file, path;
    uint16_t port;

    const String arg1 = parseStringKeepCase(Line, 2);

    if (arg1.indexOf(F("://")) != -1) {
      // Full url given
      path = splitURL(arg1, user, pass, host, port, file);
    } else {
      // Command arguments are split into: host, port, url
      if (!splitUserPass_HostPortString(
            arg1,
            user,
            pass,
            host,
            port))
      {
        return return_command_failed();
      }

      const int port_arg = event->Par2;

      if ((port_arg > 0) && (port_arg < 65536)) {
        port = port_arg;
      } else {
        if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
          String log = F("SendToHTTP: Invalid port argument: ");
          log += port_arg;
          log += F(" will use: ");
          log += port;
          addLogMove(LOG_LEVEL_ERROR, log);
        }
      }

      // FIXME TD-er: This is not using the tolerant settings option.
      // String path = tolerantParseStringKeepCase(Line, 4);
      path = parseStringToEndKeepCase(Line, 4);
    }
#ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      String log = F("SendToHTTP: Host: ");
      log += host;
      log += F(" port: ");
      log += port;
      log += F(" path: ");
      log += path;
      addLogMove(LOG_LEVEL_DEBUG, log);
    }
#endif // ifndef BUILD_NO_DEBUG

    // Some servers don't give an ack.
    // For these it is adviced to uncheck to wait for an acknowledgement.
    // However the default timeout of 4000 msec is then way too long
    const int timeout = Settings.SendToHttp_ack() 
       ? CONTROLLER_CLIENTTIMEOUT_MAX : 1000;
    // FIXME TD-er: Make sendToHttp timeout a setting.       

    int httpCode = -1;
    send_via_http(
      F("SendToHTTP"),
      timeout,
      user,
      pass,
      host,
      port,
      path,
      F("GET"),
      EMPTY_STRING, // header
      EMPTY_STRING, // poststr
      httpCode,
      Settings.SendToHttp_ack());

    if ((httpCode >= 100) && (httpCode < 300)) {
      return return_command_success();
    }
  } else {
    addLog(LOG_LEVEL_ERROR, F("SendToHTTP Not connected to network"));
  }
  return return_command_failed();
}
#endif // FEATURE_SEND_TO_HTTP
