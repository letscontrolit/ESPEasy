#include "../Helpers/AdafruitGFX_helper.h"
#include "../../_Plugin_Helper.h"

#ifdef PLUGIN_USES_ADAFRUITGFX

# include "../Helpers/StringConverter.h"
# include "../WebServer/Markup_Forms.h"
# if defined(FEATURE_SD) && defined(ADAGFX_ENABLE_BMP_DISPLAY)
#  include <SD.h>
# endif // if defined(FEATURE_SD) && defined(ADAGFX_ENABLE_BMP_DISPLAY)

# if ADAGFX_FONTS_INCLUDED
#  include "src/Static/Fonts/Seven_Segment24pt7b.h"
#  include "src/Static/Fonts/Seven_Segment18pt7b.h"
#  include "src/Static/Fonts/FreeSans9pt7b.h"
#  ifdef ADAGFX_FONTS_EXTRA_8PT_INCLUDED
#   include "src/Static/Fonts/angelina8pt7b.h"
#   include "src/Static/Fonts/NovaMono8pt7b.h"
#   include "src/Static/Fonts/unispace8pt7b.h"
#   include "src/Static/Fonts/unispace_italic8pt7b.h"
#   include "src/Static/Fonts/whitrabt8pt7b.h"
#  endif // ifdef ADAGFX_FONTS_EXTRA_8PT_INCLUDED
#  ifdef ADAGFX_FONTS_EXTRA_12PT_INCLUDED
#   include "src/Static/Fonts/angelina12pt7b.h"
#   include "src/Static/Fonts/NovaMono12pt7b.h"
#   include "src/Static/Fonts/RepetitionScrolling12pt7b.h"
#   include "src/Static/Fonts/unispace12pt7b.h"
#   include "src/Static/Fonts/unispace_italic12pt7b.h"
#   include "src/Static/Fonts/whitrabt12pt7b.h"
#  endif // ifdef ADAGFX_FONTS_EXTRA_12PT_INCLUDED
#  ifdef ADAGFX_FONTS_EXTRA_16PT_INCLUDED
#   include "src/Static/Fonts/AmerikaSans16pt7b.h"
#   include "src/Static/Fonts/whitrabt16pt7b.h"
#  endif // ifdef ADAGFX_FONTS_EXTRA_16PT_INCLUDED
#  ifdef ADAGFX_FONTS_EXTRA_18PT_INCLUDED
#   include "src/Static/Fonts/whitrabt18pt7b.h"
#  endif // ifdef ADAGFX_FONTS_EXTRA_18PT_INCLUDED
#  ifdef ADAGFX_FONTS_EXTRA_20PT_INCLUDED
#   include "src/Static/Fonts/whitrabt20pt7b.h"
#  endif // ifdef ADAGFX_FONTS_EXTRA_20PT_INCLUDED
# endif  // if ADAGFX_FONTS_INCLUDED

/******************************************************************************************
 * get the display text for a 'text print mode' enum value
 *****************************************************************************************/
const __FlashStringHelper* toString(AdaGFXTextPrintMode mode) {
  switch (mode) {
    case AdaGFXTextPrintMode::ContinueToNextLine: return F("Continue to next line");
    case AdaGFXTextPrintMode::TruncateExceedingMessage: return F("Truncate exceeding message");
    case AdaGFXTextPrintMode::ClearThenTruncate: return F("Clear then truncate exceeding message");
    case AdaGFXTextPrintMode::MAX: break;
  }
  return F("None");
}

/******************************************************************************************
 * get the display text for a color depth enum value
 *****************************************************************************************/
const __FlashStringHelper* toString(AdaGFXColorDepth colorDepth) {
  switch (colorDepth) {
    case AdaGFXColorDepth::Monochrome: return F("Monochrome");
    case AdaGFXColorDepth::BlackWhiteRed: return F("Monochrome + 1 color");
    case AdaGFXColorDepth::BlackWhite2Greyscales: return F("Monochrome + 2 grey levels");
    # if ADAGFX_SUPPORT_7COLOR
    case AdaGFXColorDepth::SevenColor: return F("eInk - 7 colors");
    # endif // if ADAGFX_SUPPORT_7COLOR
    case AdaGFXColorDepth::EightColor: return F("TFT - 8 colors");
    case AdaGFXColorDepth::SixteenColor: return F("TFT - 16 colors");
    case AdaGFXColorDepth::FullColor: return F("Full color - 65535 colors");
  }
  return F("None");
}

/*****************************************************************************************
 * Show a selector for all available 'Text print mode' options, for use in PLUGIN_WEBFORM_LOAD
 ****************************************************************************************/
void AdaGFXFormTextPrintMode(const __FlashStringHelper *id,
                             uint8_t                    selectedIndex) {
  const int textModeCount                             = static_cast<int>(AdaGFXTextPrintMode::MAX);
  const __FlashStringHelper *textModes[textModeCount] = { // Be sure to use all available modes from enum!
    toString(AdaGFXTextPrintMode::ContinueToNextLine),
    toString(AdaGFXTextPrintMode::TruncateExceedingMessage),
    toString(AdaGFXTextPrintMode::ClearThenTruncate)
  };
  const int textModeOptions[textModeCount] = {
    static_cast<int>(AdaGFXTextPrintMode::ContinueToNextLine),
    static_cast<int>(AdaGFXTextPrintMode::TruncateExceedingMessage),
    static_cast<int>(AdaGFXTextPrintMode::ClearThenTruncate)
  };

  addFormSelector(F("Text print Mode"), id, textModeCount, textModes, textModeOptions, selectedIndex);
}

void AdaGFXFormColorDepth(const __FlashStringHelper *id,
                          uint16_t                   selectedIndex,
                          bool                       enabled) {
  # if ADAGFX_SUPPORT_7COLOR
  const int colorDepthCount = 7 + 1;
  # else // if ADAGFX_SUPPORT_7COLOR
  const int colorDepthCount = 6 + 1;
  # endif // if ADAGFX_SUPPORT_7COLOR
  const __FlashStringHelper *colorDepths[colorDepthCount] = { // Be sure to use all available modes from enum!
    toString(static_cast<AdaGFXColorDepth>(0)),               // include None
    toString(AdaGFXColorDepth::Monochrome),
    toString(AdaGFXColorDepth::BlackWhiteRed),
    toString(AdaGFXColorDepth::BlackWhite2Greyscales),
    # if ADAGFX_SUPPORT_7COLOR
    toString(AdaGFXColorDepth::SevenColor),
    # endif // if ADAGFX_SUPPORT_7COLOR
    toString(AdaGFXColorDepth::EightColor),
    toString(AdaGFXColorDepth::SixteenColor),
    toString(AdaGFXColorDepth::FullColor)
  };
  const int colorDepthOptions[colorDepthCount] = {
    0,
    static_cast<int>(AdaGFXColorDepth::Monochrome),
    static_cast<int>(AdaGFXColorDepth::BlackWhiteRed),
    static_cast<int>(AdaGFXColorDepth::BlackWhite2Greyscales),
    # if ADAGFX_SUPPORT_7COLOR
    static_cast<int>(AdaGFXColorDepth::SevenColor),
    # endif // if ADAGFX_SUPPORT_7COLOR
    static_cast<int>(AdaGFXColorDepth::EightColor),
    static_cast<int>(AdaGFXColorDepth::SixteenColor),
    static_cast<int>(AdaGFXColorDepth::FullColor)
  };

  addRowLabel_tr_id(F("Display Color-depth"), id);
  addSelector(id, colorDepthCount, colorDepths, colorDepthOptions, NULL, selectedIndex, false, enabled);
}

/*****************************************************************************************
 * Show a selector for Rotation options, supported by Adafruit_GFX
 ****************************************************************************************/
void AdaGFXFormRotation(const __FlashStringHelper *id,
                        uint8_t                    selectedIndex) {
  const __FlashStringHelper *rotationOptions[] = { F("Normal"), F("+90&deg;"), F("+180&deg;"), F("+270&deg;") };
  const int rotationOptionValues[]             = { 0, 1, 2, 3 };

  addFormSelector(F("Rotation"), id, 4, rotationOptions, rotationOptionValues, selectedIndex);
}

/*****************************************************************************************
 * Show a checkbox & note to disable background-fill for text
 ****************************************************************************************/
void AdaGFXFormTextBackgroundFill(const __FlashStringHelper *id,
                                  uint8_t                    selectedIndex) {
  addFormCheckBox(F("Background-fill for text"), id, selectedIndex);
  addFormNote(F("Fill entire line-height with background color."));
}

/*****************************************************************************************
 * Show a checkbox & note to enable col/row mode for txp, txz and txtfull subcommands
 ****************************************************************************************/
void AdaGFXFormTextColRowMode(const __FlashStringHelper *id,
                              bool                       selectedState) {
  addFormCheckBox(F("Text Coordinates in col/row"), id, selectedState);
  addFormNote(F("Unchecked: Coordinates in pixels. Applies only to 'txp', 'txz' and 'txtfull' subcommands."));
}

/*****************************************************************************************
 * Show a checkbox & note to enable -1 px compatibility mode for txp and txtfull subcommands
 ****************************************************************************************/
void AdaGFXFormOnePixelCompatibilityOption(const __FlashStringHelper *id,
                                           uint8_t                    selectedIndex) {
  addFormCheckBox(F("Use -1px offset for txp &amp; txtfull"), id, selectedIndex);
  addFormNote(F("This is for compatibility with the original plugin implementation."));
}

/*****************************************************************************************
 * Show 2 input fields for Foreground and Background color, translated to known color names or hex with # prefix
 ****************************************************************************************/
void AdaGFXFormForeAndBackColors(const __FlashStringHelper *foregroundId,
                                 uint16_t                   foregroundColor,
                                 const __FlashStringHelper *backgroundId,
                                 uint16_t                   backgroundColor,
                                 AdaGFXColorDepth           colorDepth) {
  String color;

  color = AdaGFXcolorToString(foregroundColor, colorDepth);
  addFormTextBox(F("Foreground color"), foregroundId, color, 11);
  color = AdaGFXcolorToString(backgroundColor, colorDepth);
  addFormTextBox(F("Background color"), backgroundId, color, 11);
  addFormNote(F("Use Color name, '#RGB565' (# + 1..4 hex nibbles) or '#RRGGBB' (# + 6 hex nibbles RGB color)."));
  addFormNote(F("NB: Colors stored as RGB565 value!"));
}

/*****************************************************************************************
 * Show a pin selector and percentage 1..100 for Backlight settings
 ****************************************************************************************/
void AdaGFXFormBacklight(const __FlashStringHelper *backlightPinId,
                         int8_t                     backlightPin,
                         const __FlashStringHelper *backlightPercentageId,
                         uint16_t                   backlightPercentage) {
  addFormPinSelect(PinSelectPurpose::Generic_output, formatGpioName_output_optional(F("Backlight ")), backlightPinId, backlightPin);

  addFormNumericBox(F("Backlight percentage"), backlightPercentageId, backlightPercentage, 1, 100);
  addUnit(F("1-100%"));
}

/*****************************************************************************************
 * Show pin selector, inverse option and timeout inputs for Displaybutton settings
 ****************************************************************************************/
void AdaGFXFormDisplayButton(const __FlashStringHelper *buttonPinId,
                             int8_t                     buttonPin,
                             const __FlashStringHelper *buttonInverseId,
                             bool                       buttonInverse,
                             const __FlashStringHelper *displayTimeoutId,
                             int                        displayTimeout) {
  addFormPinSelect(PinSelectPurpose::Generic_input, F("Display button"), buttonPinId, buttonPin);

  addFormCheckBox(F("Inversed Logic"), buttonInverseId, buttonInverse);

  addFormNumericBox(F("Display Timeout"), displayTimeoutId, displayTimeout, 0);
  addUnit(F("0 = off"));
}

