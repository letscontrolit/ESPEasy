#include "../Helpers/AdafruitGFX_helper.h"
#include "../../_Plugin_Helper.h"

#ifdef PLUGIN_USES_ADAFRUITGFX

# include "../Helpers/StringConverter.h"
# include "../WebServer/Markup_Forms.h"

# if ADAGFX_FONTS_INCLUDED
#  include "src/Static/Fonts/Seven_Segment24pt7b.h"
#  include "src/Static/Fonts/Seven_Segment18pt7b.h"
#  include "src/Static/Fonts/FreeSans9pt7b.h"
#  ifdef ADAGFX_FONTS_EXTRA_8PT_INCLUDED
#   include "src/Static/Fonts/angelina8pt7b.h"
#   include "src/Static/Fonts/NovaMono8pt7b.h"
#   include "src/Static/Fonts/RepetitionScrolling8pt7b.h"
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
# endif  // if ADAGFX_FONTS_INCLUDED

/******************************************************************************************
 * get the display text for a 'text print mode' enum value
 *****************************************************************************************/
const __FlashStringHelper* getAdaGFXTextPrintMode(AdaGFXTextPrintMode mode) {
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
const __FlashStringHelper* getAdaGFXColorDepth(AdaGFXColorDepth colorDepth) {
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
    getAdaGFXTextPrintMode(AdaGFXTextPrintMode::ContinueToNextLine),
    getAdaGFXTextPrintMode(AdaGFXTextPrintMode::TruncateExceedingMessage),
    getAdaGFXTextPrintMode(AdaGFXTextPrintMode::ClearThenTruncate)
  };
  const int textModeOptions[textModeCount] = {
    static_cast<int>(AdaGFXTextPrintMode::ContinueToNextLine),
    static_cast<int>(AdaGFXTextPrintMode::TruncateExceedingMessage),
    static_cast<int>(AdaGFXTextPrintMode::ClearThenTruncate)
  };

  addFormSelector(F("Text print Mode"), id, textModeCount, textModes, textModeOptions, selectedIndex);
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
                              uint8_t                    selectedIndex) {
  addFormCheckBox(F("Text Coordinates in col/row"), id, selectedIndex);
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
  addFormPinSelect(formatGpioName_output_optional(F("Backlight ")), backlightPinId, backlightPin);

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
  addFormPinSelect(F("Display button"), buttonPinId, buttonPin);

  addFormCheckBox(F("Inversed Logic"), buttonInverseId, buttonInverse);

  addFormNumericBox(F("Display Timeout"), displayTimeoutId, displayTimeout);
  addUnit(F("0 = off"));
}

/*****************************************************************************************
 * Show a numeric input 1..10 for Font scaling setting
 ****************************************************************************************/
void AdaGFXFormFontScaling(const __FlashStringHelper *fontScalingId,
                           uint8_t                    fontScaling) {
  addFormNumericBox(F("Font scaling"), fontScalingId, fontScaling, 1, 10);
  addUnit(F("1x..10x"));
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
            log  = F("AdaGFX: inline cmd: ");
            log += command;
            addLog(ADAGFX_LOG_LEVEL, log);
          }
          #  endif // ifndef BUILD_NO_DEBUG

          if (gfxHelper->processCommand(command)) {   // Execute command and remove from result incl. pre/postfix
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

  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(ADAGFX_LOG_LEVEL)) {
    String log;
    log.reserve(result.length() + 24);
    log  = F("AdaGFX: parse result: '");
    log += result;
    log += '\'';
    addLog(ADAGFX_LOG_LEVEL, log);
  }
  # endif // ifndef BUILD_NO_DEBUG
  return result;
}

// AdafruitGFX_helper class methods

