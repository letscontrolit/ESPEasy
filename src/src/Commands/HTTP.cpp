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

#if FEATURE_SEND_TO_HTTP || FEATURE_POST_TO_HTTP
const __FlashStringHelper* httpEmitToHTTP(struct EventStruct        *event,
                                          const __FlashStringHelper *logIdentifier,
                                          const __FlashStringHelper *HttpMethod,
                                          const char                *Line,
                                          const int                  timeout,
                                          const bool                 waitForAck,
                                          const bool                 useHeader,
                                          const bool                 useBody)
{
  if (NetworkConnected()) {
    String   user, pass, host, file, path, header, postBody;
    uint16_t port;
    uint8_t  idx;

    const String arg1 = parseStringKeepCase(Line, 2);

    if (arg1.indexOf(F("://")) != -1) {
      // Full url given
      path = splitURL(arg1, user, pass, host, port, file);
      idx  = 3;

      if (useHeader || useBody) {
        path = parseStringKeepCase(path, 1); // Only first part is path when having header or body
      }
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
          String log = logIdentifier;
          log += F(": Invalid port argument: ");
          log += port_arg;
          log += F(" will use: ");
          log += port;
          addLogMove(LOG_LEVEL_ERROR, log);
        }
      }

      if (useHeader || useBody) {
        path = parseStringKeepCase(Line, 4);
      } else {
        path = parseStringToEndKeepCase(Line, 4);
      }
      idx = 5;
    }

    if (useHeader) {
      header = parseStringKeepCase(Line, idx);
      idx++;
    }

    if (useBody) {
      postBody = parseStringKeepCase(Line, idx);
    }
    # ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      String log = logIdentifier;
      log += F(": Host: ");
      log += host;
      log += F(" port: ");
      log += port;
      log += F(" path: ");
      log += path;
      addLogMove(LOG_LEVEL_DEBUG, log);
    }
    # endif // ifndef BUILD_NO_DEBUG

    int httpCode = -1;
    send_via_http(
      logIdentifier,
      timeout,
      user,
      pass,
      host,
      port,
      path,
      HttpMethod,
      header,
      postBody,
      httpCode,
      waitForAck);

    if ((httpCode >= 100) && (httpCode < 300)) {
      return return_command_success();
    }
  } else {
    String log = logIdentifier;
    log += F(": Not connected to network");
    addLog(LOG_LEVEL_ERROR, log);
  }
  return return_command_failed();
}

#endif // if FEATURE_SEND_TO_HTTP || FEATURE_POST_TO_HTTP

#if FEATURE_SEND_TO_HTTP

// syntax 1: SendToHttp,<[<user>:<password>@]<host>,<port>,<path>
// syntax 2: SendToHttp,http://<[<user>:<password>@]<host>[:<port>]/<path>
const __FlashStringHelper* Command_HTTP_SendToHTTP(struct EventStruct *event, const char *Line)
{
  // Some servers don't give an ack.
  // For these it is adviced to uncheck to wait for an acknowledgement.
  // However the default timeout of 4000 msec is then way too long
  // FIXME TD-er: Make sendToHttp timeout a setting.
  const int timeout = Settings.SendToHttp_ack()
         ? CONTROLLER_CLIENTTIMEOUT_MAX : 1000;

  return httpEmitToHTTP(event, F("SendToHTTP"), F("GET"), Line, timeout, Settings.SendToHttp_ack(), false, false);
}

#endif // FEATURE_SEND_TO_HTTP

#if FEATURE_POST_TO_HTTP

// syntax 1: PostToHttp,<[<user>:<password>@]<host>,<port>,<path>,<header>,<body>
// syntax 2: PostToHttp,http://<[<user>:<password>@]<host>[:<port>]/<path>,<header>,<body>
const __FlashStringHelper* Command_HTTP_PostToHTTP(struct EventStruct *event, const char *Line)
{
  // FIXME tonhuisman: Make postToHttp timeout a setting, now using a somewhat sensible default
  const int timeout = CONTROLLER_CLIENTTIMEOUT_MAX;

  // FIXME tonhuisman: make PostToHttp_ack a setting, using SendToHttp_ack for now...

  return httpEmitToHTTP(event, F("PostToHTTP"), F("POST"), Line, timeout, Settings.SendToHttp_ack(), true, true);
}

#endif // if FEATURE_POST_TO_HTTP
