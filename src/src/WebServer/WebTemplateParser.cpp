#include "../WebServer/WebTemplateParser.h"

#include "../CustomBuild/CompiletimeDefines.h"

#include "../DataTypes/ControllerIndex.h"

#include "../Globals/Settings.h"

#include "../Helpers/_CPlugin_init.h"
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
#ifdef WEBSERVER_CONFIG
  # define MENU_INDEX_CONFIG_VISIBLE true
#else
  # define MENU_INDEX_CONFIG_VISIBLE false
#endif
#endif // ifndef MENU_INDEX_CONFIG_VISIBLE

#ifndef MENU_INDEX_NETWORK_VISIBLE
#ifdef WEBSERVER_NETWORK
  # define MENU_INDEX_NETWORK_VISIBLE true
#else
  # define MENU_INDEX_NETWORK_VISIBLE false
#endif
#endif // ifndef MENU_INDEX_NETWORK_VISIBLE


#ifndef MENU_INDEX_CONTROLLERS_VISIBLE
#ifdef WEBSERVER_CONTROLLERS
  # define MENU_INDEX_CONTROLLERS_VISIBLE true
#else
  # define MENU_INDEX_CONTROLLERS_VISIBLE false
#endif
#endif // ifndef MENU_INDEX_CONTROLLERS_VISIBLE

#ifndef MENU_INDEX_HARDWARE_VISIBLE
#ifdef WEBSERVER_HARDWARE
  # define MENU_INDEX_HARDWARE_VISIBLE true
#else
  # define MENU_INDEX_HARDWARE_VISIBLE false
#endif
#endif // ifndef MENU_INDEX_HARDWARE_VISIBLE

#ifndef MENU_INDEX_INTERFACES_VISIBLE
#ifdef WEBSERVER_INTERFACES
  # define MENU_INDEX_INTERFACES_VISIBLE true
#else
  # define MENU_INDEX_INTERFACES_VISIBLE false
#endif
#endif

#ifndef MENU_INDEX_DEVICES_VISIBLE
#ifdef WEBSERVER_DEVICES
  # define MENU_INDEX_DEVICES_VISIBLE true
#else
  # define MENU_INDEX_DEVICES_VISIBLE false
#endif
#endif // ifndef MENU_INDEX_DEVICES_VISIBLE

#ifndef MENU_INDEX_RULES_VISIBLE
#ifdef WEBSERVER_RULES
  # define MENU_INDEX_RULES_VISIBLE true
#else
  # define MENU_INDEX_RULES_VISIBLE false
#endif
#endif // ifndef MENU_INDEX_RULES_VISIBLE

#ifndef MENU_INDEX_NOTIFICATIONS_VISIBLE
#if FEATURE_NOTIFIER
  # define MENU_INDEX_NOTIFICATIONS_VISIBLE true
#else
  # define MENU_INDEX_NOTIFICATIONS_VISIBLE false
#endif
#endif // ifndef MENU_INDEX_NOTIFICATIONS_VISIBLE

#ifndef MENU_INDEX_TOOLS_VISIBLE
#ifdef WEBSERVER_TOOLS
  # define MENU_INDEX_TOOLS_VISIBLE true
#else
  # define MENU_INDEX_TOOLS_VISIBLE false
#endif
#endif // ifndef MENU_INDEX_TOOLS_VISIBLE


#if defined(NOTIFIER_SET_NONE) && defined(MENU_INDEX_NOTIFICATIONS_VISIBLE)
  # undef MENU_INDEX_NOTIFICATIONS_VISIBLE
  # define MENU_INDEX_NOTIFICATIONS_VISIBLE false
#endif // if defined(NOTIFIER_SET_NONE) && defined(MENU_INDEX_NOTIFICATIONS_VISIBLE)


uint8_t navMenuIndex = MENU_INDEX_MAIN;

