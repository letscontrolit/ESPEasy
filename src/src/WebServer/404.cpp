#include "../WebServer/404.h"

#include "../WebServer/ESPEasy_WebServer.h"
#include "../WebServer/HTML_wrappers.h"
#include "../WebServer/LoadFromFS.h"
#include "../WebServer/Rules.h"

#include "../Globals/Services.h"
#include "../Globals/Settings.h"

#include "../Globals/ESPEasyWiFiEvent.h"

// ********************************************************************************
// Web Interface handle other requests
// ********************************************************************************
void handleNotFound() {
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handleNotFound"));
  #endif // ifndef BUILD_NO_RAM_TRACKER

  if (captivePortal()) { // If captive portal redirect instead of displaying the error page.
    return;
  }

  // if Wifi setup, launch setup wizard if AP_DONT_FORCE_SETUP is not set.
  if (WiFiEventData.wifiSetup && !Settings.ApDontForceSetup())
  {
    web_server.send_P(200, (PGM_P)F("text/html"), (PGM_P)F("<meta HTTP-EQUIV='REFRESH' content='0; url=/setup'>"));
    return;
  }

#ifdef WEBSERVER_RULES

  if (handle_rules_edit(web_server.uri())) { return; }
#endif // ifdef WEBSERVER_RULES

  if (loadFromFS(web_server.uri())) { return; }

  TXBuffer.startStream(F("text/plain"), F(""), 404);
  addHtml(F("URI: "));
  addHtml(web_server.uri());
  addHtml(F("\nMethod: "));
  addHtml((web_server.method() == HTTP_GET) ? F("GET") : F("POST"));

  addHtml(F("\nArguments: "));
  addHtmlInt(web_server.args());

#ifndef BUILD_NO_DEBUG
  for (uint8_t i = 0; i < web_server.args(); i++) {
    addHtml('\n');
    addHtml(F(" NAME:"));
    addHtml(web_server.argName(i));
    addHtml(F("\n VALUE:"));
    addHtml(webArg(i));
  }
  addHtml(F("\nHeaders: "));
  for (int i = web_server.headers(); i >= 0; --i) {
    if (!web_server.headerName(i).isEmpty()) {
      addHtml('\n');
      addHtml(F(" NAME:"));
      addHtml(web_server.headerName(i));
      addHtml(F("\n VALUE:"));
      addHtml(web_server.header(i));
    }
  }
#endif
  TXBuffer.endStream();
}