/*****************************************************************************************
 * Show a numeric input 1..10 for Font scaling setting
 ****************************************************************************************/
void AdaGFXFormFontScaling(const __FlashStringHelper *fontScalingId,
                           uint8_t                    fontScaling,
                           uint8_t                    maxScale) {
  addFormNumericBox(F("Font scaling"), fontScalingId, fontScaling, 1, maxScale);
  String unit = F("1x..");

  unit += maxScale;
  unit += 'x';
  addUnit(unit);
}

/****************************************************************************
 * AdaGFXparseTemplate: Replace variables and adjust unicode special characters to Adafruit font
 ***************************************************************************/
String AdaGFXparseTemplate(String            & tmpString,
                           uint8_t             lineSize,
                           AdafruitGFX_helper *gfxHelper) {
  // Änderung WDS: Tabelle vorerst Abgeschaltet !!!!
  // Perform some specific changes for LCD display
  // https://www.letscontrolit.com/forum/viewtopic.php?t=2368
  # if ADAGFX_PARSE_SUBCOMMAND

  String result = tmpString;

  if (nullptr != gfxHelper) {
    String trigger = gfxHelper->getTrigger();

    if (!trigger.isEmpty()) {
      int16_t prefixTrigger  = result.indexOf(ADAGFX_PARSE_PREFIX);
      int16_t postfixTrigger = result.indexOf(ADAGFX_PARSE_POSTFIX, prefixTrigger + 1);

      while ((prefixTrigger > -1) && (postfixTrigger > -1) && (postfixTrigger > prefixTrigger)) { // Might be valid
        String subcommand = result.substring(prefixTrigger + ADAGFX_PARSE_POSTFIX_LEN, postfixTrigger);

        if (!subcommand.isEmpty()) {
          String command;
          command.reserve(trigger.length() + 1 + subcommand.length());
          command += trigger;
          command += ',';
          command += subcommand;

          #  ifndef BUILD_NO_DEBUG

          if (loglevelActiveFor(ADAGFX_LOG_LEVEL)) {
            String log;
            log.reserve(command.length() + 20);
            log += F("AdaGFX: inline cmd: ");
            log += command;
            addLogMove(ADAGFX_LOG_LEVEL, log);
          }
          #  endif // ifndef BUILD_NO_DEBUG

          if (gfxHelper->processCommand(command)) { // Execute command and remove from result incl. pre/postfix
            result.remove(prefixTrigger, (postfixTrigger - prefixTrigger) + ADAGFX_PARSE_POSTFIX_LEN);
            prefixTrigger  = result.indexOf(ADAGFX_PARSE_PREFIX);
            postfixTrigger = result.indexOf(ADAGFX_PARSE_POSTFIX, prefixTrigger + 1);
          } else { // If the command fails, exit further processing
            prefixTrigger  = -1;
            postfixTrigger = -1;
            #  ifndef BUILD_NO_DEBUG
            addLog(ADAGFX_LOG_LEVEL, F("AdaGFX: inline cmd: unknown"));
            #  endif // ifndef BUILD_NO_DEBUG
          }
        } else {
          prefixTrigger  = -1;
          postfixTrigger = -1;
        }
      }
    }
  }
  result = parseTemplate_padded(result, lineSize);
  # else // if ADAGFX_PARSE_SUBCOMMAND
  String result = parseTemplate_padded(tmpString, lineSize);
  # endif // if ADAGFX_PARSE_SUBCOMMAND

  const char euro[4]       = { 0xe2, 0x82, 0xac, 0 }; // Unicode euro symbol
  const char euro_ascii[2] = { 0xED, 0 };             // Euro symbol
  result.replace(euro, euro_ascii);

  char unicodePrefix = 0xc2;

  if (result.indexOf(unicodePrefix) != -1) {
    const char degree[3]       = { 0xc2, 0xb0, 0 };  // Unicode degree symbol
    const char degree_ascii[2] = { 0xf7, 0 };        // degree symbol
    result.replace(degree, degree_ascii);

    const char pound[3]       = { 0xc2, 0xa3, 0 };   // Unicode pound symbol
    const char pound_ascii[2] = { 0x9C, 0 };         // pound symbol
    result.replace(pound, pound_ascii);

    const char yen[3]       = { 0xc2, 0xa5, 0 };     // Unicode yen symbol
    const char yen_ascii[2] = { 0x9D, 0 };           // yen symbol
    result.replace(yen, yen_ascii);

    const char cent[3]       = { 0xc2, 0xa2, 0 };    // Unicode cent symbol
    const char cent_ascii[2] = { 0x9B, 0 };          // cent symbol
    result.replace(cent, cent_ascii);

    const char mu[3]       = { 0xc2, 0xb5, 0 };      // Unicode mu/micro (µ) symbol
    const char mu_ascii[2] = { 0xe5, 0 };            // mu/micro symbol
    result.replace(mu, mu_ascii);

    const char plusmin[3]       = { 0xc2, 0xb1, 0 }; // Unicode plusminus symbol
    const char plusmin_ascii[2] = { 0xf0, 0 };       // plusminus symbol
    result.replace(plusmin, plusmin_ascii);

    const char laquo[3]       = { 0xc2, 0xab, 0 };   // Unicode left aquo symbol
    const char laquo_ascii[2] = { 0xae, 0 };         // left aquo symbol
    result.replace(laquo, laquo_ascii);

    const char raquo[3]       = { 0xc2, 0xbb, 0 };   // Unicode right aquote symbol
    const char raquo_ascii[2] = { 0xaf, 0 };         // right aquote symbol
    result.replace(raquo, raquo_ascii);

    const char half[3]       = { 0xc2, 0xbd, 0 };    // Unicode half 1/2 symbol
    const char half_ascii[2] = { 0xab, 0 };          // half 1/2 symbol
    result.replace(half, half_ascii);

    const char quart[3]       = { 0xc2, 0xbc, 0 };   // Unicode quart 1/4 symbol
    const char quart_ascii[2] = { 0xac, 0 };         // quart 1/4 symbol
    result.replace(quart, quart_ascii);
    delay(0);
  }

  unicodePrefix = 0xc3;

  if (result.indexOf(unicodePrefix) != -1) {
    // See: https://github.com/letscontrolit/ESPEasy/issues/2081

    const char umlautAE_uni[3]   = { 0xc3, 0x84, 0 };  // Unicode Umlaute AE
    const char umlautAE_ascii[2] = { 0x8e, 0 };        // Umlaute A
    result.replace(umlautAE_uni, umlautAE_ascii);

    const char umlaut_ae_uni[3]  = { 0xc3, 0xa4, 0 };  // Unicode Umlaute ae
    const char umlautae_ascii[2] = { 0x84, 0 };        // Umlaute a
    result.replace(umlaut_ae_uni, umlautae_ascii);

    const char umlautOE_uni[3]   = { 0xc3, 0x96, 0 };  // Unicode Umlaute OE
    const char umlautOE_ascii[2] = { 0x99, 0 };        // Umlaute O
    result.replace(umlautOE_uni, umlautOE_ascii);

    const char umlaut_oe_uni[3]  = { 0xc3, 0xb6, 0 };  // Unicode Umlaute oe
    const char umlautoe_ascii[2] = { 0x94, 0 };        // Umlaute o
    result.replace(umlaut_oe_uni, umlautoe_ascii);

    const char umlautUE_uni[3]   = { 0xc3, 0x9c, 0 };  // Unicode Umlaute UE
    const char umlautUE_ascii[2] = { 0x9a, 0 };        // Umlaute U
    result.replace(umlautUE_uni, umlautUE_ascii);

    const char umlaut_ue_uni[3]  = { 0xc3, 0xbc, 0 };  // Unicode Umlaute ue
    const char umlautue_ascii[2] = { 0x81, 0 };        // Umlaute u
    result.replace(umlaut_ue_uni, umlautue_ascii);

    const char divide_uni[3]   = { 0xc3, 0xb7, 0 };    // Unicode divide symbol
    const char divide_ascii[2] = { 0xf5, 0 };          // Divide symbol
    result.replace(divide_uni, divide_ascii);

    const char umlaut_sz_uni[3]   = { 0xc3, 0x9f, 0 }; // Unicode Umlaute sz
    const char umlaut_sz_ascii[2] = { 0xe0, 0 };       // Umlaute
    result.replace(umlaut_sz_uni, umlaut_sz_ascii);
    delay(0);
  }

  for (uint16_t l = result.length(); l > 0 && isSpace(result[l - 1]); l--) { // Right-trim
    result.remove(l - 1);
  }

  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(ADAGFX_LOG_LEVEL)) {
    String log;
    log.reserve(result.length() + 24);
    log += F("AdaGFX: parse result: '");
    log += result;
    log += '\'';
    addLogMove(ADAGFX_LOG_LEVEL, log);
  }
  # endif // ifndef BUILD_NO_DEBUG
  return result;
}

// AdafruitGFX_helper class methods

/****************************************************************************
 * parameterized constructors
 ***************************************************************************/
AdafruitGFX_helper::AdafruitGFX_helper(Adafruit_GFX       *display,
                                       const String      & trigger,
                                       uint16_t            res_x,
                                       uint16_t            res_y,
                                       AdaGFXColorDepth    colorDepth,
                                       AdaGFXTextPrintMode textPrintMode,
                                       uint8_t             fontscaling,
                                       uint16_t            fgcolor,
                                       uint16_t            bgcolor,
                                       bool                useValidation,
                                       bool                textBackFill)
  : _display(display), _trigger(trigger), _res_x(res_x), _res_y(res_y), _colorDepth(colorDepth),
  _textPrintMode(textPrintMode), _fontscaling(fontscaling), _fgcolor(fgcolor), _bgcolor(bgcolor),
  _useValidation(useValidation), _textBackFill(textBackFill)
{
  addLog(LOG_LEVEL_INFO, F("AdaGFX_helper: GFX Init."));
  initialize();
}

# ifdef ADAGFX_ENABLE_BMP_DISPLAY
AdafruitGFX_helper::AdafruitGFX_helper(Adafruit_SPITFT    *display,
                                       const String      & trigger,
                                       uint16_t            res_x,
                                       uint16_t            res_y,
                                       AdaGFXColorDepth    colorDepth,
                                       AdaGFXTextPrintMode textPrintMode,
                                       uint8_t             fontscaling,
                                       uint16_t            fgcolor,
                                       uint16_t            bgcolor,
                                       bool                useValidation,
                                       bool                textBackFill)
  : _tft(display), _trigger(trigger), _res_x(res_x), _res_y(res_y), _colorDepth(colorDepth),
  _textPrintMode(textPrintMode), _fontscaling(fontscaling), _fgcolor(fgcolor), _bgcolor(bgcolor),
  _useValidation(useValidation), _textBackFill(textBackFill)
{
  _display = _tft;
  addLog(LOG_LEVEL_INFO, F("AdaGFX_helper: TFT Init."));
  initialize();
}

# endif // ifdef ADAGFX_ENABLE_BMP_DISPLAY

void AdafruitGFX_helper::initialize()
{
  _trigger.toLowerCase(); // store trigger in lowercase
  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(ADAGFX_LOG_LEVEL)) {
    String log;
    log.reserve(65);
    log += F("AdaGFX: Init, x: ");
    log += _res_x;
    log += F(", y: ");
    log += _res_y;
    log += F(", colors: ");
    log += static_cast<uint16_t>(_colorDepth);
    log += F(", trigger: ");
    log += _trigger;
    addLogMove(ADAGFX_LOG_LEVEL, log);
  }
  # endif // ifndef BUILD_NO_DEBUG

  _display_x = _res_x; // Store initial resolution
  _display_y = _res_y;

  if (_fontscaling < 1) { _fontscaling = 1; }

  if (nullptr != _display) {
    _display->setTextSize(_fontscaling);
    _display->setTextColor(_fgcolor, _bgcolor); // initialize text colors
    _display->setTextWrap(_textPrintMode == AdaGFXTextPrintMode::ContinueToNextLine);
  }
}