// See https://github.com/letscontrolit/ESPEasy/issues/1650
const __FlashStringHelper* getGpMenuIcon(uint8_t index) {

  #define ICON(code) F(code "&#xFE0E;")

  switch (index) {
    case MENU_INDEX_MAIN:          return ICON("&#8962;");
    case MENU_INDEX_CONFIG:        return ICON("&#9881;");
    case MENU_INDEX_NETWORK:       return ICON("&#127760;"); // Alternative &#128423; (not working on Apple) 
    case MENU_INDEX_CONTROLLERS:   return ICON("&#9990;");
    case MENU_INDEX_HARDWARE:      return ICON("&#9783;");
    case MENU_INDEX_INTERFACES:    return ICON("&#10057;");
#if FEATURE_I2C
    case MENU_INDEX_INTERFACES_I2C:    return ICON("I&sup2;C");
#endif // if FEATURE_I2C
#if FEATURE_SPI
    case MENU_INDEX_INTERFACES_SPI:    return ICON("SPI");
#endif // if FEATURE_SPI
#if FEATURE_MODBUS && FEATURE_MODBUS_INTERFACES_TAB
    case MENU_INDEX_INTERFACES_MODBUS: return ICON("Modbus");
#endif // if FEATURE_MODBUS
#if FEATURE_CAN
    case MENU_INDEX_INTERFACES_CAN:    return ICON("CAN bus");
#endif // if FEATURE_CAN
#if FEATURE_WRMBUS
    case MENU_INDEX_INTERFACES_WRMBUS: return ICON("mBus");
#endif // if FEATURE_WRMBUS
#if FEATURE_WIMBUS
    case MENU_INDEX_INTERFACES_WIMBUS: return ICON("w-mBus");
#endif // if FEATURE_WIMBUS
    case MENU_INDEX_DEVICES:       return ICON("&#10070;");
    case MENU_INDEX_RULES:         return ICON("&#10740;");
    case MENU_INDEX_NOTIFICATIONS: return ICON("&#9993;");
    case MENU_INDEX_TOOLS:         return ICON("&#9888;");
  }
  return F("");
}

const __FlashStringHelper* getGpMenuLabel(uint8_t index) {
  switch (index) {
    case MENU_INDEX_MAIN:              return F("Main");
    case MENU_INDEX_CONFIG:            return F("Config");
    case MENU_INDEX_NETWORK:           return F("Network");
    case MENU_INDEX_CONTROLLERS:       return F("Controllers");
    case MENU_INDEX_HARDWARE:          return F("Hardware");
    case MENU_INDEX_INTERFACES:        return F("Interfaces");
#if FEATURE_I2C
    case MENU_INDEX_INTERFACES_I2C:
#endif // if FEATURE_I2C
#if FEATURE_SPI
    case MENU_INDEX_INTERFACES_SPI:
#endif // if FEATURE_SPI
#if FEATURE_MODBUS && FEATURE_MODBUS_INTERFACES_TAB
    case MENU_INDEX_INTERFACES_MODBUS:
#endif // if FEATURE_MODBUS
#if FEATURE_CAN
    case MENU_INDEX_INTERFACES_CAN:
#endif // if FEATURE_CAN
#if FEATURE_I2C || FEATURE_SPI || (FEATURE_MODBUS && FEATURE_MODBUS_INTERFACES_TAB) || FEATURE_CAN
      break; // No label, only an 'icon', for the second-level menu
#endif // if FEATURE_I2C || FEATURE_SPI || FEATURE_MODBUS || FEATURE_CAN
#if FEATURE_WRMBUS
    case MENU_INDEX_INTERFACES_WRMBUS: return F("(wired)");
#endif // if FEATURE_WRMBUS
#if FEATURE_WIMBUS
    case MENU_INDEX_INTERFACES_WIMBUS: return F("(wireless)");
#endif // if FEATURE_WIMBUS
    case MENU_INDEX_DEVICES:           return F("Devices");
    case MENU_INDEX_RULES:             return F("Rules");
    case MENU_INDEX_NOTIFICATIONS:     return F("Notifications");
    case MENU_INDEX_TOOLS:             return F("Tools");
  }
  return F("");
}

