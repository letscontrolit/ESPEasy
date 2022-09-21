#include "../Static/WebStaticData.h"

#include "../CustomBuild/CompiletimeDefines.h"
#include "../Globals/Settings.h"
#include "../Globals/Cache.h"
#include "../Helpers/ESPEasy_Storage.h"
#include "../WebServer/HTML_wrappers.h"
#include "../WebServer/LoadFromFS.h"

String generate_external_URL(const String& fname) {
  String url;
  url.reserve(80 + fname.length());
  url = get_CDN_url_prefix();
  url += fname;
  return url;
}

void serve_CDN_CSS(const __FlashStringHelper * fname) {
  addHtml(F("<link"));
  addHtmlAttribute(F("rel"), F("stylesheet"));
  addHtmlAttribute(F("type"), F("text/css"));
  addHtmlAttribute(F("href"), generate_external_URL(fname));
  addHtml('/', '>');
}

void serve_CDN_JS(const __FlashStringHelper * fname, const __FlashStringHelper * script_arg) {
  addHtml(F("<script"));
  addHtml(F(" defer"));
  addHtmlAttribute(F("src"), generate_external_URL(fname));
  addHtml(' ');
  addHtml(script_arg);
  addHtml('>');
  html_add_script_end();
}


void serve_CSS(CSSfiles_e cssfile) {
  const __FlashStringHelper * url = F("");
  #if !defined(WEBSERVER_CSS)
  bool useCDN = true;
  #else
  bool useCDN = false;
  #endif

  switch (cssfile) {
    case CSSfiles_e::ESPEasy_default:
      // Send CSS in chunks
#if defined(EMBED_ESPEASY_DEFAULT_MIN_CSS) || defined(WEBSERVER_EMBED_CUSTOM_CSS)
      useCDN = false;
#else
      url = F("espeasy_default.min.css");
#endif      
      break;
#if FEATURE_RULES_EASY_COLOR_CODE
    case CSSfiles_e::EasyColorCode_codemirror:
      url = F("codemirror.min.css");
      useCDN = true;
      break;
#endif
    default:
      return;      
  }

  const String cssFile = 
      (cssfile == CSSfiles_e::ESPEasy_default) ?
      F("esp.css") : url;

  if (fileExists(cssFile))
  {
    addHtml(F("<style>"));
    streamFromFS(cssFile);
    addHtml(F("</style>"));
    return;
  }
  #if !(defined(EMBED_ESPEASY_DEFAULT_MIN_CSS) || defined(WEBSERVER_EMBED_CUSTOM_CSS)) || FEATURE_RULES_EASY_COLOR_CODE
  if (useCDN) {
    serve_CDN_CSS(url);
    return;
  }
  #endif
  #if defined(EMBED_ESPEASY_DEFAULT_MIN_CSS) || defined(WEBSERVER_EMBED_CUSTOM_CSS)
  addHtml(F("<style>"));
  TXBuffer.addFlashString((PGM_P)FPSTR(DATA_ESPEASY_DEFAULT_MIN_CSS));
  addHtml(F("</style>"));
  #endif
}

void serve_favicon() {
  #ifdef WEBSERVER_FAVICON_CDN
  addHtml(F("<link"));
  addHtmlAttribute(F("rel"), F("icon"));
  addHtmlAttribute(F("type"), F("image/x-icon"));
  addHtmlAttribute(F("href"), generate_external_URL(F("favicon.ico")));
  addHtml('/');
  addHtml('>');
  #endif
}

void serve_JS(JSfiles_e JSfile) {
    const __FlashStringHelper * url = F("");
    const __FlashStringHelper * id = F("");
    #if !defined(WEBSERVER_INCLUDE_JS)
    bool useCDN = true;
    #else
    bool useCDN = false;
    #endif

    switch (JSfile) {
        case JSfiles_e::UpdateSensorValuesDevicePage:
          url = F("update_sensor_values_device_page.js");
          break;
        case JSfiles_e::FetchAndParseLog:
          url = F("fetch_and_parse_log.js");
          break;
        case JSfiles_e::SaveRulesFile:
          url = F("rules_save.js");
          break;
        case JSfiles_e::GitHubClipboard:
          url = F("github_clipboard.js");
          break;
        case JSfiles_e::Reboot:
          url = F("reboot.js");
          break;
        case JSfiles_e::Toasting:
          url = F("toasting.js");
          break;
        case JSfiles_e::SplitPasteInput:
          url = F("split_paste.js");
          break;
#if FEATURE_RULES_EASY_COLOR_CODE
        case JSfiles_e::EasyColorCode_codemirror:
          url = F("codemirror.min.js");
          useCDN = true;
          break;
        case JSfiles_e::EasyColorCode_espeasy:
          url = F("espeasy.min.js");
          useCDN = true;
          break;
        case JSfiles_e::EasyColorCode_cm_plugins:
          url = F("cm-plugins.min.js");
          id = F("id='anyword'");
          useCDN = true;
          break;
#endif

    }

    // Work-around for shortening the filename when stored on SPIFFS file system
    // Files cannot be longer than 31 characters
    const __FlashStringHelper * fname = 
      (JSfile == JSfiles_e::UpdateSensorValuesDevicePage) ?
      F("upd_values_device_page.js") : url;

    if (!fileExists(fname))
    {
        #if !defined(WEBSERVER_INCLUDE_JS) || FEATURE_RULES_EASY_COLOR_CODE
        if (useCDN) {
          serve_CDN_JS(url, id);
          return;
        }
        #else
        html_add_script_arg(id, true);
        switch (JSfile) {
          case JSfiles_e::UpdateSensorValuesDevicePage:
            #ifdef WEBSERVER_DEVICES
            TXBuffer.addFlashString((PGM_P)FPSTR(DATA_UPDATE_SENSOR_VALUES_DEVICE_PAGE_JS));
            #endif
            break;
          case JSfiles_e::FetchAndParseLog:
            #ifdef WEBSERVER_LOG
            TXBuffer.addFlashString((PGM_P)FPSTR(DATA_FETCH_AND_PARSE_LOG_JS));
            #endif
            break;
          case JSfiles_e::SaveRulesFile:
            #ifdef WEBSERVER_RULES
            TXBuffer.addFlashString((PGM_P)FPSTR(jsSaveRules));
            #endif
            break;
          case JSfiles_e::GitHubClipboard:
            #ifdef WEBSERVER_GITHUB_COPY
            TXBuffer.addFlashString((PGM_P)FPSTR(DATA_GITHUB_CLIPBOARD_JS));
            #endif
            break;
          case JSfiles_e::Reboot:
            TXBuffer.addFlashString((PGM_P)FPSTR(DATA_REBOOT_JS));
            break;
          case JSfiles_e::Toasting:
            TXBuffer.addFlashString((PGM_P)FPSTR(jsToastMessageBegin));
            // we can push custom messages here in future releases...
            addHtml(F("Submitted"));
            TXBuffer.addFlashString((PGM_P)FPSTR(jsToastMessageEnd));
            break;
          case JSfiles_e::SplitPasteInput:
            TXBuffer.addFlashString((PGM_P)FPSTR(jsSplitMultipleFields));
            // After this, make sure to call it, like this:
            // split('$', ".query-input")
            break;

        }
        html_add_script_end();
        return;
        #endif
    }
    // Now stream the file directly from the file system.
    html_add_script_arg(id, false);
    streamFromFS(String(fname));
    html_add_script_end();
}