#include "../WebServer/LoadFromFS.h"

#include "../CustomBuild/CompiletimeDefines.h"

#include "../Globals/Cache.h"
#include "../Globals/RamTracker.h"

#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/Network.h"
#include "../Helpers/Numerical.h"

#include "../WebServer/CustomPage.h"
#include "../WebServer/HTML_wrappers.h"
#include "../WebServer/ESPEasy_WebServer.h"
#include "../Static/WebStaticData.h"

// ********************************************************************************
// Helper functions to match filenames
// ********************************************************************************
bool gzipEncoded(const String& path) {
  return path.endsWith(F(".gz"));
}

bool match_ext(const String& path, const __FlashStringHelper *ext) {
  return path.endsWith(ext) ||
         (gzipEncoded(path) && path.endsWith(concat(ext, F(".gz"))));
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
  #endif // ifdef ESP8266

  return path;
}

bool matchFilename(const String& path, const __FlashStringHelper *fname) {
  return fileFromUrl(path).equalsIgnoreCase(fileFromUrl(fname));
}

// ********************************************************************************
// Handle embedded files
// ********************************************************************************
bool fileIsEmbedded(const String& path) {
#if defined(EMBED_ESPEASY_DEFAULT_MIN_CSS) || defined(WEBSERVER_EMBED_CUSTOM_CSS)

  if (matchFilename(path, F("esp.css"))) {
    return true;
  }
#endif // if defined(EMBED_ESPEASY_DEFAULT_MIN_CSS) || defined(WEBSERVER_EMBED_CUSTOM_CSS)
#ifdef WEBSERVER_FAVICON

  if (matchFilename(path, F("favicon.ico"))) {
    return true;
  }
#endif // ifdef WEBSERVER_FAVICON
  return false;
}

void do_serveEmbedded(const __FlashStringHelper* contentType, PGM_P content, int length, bool serve_inline) {
  // Serve using our own Web_StreamingBuffer
  // Serving via web_server.send_P may cause memory allocation issues when sending large flash strings.
  if (!serve_inline) {
    TXBuffer.startStream(contentType, F("*"), 200);
  }
  TXBuffer.addFlashString(content, length);
  if (!serve_inline) {
    TXBuffer.endStream();
  }
}

void serveEmbedded(const String& path, const __FlashStringHelper* contentType, bool serve_inline) {
#if defined(EMBED_ESPEASY_DEFAULT_MIN_CSS) || defined(WEBSERVER_EMBED_CUSTOM_CSS)

  if (matchFilename(path, F("esp.css"))) {
    #ifdef EMBED_ESPEASY_DEFAULT_MIN_CSS_USE_GZ
    sendHeader(F("Content-Encoding"), F("gzip"));
    do_serveEmbedded(contentType, (PGM_P)FPSTR(DATA_ESPEASY_DEFAULT_MIN_CSS_GZ), espeasy_default_min_css_gz_len, serve_inline);
    #else
    do_serveEmbedded(contentType, (PGM_P)FPSTR(DATA_ESPEASY_DEFAULT_MIN_CSS), -1, serve_inline);
    #endif
    return;
  }
#endif // if defined(EMBED_ESPEASY_DEFAULT_MIN_CSS) || defined(WEBSERVER_EMBED_CUSTOM_CSS)
#ifdef WEBSERVER_FAVICON

  if (matchFilename(path, F("favicon.ico"))) {
    do_serveEmbedded(contentType, (PGM_P)FPSTR(favicon_8b_ico), favicon_8b_ico_len, false);
    return;
  }
#endif // ifdef WEBSERVER_FAVICON
  addLog(LOG_LEVEL_ERROR, concat(F("serveEmbedded failed: "), path));
}


void serve_CSS_inline() {
  const __FlashStringHelper* fname = F("esp.css");
  addHtml(F("<style>"));
  if (fileExists(fname)) {
    streamFromFS(fname);
  } else {
    serveEmbedded(fname, F("text/css"), true);
  }
  addHtml(F("</style>"));
}

// ********************************************************************************
// Match static files and strip "static_xxx_" prefix
// ********************************************************************************
bool isStaticFile_StripPrefix(String& path) {
  // Strip any "static_xxx_" prepended for cache purposes
  const int index_static = path.indexOf(F("static_"));

  if (index_static != -1) {
    const int index_1 = path.indexOf('_');
    const int index_2 = path.indexOf('_', index_1 + 1);

    if ((index_1 > 0) && (index_2 > index_1)) {
      path = fileFromUrl(path.substring(index_2 + 1));
      return true;
    }
  }

  if (matchFilename(path, F("favicon.ico"))) {
    return true;
  }
  return false;
}

// ********************************************************************************
// Check whether a static file is requested again 
// and therefore should be read from browser cache
// ********************************************************************************
bool reply_304_not_modified(const String& path) {
  if (path.endsWith(F("favicon.ico"))) {
    // No need in serving 304 for the favicon
    return false;
  }
  const String ifNoneMatch = stripQuotes(web_server.header(F("If-None-Match")));
  unsigned int etag_num    = 0;
  bool res                 = false;

  if (validUIntFromString(ifNoneMatch, etag_num)) {
    if (fileExists(path) || fileIsEmbedded(path)) {
      // call fileExists first, as this may update Cache.fileCacheClearMoment
      if (Cache.fileCacheClearMoment == etag_num) {
        // We have a request for the same file we served earlier.
        // Reply with a 304 Not Modified
        res = true;
      }
    }
  }
#ifndef BUILD_NO_DEBUG

  if (res) {
    addLog(LOG_LEVEL_INFO, concat(F("Serve 304: "), String(etag_num)) + ' ' + path);
  }
#endif // ifndef BUILD_NO_DEBUG

  return res;
}

