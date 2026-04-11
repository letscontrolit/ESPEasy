#include "../Static/WebStaticData.h"

#include "../CustomBuild/CompiletimeDefines.h"
#include "../Globals/Settings.h"
#include "../Globals/Cache.h"
#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/StringConverter.h"
#include "../WebServer/HTML_wrappers.h"
#include "../WebServer/LoadFromFS.h"

String generate_external_URL(const String& fname, bool isEmbedded) {
  if (isEmbedded || fileExists(fname)) {
    // Generate some URL indicating static files which will need to be served with some cache-control header
    return concat(F("static_"), Cache.fileCacheClearMoment) + '_' + fname;
  }

  #if FEATURE_ALTERNATIVE_CDN_URL
  String cdn = get_CDN_url_custom();
  if (!cdn.isEmpty()) {
    cdn = parseTemplate(cdn);  // Replace system variables.
    return concat(cdn, fname); // cdn.endsWith('/') check done at save
  } 
  #endif // if FEATURE_ALTERNATIVE_CDN_URL

  return concat(get_CDN_url_prefix(), fname);
}

void serve_CDN_CSS(const __FlashStringHelper * fname, const __FlashStringHelper * fname_alt, bool isEmbedded) {
  const String url = generate_external_URL(fname, isEmbedded);

  // FIXME TD-er: For now just retry loading the same as failed loading the embedded one
  //  may slow down next page loads until the browser finally was able to fetch the embedded one.
  const String url2 = generate_external_URL(fname, isEmbedded);
//  const String url2 = generate_external_URL(fname_alt, !isEmbedded);

  // Use 'onerror' to try to reload the CSS when first attempt loading failed
  // See: https://developer.mozilla.org/en-US/docs/Web/API/Window/error_event#element.onerror
  addHtml(strformat(
    F("<link rel=\"stylesheet\" href=\"%s\" onerror=\"this.onerror=null;this.href='%s';\" />"),
    url.c_str(),
    url2.c_str()
  ));


  // <link rel="stylesheet" href="%s" onerror="this.onerror=null;this.href='%s';" />

  /*
  // Delay loading CSS till after page has loaded.
  // Disabled as it adds 'flickering' to the page loading
  addHtml(strformat(
    F("<link rel=\"preload\" href=\"%s\" as=\"style\" onload=\"this.onload=null;this.rel='stylesheet'\"><noscript><link rel=\"stylesheet\" href=\"%s\"></noscript>"),
    url.c_str(), url.c_str()
  ));
  */
}

void serve_CDN_CSS(const __FlashStringHelper * fname, bool isEmbedded) {
  serve_CDN_CSS(fname, fname, isEmbedded);
}

void serve_CDN_JS(const __FlashStringHelper * fname, 
                  const __FlashStringHelper * script_arg, 
                  bool useDefer) {
  addHtml(F("<script"));
  if (useDefer) {
    addHtml(F(" defer"));
  }
  addHtmlAttribute(F("src"), generate_external_URL(fname, false));
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
#endif      
      url = F("espeasy_default.min.css");
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

  const __FlashStringHelper * cssFile = 
      (cssfile == CSSfiles_e::ESPEasy_default) ?
      F("esp.css") : url;

  if (fileExists(cssFile))
  {
    serve_CDN_CSS(cssFile, false);
    return;
  }
  #if !(defined(EMBED_ESPEASY_DEFAULT_MIN_CSS) || defined(WEBSERVER_EMBED_CUSTOM_CSS)) || FEATURE_RULES_EASY_COLOR_CODE
  if (useCDN) {
    serve_CDN_CSS(url, false);
    return;
  }
  #endif
  #if defined(EMBED_ESPEASY_DEFAULT_MIN_CSS) || defined(WEBSERVER_EMBED_CUSTOM_CSS)
  if (cssfile == CSSfiles_e::ESPEasy_default) {
    serve_CDN_CSS(cssFile, url, true);
  } else {
    serve_CDN_CSS(cssFile, true);
  }
  #endif
}

