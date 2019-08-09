

// ********************************************************************************
// Web Interface handle other requests
// ********************************************************************************
void handleNotFound() {
  checkRAM(F("handleNotFound"));

  if (wifiSetup)
  {
    WebServer.send(200, F("text/html"), F("<meta HTTP-EQUIV='REFRESH' content='0; url=/setup'>"));
    return;
  }

  if (!isLoggedIn()) { return; }

  if (handle_rules_edit(WebServer.uri())) { return; }

  if (loadFromFS(true, WebServer.uri())) { return; }

  if (loadFromFS(false, WebServer.uri())) { return; }
  String message = F("URI: ");
  message += WebServer.uri();
  message += F("\nMethod: ");
  message += (WebServer.method() == HTTP_GET) ? F("GET") : F("POST");
  message += F("\nArguments: ");
  message += WebServer.args();
  message += "\n";

  for (uint8_t i = 0; i < WebServer.args(); i++) {
    message += F(" NAME:");
    message += WebServer.argName(i);
    message += F("\n VALUE:");
    message += WebServer.arg(i);
    message += '\n';
  }
  WebServer.send(404, F("text/plain"), message);
}
