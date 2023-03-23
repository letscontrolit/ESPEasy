#include "../WebServer/WebTemplateParser.h"

#include "../CustomBuild/CompiletimeDefines.h"

#include "../DataTypes/ControllerIndex.h"

#include "../Globals/Protocol.h"
#include "../Globals/Settings.h"

#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/StringConverter.h"

#include "../Static/WebStaticData.h"

#include "../WebServer/HTML_wrappers.h"
#include "../WebServer/LoadFromFS.h"

#include "../../ESPEasy_common.h"

// Determine what pages should be visible
#ifndef MENU_INDEX_MAIN_VISIBLE
  # define MENU_INDEX_MAIN_VISIBLE true
#endif // ifndef MENU_INDEX_MAIN_VISIBLE

#ifndef MENU_INDEX_CONFIG_VISIBLE
  # define MENU_INDEX_CONFIG_VISIBLE true
#endif // ifndef MENU_INDEX_CONFIG_VISIBLE

#ifndef MENU_INDEX_CONTROLLERS_VISIBLE
  # define MENU_INDEX_CONTROLLERS_VISIBLE true
#endif // ifndef MENU_INDEX_CONTROLLERS_VISIBLE

#ifndef MENU_INDEX_HARDWARE_VISIBLE
  # define MENU_INDEX_HARDWARE_VISIBLE true
#endif // ifndef MENU_INDEX_HARDWARE_VISIBLE

#ifndef MENU_INDEX_DEVICES_VISIBLE
  # define MENU_INDEX_DEVICES_VISIBLE true
#endif // ifndef MENU_INDEX_DEVICES_VISIBLE

#ifndef MENU_INDEX_RULES_VISIBLE
  # define MENU_INDEX_RULES_VISIBLE true
#endif // ifndef MENU_INDEX_RULES_VISIBLE

#ifndef MENU_INDEX_NOTIFICATIONS_VISIBLE
  # define MENU_INDEX_NOTIFICATIONS_VISIBLE true
#endif // ifndef MENU_INDEX_NOTIFICATIONS_VISIBLE

#ifndef MENU_INDEX_TOOLS_VISIBLE
  # define MENU_INDEX_TOOLS_VISIBLE true
#endif // ifndef MENU_INDEX_TOOLS_VISIBLE


#if defined(NOTIFIER_SET_NONE) && defined(MENU_INDEX_NOTIFICATIONS_VISIBLE)
  # undef MENU_INDEX_NOTIFICATIONS_VISIBLE
  # define MENU_INDEX_NOTIFICATIONS_VISIBLE false
#endif // if defined(NOTIFIER_SET_NONE) && defined(MENU_INDEX_NOTIFICATIONS_VISIBLE)


uint8_t navMenuIndex = MENU_INDEX_MAIN;

// See https://github.com/letscontrolit/ESPEasy/issues/1650
const __FlashStringHelper* getGpMenuIcon(uint8_t index) {
  switch (index) {
    case MENU_INDEX_MAIN: return F("&#8962;");
    case MENU_INDEX_CONFIG: return F("&#9881;");
    case MENU_INDEX_CONTROLLERS: return F("&#128172;");
    case MENU_INDEX_HARDWARE: return F("&#128204;");
    case MENU_INDEX_DEVICES: return F("&#128268;");
    case MENU_INDEX_RULES: return F("&#10740;");
    case MENU_INDEX_NOTIFICATIONS: return F("&#9993;");
    case MENU_INDEX_TOOLS: return F("&#128295;");
  }
  return F("");
}

const __FlashStringHelper* getGpMenuLabel(uint8_t index) {
  switch (index) {
    case MENU_INDEX_MAIN: return F("Main");
    case MENU_INDEX_CONFIG: return F("Config");
    case MENU_INDEX_CONTROLLERS: return F("Controllers");
    case MENU_INDEX_HARDWARE: return F("Hardware");
    case MENU_INDEX_DEVICES: return F("Devices");
    case MENU_INDEX_RULES: return F("Rules");
    case MENU_INDEX_NOTIFICATIONS: return F("Notifications");
    case MENU_INDEX_TOOLS: return F("Tools");
  }
  return F("");
}

