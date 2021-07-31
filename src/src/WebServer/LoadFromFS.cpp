#include "../WebServer/LoadFromFS.h"

#include "../WebServer/WebServer.h"
#include "../WebServer/CustomPage.h"
#include "../Globals/RamTracker.h"
#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/Network.h"

#ifdef FEATURE_SD
#include <SD.h>
#endif

bool match_ext(const String& path, const __FlashStringHelper * ext) {
  return (path.endsWith(ext) || path.endsWith(String(ext) + F(".gz")));
}


// ********************************************************************************
// Web Interface server web file from FS
// ********************************************************************************
bool loadFromFS(boolean spiffs, String path) {
  // path is a deepcopy, since it will be changed here.
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("loadFromFS"));
  #endif

  statusLED(true);

  String dataType = F("text/plain");
  bool mustCheckCredentials = false;

  if (!path.startsWith(F("/"))) {
    path = String(F("/")) + path;
  }

  if (path.endsWith(F("/"))) { path += F("index.htm"); }

  if (path.endsWith(F(".src"))) { path = path.substring(0, path.lastIndexOf(".")); }
  else if (match_ext(path, F(".htm")) || match_ext(path, F(".html"))) { dataType = F("text/html"); }
  else if (match_ext(path, F(".css"))) { dataType = F("text/css"); }
  else if (match_ext(path, F(".js"))) { dataType = F("application/javascript"); }
  else if (match_ext(path, F(".png"))) { dataType = F("image/png"); }
  else if (match_ext(path, F(".gif"))) { dataType = F("image/gif"); }
  else if (match_ext(path, F(".jpg"))) { dataType = F("image/jpeg"); }
  else if (path.endsWith(F(".ico"))) { dataType = F("image/x-icon"); }
  else if (path.endsWith(F(".svg"))) { dataType = F("image/svg+xml"); }
  else if (path.endsWith(F(".json"))) { dataType = F("application/json"); }
  else if (path.endsWith(F(".txt")) ||
           path.endsWith(F(".dat"))) { 
    mustCheckCredentials = true;
    dataType = F("application/octet-stream"); 
  }
#ifdef WEBSERVER_CUSTOM
  else if (path.endsWith(F(".esp"))) {
    return handle_custom(path); 
  }
#endif

  if (mustCheckCredentials) {
    if (!isLoggedIn()) { return false; }
  }

#ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log = F("HTML : Request file ");
    log += path;
    addLog(LOG_LEVEL_DEBUG, log);
  }
#endif // ifndef BUILD_NO_DEBUG

#if !defined(ESP32)
  path = path.substring(1);
#endif // if !defined(ESP32)

  if (spiffs)
  {
    if (!fileExists(path)) {
      return false;
    }
    fs::File dataFile = tryOpenFile(path.c_str(), "r");

    if (!dataFile) {
      return false;
    }

    // prevent reloading stuff on every click
    web_server.sendHeader(F("Cache-Control"), F("max-age=3600, public"));
    web_server.sendHeader(F("Vary"),          "*");
    web_server.sendHeader(F("ETag"),          F("\"2.0.0\""));

    if (path.endsWith(F(".dat"))) {
      web_server.sendHeader(F("Content-Disposition"), F("attachment;"));
    }

    web_server.streamFile(dataFile, dataType);
    dataFile.close();
  }
  else
  {
#ifdef FEATURE_SD
    File dataFile = SD.open(path.c_str());

    if (!dataFile) {
      return false;
    }

    if (path.endsWith(F(".DAT"))) {
      web_server.sendHeader(F("Content-Disposition"), F("attachment;"));
    }
    web_server.streamFile(dataFile, dataType);
    dataFile.close();
#else // ifdef FEATURE_SD

    // File from SD requested, but no SD support.
    return false;
#endif // ifdef FEATURE_SD
  }
  statusLED(true);
  return true;
}