void serve_favicon() {
  #ifdef WEBSERVER_FAVICON_CDN
  addHtml(F("<link rel='icon' type='image/x-icon' href='"));
  addHtml(generate_external_URL(F("favicon.ico"), true));
  addHtml(F("'/>"));

  /*
  addHtml(F("<link"));
  addHtmlAttribute(F("rel"), F("icon"));
  addHtmlAttribute(F("type"), F("image/x-icon"));
  addHtmlAttribute(F("href"), generate_external_URL(F("favicon.ico"), true));
  addHtml('/', '>');
*/
  #endif
  #ifndef LIMIT_BUILD_SIZE
  addHtml(F("<link"));
  addHtmlAttribute(F("rel"), F("icon"));
  addHtmlAttribute(F("type"), F("image/svg+xml"));
  addHtmlAttribute(F("href"), F("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' width='113.392' height='113.392' viewBox='0 0 30.002 30.002'%3E%3Cpath fill='%23f9f9f9' d='M0 24.728V5.272A5.272 5.272 0 0 1 5.272 0h19.456c2.91 0 5.274 2.36 5.274 5.272v19.456a5.276 5.276 0 0 1-5.274 5.274H5.272A5.274 5.274 0 0 1 0 24.728zM30.002 0'/%3E%3Cpath fill='%2307d' stroke='%2307d' stroke-linecap='round' stroke-width='2.636' d='m3.75 11.25 7.5-7.5M3.75 18.75l15-15M3.75 26.25l22.5-22.5M11.25 26.25l15-15'/%3E%3Cpath fill='%2307d' d='M27.374 24.377a2.999 2.999 0 1 0-5.999 0 2.999 2.999 0 0 0 3.002 2.997 2.997 2.997 0 0 0 2.997-2.997zm-2.997 0'/%3E%3C/svg%3E"));
  addHtml('/', '>');

/*
  <link rel="icon" type="image/svg+xml"
      href="data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' width='113.392' height='113.392' viewBox='0 0 30.002 30.002'%3E%3Cpath fill='%23f9f9f9' d='M0 24.728V5.272A5.272 5.272 0 0 1 5.272 0h19.456c2.91 0 5.274 2.36 5.274 5.272v19.456a5.276 5.276 0 0 1-5.274 5.274H5.272A5.274 5.274 0 0 1 0 24.728zM30.002 0'/%3E%3Cpath fill='%2307d' stroke='%2307d' stroke-linecap='round' stroke-width='2.636' d='m3.75 11.25 7.5-7.5M3.75 18.75l15-15M3.75 26.25l22.5-22.5M11.25 26.25l15-15'/%3E%3Cpath fill='%2307d' d='M27.374 24.377a2.999 2.999 0 1 0-5.999 0 2.999 2.999 0 0 0 3.002 2.997 2.997 2.997 0 0 0 2.997-2.997zm-2.997 0'/%3E%3C/svg%3E" />
*/
#endif
}

void serve_JS(JSfiles_e JSfile) {
    const __FlashStringHelper * url = F("");
    const __FlashStringHelper * id = F("");
    bool useDefer = true;
    #if defined(WEBSERVER_INCLUDE_JS)
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
          useDefer = false;
          #if defined(WEBSERVER_INCLUDE_JS)
          useCDN = true;
          #endif
          break;
        case JSfiles_e::EasyColorCode_espeasy:
          url = F("espeasy.min.js");
          useDefer = false;
          #if defined(WEBSERVER_INCLUDE_JS)
          useCDN = true;
          #endif
          break;
        case JSfiles_e::EasyColorCode_cm_plugins:
          url = F("cm-plugins.min.js");
          id = F("id='anyword'");
          useDefer = false;
          #if defined(WEBSERVER_INCLUDE_JS)
          useCDN = true;
          #endif
          break;
#endif
#ifdef USES_P113
        case JSfiles_e::P113_script:
          url = F("p113_script.js");
          break;
#endif // ifdef USES_P113
#ifdef USES_P165
        case JSfiles_e::P165_digit:
          url = F("p165_digit.js");
          break;
#endif // ifdef USES_P165

    }

    // Work-around for shortening the filename when stored on SPIFFS file system
    // Files cannot be longer than 31 characters
    const __FlashStringHelper * fname = 
      (JSfile == JSfiles_e::UpdateSensorValuesDevicePage) ?
      F("upd_values_device_page.js") : url;

    if (!fileExists(fname))
    {
        #if defined(WEBSERVER_INCLUDE_JS)
        if (!useCDN) {
          html_add_script_arg(id, useDefer);
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
#if FEATURE_RULES_EASY_COLOR_CODE
            case JSfiles_e::EasyColorCode_codemirror:
            case JSfiles_e::EasyColorCode_espeasy:
            case JSfiles_e::EasyColorCode_cm_plugins:
              break;
#endif
#ifdef USES_P113
            case JSfiles_e::P113_script:
              TXBuffer.addFlashString((PGM_P)FPSTR(p113_script));
              break;
#endif // ifdef USES_P113
#ifdef USES_P165
            case JSfiles_e::P165_digit:
              TXBuffer.addFlashString((PGM_P)FPSTR(DATA_P165_DIGIT_JS));
              break;
#endif // ifdef USES_P165
          }
          html_add_script_end();
          return;
        }
        #endif
        serve_CDN_JS(url, id, useDefer);
    } else {
      serve_CDN_JS(fname, id, useDefer);
    }
}