const __FlashStringHelper* getGpMenuURL(uint8_t index) {
  switch (index) {
    case MENU_INDEX_MAIN: return F("/");
    case MENU_INDEX_CONFIG: return F("/config");
    case MENU_INDEX_CONTROLLERS: return F("/controllers");
    case MENU_INDEX_HARDWARE: return F("/hardware");
    case MENU_INDEX_DEVICES: return F("/devices");
    case MENU_INDEX_RULES: return F("/rules");
    case MENU_INDEX_NOTIFICATIONS: return F("/notifications");
    case MENU_INDEX_TOOLS: return F("/tools");
  }
  return F("");
}

bool GpMenuVisible(uint8_t index) {
  switch (index) {
    case MENU_INDEX_MAIN: return MENU_INDEX_MAIN_VISIBLE;
    case MENU_INDEX_CONFIG: return MENU_INDEX_CONFIG_VISIBLE;
    case MENU_INDEX_CONTROLLERS: return MENU_INDEX_CONTROLLERS_VISIBLE;
    case MENU_INDEX_HARDWARE: return MENU_INDEX_HARDWARE_VISIBLE;
    case MENU_INDEX_DEVICES: return MENU_INDEX_DEVICES_VISIBLE;
    case MENU_INDEX_RULES: return MENU_INDEX_RULES_VISIBLE;
    case MENU_INDEX_NOTIFICATIONS: return MENU_INDEX_NOTIFICATIONS_VISIBLE;
    case MENU_INDEX_TOOLS: return MENU_INDEX_TOOLS_VISIBLE;
  }
  return false;
}

bool WebTemplateParser::process(const char c) {
  switch (c) {
    case '{':
    case '}':

      if (prev == c) {
        parsingVarName = c == '{';

        if (c == '}') {
          // Done parsing varName, still need to process it.
          if (varName.equalsIgnoreCase(F("content"))) {
            contentVarFound = true;
          } else if (Tail == contentVarFound) {
            processVarName();
          }
          varName = String();
        }
      }
      break;
    default:

      if (parsingVarName) {
        varName += c;
      } else {
        // FIXME TD-er: if a template has single '{' or '}' they will not be sent. Is that a problem?
        if (Tail == contentVarFound) {
          // only send the template tail after {{content}} is found
          // Or send all until the {{content}} tag.
          addHtml(c);
        }
      }
      break;
  }


  prev = c;

  if (!Tail) { return !contentVarFound; }
  return true;
}

bool WebTemplateParser::process(const __FlashStringHelper *pstr)
{
  return process((PGM_P)pstr);
}

bool WebTemplateParser::process(PGM_P pstr)
{
  if (!pstr) { return false; }

  #ifdef USE_SECOND_HEAP

  if (mmu_is_iram(pstr)) {
    // Have to copy the string using mmu_get functions
    // This is not a flash string.
    bool done            = false;
    const char *cur_char = pstr;

    while (!done) {
      const uint8_t ch = mmu_get_uint8(cur_char++);

      if (ch == 0) {
        done = true;
      }

      if (!process(ch)) { return false; }
    }
    return true;
  }
  #endif // ifdef USE_SECOND_HEAP


  const char *c = pstr;
  size_t length = strlen_P((PGM_P)pstr);

  while (length-- > 0) {
    if (!process(static_cast<char>(pgm_read_byte(c++)))) { return false; }
  }
  return true;
}

bool WebTemplateParser::process(const String& str)
{
  size_t length    = str.length();
  const char *cstr = str.begin();

  while (length-- > 0) {
    if (!process(*(cstr++))) { return false; }
  }
  return true;
}

void WebTemplateParser::processVarName()
{
  if (!varName.length()) { return; }
  varName.toLowerCase();

  if (equals(varName, F("error"))) {
    getErrorNotifications();
  }
  else if (equals(varName, F("meta"))) {
    if (Rebooting) {
      addHtml(F("<meta http-equiv='refresh' content='10 url=/'>"));
    }
  }
  else {
    getWebPageTemplateVar(varName);
  }
}

void WebTemplateParser::getErrorNotifications() {
  // Check number of MQTT controllers active.
  int nrMQTTenabled = 0;

  for (controllerIndex_t x = 0; x < CONTROLLER_MAX; x++) {
    if (Settings.Protocol[x] != 0) {
      protocolIndex_t ProtocolIndex = getProtocolIndex_from_ControllerIndex(x);

      if (validProtocolIndex(ProtocolIndex) && Settings.ControllerEnabled[x] && Protocol[ProtocolIndex].usesMQTT) {
        ++nrMQTTenabled;
      }
    }
  }

  if (nrMQTTenabled > 1) {
    // Add warning, only one MQTT protocol should be used.
    addHtmlError(F("Only one MQTT controller should be active."));
  }

  // Check checksum of stored settings.
}