/****************************************************************************
 * parameterized constructor
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
  _trigger.toLowerCase(); // store trigger in lowercase
  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(ADAGFX_LOG_LEVEL)) {
    String log;
    log.reserve(65);
    log  = F("AdaGFX: Init, x: ");
    log += _res_x;
    log += F(", y: ");
    log += _res_y;
    log += F(", colors: ");
    log += static_cast<uint16_t>(colorDepth);
    log += F(", trigger: ");
    log += _trigger;
    addLog(ADAGFX_LOG_LEVEL, log);
  }
  # endif // ifndef BUILD_NO_DEBUG

  _display_x = res_x; // Store initial resolution
  _display_y = res_y;

  calculateTextMetrics(6, 10);                  // Defaults for built-in font

  if (nullptr != _display) {
    _display->setTextColor(_fgcolor, _bgcolor); // initialize text colors
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
    log  = F("AdaGFX: command: ");
    log += _trigger;
    log += F(" argCount: ");
    log += argCount;
    log += ':';
    log += string;
    addLog(ADAGFX_LOG_LEVEL, log);
  }
  # endif // ifndef BUILD_NO_DEBUG

  if (subcommand.equals(F("txt"))) // txt: Print text at last cursor position, ends at next line!
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
      calculateTextMetrics(_fontwidth, _fontheight);
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
  else if (subcommand.equals(F("clear")))
  {
    if (argCount >= 1) {
      _display->fillScreen(AdaGFXparseColor(sParams[0], _colorDepth));
    } else {
      _display->fillScreen(_bgcolor);
    }
  }
  else if (subcommand.equals(F("rot")) && (argCount == 1))
  {
    if ((nParams[0] < 0) || (nParams[0] > 3)) {
      success = false;
    } else {
      setRotation(nParams[0]);
    }
  }
  # if ADAGFX_USE_ASCIITABLE
  else if (subcommand.equals(F("asciitable")))
  {
    String  line;
    int16_t start        = 0x80 + (argCount >= 1 && nParams[0] >= -4 && nParams[0] < 4 ? nParams[0] * 0x20 : 0);
    uint8_t scale        = (argCount == 2 && nParams[1] > 0 && nParams[1] <= 10 ? nParams[1] : 2);
    uint8_t currentScale = _fontscaling;

    if (_fontscaling != scale) { // Set fontscaling
      _fontscaling = scale;
      _display->setTextSize(_fontscaling);
      calculateTextMetrics(_fontwidth, _fontheight);
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
      calculateTextMetrics(_fontwidth, _fontheight);
    }
  }
  # endif // if ADAGFX_USE_ASCIITABLE
  else if (subcommand.equals(F("font")) && (argCount == 1)) { // font: Change font
    # if ADAGFX_FONTS_INCLUDED
    sParams[0].toLowerCase();

    if (sParams[0].equals(F("sevenseg24"))) {
      _display->setFont(&Seven_Segment24pt7b);
      calculateTextMetrics(21, 48);
    } else if (sParams[0].equals(F("sevenseg18"))) {
      _display->setFont(&Seven_Segment18pt7b);
      calculateTextMetrics(16, 34);
    } else if (sParams[0].equals(F("freesans"))) {
      _display->setFont(&FreeSans9pt7b);
      calculateTextMetrics(10, 21);

      // Extra 8pt fonts:
    #  ifdef ADAGFX_FONTS_EXTRA_8PT_INCLUDED
    #   ifdef ADAGFX_FONTS_EXTRA_8PT_ANGELINA
    } else if (sParams[0].equals(F("angelina8prop"))) { // Proportional font!
      _display->setFont(&angelina8pt7b);
      calculateTextMetrics(6, 16);
    #   endif // ifdef ADAGFX_FONTS_EXTRA_8PT_ANGELINA
    #   ifdef ADAGFX_FONTS_EXTRA_8PT_NOVAMONO
    } else if (sParams[0].equals(F("novamono8pt"))) {
      _display->setFont(&NovaMono8pt7b);
      calculateTextMetrics(9, 16);
    #   endif // ifdef ADAGFX_FONTS_EXTRA_8PT_NOVAMONO
    #   ifdef ADAGFX_FONTS_EXTRA_8PT_REPETITIONSCROLLiNG
    } else if (sParams[0].equals(F("repetitionscrolling8pt"))) {
      _display->setFont(&RepetitionScrolling8pt7b);
      calculateTextMetrics(9, 16);
    #   endif // ifdef ADAGFX_FONTS_EXTRA_8PT_REPETITIONSCROLLiNG
    #   ifdef ADAGFX_FONTS_EXTRA_8PT_UNISPACE
    } else if (sParams[0].equals(F("unispace8pt"))) {
      _display->setFont(&unispace8pt7b);
      calculateTextMetrics(12, 24);
    #   endif // ifdef ADAGFX_FONTS_EXTRA_8PT_UNISPACE
    #   ifdef ADAGFX_FONTS_EXTRA_8PT_UNISPACEITALIC
    } else if (sParams[0].equals(F("unispaceitalic8pt"))) {
      _display->setFont(&unispace_italic8pt7b);
      calculateTextMetrics(12, 24);
    #   endif // ifdef ADAGFX_FONTS_EXTRA_8PT_UNISPACEITALIC
    #   ifdef ADAGFX_FONTS_EXTRA_8PT_WHITERABBiT
    } else if (sParams[0].equals(F("whiterabbit8pt"))) {
      _display->setFont(&whitrabt8pt7b);
      calculateTextMetrics(12, 24);
    #   endif // ifdef ADAGFX_FONTS_EXTRA_8PT_WHITERABBiT
    #  endif  // ifdef ADAGFX_FONTS_EXTRA_8PT_INCLUDED
      // Extra 12pt fonts:
    #  ifdef ADAGFX_FONTS_EXTRA_12PT_INCLUDED
    #   ifdef ADAGFX_FONTS_EXTRA_12PT_ANGELINA
    } else if (sParams[0].equals(F("angelina12prop"))) { // Proportional font!
      _display->setFont(&angelina12pt7b);
      calculateTextMetrics(8, 24);
    #   endif // ifdef ADAGFX_FONTS_EXTRA_12PT_ANGELINA
    #   ifdef ADAGFX_FONTS_EXTRA_12PT_NOVAMONO
    } else if (sParams[0].equals(F("novamono12pt"))) {
      _display->setFont(&NovaMono12pt7b);
      calculateTextMetrics(13, 34);
    #   endif // ifdef ADAGFX_FONTS_EXTRA_12PT_NOVAMONO
    #   ifdef ADAGFX_FONTS_EXTRA_12PT_REPETITIONSCROLLiNG
    } else if (sParams[0].equals(F("repetitionscrolling12pt"))) {
      _display->setFont(&RepetitionScrolling12pt7b);
      calculateTextMetrics(13, 24);
    #   endif // ifdef ADAGFX_FONTS_EXTRA_12PT_REPETITIONSCROLLiNG
    #   ifdef ADAGFX_FONTS_EXTRA_12PT_UNISPACE
    } else if (sParams[0].equals(F("unispace12pt"))) {
      _display->setFont(&unispace12pt7b);
      calculateTextMetrics(13, 18);
    #   endif // ifdef ADAGFX_FONTS_EXTRA_12PT_UNISPACE
    #   ifdef ADAGFX_FONTS_EXTRA_12PT_UNISPACEITALIC
    } else if (sParams[0].equals(F("unispaceitalic12pt"))) {
      _display->setFont(&unispace_italic12pt7b);
      calculateTextMetrics(13, 18);
    #   endif // ifdef ADAGFX_FONTS_EXTRA_12PT_UNISPACEITALIC
    #   ifdef ADAGFX_FONTS_EXTRA_12PT_WHITERABBiT
    } else if (sParams[0].equals(F("whiterabbir12pt"))) {
      _display->setFont(&whitrabt12pt7b);
      calculateTextMetrics(13, 18);
    #   endif // ifdef ADAGFX_FONTS_EXTRA_12PT_WHITERABBiT
    #  endif  // ifdef ADAGFX_FONTS_EXTRA_12PT_INCLUDED
    } else if (sParams[0].equals(F("default"))) { // font,default is always available!
      _display->setFont();
      calculateTextMetrics(6, 10);
    } else {
      success = false;
    }
    # else // if ADAGFX_FONTS_INCLUDED
    success = false;
    # endif // if ADAGFX_FONTS_INCLUDED
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
    # endif // if ADAGFX_ARGUMENT_VALIDATION
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
    # endif // if ADAGFX_ARGUMENT_VALIDATION
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
    # endif // if ADAGFX_ARGUMENT_VALIDATION
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
    # endif // if ADAGFX_ARGUMENT_VALIDATION
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
      uint8_t h = 0;
      uint8_t v = 0;

      if (subcommand.equals(F("pxh"))) {
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

          if (subcommand.equals(F("pxh"))) {
            h++;
          } else {
            v++;
          }
        }
        delay(0);
      }
      _display->endWrite();
    }
  } else {
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
  uint16_t _x = X;
  uint16_t _y = Y;
  uint16_t _w = 0;
  uint8_t  _h = 0;

  if (_columnRowMode) {
    _x = X * (_fontwidth * textSize); // We need this multiple times
    _y = Y * (_fontheight * textSize);
  }

  if (_textBackFill && (color != bkcolor)) { // Draw extra lines above text
    // Estimate used width
    _w = _textPrintMode == AdaGFXTextPrintMode::ContinueToNextLine ?
         strlen(string) * _fontwidth * textSize :
         (_textcols * _fontwidth * textSize) - _x;

    do {
      _display->drawLine(_x, _y, _x + _w, _y, bkcolor);
      _h++;
      _y++; // Shift down entire line
    } while (_h < textSize);
  }

  _display->setCursor(_x, _y);
  _display->setTextColor(color, bkcolor);
  _display->setTextSize(textSize);

  String newString = string;

  if ((_textPrintMode != AdaGFXTextPrintMode::ContinueToNextLine) &&
      (newString.length() > static_cast<unsigned int>(_textcols - (_x / (_fontwidth * textSize))))) {
    newString = newString.substring(0, _textcols - (_x / (_fontwidth * textSize)));
  }

  if (_textPrintMode == AdaGFXTextPrintMode::ClearThenTruncate) { // Clear before print
    _display->setCursor(_x, _y);

    for (uint16_t c = 0; c < newString.length(); c++) {
      _display->print(' ');
    }
    delay(0);
  }

  _display->setCursor(_x, _y);
  _display->print(newString);

  for (uint16_t c = _x + newString.length() + 1; c <= _textcols && _textPrintMode != AdaGFXTextPrintMode::ContinueToNextLine; c++) {
    _display->print(' ');
  }

  if (_textBackFill && (color != bkcolor)) { // Draw extra lines below text
    _y += ((_fontheight - 1) * textSize);
    _h  = 0;

    do {
      _display->drawLine(_x, _y - _h, _x + _w, _y - _h, bkcolor);
      _h++;
    } while (_h <= textSize);
  }

  // _display->println(); // Leave cursor at next (new) line?
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
// return : color (default ADAGFX_WHITE)
uint16_t AdaGFXparseColor(String& s, AdaGFXColorDepth colorDepth) {
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

  if ((result == -1) || (result == ADAGFX_WHITE)) {                                  // Default & don't convert white
    if ((colorDepth >= AdaGFXColorDepth::SevenColor) &&
        (colorDepth <= AdaGFXColorDepth::SixteenColor)) {
      result = static_cast<uint16_t>(AdaGFXMonoRedGreyscaleColors::ADAGFXEPD_BLACK); // Monochrome fallback, compatible 7-color
    } else {
      result = ADAGFX_WHITE;                                                         // Color fallback value
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
      # endif // if ADAGFX_SUPPORT_7COLOR
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

/*****************************************************************************************
 * Convert an RGB565 color (number) to it's name or the #rgb565 hex string, based on depth
 ****************************************************************************************/