/****************************************************************************
 * getCursorXY: get the current (text) cursor coordinates, either in pixels or cols/rows, depending on related setting
 ***************************************************************************/
void AdafruitGFX_helper::getCursorXY(int16_t& currentX,
                                     int16_t& currentY) {
  _lastX = _display->getCursorX();
  _lastY = _display->getCursorY();

  if (_columnRowMode && (_lastX != 0)) { _lastX /= _fontwidth; }

  if (_columnRowMode && (_lastY != 0)) { _lastY /= _fontheight; }
  currentX = _lastX;
  currentY = _lastY;
}

/****************************************************************************
 * processCommand: Parse string to <command>,<subcommand>[,<arguments>...] and execute that command
 ***************************************************************************/
bool AdafruitGFX_helper::processCommand(const String& string) {
  bool success = false;

  if ((nullptr == _display) || _trigger.isEmpty()) { return success; }

  String cmd        = parseString(string, 1); // lower case
  String subcommand = parseString(string, 2);

  if (!cmd.equals(_trigger) || subcommand.isEmpty()) { return success; } // Only support own trigger, and at least a non=empty subcommand

  String log;
  String sParams[ADAGFX_PARSE_MAX_ARGS + 1];
  int    nParams[ADAGFX_PARSE_MAX_ARGS + 1];
  int    argCount = 0;
  bool   loop     = true;

  while (argCount <= ADAGFX_PARSE_MAX_ARGS && loop) {
    sParams[argCount] = parseStringKeepCase(string, argCount + 3); // 0-offset + 1st and 2nd argument used by trigger/subcommand
    validIntFromString(sParams[argCount], nParams[argCount]);
    loop = !sParams[argCount].isEmpty();

    # ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG_DEV)) {
      log  = ':';
      log += argCount;
      log += ' ';
      log += sParams[argCount];
      addLog(LOG_LEVEL_DEBUG_DEV, log);
    }
    # endif // ifndef BUILD_NO_DEBUG

    if (loop) { argCount++; }
  }
  success = true; // If we get this far, we'll flip the flag if something wrong is found

  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(ADAGFX_LOG_LEVEL)) {
    log.reserve(90);
    log.clear();
    log += F("AdaGFX: command: ");
    log += _trigger;
    log += F(" argCount: ");
    log += argCount;
    log += ':';
    log += string;
    addLog(ADAGFX_LOG_LEVEL, log);
  }
  # endif // ifndef BUILD_NO_DEBUG

  if (subcommand.equals(F("txt")))                          // txt: Print text at last cursor position, ends at next line!
  {
    _display->println(parseStringToEndKeepCase(string, 3)); // Print entire rest of provided line
  }
  else if (subcommand.equals(F("txp")) && (argCount == 2))  // txp: Text position
  {
    # if ADAGFX_ARGUMENT_VALIDATION

    if (invalidCoordinates(nParams[0], nParams[1], _columnRowMode)) {
      success = false;
    } else
    # endif // if ADAGFX_ARGUMENT_VALIDATION
    {
      if (_columnRowMode) {
        _display->setCursor(nParams[0] * _fontwidth, nParams[1] * _fontheight);
      } else {
        _display->setCursor(nParams[0] - _p095_compensation, nParams[1] - _p095_compensation);
      }
    }
  }
  else if (subcommand.equals(F("txz")) && (argCount >= 3)) // txz: Text at position
  {
    # if ADAGFX_ARGUMENT_VALIDATION

    if (invalidCoordinates(nParams[0], nParams[1], _columnRowMode)) {
      success = false;
    } else
    # endif // if ADAGFX_ARGUMENT_VALIDATION
    {
      if (_columnRowMode) {
        _display->setCursor(nParams[0] * _fontwidth, nParams[1] * _fontheight);
      } else {
        _display->setCursor(nParams[0], nParams[1]);
      }
      _display->println(parseStringToEndKeepCase(string, 5));                   // Print entire rest of provided line
    }
  }
  else if (subcommand.equals(F("txc")) && ((argCount == 1) || (argCount == 2))) // txc: Textcolor, fg and opt. bg colors
  {
    _fgcolor = AdaGFXparseColor(sParams[0], _colorDepth);

    if (argCount == 1) {
      _bgcolor = _fgcolor; // Transparent background
      _display->setTextColor(_fgcolor);
    } else {               // argCount=2
      _bgcolor = AdaGFXparseColor(sParams[1], _colorDepth);
      _display->setTextColor(_fgcolor, _bgcolor);
    }
  }
  else if (subcommand.equals(F("txs")) && (argCount == 1))
  {
    if ((nParams[0] >= 0) || (nParams[0] <= 10)) {
      _fontscaling = nParams[0];
      _display->setTextSize(_fontscaling);
      calculateTextMetrics(_fontwidth, _fontheight, _heightOffset, _isProportional);
    } else {
      success = false;
    }
  }
  else if (subcommand.equals(F("txtfull")) && (argCount >= 3) && (argCount <= 6)) { // txtfull: Text at position, with size and color
    switch (argCount) {
      case 3:                                                                       // single text

        # if ADAGFX_ARGUMENT_VALIDATION

        if (invalidCoordinates(nParams[0] - _p095_compensation, nParams[1] - _p095_compensation, _columnRowMode)) {
          success = false;
        } else
        # endif // if ADAGFX_ARGUMENT_VALIDATION
        {
          printText(sParams[2].c_str(),
                    nParams[0] - _p095_compensation,
                    nParams[1] - _p095_compensation,
                    _fontscaling,
                    _fgcolor,
                    _fgcolor); // transparent bg
        }
        break;
      case 4:                  // text + size

        # if ADAGFX_ARGUMENT_VALIDATION

        if (invalidCoordinates(nParams[0] - _p095_compensation, nParams[1] - _p095_compensation, _columnRowMode)) {
          success = false;
        } else
        # endif // if ADAGFX_ARGUMENT_VALIDATION
        {
          printText(sParams[3].c_str(),
                    nParams[0] - _p095_compensation,
                    nParams[1] - _p095_compensation,
                    nParams[2],
                    _fgcolor,
                    _fgcolor); // transparent bg
        }
        break;
      case 5:                  // text + size + color

        # if ADAGFX_ARGUMENT_VALIDATION

        if (invalidCoordinates(nParams[0] - _p095_compensation, nParams[1] - _p095_compensation, _columnRowMode)) {
          success = false;
        } else
        # endif // if ADAGFX_ARGUMENT_VALIDATION
        {
          uint16_t color = AdaGFXparseColor(sParams[3], _colorDepth);
          printText(sParams[4].c_str(),
                    nParams[0] - _p095_compensation,
                    nParams[1] - _p095_compensation,
                    nParams[2],
                    color,
                    color); // transparent bg
        }
        break;
      case 6:               // text + size + color + bkcolor

        # if ADAGFX_ARGUMENT_VALIDATION

        if (invalidCoordinates(nParams[0] - _p095_compensation, nParams[1] - _p095_compensation, _columnRowMode)) {
          success = false;
        } else
        # endif // if ADAGFX_ARGUMENT_VALIDATION
        {
          printText(sParams[5].c_str(),
                    nParams[0] - _p095_compensation,
                    nParams[1] - _p095_compensation,
                    nParams[2],
                    AdaGFXparseColor(sParams[3], _colorDepth),
                    AdaGFXparseColor(sParams[4], _colorDepth));
        }
        break;
      default:
        success = false;
        break;
    }
  }
  else if (subcommand.equals(F("clear"))) // Clear display
  {
    if (argCount >= 1) {
      _display->fillScreen(AdaGFXparseColor(sParams[0], _colorDepth));
    } else {
      _display->fillScreen(_bgcolor);
    }
  }
  else if (subcommand.equals(F("rot")) && (argCount == 1)) // Rotation
  {
    if ((nParams[0] < 0) || (nParams[0] > 3)) {
      success = false;
    } else {
      setRotation(nParams[0]);
    }
  }
  else if (subcommand.equals(F("tpm")) && (argCount == 1)) // Text Print Mode
  {
    if ((nParams[0] < 0) || (nParams[0] >= static_cast<int>(AdaGFXTextPrintMode::MAX))) {
      success = false;
    } else {
      _textPrintMode = static_cast<AdaGFXTextPrintMode>(nParams[0]);
      _display->setTextWrap(_textPrintMode == AdaGFXTextPrintMode::ContinueToNextLine);
    }
  }
  # if ADAGFX_USE_ASCIITABLE
  else if (subcommand.equals(F("asciitable"))) // Show ASCII table
  {
    String  line;
    int16_t start        = 0x80 + (argCount >= 1 && nParams[0] >= -4 && nParams[0] < 4 ? nParams[0] * 0x20 : 0);
    uint8_t scale        = (argCount == 2 && nParams[1] > 0 && nParams[1] <= 10 ? nParams[1] : 2);
    uint8_t currentScale = _fontscaling;

    if (_fontscaling != scale) { // Set fontscaling
      _fontscaling = scale;
      _display->setTextSize(_fontscaling);
      calculateTextMetrics(_fontwidth, _fontheight, _heightOffset, _isProportional);
    }
    line.reserve(_textcols);
    _display->setCursor(0, 0);
    int16_t row     = 0;
    bool    colMode = _columnRowMode;
    _columnRowMode = true;

    for (int16_t i = start; i <= 0xFF && row < _textrows; i++) {
      if ((i % 4 == 0) && (line.length() > (_textcols - 8u))) { // 8 = 4x space + char
        printText(line.c_str(), 0, row, _fontscaling, _fgcolor, _bgcolor);
        line.clear();
        row++;
      }

      if (line.isEmpty()) {
        line += F("0x");

        if (i < 0x10) { line += '0'; }
        line += String(i, HEX);
      }
      line += ' ';
      line += static_cast<char>(((i == 0x0A) || (i == 0x0D) ? 0x20 : i)); // Show a space instead of CR/LF
    }

    if (row < _textrows) {
      printText(line.c_str(), 0, row, _fontscaling, _fgcolor, _bgcolor);
    }

    _columnRowMode = colMode;           // Restore

    if (_fontscaling != currentScale) { // Restore if needed
      _fontscaling = currentScale;
      _display->setTextSize(_fontscaling);
      calculateTextMetrics(_fontwidth, _fontheight, _heightOffset, _isProportional);
    }
  }
  # endif // if ADAGFX_USE_ASCIITABLE
  else if (subcommand.equals(F("font")) && (argCount == 1)) { // font: Change font
    # if ADAGFX_FONTS_INCLUDED
    sParams[0].toLowerCase();

    if (sParams[0].equals(F("sevenseg24"))) {
      _display->setFont(&Seven_Segment24pt7b);
      calculateTextMetrics(21, 42, 35);
    } else if (sParams[0].equals(F("sevenseg18"))) {
      _display->setFont(&Seven_Segment18pt7b);
      calculateTextMetrics(16, 32, 25);
    } else if (sParams[0].equals(F("freesans"))) {
      _display->setFont(&FreeSans9pt7b);
      calculateTextMetrics(10, 16, 12);

      // Extra 8pt fonts:
    #  ifdef ADAGFX_FONTS_EXTRA_8PT_INCLUDED
    #   ifdef ADAGFX_FONTS_EXTRA_8PT_ANGELINA
    } else if (sParams[0].equals(F("angelina8prop"))) { // Proportional font!
      _display->setFont(&angelina8pt7b);
      calculateTextMetrics(6, 16, 12, true);
    #   endif // ifdef ADAGFX_FONTS_EXTRA_8PT_ANGELINA
    #   ifdef ADAGFX_FONTS_EXTRA_8PT_NOVAMONO
    } else if (sParams[0].equals(F("novamono8pt"))) {
      _display->setFont(&NovaMono8pt7b);
      calculateTextMetrics(9, 16, 12);
    #   endif // ifdef ADAGFX_FONTS_EXTRA_8PT_NOVAMONO
    #   ifdef ADAGFX_FONTS_EXTRA_8PT_UNISPACE
    } else if (sParams[0].equals(F("unispace8pt"))) {
      _display->setFont(&unispace8pt7b);
      calculateTextMetrics(13, 24, 20);
    #   endif // ifdef ADAGFX_FONTS_EXTRA_8PT_UNISPACE
    #   ifdef ADAGFX_FONTS_EXTRA_8PT_UNISPACEITALIC
    } else if (sParams[0].equals(F("unispaceitalic8pt"))) {
      _display->setFont(&unispace_italic8pt7b);
      calculateTextMetrics(13, 24, 20);
    #   endif // ifdef ADAGFX_FONTS_EXTRA_8PT_UNISPACEITALIC
    #   ifdef ADAGFX_FONTS_EXTRA_8PT_WHITERABBiT
    } else if (sParams[0].equals(F("whiterabbit8pt"))) {
      _display->setFont(&whitrabt8pt7b);
      calculateTextMetrics(10, 16, 12);
    #   endif // ifdef ADAGFX_FONTS_EXTRA_8PT_WHITERABBiT
    #  endif  // ifdef ADAGFX_FONTS_EXTRA_8PT_INCLUDED
      // Extra 12pt fonts:
    #  ifdef ADAGFX_FONTS_EXTRA_12PT_INCLUDED
    #   ifdef ADAGFX_FONTS_EXTRA_12PT_ANGELINA
    } else if (sParams[0].equals(F("angelina12prop"))) { // Proportional font!
      _display->setFont(&angelina12pt7b);
      calculateTextMetrics(8, 22, 18, true);
    #   endif // ifdef ADAGFX_FONTS_EXTRA_12PT_ANGELINA
    #   ifdef ADAGFX_FONTS_EXTRA_12PT_NOVAMONO
    } else if (sParams[0].equals(F("novamono12pt"))) {
      _display->setFont(&NovaMono12pt7b);
      calculateTextMetrics(13, 26, 22);
    #   endif // ifdef ADAGFX_FONTS_EXTRA_12PT_NOVAMONO
    #   ifdef ADAGFX_FONTS_EXTRA_12PT_REPETITIONSCROLLiNG
    } else if (sParams[0].equals(F("repetitionscrolling12pt"))) {
      _display->setFont(&RepetitionScrolling12pt7b);
      calculateTextMetrics(13, 22, 18);
    #   endif // ifdef ADAGFX_FONTS_EXTRA_12PT_REPETITIONSCROLLiNG
    #   ifdef ADAGFX_FONTS_EXTRA_12PT_UNISPACE
    } else if (sParams[0].equals(F("unispace12pt"))) {
      _display->setFont(&unispace12pt7b);
      calculateTextMetrics(18, 30, 26);
    #   endif // ifdef ADAGFX_FONTS_EXTRA_12PT_UNISPACE
    #   ifdef ADAGFX_FONTS_EXTRA_12PT_UNISPACEITALIC
    } else if (sParams[0].equals(F("unispaceitalic12pt"))) {
      _display->setFont(&unispace_italic12pt7b);
      calculateTextMetrics(18, 30, 26);
    #   endif // ifdef ADAGFX_FONTS_EXTRA_12PT_UNISPACEITALIC
    #   ifdef ADAGFX_FONTS_EXTRA_12PT_WHITERABBiT
    } else if (sParams[0].equals(F("whiterabbit12pt"))) {
      _display->setFont(&whitrabt12pt7b);
      calculateTextMetrics(13, 20, 16);
    #   endif // ifdef ADAGFX_FONTS_EXTRA_12PT_WHITERABBiT
    #  endif  // ifdef ADAGFX_FONTS_EXTRA_12PT_INCLUDED
    #  ifdef ADAGFX_FONTS_EXTRA_16PT_INCLUDED
    #   ifdef ADAGFX_FONTS_EXTRA_16PT_AMERIKASANS
    } else if (sParams[0].equals(F("amerikasans16pt"))) { // Proportional font!
      _display->setFont(&AmerikaSans16pt7b);
      calculateTextMetrics(17, 30, 26, true);
    #   endif // ifdef ADAGFX_FONTS_EXTRA_16PT_AMERIKASANS
    #   ifdef ADAGFX_FONTS_EXTRA_16PT_WHITERABBiT
    } else if (sParams[0].equals(F("whiterabbit16pt"))) {
      _display->setFont(&whitrabt16pt7b);
      calculateTextMetrics(18, 26, 22);
    #   endif // ifdef ADAGFX_FONTS_EXTRA_16PT_WHITERABBiT
    #  endif  // ifdef ADAGFX_FONTS_EXTRA_16PT_INCLUDED
    #  ifdef ADAGFX_FONTS_EXTRA_18PT_INCLUDED
    #   ifdef ADAGFX_FONTS_EXTRA_18PT_WHITERABBiT
    } else if (sParams[0].equals(F("whiterabbit18pt"))) {
      _display->setFont(&whitrabt18pt7b);
      calculateTextMetrics(21, 30, 26);
      #   endif // ifdef ADAGFX_FONTS_EXTRA_18PT_WHITERABBiT
    #  endif    // ifdef ADAGFX_FONTS_EXTRA_18PT_WHITERABBiT
    #  ifdef ADAGFX_FONTS_EXTRA_20PT_INCLUDED
    #   ifdef ADAGFX_FONTS_EXTRA_20PT_WHITERABBiT
    } else if (sParams[0].equals(F("whiterabbit20pt"))) {
      _display->setFont(&whitrabt20pt7b);
      calculateTextMetrics(24, 32, 28);
    #   endif // ifdef ADAGFX_FONTS_EXTRA_20PT_WHITERABBiT
    #  endif  // ifdef ADAGFX_FONTS_EXTRA_20PT_INCLUDED
    } else if (sParams[0].equals(F("default"))) { // font,default is always available!
      _display->setFont();
      calculateTextMetrics(6, 10);
    } else {
      success = false;
    }
    # else // if ADAGFX_FONTS_INCLUDED
    success = false;
    # endif  // if ADAGFX_FONTS_INCLUDED
  }
  else if (subcommand.equals(F("l")) && (argCount == 5)) { // l: Line
    # if ADAGFX_ARGUMENT_VALIDATION

    if (invalidCoordinates(nParams[0], nParams[1]) ||
        invalidCoordinates(nParams[2], nParams[3])) {
      success = false;
    } else
    # endif // if ADAGFX_ARGUMENT_VALIDATION
    {
      _display->drawLine(nParams[0], nParams[1], nParams[2], nParams[3], AdaGFXparseColor(sParams[4], _colorDepth));
    }
  }
  else if (subcommand.equals(F("lh")) && (argCount == 3)) { // lh: Horizontal line
    # if ADAGFX_ARGUMENT_VALIDATION

    if ((nParams[0] < 0) || (nParams[0] > _res_x)) {
      success = false;
    } else
    # endif // if ADAGFX_ARGUMENT_VALIDATION
    {
      _display->drawFastHLine(0, nParams[0], nParams[1], AdaGFXparseColor(sParams[2], _colorDepth));
    }
  }
  else if (subcommand.equals(F("lv")) && (argCount == 3)) { // lv: Vertical line
    # if ADAGFX_ARGUMENT_VALIDATION

    if ((nParams[0] < 0) || (nParams[0] > _res_y)) {
      success = false;
    } else
    # endif // if ADAGFX_ARGUMENT_VALIDATION
    {
      _display->drawFastVLine(nParams[0], 0, nParams[1], AdaGFXparseColor(sParams[2], _colorDepth));
    }
  }
  # if ADAGFX_ENABLE_EXTRA_CMDS
  else if ((subcommand.equals(F("lm")) || subcommand.equals(F("lmr"))) && (argCount >= 5)) { // lm/lmr: Multi-line, multiple coordinates
    uint16_t mcolor   = AdaGFXparseColor(sParams[0], _colorDepth);
    bool     mloop    = true;
    uint8_t  parCount = 0;
    uint8_t  optCount = 0;
    int  cx           = -1;
    int  cy           = -1;
    bool closeLine    = false;
    bool relativeMode = subcommand.equals(F("lmr")); // Use Relative mode
    #  ifndef BUILD_NO_DEBUG
    String log;
    log.reserve(40);
    #  endif // ifndef BUILD_NO_DEBUG

    while (mloop) {
      sParams[optCount] = parseString(string, parCount + 4);       // 0-offset + 1st and 2nd cmd-argument and 1 for color argument

      if (!validIntFromString(sParams[optCount], nParams[optCount]) && !sParams[optCount].isEmpty()) {
        mcolor = AdaGFXparseColor(sParams[optCount], _colorDepth); // Interpret as a color

        if (optCount > 0) { optCount--; }
      }
      mloop     = !sParams[optCount].isEmpty();
      closeLine = sParams[optCount].equals(F("c"));

      if (mloop) { parCount++; optCount++; } // Next argument

      if ((optCount == 4) || closeLine) { // 0..3 = 4th argument or close the line
        if (relativeMode) {
          nParams[2] += nParams[0];
          nParams[3] += nParams[1];
        }
        #  if ADAGFX_ARGUMENT_VALIDATION

        if (invalidCoordinates(nParams[0], nParams[1]) ||
            invalidCoordinates(nParams[2], nParams[3])) {
          success = false;
          mloop   = false; // break out
        } else
        #  endif // if ADAGFX_ARGUMENT_VALIDATION
        {
          if (closeLine) {
            nParams[2] = cx;
            nParams[3] = cy;
            mloop      = false; // Exit after closing the line
          }
          #  ifndef BUILD_NO_DEBUG
          log.clear();
          log += F("AdaGFX: cmd: lm x/y/x1/y1:");
          log += nParams[0];
          log += '/';
          log += nParams[1];
          log += '/';
          log += nParams[2];
          log += '/';
          log += nParams[3];
          log += F(" loop:");
          log += mloop ? 'T' : 'f';
          log += F(" color:");
          log += AdaGFXcolorToString(mcolor, _colorDepth);
          addLog(LOG_LEVEL_INFO, log);
          #  endif // ifndef BUILD_NO_DEBUG
          _display->drawLine(nParams[0], nParams[1], nParams[2], nParams[3], mcolor);

          if ((cx == -1) && (cy == -1)) {
            cx = nParams[0];
            cy = nParams[1];
          }
          nParams[0] = nParams[2]; // Move second set to first set
          nParams[1] = nParams[3];
          optCount   = 2;          // Get second set of arguments only
        }
      }
    }
  }
  # endif // if ADAGFX_ENABLE_EXTRA_CMDS
  else if (subcommand.equals(F("r")) && (argCount == 5)) { // r: Rectangle
    # if ADAGFX_ARGUMENT_VALIDATION

    if (invalidCoordinates(nParams[0], nParams[1]) ||
        invalidCoordinates(nParams[0] + nParams[2], nParams[1] + nParams[3])) {
      success = false;
    } else
    # endif // if ADAGFX_ARGUMENT_VALIDATION
    {
      _display->drawRect(nParams[0], nParams[1], nParams[2], nParams[3], AdaGFXparseColor(sParams[4], _colorDepth));
    }
  }
  else if (subcommand.equals(F("rf")) && (argCount == 6)) { // rf: Rectangled, filled
    # if ADAGFX_ARGUMENT_VALIDATION

    if (invalidCoordinates(nParams[0], nParams[1]) ||
        invalidCoordinates(nParams[0] + nParams[2], nParams[1] + nParams[3])) {
      success = false;
    } else
    # endif // if ADAGFX_ARGUMENT_VALIDATION
    {
      _display->fillRect(nParams[0], nParams[1], nParams[2], nParams[3], AdaGFXparseColor(sParams[5], _colorDepth));
      _display->drawRect(nParams[0], nParams[1], nParams[2], nParams[3], AdaGFXparseColor(sParams[4], _colorDepth));
    }
  }
  else if (subcommand.equals(F("c")) && (argCount == 4)) { // c: Circle
    # if ADAGFX_ARGUMENT_VALIDATION

    if (invalidCoordinates(nParams[0], nParams[1]) ||
        invalidCoordinates(nParams[2], 0)) { // Also check radius
      success = false;
    } else
    # endif  // if ADAGFX_ARGUMENT_VALIDATION
    {
      _display->drawCircle(nParams[0], nParams[1], nParams[2], AdaGFXparseColor(sParams[3], _colorDepth));
    }
  }
  else if (subcommand.equals(F("cf")) && (argCount == 5)) { // cf: Circle, filled
    # if ADAGFX_ARGUMENT_VALIDATION

    if (invalidCoordinates(nParams[0], nParams[1]) ||
        invalidCoordinates(nParams[2], 0)) { // Also check radius
      success = false;
    } else
    # endif  // if ADAGFX_ARGUMENT_VALIDATION
    {
      _display->fillCircle(nParams[0], nParams[1], nParams[2], AdaGFXparseColor(sParams[4], _colorDepth));
      _display->drawCircle(nParams[0], nParams[1], nParams[2], AdaGFXparseColor(sParams[3], _colorDepth));
    }
  }
  else if (subcommand.equals(F("t")) && (argCount == 7)) { // t: Triangle
    # if ADAGFX_ARGUMENT_VALIDATION

    if (invalidCoordinates(nParams[0], nParams[1]) ||
        invalidCoordinates(nParams[2], nParams[3]) ||
        invalidCoordinates(nParams[4], nParams[5])) {
      success = false;
    } else
    # endif // if ADAGFX_ARGUMENT_VALIDATION
    {
      _display->drawTriangle(nParams[0], nParams[1], nParams[2], nParams[3], nParams[4], nParams[5],
                             AdaGFXparseColor(sParams[6], _colorDepth));
    }
  }
  else if (subcommand.equals(F("tf")) && (argCount == 8)) { // tf: Triangle, filled
    # if ADAGFX_ARGUMENT_VALIDATION

    if (invalidCoordinates(nParams[0], nParams[1]) ||
        invalidCoordinates(nParams[2], nParams[3]) ||
        invalidCoordinates(nParams[4], nParams[5])) {
      success = false;
    } else
    # endif // if ADAGFX_ARGUMENT_VALIDATION
    {
      _display->fillTriangle(nParams[0],
                             nParams[1],
                             nParams[2],
                             nParams[3],
                             nParams[4],
                             nParams[5],
                             AdaGFXparseColor(sParams[7], _colorDepth));
      _display->drawTriangle(nParams[0],
                             nParams[1],
                             nParams[2],
                             nParams[3],
                             nParams[4],
                             nParams[5],
                             AdaGFXparseColor(sParams[6], _colorDepth));
    }
  }
  else if (subcommand.equals(F("rr")) && (argCount == 6)) { // rr: Rounded rectangle
    # if ADAGFX_ARGUMENT_VALIDATION

    if (invalidCoordinates(nParams[0], nParams[1]) ||
        invalidCoordinates(nParams[0] + nParams[2], nParams[1] + nParams[3]) ||
        invalidCoordinates(nParams[4],              0)) { // Also check radius
      success = false;
    } else
    # endif  // if ADAGFX_ARGUMENT_VALIDATION
    {
      _display->drawRoundRect(nParams[0], nParams[1], nParams[2], nParams[3], nParams[4], AdaGFXparseColor(sParams[5], _colorDepth));
    }
  }
  else if (subcommand.equals(F("rrf")) && (argCount == 7)) { // rrf: Rounded rectangle, filled
    # if ADAGFX_ARGUMENT_VALIDATION

    if (invalidCoordinates(nParams[0], nParams[1]) ||
        invalidCoordinates(nParams[0] + nParams[2], nParams[1] + nParams[3]) ||
        invalidCoordinates(nParams[4],              0)) { // Also check radius
      success = false;
    } else
    # endif  // if ADAGFX_ARGUMENT_VALIDATION
    {
      _display->fillRoundRect(nParams[0], nParams[1], nParams[2], nParams[3], nParams[4], AdaGFXparseColor(sParams[6], _colorDepth));
      _display->drawRoundRect(nParams[0], nParams[1], nParams[2], nParams[3], nParams[4], AdaGFXparseColor(sParams[5], _colorDepth));
    }
  }
  else if (subcommand.equals(F("px")) && (argCount == 3)) { // px: Pixel
    # if ADAGFX_ARGUMENT_VALIDATION

    if (invalidCoordinates(nParams[0], nParams[1])) {
      success = false;
    } else
    # endif // if ADAGFX_ARGUMENT_VALIDATION
    {
      _display->drawPixel(nParams[0], nParams[1], AdaGFXparseColor(sParams[2], _colorDepth));
    }
  }
  else if ((subcommand.equals(F("pxh")) || subcommand.equals(F("pxv"))) && (argCount > 2)) { // pxh/pxv: Pixels, hor./vert. incremented
    # if ADAGFX_ARGUMENT_VALIDATION                                                          // merged loop is smaller than 2 separate loops

    if (invalidCoordinates(nParams[0], nParams[1])) {
      success = false;
    } else
    # endif // if ADAGFX_ARGUMENT_VALIDATION
    {
      _display->startWrite();
      _display->writePixel(nParams[0], nParams[1], AdaGFXparseColor(sParams[2], _colorDepth));
      loop = true;
      uint8_t h     = 0;
      uint8_t v     = 0;
      bool    isPxh = subcommand.equals(F("pxh"));

      if (isPxh) {
        h++;
      } else {
        v++;
      }

      while (loop) {
        String color = parseString(string, h + v + 5); // 5 = 2 + 3 already parsed merged loop is smaller than 2 separate loops

        if (color.isEmpty()
            # if ADAGFX_ARGUMENT_VALIDATION
            || invalidCoordinates(nParams[0] + h, nParams[1] + v)
            # endif // if ADAGFX_ARGUMENT_VALIDATION
            ) {
          loop = false;
        } else {
          _display->writePixel(nParams[0] + h, nParams[1] + v, AdaGFXparseColor(color, _colorDepth));

          if (isPxh) {
            h++;
          } else {
            v++;
          }
        }
        delay(0);
      }
      _display->endWrite();
    }
  }
  # if ADAGFX_ENABLE_BMP_DISPLAY
  else if (subcommand.equals(F("bmp")) && (argCount == 3)) { // bmp,x,y,filename.bmp : show bmp from file
    if (!sParams[2].isEmpty()) {
      success = showBmp(sParams[2], nParams[0], nParams[1]);
    } else {
      success = false;
    }
  }
  # endif // if ADAGFX_ENABLE_BMP_DISPLAY
  else {
    success = false;
  }

  return success;
}