void WebTemplateParser::getWebPageTemplateVar(const String& varName)
{
  // serialPrint(varName); serialPrint(" : free: "); serialPrint(ESP.getFreeHeap());   serialPrint("var len before:  "); serialPrint
  // (varValue.length()) ;serialPrint("after:  ");
  // varValue = "";

  if (equals(varName, F("name")))
  {
    addHtml(Settings.getHostname());
  }

  else if (equals(varName, F("unit")))
  {
    addHtmlInt(Settings.Unit);
  }
  
  else if (equals(varName, F("build")))
  {
    #if BUILD_IN_WEBFOOTER
    // In the footer, show full build binary name, will be 'firmware.bin' when compiled using Arduino IDE.
    addHtml(get_binary_filename());
    #endif
  }

  else if (equals(varName, F("date")))
  {
    #if BUILD_IN_WEBFOOTER
    // Add the compile-date
    addHtml(get_build_date());
    #endif
  }

  else if (equals(varName, F("menu")))
  {
    addHtml(F("<div class='menubar'>"));

    for (uint8_t i = 0; i < 8; i++)
    {
      if (!GpMenuVisible(i)) {
        // hide menu item
        continue;
      }

      if ((i == MENU_INDEX_RULES) && !Settings.UseRules) { // hide rules menu item
        continue;
      }
#if !FEATURE_NOTIFIER

      if (i == MENU_INDEX_NOTIFICATIONS) { // hide notifications menu item
        continue;
      }
#endif // if !FEATURE_NOTIFIER

      addHtml(F("<a "));
      addHtmlAttribute(F("class"), (i == navMenuIndex) ? F("menu active") : F("menu"));
      addHtmlAttribute(F("href"),  getGpMenuURL(i));
      addHtml('>');
      addHtml(getGpMenuIcon(i));
      addHtml(F("<span class='showmenulabel'>"));
      addHtml(getGpMenuLabel(i));
      addHtml(F("</span></a>"));
    }

    addHtml(F("</div>"));
  }

  else if (equals(varName, F("logo")))
  {
    if (fileExists(F("esp.png")))
    {
      addHtml(F("<img src=\"esp.png\" width=48 height=48 align=right>"));
    }
  }

  else if (equals(varName, F("css")))
  {
    serve_favicon();
    if (MENU_INDEX_SETUP == navMenuIndex) {
      // Serve embedded CSS
      serve_CSS_inline();
    } else {
      serve_CSS(CSSfiles_e::ESPEasy_default);
    }
    #if FEATURE_RULES_EASY_COLOR_CODE
    if (MENU_INDEX_RULES == navMenuIndex ||
        MENU_INDEX_CUSTOM_PAGE == navMenuIndex) {
      serve_CSS(CSSfiles_e::EasyColorCode_codemirror);
    }
    #endif
  }


  else if (equals(varName, F("js")))
  {
    html_add_JQuery_script();

    #if FEATURE_CHART_JS
    html_add_ChartJS_script();
    #endif // if FEATURE_CHART_JS

    #if FEATURE_RULES_EASY_COLOR_CODE
    if (MENU_INDEX_RULES == navMenuIndex ||
        MENU_INDEX_CUSTOM_PAGE == navMenuIndex) {
      html_add_Easy_color_code_script();
    }
    #endif
    if (MENU_INDEX_RULES == navMenuIndex) {
      serve_JS(JSfiles_e::SaveRulesFile);
    }
    
    html_add_autosubmit_form();
    serve_JS(JSfiles_e::Toasting);
  }

  else if (equals(varName, F("error")))
  {
    // print last error - not implemented yet
  }

  else if (equals(varName, F("debug")))
  {
    // print debug messages - not implemented yet
  }

  else
  {
    #ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
      String log = F("Templ: Unknown Var : ");
      log += varName;
      addLogMove(LOG_LEVEL_ERROR, log);
    }
    #endif // ifndef BUILD_NO_DEBUG

    // no return string - eat var name
  }
}