const __FlashStringHelper* getGpMenuURL(uint8_t index) {
  switch (index) {
    case MENU_INDEX_MAIN: return F("/");
    case MENU_INDEX_CONFIG: return F("/config");
    case MENU_INDEX_NETWORK: return F("/network");
    case MENU_INDEX_CONTROLLERS: return F("/controllers");
    case MENU_INDEX_HARDWARE: return F("/hardware");
    case MENU_INDEX_INTERFACES: return F("/interfaces");
#if FEATURE_I2C
    case MENU_INDEX_INTERFACES_I2C: return F("/interfaces_i2c");
#endif // if FEATURE_I2C
#if FEATURE_SPI
    case MENU_INDEX_INTERFACES_SPI: return F("/interfaces_spi");
#endif // if FEATURE_SPI
#if FEATURE_MODBUS && FEATURE_MODBUS_INTERFACES_TAB
    case MENU_INDEX_INTERFACES_MODBUS: return F("/interfaces_modbus");
#endif // if FEATURE_MODBUS
#if FEATURE_CAN
   case MENU_INDEX_INTERFACES_CAN: return F("/interfaces_can");
#endif // if FEATURE_CAN
#if FEATURE_WRMBUS
   case MENU_INDEX_INTERFACES_WRMBUS: return F("/interfaces_wrmbus");
#endif // if FEATURE_WRMBUS
#if FEATURE_WIMBUS
   case MENU_INDEX_INTERFACES_WIMBUS: return F("/interfaces_wimbus");
#endif // if FEATURE_WIMBUS
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
    case MENU_INDEX_NETWORK: return MENU_INDEX_NETWORK_VISIBLE;
    case MENU_INDEX_CONTROLLERS: return MENU_INDEX_CONTROLLERS_VISIBLE;
    case MENU_INDEX_HARDWARE: return MENU_INDEX_HARDWARE_VISIBLE;
    case MENU_INDEX_INTERFACES: return MENU_INDEX_INTERFACES_VISIBLE;
    case MENU_INDEX_INTERFACES_I2C: return (1 == FEATURE_I2C);
    case MENU_INDEX_INTERFACES_SPI: return (1 == FEATURE_SPI);
    case MENU_INDEX_INTERFACES_MODBUS: return (1 == FEATURE_MODBUS && 1 == FEATURE_MODBUS_INTERFACES_TAB);
  #if defined(FEATURE_CAN)
    case MENU_INDEX_INTERFACES_CAN: return (1 == FEATURE_CAN);
  #endif // if defined(FEATURE_CAN)
  #if defined(FEATURE_WRMBUS)
    case MENU_INDEX_INTERFACES_WRMBUS: return (1 == FEATURE_WRMBUS);
  #endif // if defined(FEATURE_WRMBUS)
  #if defined(FEATURE_WIMBUS)
    case MENU_INDEX_INTERFACES_WIMBUS: return (1 == FEATURE_WIMBUS);
  #endif // if defined(FEATURE_WIMBUS)
    case MENU_INDEX_DEVICES: return MENU_INDEX_DEVICES_VISIBLE;
    case MENU_INDEX_RULES: return MENU_INDEX_RULES_VISIBLE;
    case MENU_INDEX_NOTIFICATIONS: return MENU_INDEX_NOTIFICATIONS_VISIBLE;
    case MENU_INDEX_TOOLS: return MENU_INDEX_TOOLS_VISIBLE;
  }
  return false;
}