/****************************************************************************
 * printText: Print text on display at a specific pixel or column/row location
 ***************************************************************************/
void AdafruitGFX_helper::printText(const char    *string,
                                   int            X,
                                   int            Y,
                                   unsigned int   textSize,
                                   unsigned short color,
                                   unsigned short bkcolor) {
  int16_t  _x = X;
  int16_t  _y = Y + (_heightOffset * textSize);
  uint16_t _w = 0;
  uint8_t  _h = 0;
  int16_t  x1, y1;
  uint16_t w0, w1, h1;

  if (_columnRowMode) {
    _x = X * (_fontwidth * textSize); // We need this multiple times
    _y = (Y * (_fontheight * textSize))  + (_heightOffset * textSize);
  }

  if (_textBackFill && (color != bkcolor)) { // Draw extra lines above text
    // Estimate used width
    _w = _textPrintMode == AdaGFXTextPrintMode::ContinueToNextLine ?
         strlen(string) * _fontwidth * textSize :
         ((_textcols + 1) * _fontwidth * textSize) - _x;

    do {
      _display->drawLine(_x, _y, _x + _w - 1, _y, bkcolor);
      _h++;
      _y++; // Shift down entire line
    } while (_h < textSize);
  }

  _display->setCursor(_x, _y);
  _display->setTextColor(color, bkcolor);
  _display->setTextSize(textSize);

  String newString = string;

  if (_textPrintMode != AdaGFXTextPrintMode::ContinueToNextLine) {
    if (_isProportional) {                                              // Proportional font. This is rather slow!
      _display->getTextBounds(newString, _x, _y, &x1, &y1, &w1, &h1);   // Count length

      while ((newString.length() > 0) && ((_x + w1) > _res_x)) {
        newString.remove(newString.length() - 1);                       // Cut last character off
        _display->getTextBounds(newString, _x, _y, &x1, &y1, &w1, &h1); // Re-count length
      }
    }
    else {                                                              // Fixed width font
      if (newString.length() > static_cast<uint16_t>(_textcols - (_x / (_fontwidth * textSize)))) {
        newString = newString.substring(0, _textcols - (_x / (_fontwidth * textSize)));
      }
    }
  }

  w1 = 0;

  _display->getTextBounds(F(" "), _x, _y, &x1, &y1, &w1, &h1);

  if (w1 == 0) { w1 = _fontwidth; } // Some fonts seem to have a 0-wide space, this is an endless loop protection

  if (_textPrintMode == AdaGFXTextPrintMode::ClearThenTruncate) { // Clear before print
    _display->setCursor(_x, _y);
    w0 = 0;
    uint16_t w2 = 0;

    _display->getTextBounds(newString, _x, _y, &x1, &y1, &w2, &h1); // Count length in pixels

    for (; (w0 < w2); w0 += w1) {                                   // Clear previously used text with spaces
      _display->print(' ');
    }
    delay(0);
  }

  _display->setCursor(_x, _y);
  _display->print(newString);

  _display->getTextBounds(newString, _x, _y, &x1, &y1, &w0, &h1); // Count length in pixels

  for (; ((_x + w0) < _res_x) && _textPrintMode != AdaGFXTextPrintMode::ContinueToNextLine; w0 += w1) {
    _display->print(' ');
  }

  if (_textBackFill && (color != bkcolor)) { // Draw extra lines below text
    _y += ((_fontheight - 1) * textSize);
    _h  = 0;

    do {
      _display->drawLine(_x, _y - _h, _x + _w - 1, _y - _h, bkcolor);
      _h++;
    } while (_h <= textSize);
  }
}

