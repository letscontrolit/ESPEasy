#include "../WebServer/LoadFromFS.h"

#include "../Globals/RamTracker.h"

#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/Network.h"

#include "../WebServer/CustomPage.h"
#include "../WebServer/HTML_wrappers.h"
#include "../WebServer/WebServer.h"

#ifdef FEATURE_SD
# include <SD.h>
#endif // ifdef FEATURE_SD

bool match_ext(const String& path, const __FlashStringHelper *ext) {
  return path.endsWith(ext) || path.endsWith(String(ext) + F(".gz"));
}

bool gzipEncoded(const String& path) {
  return path.endsWith(F(".gz"));
}

String fileFromUrl(String path) {
  const int questionmarkPos = path.indexOf('?');

  if (questionmarkPos >= 0) {
    path = path.substring(0, questionmarkPos);
  }

  // First prepend slash
  if (!path.startsWith(F("/"))) {
    path = String('/') + path;
  }

  if (path.endsWith(F("/"))) { path += F("index.htm"); }

  #ifdef ESP8266
  // Remove leading slash to generate filename from it.
  if (path.startsWith(F("/"))) {
    path = path.substring(1);
  }
  #endif

  return path;
}

// ********************************************************************************
// Web Interface server web file from FS
// ********************************************************************************
bool loadFromFS(String path) {
  // path is a deepcopy, since it will be changed here.
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("loadFromFS"));
  #endif // ifndef BUILD_NO_RAM_TRACKER

  statusLED(true);

  const __FlashStringHelper* dataType = F("text/plain");
  bool   mustCheckCredentials = false;

  path = fileFromUrl(path);

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
    dataType             = F("application/octet-stream");
  }
#ifdef WEBSERVER_CUSTOM
  else if (path.endsWith(F(".esp"))) {
    return handle_custom(path);
  }
#endif // ifdef WEBSERVER_CUSTOM
  else {
    mustCheckCredentials = true;
  }

  if (mustCheckCredentials) {
    if (!isLoggedIn()) { return false; }
  }

#ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log = F("HTML : Request file ");
    log += path;
    addLogMove(LOG_LEVEL_DEBUG, log);
  }
#endif // ifndef BUILD_NO_DEBUG

  fs::File f;

  // Search flash file system first, then SD if present
  f = tryOpenFile(path.c_str(), "r");
  #ifdef FEATURE_SD
  if (!f) {
    f = SD.open(path.c_str(), "r");
  }
  #endif // ifdef FEATURE_SD

  if (!f) {
    return false;
  }

  // prevent reloading stuff on every click
  web_server.sendHeader(F("Cache-Control"), F("max-age=3600, public"));
  web_server.sendHeader(F("Vary"),          "*");
  web_server.sendHeader(F("ETag"),          F("\"2.0.0\""));

  if (path.endsWith(F(".dat"))) {
    web_server.sendHeader(F("Content-Disposition"), F("attachment;"));
  }

  web_server.streamFile(f, dataType);
  f.close();

  statusLED(true);
  return true;
}

size_t streamFromFS(String path, bool htmlEscape) {
  // path is a deepcopy, since it will be changed here.
  path = fileFromUrl(path);
  statusLED(true);

  size_t bytesStreamed = 0;

  fs::File f;

  // Search flash file system first, then SD if present
  f = tryOpenFile(path.c_str(), "r");
  #ifdef FEATURE_SD
  if (!f) {
    f = SD.open(path.c_str(), "r");
  }
  #endif // ifdef FEATURE_SD

  if (!f) {
    return bytesStreamed;
  }

  int available = f.available();
  String escaped;
  while (available > 0) {
    uint32_t chunksize = 64;
    if (available < static_cast<int>(chunksize)) {
      chunksize = available;
    }
    uint8_t buf[64] = {0};
    const size_t read = f.read(buf, chunksize);
    if (read == chunksize) {
      for (uint32_t i = 0; i < chunksize; ++i) {
        const char c = (char)buf[i];
        if (htmlEscape && htmlEscapeChar(c, escaped)) {
          addHtml(escaped);
        } else {
          addHtml(c);
        }
      }
      bytesStreamed += read;
      available = f.available();
    } else {
      available = 0;
    }
  }
  statusLED(true);

  f.close();
  return bytesStreamed;
}