// ********************************************************************************
// Determine HTTP content type
// ********************************************************************************
const __FlashStringHelper* get_ContentType(String& path, bool& mustCheckCredentials)
{
  mustCheckCredentials = false;
  const __FlashStringHelper *contentType = F("text/plain");

  if (path.endsWith(F(".src"))) {
    // FIXME TD-er: is this ".src" still used by anyone? Better remove it.
    path = path.substring(0, path.lastIndexOf("."));

    // Must check credentials, or else someone may download any file by simply adding .src in the GET url.
    mustCheckCredentials = true;
  }
  else if (match_ext(path, F(".html")) ||
           match_ext(path, F(".htm"))) { contentType = F("text/html"); }
  else if (match_ext(path, F(".css"))) { contentType = F("text/css"); }
  else if (match_ext(path, F(".js")))  { contentType = F("application/javascript"); }
  else if (match_ext(path, F(".png"))) { contentType = F("image/png"); }
  else if (match_ext(path, F(".gif"))) { contentType = F("image/gif"); }
  else if (match_ext(path, F(".jpg"))) { contentType = F("image/jpeg"); }
  else if (path.endsWith(F(".ico")))   { contentType = F("image/x-icon"); }
  else if (path.endsWith(F(".svg")))   { contentType = F("image/svg+xml"); }
  else if (path.endsWith(F(".json")))  { contentType = F("application/json"); }
  else if (path.endsWith(F(".txt")) ||
           path.endsWith(F(".dat"))) {
    mustCheckCredentials = true;
    contentType          = F("application/octet-stream");
  } else {
    mustCheckCredentials = true;
  }
  return contentType;
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

  path = fileFromUrl(path);

  // isStaticFile_StripPrefix may alter the path by stripping off the "static_" prefix.
  // Thus do this before checking whether a file is present on the file system.
  const bool static_file = isStaticFile_StripPrefix(path);
#ifndef BUILD_NO_DEBUG

  if (static_file) {
    addLog(LOG_LEVEL_INFO, concat(F("static_file: "), path));
  }
#endif // ifndef BUILD_NO_DEBUG
  const bool fileEmbedded = fileIsEmbedded(path);

  if (!fileExists(path) && !fileEmbedded) {
    return false;
  }

#ifdef WEBSERVER_CUSTOM

  if (path.endsWith(F(".esp"))) {
    return handle_custom(path);
  }
#endif // ifdef WEBSERVER_CUSTOM

  bool mustCheckCredentials = false;
  const __FlashStringHelper* contentType  = get_ContentType(path, mustCheckCredentials);

  const bool serve_304 = static_file && 
                         reply_304_not_modified(path); // Reply with a 304 Not Modified

#ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log = F("HTML : Request file ");
    log += path;
    addLogMove(LOG_LEVEL_DEBUG, log);
  }
#endif // ifndef BUILD_NO_DEBUG

  // prevent reloading stuff on every click
  if (static_file) {
    sendHeader(F("Cache-Control"), F("public, max-age=31536000, immutable"));

    //    sendHeader(F("Cache-Control"), F("max-age=86400"));
    sendHeader(F("Expires"),       F("-1"));
    if (fileEmbedded && !fileExists(path)) {
      sendHeader(F("Last-Modified"), get_build_date_RFC1123());
    }
    sendHeader(F("Age"),           F("100"));
    sendHeader(F("ETag"),          wrap_String(String(Cache.fileCacheClearMoment) + F("-a"), '"')); // added "-a" to the ETag to
                                                                                                               // match the same encoding
  } else {
    sendHeader(F("Cache-Control"), F("no-cache"));
    sendHeader(F("ETag"),          F("\"2.0.0\""));
  }
  sendHeader(F("Vary"), "*");

  if (path.endsWith(F(".dat"))) {
    sendHeader(F("Content-Disposition"), F("attachment;"));
  }

  if (serve_304) {
    web_server.send(304, String(contentType), EMPTY_STRING);
  } else {
    if (fileExists(path)) {
      fs::File f = tryOpenFile(path.c_str(), "r");

      if (!f) {
        return false;
      }
      web_server.streamFile(f, String(contentType));
      f.close();
    }

    if (!fileEmbedded) {
      return false;
    }
    serveEmbedded(path, contentType, false);
  }

  statusLED(true);
  return true;
}

size_t streamFromFS(String path, bool htmlEscape) {
  // path is a deepcopy, since it will be changed here.
  path = fileFromUrl(path);
  statusLED(true);

  size_t bytesStreamed = 0;

  fs::File f = tryOpenFile(path.c_str(), "r");

  if (!f) {
    return bytesStreamed;
  }

  size_t available = f.available();
  String escaped;

  while (available > 0) {
    size_t chunksize = 64;

    if (available < chunksize) {
      chunksize = available;
    }
    uint8_t   buf[64] = { 0 };
    const size_t read = f.read(buf, chunksize);

    if (read == chunksize) {
      for (size_t i = 0; i < chunksize; ++i) {
        const char c = (char)buf[i];

        if (htmlEscape && htmlEscapeChar(c, escaped)) {
          addHtml(escaped);
        } else {
          addHtml(c);
        }
      }
      bytesStreamed += read;
      available      = f.available();
    } else {
      available = 0;
    }
  }

  // FIXME TD-er: Is this while loop still useful?
  while (f.available()) {
    addHtml((char)f.read());
  }

  statusLED(true);

  f.close();
  return bytesStreamed;
}