/****************************************************************************
 * getTextSize length and height in pixels
 ***************************************************************************/
uint16_t AdafruitGFX_helper::getTextSize(const String& text,
                                         uint16_t    & h) {
  int16_t  x;
  int16_t  y;
  uint16_t w;

  _display->getTextBounds(text.c_str(), 0, 0, &x, &y, &w, &h); // Count length and height in pixels
  return w;
}

/****************************************************************************
 * color565: convert r, g, b colors to rgb565 (by bit-shifting)
 ***************************************************************************/
uint16_t color565(uint8_t red, uint8_t green, uint8_t blue) {
  return ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3);
}

/****************************************************************************
 * AdaGFXparseColor: translate color name, rgb565 hex #rGgb or rgb hex #RRGGBB to an RGB565 value,
 * also applies color reduction to mono(2), duo(3), quadro(4), septo(7), octo(8), quinto(16)-chrome colors
 ***************************************************************************/

// Parse color string to RGB565 color
// param [in] s : The color string (white, red, ...)
// Param [in] colorDepth: The requiresed color depth, default: FullColor
// param [in] defaultWhite: Return White color if empty, default: true
// return : color (default ADAGFX_WHITE)
uint16_t AdaGFXparseColor(String& s, AdaGFXColorDepth colorDepth, bool emptyIsBlack) {
  s.toLowerCase();
  int32_t result = -1; // No result yet

  if ((colorDepth == AdaGFXColorDepth::Monochrome) ||
      (colorDepth == AdaGFXColorDepth::BlackWhiteRed) ||
      (colorDepth == AdaGFXColorDepth::BlackWhite2Greyscales)) { // Only a limited set of colors is supported
    if (s.equals(F("black")))   { return static_cast<uint16_t>(AdaGFXMonoRedGreyscaleColors::ADAGFXEPD_BLACK); }

    if (s.equals(F("white")))   { return static_cast<uint16_t>(AdaGFXMonoRedGreyscaleColors::ADAGFXEPD_WHITE); }

    if (s.equals(F("inverse"))) { return static_cast<uint16_t>(AdaGFXMonoRedGreyscaleColors::ADAGFXEPD_INVERSE); }

    if (s.equals(F("red")))     { return static_cast<uint16_t>(AdaGFXMonoRedGreyscaleColors::ADAGFXEPD_RED); }

    // Synonym for red
    if (s.equals(F("yellow")))  { return static_cast<uint16_t>(AdaGFXMonoRedGreyscaleColors::ADAGFXEPD_RED); }

    if (s.equals(F("dark")))    { return static_cast<uint16_t>(AdaGFXMonoRedGreyscaleColors::ADAGFXEPD_DARK); }

    if (s.equals(F("light")))   { return static_cast<uint16_t>(AdaGFXMonoRedGreyscaleColors::ADAGFXEPD_LIGHT); }

    // If we get this far, return the default
    return static_cast<uint16_t>(AdaGFXMonoRedGreyscaleColors::ADAGFXEPD_WHITE);
  # if ADAGFX_SUPPORT_7COLOR
  } else if (colorDepth == AdaGFXColorDepth::SevenColor) {
    if (s.equals(F("black")))  { result = static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_BLACK); }

    if (s.equals(F("white")))  { result = static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_WHITE); }

    if (s.equals(F("green")))  { result = static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_GREEN); }

    if (s.equals(F("blue")))   { result = static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_BLUE); }

    if (s.equals(F("red")))    { result = static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_RED); }

    if (s.equals(F("yellow"))) { result = static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_YELLOW); }

    if (s.equals(F("orange"))) { result = static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_ORANGE); }
  # endif // if ADAGFX_SUPPORT_7COLOR
  } else { // Some predefined colors
    if (s.equals(F("black")))       { result = ADAGFX_BLACK; }

    if (s.equals(F("navy")))        { result = ADAGFX_NAVY; }

    if (s.equals(F("darkgreen")))   { result = ADAGFX_DARKGREEN; }

    if (s.equals(F("darkcyan")))    { result = ADAGFX_DARKCYAN; }

    if (s.equals(F("maroon")))      { result = ADAGFX_MAROON; }

    if (s.equals(F("purple")))      { result = ADAGFX_PURPLE; }

    if (s.equals(F("olive")))       { result = ADAGFX_OLIVE; }

    if (s.equals(F("lightgrey")))   { result = ADAGFX_LIGHTGREY; }

    if (s.equals(F("darkgrey")))    { result = ADAGFX_DARKGREY; }

    if (s.equals(F("blue")))        { result = ADAGFX_BLUE; }

    if (s.equals(F("green")))       { result = ADAGFX_GREEN; }

    if (s.equals(F("cyan")))        { result = ADAGFX_CYAN; }

    if (s.equals(F("red")))         { result = ADAGFX_RED; }

    if (s.equals(F("magenta")))     { result = ADAGFX_MAGENTA; }

    if (s.equals(F("yellow")))      { result = ADAGFX_YELLOW; }

    if (s.equals(F("white")))       { result = ADAGFX_WHITE; }

    if (s.equals(F("orange")))      { result = ADAGFX_ORANGE; }

    if (s.equals(F("greenyellow"))) { result = ADAGFX_GREENYELLOW; }

    if (s.equals(F("pink")))        { result = ADAGFX_PINK; }
  }

  // Parse default hex #rgb565 (hex) string (1-4 hex nibbles accepted!)
  if ((result == -1) && (s.length() >= 2) && (s.length() <= 5) && (s[0] == '#')) {
    result = hexToUL(&s[1]);
  }

  // Parse default hex #RRGGBB string (must be 6 hex nibbles!)
  if ((result == -1) && (s.length() == 7) && (s[0] == '#')) {
    // convrt to long value in base16, then split up into r, g, b values
    uint32_t number = hexToUL(&s[1]);

    // uint32_t r = number >> 16 & 0xFF;
    // uint32_t g = number >> 8 & 0xFF;
    // uint32_t b = number & 0xFF;
    // convert to color565 (as used by adafruit lib)
    result = color565(number >> 16 & 0xFF, number >> 8 & 0xFF, number & 0xFF);
  }

  if ((result == -1) || (result == ADAGFX_WHITE)) { // Default & don't convert white
    if (
      # if ADAGFX_SUPPORT_7COLOR
      (colorDepth >= AdaGFXColorDepth::SevenColor) &&
      # endif // if ADAGFX_SUPPORT_7COLOR
      (colorDepth <= AdaGFXColorDepth::SixteenColor)) {
      result = static_cast<uint16_t>(AdaGFXMonoRedGreyscaleColors::ADAGFXEPD_BLACK); // Monochrome fallback, compatible 7-color
    } else {
      if (emptyIsBlack) {
        result = ADAGFX_BLACK;
      } else {
        result = ADAGFX_WHITE; // Color fallback value
      }
    }
  } else {
    // Reduce colors?
    switch (colorDepth) {
      case AdaGFXColorDepth::Monochrome:
      case AdaGFXColorDepth::BlackWhiteRed:
      case AdaGFXColorDepth::BlackWhite2Greyscales:
        // Unsupported at this point, but compiler needs the cases because of the enum class
        break;
      # if ADAGFX_SUPPORT_7COLOR
      case AdaGFXColorDepth::SevenColor:
        result = AdaGFXrgb565ToColor7(result); // Convert
        break;
      # endif  // if ADAGFX_SUPPORT_7COLOR
      case AdaGFXColorDepth::EightColor:
        result = color565((result >> 11 & 0x1F) / 4, (result >> 5 & 0x3F) / 4, (result & 0x1F) / 4); // reduce colors factor 4
        break;
      case AdaGFXColorDepth::SixteenColor:
        result = color565((result >> 11 & 0x1F) / 2, (result >> 5 & 0x3F) / 2, (result & 0x1F) / 2); // reduce colors factor 2
        break;
      case AdaGFXColorDepth::FullColor:
        // No color reduction
        break;
    }
  }
  return result;
}

