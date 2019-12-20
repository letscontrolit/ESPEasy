

// ********************************************************************************
// Web Interface handle other requests
// ********************************************************************************
void handleNotFound(void) {
  checkRAM(F("handleNotFound"));

  if (wifiSetup)
  {
    WebServer.send(200, F("text/html"), F("<meta HTTP-EQUIV='REFRESH' content='0; url=/setup'>"));
    return;
  }

  if (!isLoggedIn(void)) { return; }

#ifdef WEBSERVER_RULES
  if (handle_rules_edit(WebServer.uri(void))) { return; }
#endif

  if (loadFromFS(true, WebServer.uri(void))) { return; }

  if (loadFromFS(false, WebServer.uri(void))) { return; }
  String message = F("URI: ");
  message += WebServer.uri(void);
  message += F("\nMethod: ");
  message += (WebServer.method(void) == HTTP_GET) ? F("GET") : F("POST");
  message += F("\nArguments: ");
  message += WebServer.args(void);
  message += "\n";

  for (uint8_t i = 0; i < WebServer.args(void); i++) {
    message += F(" NAME:");
    message += WebServer.argName(i);
    message += F("\n VALUE:");
    message += WebServer.arg(i);
    message += '\n';
  }
  WebServer.send(404, F("text/plain"), message);
}
