#include "../WebServer/LoadFromFS.h"

#include "../WebServer/WebServer.h"
#include "../WebServer/CustomPage.h"
#include "../Globals/RamTracker.h"
#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/Network.h"

#ifdef FEATURE_SD
#include <SD.h>
#endif

// ********************************************************************************
// Web Interface server web file from FS
// ********************************************************************************
bool loadFromFS(boolean spiffs, String path) {
  // path is a deepcopy, since it will be changed here.
  checkRAM(F("loadFromFS"));

  if (!isLoggedIn()) { return false; }

  statusLED(true);

  String dataType = F("text/plain");

  if (path.endsWith("/")) { path += F("index.htm"); }

  if (path.endsWith(F(".src"))) { path = path.substring(0, path.lastIndexOf(".")); }
  else if (path.endsWith(F(".htm")) || path.endsWith(F(".html")) || path.endsWith(F(".htm.gz")) || path.endsWith(F(".html.gz"))) { dataType = F("text/html"); }
  else if (path.endsWith(F(".css")) || path.endsWith(F(".css.gz"))) { dataType = F("text/css"); }
  else if (path.endsWith(F(".js")) || path.endsWith(F(".js.gz"))) { dataType = F("application/javascript"); }
  else if (path.endsWith(F(".png")) || path.endsWith(F(".png.gz"))) { dataType = F("image/png"); }
  else if (path.endsWith(F(".gif")) || path.endsWith(F(".gif.gz"))) { dataType = F("image/gif"); }
  else if (path.endsWith(F(".jpg")) || path.endsWith(F(".jpg.gz"))) { dataType = F("image/jpeg"); }
  else if (path.endsWith(F(".ico"))) { dataType = F("image/x-icon"); }
  else if (path.endsWith(F(".svg"))) { dataType = F("image/svg+xml"); }
  else if (path.endsWith(F(".json"))) { dataType = F("application/json"); }
  else if (path.endsWith(F(".txt")) ||
           path.endsWith(F(".dat"))) { dataType = F("application/octet-stream"); }
  else if (path.endsWith(F(".esp"))) { return handle_custom(path); }

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