String AdaGFXcolorToString(uint16_t         color,
                           AdaGFXColorDepth colorDepth) {
  switch (colorDepth) {
    case AdaGFXColorDepth::Monochrome:
    case AdaGFXColorDepth::BlackWhiteRed:
    case AdaGFXColorDepth::BlackWhite2Greyscales:
    {
      switch (color) {
        case static_cast<uint16_t>(AdaGFXMonoRedGreyscaleColors::ADAGFXEPD_BLACK): return F("black");
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
    # ifdef ADAGFX_SUPPORT_7COLOR
    case AdaGFXColorDepth::SevenColor:
    {
      switch (color) {
        case static_cast<uint16_t>(AdaGFX7Colors::ADAGFX7C_BLACK): return F("black");
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
    # endif // ifdef ADAGFX_SUPPORT_7COLOR
    case AdaGFXColorDepth::EightColor:
    case AdaGFXColorDepth::SixteenColor:
    case AdaGFXColorDepth::FullColor:
    {
      switch (color) {
        case ADAGFX_BLACK:  return F("black");
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
  String result;
  result  = '#';
  result += String(color, HEX);
  result.toUpperCase();
  return result;
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
                                        uint8_t & fontscaling) {
  textcols    = _textcols;
  textrows    = _textrows;
  fontwidth   = _fontwidth;
  fontheight  = _fontheight;
  fontscaling = _fontscaling;
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
 * calculateTextMetrix: Recalculate the text mertics based on supplied font parameters
 ***************************************************************************/
void AdafruitGFX_helper::calculateTextMetrics(uint8_t fontwidth,
                                              uint8_t fontheight) {
  _fontwidth  = fontwidth;
  _fontheight = fontheight;
  _textcols   = _res_x / (_fontwidth * _fontscaling);
  _textrows   = _res_y / (_fontheight * _fontscaling);

  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(ADAGFX_LOG_LEVEL)) {
    String log;
    log.reserve(60);
    log = F("AdaGFX:");

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
    addLog(ADAGFX_LOG_LEVEL, log);
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
    log  = F("invalidCoordinates: X:");
    log += X;
    log += '/';
    log += (colRowMode ? _textcols : _res_x);
    log += F(" Y:");
    log += Y;
    log += '/';
    log += (colRowMode ? _textrows : _res_y);
    addLog(ADAGFX_LOG_LEVEL, log);
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
}

#endif // ifdef PLUGIN_USES_ADAFRUITGFX
