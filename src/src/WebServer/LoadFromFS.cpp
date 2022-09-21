#include "../WebServer/LoadFromFS.h"

#include "../Globals/Cache.h"
#include "../Globals/RamTracker.h"

#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/Network.h"
#include "../Helpers/Numerical.h"

#include "../WebServer/CustomPage.h"
#include "../WebServer/HTML_wrappers.h"
#include "../WebServer/ESPEasy_WebServer.h"

#if FEATURE_SD
# include <SD.h>
#endif // if FEATURE_SD

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
  bool   static_file = false;
  bool   serve_304 = false;  // Reply with a 304 Not Modified

  path = fileFromUrl(path);

  {
    // Strip any "static_xxx_" prepended for cache purposes
    const int index_static = path.indexOf(F("static_"));
    if (index_static != -1) {
      const int index_1 = path.indexOf('_');
      const int index_2 = path.indexOf('_', index_1 + 1);
      if (index_1 > 0 && index_2 > index_1) {
        path = fileFromUrl(path.substring(index_2 + 1));
        static_file = true;
/*
        for (int i = web_server.headers(); i >= 0; --i) {
          addLog(LOG_LEVEL_INFO, concat(F("header: "), web_server.headerName(i)) + ':' + web_server.header(i));
        }
*/
        const String ifNoneMatch = stripQuotes(web_server.header(F("If-None-Match")));
        unsigned int etag_num = 0;
        if (validUIntFromString(ifNoneMatch, etag_num)) {
          if (fileExists(path)) {
            // call fileExists first, as this may update Cache.fileCacheClearMoment
            if (Cache.fileCacheClearMoment == etag_num) {
              // We have a request for the same file we served earlier.
              // Reply with a 304 Not Modified
              serve_304 = true;
              #ifndef BUILD_NO_DEBUG
              addLog(LOG_LEVEL_INFO, concat(F("Serve 304: "), etag_num));
              #endif
            }
          }
        }
      }
      #ifndef BUILD_NO_DEBUG
      addLog(LOG_LEVEL_INFO, concat(F("static_file: "), path));
      #endif
    }
  }

  if (!fileExists(path)) {
    return false;
  }

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

  // prevent reloading stuff on every click
  if (static_file) {
    web_server.sendHeader(F("Cache-Control"), F("public, max-age=31536000, immutable"));
//    web_server.sendHeader(F("Cache-Control"), F("max-age=86400"));
    web_server.sendHeader(F("Age"),           F("100"));
    web_server.sendHeader(F("ETag"),          wrap_String(String(Cache.fileCacheClearMoment) + F("-a"), '"')); // added "-a" to the ETag to match the same encoding
  } else {
    web_server.sendHeader(F("Cache-Control"), F("no-cache"));
    web_server.sendHeader(F("ETag"),          F("\"2.0.0\""));
  }
  web_server.sendHeader(F("Vary"),          "*");

  if (path.endsWith(F(".dat"))) {
    web_server.sendHeader(F("Content-Disposition"), F("attachment;"));
  }
  if (serve_304) {
    web_server.send(304, dataType, F(""));
  } else {
    fs::File f;

    // Search flash file system first, then SD if present
    f = tryOpenFile(path.c_str(), "r");
    #if FEATURE_SD
    if (!f) {
      f = SD.open(path.c_str(), "r");
    }
    #endif // if FEATURE_SD

    if (!f) {
      return false;
    }
    web_server.streamFile(f, dataType);
    f.close();
  }

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
  #if FEATURE_SD
  if (!f) {
    f = SD.open(path.c_str(), "r");
  }
  #endif // if FEATURE_SD

  if (!f) {
    return bytesStreamed;
  }

  int available = f.available();
  String escaped;
  while (available > 0) {
    int32_t chunksize = 64;
    if (available < chunksize) {
      chunksize = available;
    }
    uint8_t buf[64] = {0};
    const int read = f.read(buf, chunksize);
    if (read == chunksize) {
      for (int32_t i = 0; i < chunksize; ++i) {
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

  while (f.available()) { 
    addHtml((char)f.read()); 
  }
  statusLED(true);

  f.close();
  return bytesStreamed;
}