const __FlashStringHelper* AdaGFXcolorToString_internal(uint16_t         color,
                                                        AdaGFXColorDepth colorDepth,
                                                        bool             blackIsEmpty);

// Add a single optionvalue of a color to a datalist (internal/private)
void AdaGFXaddHtmlDataListColorOptionValue(uint16_t         color,
                                           AdaGFXColorDepth colorDepth) {
  const __FlashStringHelper *clr = AdaGFXcolorToString_internal(color, colorDepth, false);

  if (clr != F("*")) {
    addHtml(F("<option value=\""));
    addHtml(clr);
    addHtml(F("\">"));
    addHtml(clr);
    addHtml(F("</option>"));
  }
}

/*****************************************************************************************
 * Generate a html 'datalist' of the colors available for selected colorDepth, with id provided
 ****************************************************************************************/
void AdaGFXHtmlColorDepthDataList(const __FlashStringHelper *id,
                                  AdaGFXColorDepth           colorDepth) {
  addHtml(F("<datalist id=\""));
  addHtml(id);
  addHtml(F("\">"));

  switch (colorDepth) {
    case AdaGFXColorDepth::BlackWhiteRed:
    case AdaGFXColorDepth::BlackWhite2Greyscales:
      AdaGFXaddHtmlDataListColorOptionValue(static_cast<uint16_t>(AdaGFXMonoRedGreyscaleColors::ADAGFXEPD_RED),     colorDepth);
      AdaGFXaddHtmlDataListColorOptionValue(static_cast<uint16_t>(AdaGFXMonoRedGreyscaleColors::ADAGFXEPD_DARK),    colorDepth);
      AdaGFXaddHtmlDataListColorOptionValue(static_cast<uint16_t>(AdaGFXMonoRedGreyscaleColors::ADAGFXEPD_LIGHT),   colorDepth);
    case AdaGFXColorDepth::Monochrome:
      AdaGFXaddHtmlDataListColorOptionValue(static_cast<uint16_t>(AdaGFXMonoRedGreyscaleColors::ADAGFXEPD_BLACK),   colorDepth);
      AdaGFXaddHtmlDataListColorOptionValue(static_cast<uint16_t>(AdaGFXMonoRedGreyscaleColors::ADAGFXEPD_WHITE),   colorDepth);
      AdaGFXaddHtmlDataListColorOptionValue(static_cast<uint16_t>(AdaGFXMonoRedGreyscaleColors::ADAGFXEPD_INVERSE), colorDepth);
      break;
    # if ADAGFX_SUPPORT_7COLOR
    case AdaGFXColorDepth::SevenColor:
    {
      AdaGFXaddHtmlDataListColorOptionValue(static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_BLACK),  colorDepth);
      AdaGFXaddHtmlDataListColorOptionValue(static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_WHITE),  colorDepth);
      AdaGFXaddHtmlDataListColorOptionValue(static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_GREEN),  colorDepth);
      AdaGFXaddHtmlDataListColorOptionValue(static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_BLUE),   colorDepth);
      AdaGFXaddHtmlDataListColorOptionValue(static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_RED),    colorDepth);
      AdaGFXaddHtmlDataListColorOptionValue(static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_YELLOW), colorDepth);
      AdaGFXaddHtmlDataListColorOptionValue(static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_ORANGE), colorDepth);
      break;
    }
    # endif // if ADAGFX_SUPPORT_7COLOR
    case AdaGFXColorDepth::EightColor: // TODO: Sort out the actual 8/16 color options
    case AdaGFXColorDepth::SixteenColor:
    case AdaGFXColorDepth::FullColor:
    {
      AdaGFXaddHtmlDataListColorOptionValue(ADAGFX_BLACK,       colorDepth);
      AdaGFXaddHtmlDataListColorOptionValue(ADAGFX_NAVY,        colorDepth);
      AdaGFXaddHtmlDataListColorOptionValue(ADAGFX_DARKGREEN,   colorDepth);
      AdaGFXaddHtmlDataListColorOptionValue(ADAGFX_DARKCYAN,    colorDepth);
      AdaGFXaddHtmlDataListColorOptionValue(ADAGFX_MAROON,      colorDepth);
      AdaGFXaddHtmlDataListColorOptionValue(ADAGFX_PURPLE,      colorDepth);
      AdaGFXaddHtmlDataListColorOptionValue(ADAGFX_OLIVE,       colorDepth);
      AdaGFXaddHtmlDataListColorOptionValue(ADAGFX_LIGHTGREY,   colorDepth);
      AdaGFXaddHtmlDataListColorOptionValue(ADAGFX_DARKGREY,    colorDepth);
      AdaGFXaddHtmlDataListColorOptionValue(ADAGFX_BLUE,        colorDepth);
      AdaGFXaddHtmlDataListColorOptionValue(ADAGFX_GREEN,       colorDepth);
      AdaGFXaddHtmlDataListColorOptionValue(ADAGFX_CYAN,        colorDepth);
      AdaGFXaddHtmlDataListColorOptionValue(ADAGFX_RED,         colorDepth);
      AdaGFXaddHtmlDataListColorOptionValue(ADAGFX_MAGENTA,     colorDepth);
      AdaGFXaddHtmlDataListColorOptionValue(ADAGFX_YELLOW,      colorDepth);
      AdaGFXaddHtmlDataListColorOptionValue(ADAGFX_WHITE,       colorDepth);
      AdaGFXaddHtmlDataListColorOptionValue(ADAGFX_ORANGE,      colorDepth);
      AdaGFXaddHtmlDataListColorOptionValue(ADAGFX_GREENYELLOW, colorDepth);
      AdaGFXaddHtmlDataListColorOptionValue(ADAGFX_PINK,        colorDepth);
      break;
    }
  }
  addHtml(F("</datalist>"));
}

