#include "../WebServer/404.h"

#include "../WebServer/WebServer.h"
#include "../WebServer/LoadFromFS.h"
#include "../WebServer/Rules.h"

#include "../Globals/Services.h"

#include "../Globals/ESPEasyWiFiEvent.h"

// ********************************************************************************
// Web Interface handle other requests
// ********************************************************************************
void handleNotFound() {
  checkRAM(F("handleNotFound"));

  if (WiFiEventData.wifiSetup)
  {
    web_server.send(200, F("text/html"), F("<meta HTTP-EQUIV='REFRESH' content='0; url=/setup'>"));
    return;
  }

  if (!isLoggedIn()) { return; }

#ifdef WEBSERVER_RULES
  if (handle_rules_edit(web_server.uri())) { return; }
#endif

  if (loadFromFS(true, web_server.uri())) { return; }

  if (loadFromFS(false, web_server.uri())) { return; }
  String message = F("URI: ");
  message += web_server.uri();
  message += F("\nMethod: ");
  message += (web_server.method() == HTTP_GET) ? F("GET") : F("POST");
  message += F("\nArguments: ");
  message += web_server.args();
  message += "\n";

  for (uint8_t i = 0; i < web_server.args(); i++) {
    message += F(" NAME:");
    message += web_server.argName(i);
    message += F("\n VALUE:");
    message += web_server.arg(i);
    message += '\n';
  }
  web_server.send(404, F("text/plain"), message);
}