bool isGpMenuSecondLevel(uint8_t index) {
  switch (index) {
    case MENU_INDEX_MAIN:
    case MENU_INDEX_CONFIG:
    case MENU_INDEX_NETWORK:
    case MENU_INDEX_CONTROLLERS:
    case MENU_INDEX_HARDWARE:
    case MENU_INDEX_INTERFACES:
    case MENU_INDEX_DEVICES:
    case MENU_INDEX_RULES:
    case MENU_INDEX_NOTIFICATIONS:
    case MENU_INDEX_TOOLS: return false;
    case MENU_INDEX_INTERFACES_WIMBUS:
    case MENU_INDEX_INTERFACES_WRMBUS:
    case MENU_INDEX_INTERFACES_CAN:
    case MENU_INDEX_INTERFACES_MODBUS:
    case MENU_INDEX_INTERFACES_SPI:
    case MENU_INDEX_INTERFACES_I2C: return true;
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
          free_string(varName);
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

  getWebPageTemplateVar(varName);
}

void WebTemplateParser::getErrorNotifications() {
  // Check number of MQTT controllers active.
  int nrMQTTenabled = 0;

  for (controllerIndex_t x = 0; x < CONTROLLER_MAX; x++) {
    if (Settings.Protocol[x] != 0) {
      const protocolIndex_t ProtocolIndex = getProtocolIndex_from_ControllerIndex(x);

      if (Settings.ControllerEnabled[x] &&
          validProtocolIndex(ProtocolIndex) &&
          getProtocolStruct(ProtocolIndex).usesMQTT) {
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
  if (varName.length() == 0) { return; }


  // serialPrint(varName); serialPrint(" : free: "); serialPrint(ESP.getFreeHeap());   serialPrint("var len before:  "); serialPrint
  // (varValue.length()) ;serialPrint("after:  ");
  // varValue = "";

  switch (varName[0]) {
    case 'b':

      if (equals(varName, F("build")))
      {
    #if BUILD_IN_WEBFOOTER

        // In the footer, show full build binary name, will be 'firmware.bin' when compiled using Arduino IDE.
        addHtml(get_binary_filename());
    #endif // if BUILD_IN_WEBFOOTER
        return;
      }
      break;
    case 'c':

      if (equals(varName, F("css")))
      {
        serve_favicon();

        /*
                bool defaultCssServed = false;

                if (MENU_INDEX_SETUP == navMenuIndex) {
                  // Serve embedded CSS
                  defaultCssServed = serve_CSS_inline();
                }
                if (!defaultCssServed) {
         */
        serve_CSS(CSSfiles_e::ESPEasy_default);

        //        }
    #if FEATURE_RULES_EASY_COLOR_CODE

        if (!Settings.DisableRulesCodeCompletion() &&
            ((MENU_INDEX_RULES == navMenuIndex) ||
             (MENU_INDEX_CUSTOM_PAGE == navMenuIndex))) {
          serve_CSS(CSSfiles_e::EasyColorCode_codemirror);
        }
    #endif // if FEATURE_RULES_EASY_COLOR_CODE
        return;
      }
      break;
    case 'd':

      if (equals(varName, F("date")))
      {
    #if BUILD_IN_WEBFOOTER

        // Add the compile-date
        addHtml(get_build_date());
    #endif // if BUILD_IN_WEBFOOTER
        return;
      }
      else if (equals(varName, F("debug")))
      {
        // print debug messages - not implemented yet
        return;
      }

      break;
    case 'e':

      if (equals(varName, F("error")))
      {
        getErrorNotifications();
        return;
      }
      break;
    case 'j':

      if (equals(varName, F("js")))
      {
        html_add_JQuery_script();

    #if FEATURE_CHART_JS
        html_add_ChartJS_script();
    #endif // if FEATURE_CHART_JS

    #if FEATURE_RULES_EASY_COLOR_CODE

        if (!Settings.DisableRulesCodeCompletion() &&
            ((MENU_INDEX_RULES == navMenuIndex) ||
             (MENU_INDEX_CUSTOM_PAGE == navMenuIndex))) {
          html_add_Easy_color_code_script();
        }
    #endif // if FEATURE_RULES_EASY_COLOR_CODE

        if (MENU_INDEX_RULES == navMenuIndex) {
          serve_JS(JSfiles_e::SaveRulesFile);
        }

        html_add_autosubmit_form();
        serve_JS(JSfiles_e::Toasting);
        return;
      }
      break;
    case 'l':

      if (equals(varName, F("logo")))
      {
        if (fileExists(F("esp.png")))
        {
          addHtml(F("<img src=\"esp.png\" width=48 height=48 align=right>"));
        }
        return;
      }
      break;
    case 'm':

      if (equals(varName, F("menu")))
      {
        uint8_t menuLoops = 1;
        uint8_t minMenu = 0;
        uint8_t maxMenu = MENU_MAX_INDEX_SHOWN;
        uint8_t navAltIndex = MENU_INDEX_IGNORE;
        if (isGpMenuSecondLevel(navMenuIndex)) {
          // if (navMenuIndex >= MENU_MIN_INTERFACES_SHOWN && navMenuIndex <= MENU_MAX_INTERFACES_SHOWN) { // Enable check if more menus with sub-tab menus are added
            navAltIndex = MENU_INDEX_INTERFACES;
          // }
          menuLoops = 2;
        }
        while (menuLoops > 0) {
        addHtml(strformat(F("<div class='menubar%c'>"), minMenu != 0 ? '2' : ' '));

        for (uint8_t i = minMenu; i <= maxMenu; i++)
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
          addHtmlAttribute(F("class"), ((i == navMenuIndex) || (i == navAltIndex)) ? F("menu active") : F("menu"));
          addHtmlAttribute(F("href"),  getGpMenuURL(i));
          addHtml('>');
          addHtml(getGpMenuIcon(i));
          addHtml(F("<span class='showmenulabel'>"));
          addHtml(getGpMenuLabel(i));
          addHtml(F("</span></a>"));
        }

        addHtml(F("</div>"));
        if (isGpMenuSecondLevel(navMenuIndex)) {
          // if (navMenuIndex >= MENU_MIN_INTERFACES_SHOWN && navMenuIndex <= MENU_MAX_INTERFACES_SHOWN) { // Enable check if more menus with sub-tab menus are added
            minMenu = MENU_MIN_INTERFACES_SHOWN;
            maxMenu = MENU_MAX_INTERFACES_SHOWN;
          // }
        }
        --menuLoops;
        }
        return;
      }
      else if (equals(varName, F("meta"))) {
        if (Rebooting) {
          addHtml(F("<meta http-equiv='refresh' content='10 url=/'>"));
        }
        return;
      }

      break;
    case 'n':

      if (equals(varName, F("name")))
      {
        addHtml(Settings.getHostname());
        return;
      }
      break;
    case 'u':

      if (equals(varName, F("unit")))
      {
        addHtmlInt(Settings.Unit);
        return;
      }

      break;
  }
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