/*****************************************************************************************
 * Convert an RGB565 color (number) to it's name or the #rgb565 hex string, based on depth
 ****************************************************************************************/
String AdaGFXcolorToString(uint16_t         color,
                           AdaGFXColorDepth colorDepth,
                           bool             blackIsEmpty) {
  String result = AdaGFXcolorToString_internal(color, colorDepth, blackIsEmpty);

  if (result.equals(F("*"))) {
    result  = '#';
    result += String(color, HEX);
    result.toUpperCase();
  }
  return result;
}

const __FlashStringHelper* AdaGFXcolorToString_internal(uint16_t         color,
                                                        AdaGFXColorDepth colorDepth,
                                                        bool             blackIsEmpty) {
  switch (colorDepth) {
    case AdaGFXColorDepth::Monochrome:
    case AdaGFXColorDepth::BlackWhiteRed:
    case AdaGFXColorDepth::BlackWhite2Greyscales:
    {
      switch (color) {
        case static_cast<uint16_t>(AdaGFXMonoRedGreyscaleColors::ADAGFXEPD_BLACK): return blackIsEmpty ? F("") : F("black");
        case ADAGFX_WHITE: // Fall through
        case static_cast<uint16_t>(AdaGFXMonoRedGreyscaleColors::ADAGFXEPD_WHITE): return F("white");
        case static_cast<uint16_t>(AdaGFXMonoRedGreyscaleColors::ADAGFXEPD_INVERSE): return F("inverse");
        case static_cast<uint16_t>(AdaGFXMonoRedGreyscaleColors::ADAGFXEPD_RED): return F("red");
        case static_cast<uint16_t>(AdaGFXMonoRedGreyscaleColors::ADAGFXEPD_DARK): return F("dark");
        case static_cast<uint16_t>(AdaGFXMonoRedGreyscaleColors::ADAGFXEPD_LIGHT): return F("light");
        default:
          break;
      }
      break;
    }
    # if ADAGFX_SUPPORT_7COLOR
    case AdaGFXColorDepth::SevenColor:
    {
      switch (color) {
        case static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_BLACK): return blackIsEmpty ? F("") : F("black");
        case ADAGFX_WHITE: // Fall through
        case static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_WHITE): return F("white");
        case static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_GREEN): return F("green");
        case static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_BLUE): return F("blue");
        case static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_RED): return F("red");
        case static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_YELLOW): return F("yellow");
        case static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_ORANGE): return F("orange");
        default:
          break;
      }
      break;
    }
    # endif // if ADAGFX_SUPPORT_7COLOR
    case AdaGFXColorDepth::EightColor:
    case AdaGFXColorDepth::SixteenColor:
    case AdaGFXColorDepth::FullColor:
    {
      switch (color) {
        case ADAGFX_BLACK:  return blackIsEmpty ? F("") : F("black");
        case ADAGFX_NAVY: return F("navy");
        case ADAGFX_DARKGREEN: return F("darkgreen");
        case ADAGFX_DARKCYAN: return F("darkcyan");
        case ADAGFX_MAROON: return F("maroon");
        case ADAGFX_PURPLE: return F("purple");
        case ADAGFX_OLIVE: return F("olive");
        case ADAGFX_LIGHTGREY: return F("lightgrey");
        case ADAGFX_DARKGREY: return F("darkgrey");
        case ADAGFX_BLUE: return F("blue");
        case ADAGFX_GREEN: return F("green");
        case ADAGFX_CYAN: return F("cyan");
        case ADAGFX_RED: return F("red");
        case ADAGFX_MAGENTA: return F("magenta");
        case ADAGFX_YELLOW: return F("yellow");
        case ADAGFX_WHITE: return F("white");
        case ADAGFX_ORANGE: return F("orange");
        case ADAGFX_GREENYELLOW: return F("greenyellow");
        case ADAGFX_PINK: return F("pink");
        default:
          break;
      }
      break;
    }
  }
  return F("*");
}

# if ADAGFX_SUPPORT_7COLOR

/****************************************************************************
 * AdaGFXrgb565ToColor7: Convert a rgb565 color to the 7 colors supported by 7-color eInk displays
 * Borrowed from https://github.com/ZinggJM/GxEPD2 color7() routine
 ***************************************************************************/
uint16_t AdaGFXrgb565ToColor7(uint16_t color) {
  uint16_t cv7 = static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_WHITE); // Default = white

  uint16_t red   = (color & 0xF800);
  uint16_t green = (color & 0x07E0) << 5;
  uint16_t blue  = (color & 0x001F) << 11;

  if ((red < 0x8000) && (green < 0x8000) && (blue < 0x8000)) {
    cv7 = static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_BLACK); // black
  }
  else if ((red >= 0x8000) && (green >= 0x8000) && (blue >= 0x8000)) {
    cv7 = static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_WHITE); // white
  }
  else if ((red >= 0x8000) && (blue >= 0x8000)) {
    if (red > blue) {
      cv7 = static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_RED);
    } else {
      cv7 = static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_BLUE); // red, blue
    }
  }
  else if ((green >= 0x8000) && (blue >= 0x8000)) {
    if (green > blue) {
      cv7 = static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_GREEN);
    } else {
      cv7 = static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_BLUE); // green, blue
    }
  }
  else if ((red >= 0x8000) && (green >= 0x8000)) {
    static const uint16_t y2o_lim = ((ADAGFX_YELLOW - ADAGFX_ORANGE) / 2 + (ADAGFX_ORANGE & 0x07E0)) << 5;

    if (green > y2o_lim) {
      cv7 = static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_YELLOW);
    } else {
      cv7 = static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_ORANGE); // yellow, orange
    }
  }
  else if (red >= 0x8000) {
    cv7 = static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_RED);   // red
  }
  else if (green >= 0x8000) {
    cv7 = static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_GREEN); // green
  }
  else {
    cv7 = static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_BLUE);  // blue
  }
  return cv7;
}

# endif // if ADAGFX_SUPPORT_7COLOR

/****************************************************************************
 * getTextMetrics: Returns the metrics related to current font
 ***************************************************************************/
void AdafruitGFX_helper::getTextMetrics(uint16_t& textcols,
                                        uint16_t& textrows,
                                        uint8_t & fontwidth,
                                        uint8_t & fontheight,
                                        uint8_t & fontscaling,
                                        uint8_t & heightOffset,
                                        uint16_t& xpix,
                                        uint16_t& ypix) {
  textcols     = _textcols;
  textrows     = _textrows;
  fontwidth    = _fontwidth;
  fontheight   = _fontheight;
  fontscaling  = _fontscaling;
  heightOffset = _heightOffset;
  xpix         = _res_x;
  ypix         = _res_y;
}

/****************************************************************************
 * getColors: Returns the current text colors
 ***************************************************************************/
void AdafruitGFX_helper::getColors(uint16_t& fgcolor,
                                   uint16_t& bgcolor) {
  fgcolor = _fgcolor;
  bgcolor = _bgcolor;
}

/****************************************************************************
 * calculateTextMetrics: Recalculate the text mertics based on supplied font parameters
 ***************************************************************************/
void AdafruitGFX_helper::calculateTextMetrics(uint8_t fontwidth,
                                              uint8_t fontheight,
                                              int8_t  heightOffset,
                                              bool    isProportional) {
  _fontwidth      = fontwidth;
  _fontheight     = fontheight;
  _heightOffset   = heightOffset;
  _isProportional = isProportional;
  _textcols       = _res_x / (_fontwidth * _fontscaling);
  _textrows       = _res_y / ((_fontheight + _heightOffset) * _fontscaling);

  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(ADAGFX_LOG_LEVEL)) {
    String log;
    log.reserve(60);
    log += F("AdaGFX:");

    if (!_trigger.isEmpty()) {
      log += F(" tr: ");
      log += _trigger;
    }
    log += F(" x: ");
    log += _res_x;
    log += F(", y: ");
    log += _res_y;
    log += F(", text columns: ");
    log += _textcols;
    log += F(" rows: ");
    log += _textrows;
    addLogMove(ADAGFX_LOG_LEVEL, log);
  }
  # endif // ifndef BUILD_NO_DEBUG
}

# if ADAGFX_ARGUMENT_VALIDATION

/****************************************************************************
 * invalidCoordinates: Check if X/Y coordinates stay within the limits of the display,
 * default pixel-mode, colRowMode true = character mode.
 * If Y == 0 then X is allowed the max. value of the display size.
 * *** Returns TRUE when invalid !! ***
 ***************************************************************************/
bool AdafruitGFX_helper::invalidCoordinates(int  X,
                                            int  Y,
                                            bool colRowMode) {
  #  ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(ADAGFX_LOG_LEVEL)) {
    String log;

    log.reserve(49);
    log += F("invalidCoordinates: X:");
    log += X;
    log += '/';
    log += (colRowMode ? _textcols : _res_x);
    log += F(" Y:");
    log += Y;
    log += '/';
    log += (colRowMode ? _textrows : _res_y);
    addLogMove(ADAGFX_LOG_LEVEL, log);
  }
  #  endif // ifndef BUILD_NO_DEBUG

  if (!_useValidation) { return false; }

  if (colRowMode) {
    return !((X >= 0) && (X <= _textcols) &&
             (Y >= 0) && (Y <= _textrows));
  } else {
    if (Y == 0) { // Y == 0: Accept largest x/y size value for x
      return !((X >= 0) && (X <= std::max(_res_x, _res_y)));
    } else {
      return !((X >= 0) && (X <= _res_x) &&
               (Y >= 0) && (Y <= _res_y));
    }
  }
}

# endif // if ADAGFX_ARGUMENT_VALIDATION

void AdafruitGFX_helper::setValidation(const bool& state) {
  _useValidation = state;
}

void AdafruitGFX_helper::setRotation(uint8_t m) {
  uint8_t rotation = m & 3;

  _display->setRotation(m); // Set rotation 0/1/2/3

  switch (rotation) {
    case 0:
    case 2:
      _res_x = _display_x;
      _res_y = _display_y;
      break;
    case 1:
    case 3:
      _res_x = _display_y;
      _res_y = _display_x;
      break;
  }
  calculateTextMetrics(_fontwidth, _fontheight, _heightOffset, _isProportional);
}

# if ADAGFX_ENABLE_BMP_DISPLAY

/**
 * CPA (Copy/paste/adapt) from Adafruit_ImageReader::coreBMP()
 * Changes:
 * - No 'load to memory' feature
 * - No special handling of SD Filesystem/FAT, but File only
 * - Adds support for non-SPI displays (like NeoPixel Matrix, and possibly I2C displays, once supported)
 */
bool AdafruitGFX_helper::showBmp(const String& filename,
                                 int16_t       x,
                                 int16_t       y) {
  bool transact = true;           // Enable transaction support to work proper with SD czrd, when enabled
  bool status   = false;          // IMAGE_SUCCESS on valid file
  uint16_t  tftbuf[BUFPIXELS];
  uint16_t *dest = tftbuf;        // TFT working buffer, or NULL if to canvas
  uint32_t  offset;               // Start of image data in file
  uint32_t  headerSize;           // Indicates BMP version
  uint8_t   planes;               // BMP planes
  uint8_t   depth;                // BMP bit depth
  uint32_t  compression = 0;      // BMP compression mode
  uint32_t  colors      = 0;      // Number of colors in palette
  uint16_t *quantized   = NULL;   // 16-bit 5/6/5 color palette
  uint32_t  rowSize;              // >bmpWidth if scanline padding
  uint8_t   sdbuf[3 * BUFPIXELS]; // BMP read buf (R+G+B/pixel)
  int bmpWidth, bmpHeight;        // BMP width & height in pixels

  #  if ((3 * BUFPIXELS) <= 255)
  uint8_t srcidx = sizeof sdbuf;  // Current position in sdbuf
  #  else // if ((3 * BUFPIXELS) <= 255)
  uint16_t srcidx = sizeof sdbuf;
  #  endif // if ((3 * BUFPIXELS) <= 255)
  uint32_t destidx = 0;
  bool     flip = true;      // BMP is stored bottom-to-top
  uint32_t bmpPos = 0;       // Next pixel position in file
  int loadWidth, loadHeight, // Region being loaded (clipped)
      loadX, loadY;          // "
  int row, col;              // Current pixel pos.
  uint8_t r, g, b;           // Current pixel color
  uint8_t bitIn = 0;         // Bit number for 1-bit data in
  int16_t drow  = 0;
  int16_t dcol  = 0;

  bool canTransact = (nullptr != _tft);

  // If BMP is being drawn off the right or bottom edge of the screen,
  // nothing to do here. NOT an error, just a trivial clip operation.
  if (_tft && ((x >= _tft->width()) || (y >= _tft->height()))) {
    addLog(LOG_LEVEL_INFO, F("showBmp: coordinates off display"));
    return false;
  }

  // Open requested file on storage
  // Search flash file system first, then SD if present
  file = tryOpenFile(filename, "r");
  #  ifdef FEATURE_SD

  if (!file) {
    file = SD.open(filename.c_str(), "r");
  }
  #  endif // ifdef FEATURE_SD

  if (!file) {
    addLog(LOG_LEVEL_ERROR, F("showBmp: file not found"));
    return false;
  }

  // Parse BMP header. 0x4D42 (ASCII 'BM') is the Windows BMP signature.
  // There are other values possible in a .BMP file but these are super
  // esoteric (e.g. OS/2 struct bitmap array) and NOT supported here!
  if (readLE16() == 0x4D42) { // BMP signature
    (void)readLE32();         // Read & ignore file size
    (void)readLE32();         // Read & ignore creator bytes
    offset = readLE32();      // Start of image data
    // Read DIB header
    headerSize = readLE32();
    bmpWidth   = readLE32();
    bmpHeight  = readLE32();

    // If bmpHeight is negative, image is in top-down order.
    // This is not canon but has been observed in the wild.
    if (bmpHeight < 0) {
      bmpHeight = -bmpHeight;
      flip      = false;
    }
    planes = readLE16();
    depth  = readLE16(); // Bits per pixel

    // Compression mode is present in later BMP versions (default = none)
    if (headerSize > 12) {
      compression = readLE32();
      (void)readLE32();    // Raw bitmap data size; ignore
      (void)readLE32();    // Horizontal resolution, ignore
      (void)readLE32();    // Vertical resolution, ignore
      colors = readLE32(); // Number of colors in palette, or 0 for 2^depth
      (void)readLE32();    // Number of colors used (ignore)
      // File position should now be at start of palette (if present)
    }

    if (!colors) {
      colors = 1 << depth;
    }
    #  ifndef BUILD_NO_DEBUG
    String log;

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      log.reserve(80);
      log += F("showBmp: bitmap w:");
      log += bmpWidth;
      log += F(", h:");
      log += bmpHeight;
      log += F(", dpt:");
      log += depth;
      log += F(", colors:");
      log += colors;
      log += F(", cmp:");
      log += compression;
      log += F(", pl:");
      log += planes;
      log += F(", x:");
      log += x;
      log += F(", y:");
      log += y;
      addLog(LOG_LEVEL_INFO, log);
    }
    #  endif // ifndef BUILD_NO_DEBUG

    loadWidth  = bmpWidth;
    loadHeight = bmpHeight;
    loadX      = 0;
    loadY      = 0;

    if (_display) {
      // Crop area to be loaded (if destination is TFT)
      if (x < 0) {
        loadX      = -x;
        loadWidth += x;
        x          = 0;
      }

      if (y < 0) {
        loadY       = -y;
        loadHeight += y;
        y           = 0;
      }

      if ((x + loadWidth) > _display->width()) {
        loadWidth = _display->width() - x;
      }

      if ((y + loadHeight) > _display->height()) {
        loadHeight = _display->height() - y;
      }
    }
    #  ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      log.clear();
      log += F("showBmp: x: ");
      log += x;
      log += F(", y:");
      log += y;
      log += F(", dw:");
      log += _display->width();
      log += F(", dh:");
      log += _display->height();
      addLogMove(LOG_LEVEL_INFO, log);
    }
    #  endif // ifndef BUILD_NO_DEBUG

    if ((planes == 1) && (compression == 0)) { // Only uncompressed is handled
      // BMP rows are padded (if needed) to 4-byte boundary
      rowSize = ((depth * bmpWidth + 31) / 32) * 4;

      if ((depth == 24) || (depth == 1)) {           // BGR or 1-bit bitmap format
        if (dest) {                                  // Supported format, alloc OK, etc.
          status = true;

          if ((loadWidth > 0) && (loadHeight > 0)) { // Clip top/left
            _display->startWrite();                  // Start SPI (regardless of transact)

            if (canTransact) {
              _tft->setAddrWindow(x, y, loadWidth, loadHeight);
            }

            if ((depth >= 16) ||
                (quantized = (uint16_t *)malloc(colors * sizeof(uint16_t)))) {
              if (depth < 16) {
                // Load and quantize color table
                for (uint16_t c = 0; c < colors; c++) {
                  b = file.read();
                  g = file.read();
                  r = file.read();
                  (void)file.read(); // Ignore 4th byte
                  quantized[c] =
                    ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
                }
              }

              for (row = 0; row < loadHeight; row++) { // For each scanline...
                delay(0);                              // Keep ESP8266 happy

                // Seek to start of scan line.  It might seem labor-intensive
                // to be doing this on every line, but this method covers a
                // lot of gritty details like cropping, flip and scanline
                // padding. Also, the seek only takes place if the file
                // position actually needs to change (avoids a lot of cluster
                // math in SD library).
                if (flip) { // Bitmap is stored bottom-to-top order (normal BMP)
                  bmpPos = offset + (bmpHeight - 1 - (row + loadY)) * rowSize;
                } else {    // Bitmap is stored top-to-bottom
                  bmpPos = offset + (row + loadY) * rowSize;
                }

                if (depth == 24) {
                  bmpPos += loadX * 3;
                } else {
                  bmpPos += loadX / 8;
                  bitIn   = 7 - (loadX & 7);
                }

                if (file.position() != bmpPos) {        // Need seek?
                  if (transact && canTransact) {
                    _tft->dmaWait();
                    _tft->endWrite();                   // End TFT SPI transaction
                  }
                  file.seek(bmpPos);                    // Seek = SD transaction
                  srcidx = sizeof sdbuf;                // Force buffer reload
                }

                for (col = 0; col < loadWidth; col++) { // For each pixel...
                  if (srcidx >= sizeof sdbuf) {         // Time to load more?
                    if (transact && canTransact) {
                      _tft->dmaWait();
                      _tft->endWrite();                 // End TFT SPI transact
                    }
                    file.read(sdbuf, sizeof sdbuf);     // Load from SD

                    if (transact && canTransact) {
                      _display->startWrite();           // Start TFT SPI transact
                    }

                    if (destidx) {                      // If buffered TFT data
                      // Non-blocking writes (DMA) have been temporarily
                      // disabled until this can be rewritten with two
                      // alternating 'dest' buffers (else the nonblocking
                      // data out is overwritten in the dest[] write below).
                      // tft->writePixels(dest, destidx, false); // Write it
                      delay(0);

                      if (canTransact) {
                        _tft->writePixels(dest, destidx, true); // Write it
                      } else {
                        // loop over buffer

                        for (uint16_t p = 0; p < destidx; p++) {
                          _display->drawPixel(x + p, y + drow, dest[p]);
                        }
                      }

                      if (col % 33 == 0) { delay(0); }
                      destidx = 0; // and reset dest index
                    }

                    srcidx = 0;    // Reset bmp buf index
                  }

                  if (depth == 24) {
                    // Convert each pixel from BMP to 565 format, save in dest
                    b               = sdbuf[srcidx++];
                    g               = sdbuf[srcidx++];
                    r               = sdbuf[srcidx++];
                    dest[destidx++] =
                      ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
                  } else {
                    // Extract 1-bit color index
                    uint8_t n = (sdbuf[srcidx] >> bitIn) & 1;

                    if (!bitIn) {
                      srcidx++;
                      bitIn = 7;
                    } else {
                      bitIn--;
                    }

                    // Look up in palette, store in tft dest buf
                    dest[destidx++] = quantized[n];
                  }
                  dcol++;
                }                                           // end pixel loop

                if (_tft) {                                 // Drawing to TFT?
                  delay(0);

                  if (destidx) {                            // Any remainders?
                    // See notes above re: DMA
                    _tft->writePixels(dest, destidx, true); // Write it
                    destidx = 0;                            // and reset dest index
                  }
                  _tft->dmaWait();
                  _tft->endWrite();                         // update display
                } else {
                  // loop over buffer
                  if (destidx) {
                    for (uint16_t p = 0; p < destidx; p++) {
                      _display->drawPixel(x + p, y + drow, dest[p]);

                      if (p % 100 == 0) { delay(0); }
                    }
                    destidx = 0; // and reset dest index
                  }
                }

                drow++;
                dcol = 0;
              } // end scanline loop

              if (quantized) {
                free(quantized);  // Palette no longer needed
              }
              delay(0);
            } // end depth>24 or quantized malloc OK
          }                       // end top/left clip
        }                         // end malloc check
      }       // end depth check
    } // end planes/compression check
  }   // end signature

  file.close();
  #  ifndef BUILD_NO_DEBUG
  addLog(LOG_LEVEL_INFO, F("showBmp: Done."));
  #  endif // ifndef BUILD_NO_DEBUG
  return status;

  // }
}

/*!
    @brief   Reads a little-endian 16-bit unsigned value from currently-
             open File, converting if necessary to the microcontroller's
             native endianism. (BMP files use little-endian values.)
    @return  Unsigned 16-bit value, native endianism.
 */
uint16_t AdafruitGFX_helper::readLE16(void) {
  // Big-endian or unknown. Byte-by-byte read will perform reversal if needed.
  return file.read() | ((uint16_t)file.read() << 8);
}

/*!
    @brief   Reads a little-endian 32-bit unsigned value from currently-
             open File, converting if necessary to the microcontroller's
             native endianism. (BMP files use little-endian values.)
    @return  Unsigned 32-bit value, native endianism.
 */
uint32_t AdafruitGFX_helper::readLE32(void) {
  // Big-endian or unknown. Byte-by-byte read will perform reversal if needed.
  return file.read() | ((uint32_t)file.read() << 8) |
         ((uint32_t)file.read() << 16) | ((uint32_t)file.read() << 24);
}

# endif // if ADAGFX_ENABLE_BMP_DISPLAY

#endif // ifdef PLUGIN_USES_ADAFRUITGFX
