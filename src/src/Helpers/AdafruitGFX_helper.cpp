#include "../Helpers/AdafruitGFX_helper.h"
#include "../../_Plugin_Helper.h"

#ifdef PLUGIN_USES_ADAFRUITGFX

# include "../Helpers/StringConverter.h"
# include "../WebServer/Markup_Forms.h"

# ifdef ADAGFX_FONTS_INCLUDED
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
# endif  // ifdef ADAGFX_FONTS_INCLUDED

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

/*****************************************************************************************
 * Show a selector for all available 'Text print mode' options, for use in PLUGIN_WEBFORM_LOAD
 ****************************************************************************************/
void AdaGFXFormTextPrintMode(const __FlashStringHelper *id,
                             uint8_t                    selectedIndex) {
  const int textModeCount                            = static_cast<int>(AdaGFXTextPrintMode::MAX);
  const __FlashStringHelper *options3[textModeCount] = { // Be sure to use all available modes from enum!
    getAdaGFXTextPrintMode(AdaGFXTextPrintMode::ContinueToNextLine),
    getAdaGFXTextPrintMode(AdaGFXTextPrintMode::TruncateExceedingMessage),
    getAdaGFXTextPrintMode(AdaGFXTextPrintMode::ClearThenTruncate)
  };
  const int optionValues3[textModeCount] = {
    static_cast<int>(AdaGFXTextPrintMode::ContinueToNextLine),
    static_cast<int>(AdaGFXTextPrintMode::TruncateExceedingMessage),
    static_cast<int>(AdaGFXTextPrintMode::ClearThenTruncate)
  };

  addFormSelector(F("Text print Mode"), id, textModeCount, options3, optionValues3, selectedIndex);
}

// AdafruitGFX_helper class methods

/****************************************************************************
 * parameterized constructor
 ***************************************************************************/
AdafruitGFX_helper::AdafruitGFX_helper(Adafruit_GFX       *display,
                                       const String      & trigger,
                                       uint16_t            res_x,
                                       uint16_t            res_y,
                                       ColorDepth          colorDepth,
                                       AdaGFXTextPrintMode textPrintMode,
                                       uint8_t             fontscaling)
  : _display(display), _trigger(trigger), _res_x(res_x), _res_y(res_y), _colorDepth(colorDepth),
  _textPrintMode(textPrintMode), _fontscaling(fontscaling)
{
  _trigger.toLowerCase();      // store trigger in lowercase
  calculateTextMetrics(6, 10); // Defaults for built-in font
}

/****************************************************************************
 * processCommand: Parse string to <command>,<subcommand>[,<arguments>...] and execute that command
 ***************************************************************************/
bool AdafruitGFX_helper::processCommand(const String& string) {
  bool success = false;

  if ((_display == nullptr) || _trigger.isEmpty()) { return success; }

  String cmd        = parseString(string, 1); // lower case
  String subcommand = parseString(string, 2);

  if (!cmd.equals(_trigger) || subcommand.isEmpty()) { return success; } // Only support own trigger, and at least a non=empty subcommand

  String log;
  String sParams[ADAGFX_PARSE_MAX_ARGS];
  int    nParams[ADAGFX_PARSE_MAX_ARGS];
  int    argCount = 0;
  bool   loop     = true;

  while (argCount < ADAGFX_PARSE_MAX_ARGS && loop) {
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

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    log.reserve(90);
    log  = F("AdaGFX: parseCommand: ");
    log += _trigger;
    log += F(" argCount: ");
    log += argCount;
    log += ':';
    log += string;
    addLog(LOG_LEVEL_INFO, log);
  }
  # endif // ifndef BUILD_NO_DEBUG

  if (subcommand.equals(F("txt"))) // txt: Print text at last cursor position, ends at next line!
  {
    for (uint8_t n = 0; n < argCount; n++) {
      _display->print(sParams[n]);                         // write all pending cars

      if (n < argCount - 1) {
        _display->print(' ');                              // a space-separator if not at end
      }
    }
    _display->println();                                   // Next line
  }
  else if (subcommand.equals(F("txp")) && (argCount == 2)) // txp: Text position
  {
    # ifdef ADAGFX_ARGUMENT_VALIDATION

    if (invalidCoordinates(nParams[0], nParams[1], _columnRowMode)) {
      success = false;
    } else
    # endif // ifdef ADAGFX_ARGUMENT_VALIDATION
    {
      if (_columnRowMode) {
        _display->setCursor(nParams[0] * _fontwidth, nParams[1] * _fontheight);
      } else {
        _display->setCursor(nParams[0], nParams[1]);
      }
    }
  }
  else if (subcommand.equals(F("txc")) && ((argCount == 1) || (argCount == 2))) // txc: Textcolor, fg and opt. bg colors
  {
    _fgcolor = parseColor(sParams[0]);

    if (argCount == 1) {
      _display->setTextColor(_fgcolor);
    } else { // argCount=2
      _bgcolor = parseColor(sParams[1]);
      _display->setTextColor(_fgcolor, _bgcolor);
    }
  }
  else if (subcommand.equals(F("txs")) && (argCount == 1))
  {
    _fontscaling = nParams[0];
    _display->setTextSize(_fontscaling);
  }
  else if (subcommand.equals(F("txtfull")) && (argCount >= 3) && (argCount <= 6)) { // txtfull: Text at position, with size and color
    switch (argCount) {
      case 3:                                                                       // single text

        # ifdef ADAGFX_ARGUMENT_VALIDATION

        if (invalidCoordinates(nParams[0] - _p095_compensation, nParams[1] - _p095_compensation, true)) {
          success = false;
        } else
        # endif // ifdef ADAGFX_ARGUMENT_VALIDATION
        {
          printText(sParams[2].c_str(),
                    nParams[0] - _p095_compensation,
                    nParams[1] - _p095_compensation,
                    _fontscaling,
                    _fgcolor,
                    _bgcolor);
        }
        break;
      case 4: // text + size

        # ifdef ADAGFX_ARGUMENT_VALIDATION

        if (invalidCoordinates(nParams[0] - _p095_compensation, nParams[1] - _p095_compensation, true)) {
          success = false;
        } else
        # endif // ifdef ADAGFX_ARGUMENT_VALIDATION
        {
          printText(sParams[3].c_str(),
                    nParams[0] - _p095_compensation,
                    nParams[1] - _p095_compensation,
                    nParams[2],
                    _fgcolor,
                    _bgcolor);
        }
        break;
      case 5: // text + size + color

        # ifdef ADAGFX_ARGUMENT_VALIDATION

        if (invalidCoordinates(nParams[0] - _p095_compensation, nParams[1] - _p095_compensation, true)) {
          success = false;
        } else
        # endif // ifdef ADAGFX_ARGUMENT_VALIDATION
        {
          printText(sParams[4].c_str(),
                    nParams[0] - _p095_compensation,
                    nParams[1] - _p095_compensation,
                    nParams[2],
                    parseColor(sParams[3]),
                    _bgcolor);
        }
        break;
      case 6: // text + size + color + bkcolor

        # ifdef ADAGFX_ARGUMENT_VALIDATION

        if (invalidCoordinates(nParams[0] - _p095_compensation, nParams[1] - _p095_compensation, true)) {
          success = false;
        } else
        # endif // ifdef ADAGFX_ARGUMENT_VALIDATION
        {
          printText(sParams[5].c_str(),
                    nParams[0] - _p095_compensation,
                    nParams[1] - _p095_compensation,
                    nParams[2],
                    parseColor(sParams[3]),
                    parseColor(sParams[4]));
        }
        break;
      default:
        success = false;
        break;
    }
  }
  else if (subcommand.equals(F("clear")))
  {
    _display->fillScreen(ADAGFX_BLACK);
  }
  else if (subcommand.equals(F("font")) && (argCount == 1)) { // font: Change font
    # ifdef ADAGFX_FONTS_INCLUDED
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
    # else // ifdef ADAGFX_FONTS_INCLUDED
    success = false;
    # endif // ifdef ADAGFX_FONTS_INCLUDED
  }
  else if (subcommand.equals(F("l")) && (argCount == 5)) { // l: Line
    # ifdef ADAGFX_ARGUMENT_VALIDATION

    if (invalidCoordinates(nParams[0], nParams[1]) ||
        invalidCoordinates(nParams[2], nParams[3])) {
      success = false;
    } else
    # endif // ifdef ADAGFX_ARGUMENT_VALIDATION
    {
      _display->drawLine(nParams[0], nParams[1], nParams[2], nParams[3], parseColor(sParams[4]));
    }
  }
  else if (subcommand.equals(F("lh")) && (argCount == 3)) { // lh: Horizontal line
    # ifdef ADAGFX_ARGUMENT_VALIDATION

    if (invalidCoordinates(nParams[0], nParams[1])) {
      success = false;
    } else
    # endif // ifdef ADAGFX_ARGUMENT_VALIDATION
    {
      _display->drawFastHLine(0, nParams[0], nParams[1], parseColor(sParams[2]));
    }
  }
  else if (subcommand.equals(F("lv")) && (argCount == 3)) { // lv: Vertical line
    # ifdef ADAGFX_ARGUMENT_VALIDATION

    if (invalidCoordinates(nParams[0], nParams[1])) {
      success = false;
    } else
    # endif // ifdef ADAGFX_ARGUMENT_VALIDATION
    {
      _display->drawFastVLine(nParams[0], 0, nParams[1], parseColor(sParams[2]));
    }
  }
  else if (subcommand.equals(F("r")) && (argCount == 5)) { // r: Rectangle
    # ifdef ADAGFX_ARGUMENT_VALIDATION

    if (invalidCoordinates(nParams[0], nParams[1]) ||
        invalidCoordinates(nParams[2], nParams[3])) {
      success = false;
    } else
    # endif // ifdef ADAGFX_ARGUMENT_VALIDATION
    {
      _display->drawRect(nParams[0], nParams[1], nParams[2], nParams[3], parseColor(sParams[4]));
    }
  }
  else if (subcommand.equals(F("rf")) && (argCount == 6)) { // rf: Rectangled, filled
    # ifdef ADAGFX_ARGUMENT_VALIDATION

    if (invalidCoordinates(nParams[0], nParams[1]) ||
        invalidCoordinates(nParams[2], nParams[3])) {
      success = false;
    } else
    # endif // ifdef ADAGFX_ARGUMENT_VALIDATION
    {
      _display->fillRect(nParams[0], nParams[1], nParams[2], nParams[3], parseColor(sParams[5]));
      _display->drawRect(nParams[0], nParams[1], nParams[2], nParams[3], parseColor(sParams[4]));
    }
  }
  else if (subcommand.equals(F("c")) && (argCount == 4)) { // c: Circle
    # ifdef ADAGFX_ARGUMENT_VALIDATION

    if (invalidCoordinates(nParams[0], nParams[1]) ||
        invalidCoordinates(nParams[2], 0)) { // Also check radius
      success = false;
    } else
    # endif // ifdef ADAGFX_ARGUMENT_VALIDATION
    {
      _display->drawCircle(nParams[0], nParams[1], nParams[2], parseColor(sParams[3]));
    }
  }
  else if (subcommand.equals(F("cf")) && (argCount == 5)) { // cf: Circle, filled
    # ifdef ADAGFX_ARGUMENT_VALIDATION

    if (invalidCoordinates(nParams[0], nParams[1]) ||
        invalidCoordinates(nParams[2], 0)) { // Also check radius
      success = false;
    } else
    # endif // ifdef ADAGFX_ARGUMENT_VALIDATION
    {
      _display->fillCircle(nParams[0], nParams[1], nParams[2], parseColor(sParams[4]));
      _display->drawCircle(nParams[0], nParams[1], nParams[2], parseColor(sParams[3]));
    }
  }
  else if (subcommand.equals(F("t")) && (argCount == 7)) { // t: Triangle
    # ifdef ADAGFX_ARGUMENT_VALIDATION

    if (invalidCoordinates(nParams[0], nParams[1]) ||
        invalidCoordinates(nParams[2], nParams[3]) ||
        invalidCoordinates(nParams[4], nParams[5])) {
      success = false;
    } else
    # endif // ifdef ADAGFX_ARGUMENT_VALIDATION
    {
      _display->drawTriangle(nParams[0], nParams[1], nParams[2], nParams[3], nParams[4], nParams[5], parseColor(sParams[6]));
    }
  }
  else if (subcommand.equals(F("tf")) && (argCount == 8)) { // tf: Triangle, filled
    # ifdef ADAGFX_ARGUMENT_VALIDATION

    if (invalidCoordinates(nParams[0], nParams[1]) ||
        invalidCoordinates(nParams[2], nParams[3]) ||
        invalidCoordinates(nParams[4], nParams[5])) {
      success = false;
    } else
    # endif // ifdef ADAGFX_ARGUMENT_VALIDATION
    {
      _display->fillTriangle(nParams[0], nParams[1], nParams[2], nParams[3], nParams[4], nParams[5], parseColor(sParams[7]));
      _display->drawTriangle(nParams[0], nParams[1], nParams[2], nParams[3], nParams[4], nParams[5], parseColor(sParams[6]));
    }
  }
  else if (subcommand.equals(F("rr")) && (argCount == 6)) { // rr: Rounded rectangle
    # ifdef ADAGFX_ARGUMENT_VALIDATION

    if (invalidCoordinates(nParams[0], nParams[1]) ||
        invalidCoordinates(nParams[2], nParams[3]) ||
        invalidCoordinates(nParams[4], 0)) { // Also check radius
      success = false;
    } else
    # endif // ifdef ADAGFX_ARGUMENT_VALIDATION
    {
      _display->drawRoundRect(nParams[0], nParams[1], nParams[2], nParams[3], nParams[4], parseColor(sParams[5]));
    }
  }
  else if (subcommand.equals(F("rrf")) && (argCount == 7)) { // rrf: Rounded rectangle, filled
    # ifdef ADAGFX_ARGUMENT_VALIDATION

    if (invalidCoordinates(nParams[0], nParams[1]) ||
        invalidCoordinates(nParams[2], nParams[3])) {
      success = false;
    } else
    # endif // ifdef ADAGFX_ARGUMENT_VALIDATION
    {
      _display->fillRoundRect(nParams[0], nParams[1], nParams[2], nParams[3], nParams[4], parseColor(sParams[6]));
      _display->drawRoundRect(nParams[0], nParams[1], nParams[2], nParams[3], nParams[4], parseColor(sParams[5]));
    }
  }
  else if (subcommand.equals(F("px")) && (argCount == 3)) { // px: Pixel
    # ifdef ADAGFX_ARGUMENT_VALIDATION

    if (invalidCoordinates(nParams[0], nParams[1])) {
      success = false;
    } else
    # endif // ifdef ADAGFX_ARGUMENT_VALIDATION
    {
      _display->drawPixel(nParams[0], nParams[1], parseColor(sParams[2]));
    }
  }
  else if ((subcommand.equals(F("pxh")) || subcommand.equals(F("pxv"))) && (argCount > 2)) { // pxh/pxv: Pixels, hor./vert. incremented
    # ifdef ADAGFX_ARGUMENT_VALIDATION                                                       // merged loop is smaller than 2 separate loops

    if (invalidCoordinates(nParams[0], nParams[1])) {
      success = false;
    } else
    # endif // ifdef ADAGFX_ARGUMENT_VALIDATION
    {
      _display->startWrite();
      _display->writePixel(nParams[0], nParams[1], parseColor(sParams[2]));
      loop = true;
      uint8_t h = 0;
      uint8_t v = 0;

      if (subcommand.equals(F("pxh"))) {
        h++;
      } else {
        v++;
      }

      while (loop) {
        String color = parseString(string, h + v + 5); // 5 = 2 + 3 already parsed

        if (color.isEmpty()) {
          loop = false;
        } else {
          _display->writePixel(nParams[0] + h, nParams[1] + v, parseColor(color));

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
void AdafruitGFX_helper::printText(const char *string, int X, int Y, unsigned int textSize, unsigned short color, unsigned short bkcolor) {
  if (_columnRowMode) {
    _display->setCursor(X * _fontwidth, Y * _fontheight);
  } else {
    _display->setCursor(X, Y);
  }
  _display->setTextColor(color, bkcolor);
  _display->setTextSize(textSize);
  _display->println(string);
}

/****************************************************************************
 * color565: convert r, g, b colors to rgb565 (by bit-shifting)
 ***************************************************************************/
uint16_t color565(uint8_t red, uint8_t green, uint8_t blue) {
  return ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3);
}

/****************************************************************************
 * parseColor: translate color name, rgb565 hex #rGgb or rgb hex #RRGGBB to an RGB565 value,
 * also applies color reduction to mono(2), duo(3), quadro(4), octo(8), quinto(16)-chrome colors
 ***************************************************************************/

// Parse color string to RGB565 color
// param [in] s : The color string (white, red, ...)
// return : color (default ADAGFX_WHITE)
uint16_t AdafruitGFX_helper::parseColor(String& string) {
  String s = string;

  s.toLowerCase();
  int32_t result = -1; // No result yet

  if (s.equals(F("black"))) { result = ADAGFX_BLACK; }

  if (s.equals(F("navy"))) { result = ADAGFX_NAVY; }

  if (s.equals(F("darkgreen"))) { result = ADAGFX_DARKGREEN; }

  if (s.equals(F("darkcyan"))) { result = ADAGFX_DARKCYAN; }

  if (s.equals(F("maroon"))) { result = ADAGFX_MAROON; }

  if (s.equals(F("purple"))) { result = ADAGFX_PURPLE; }

  if (s.equals(F("olive"))) { result = ADAGFX_OLIVE; }

  if (s.equals(F("lightgrey"))) { result = ADAGFX_LIGHTGREY; }

  if (s.equals(F("darkgrey"))) { result = ADAGFX_DARKGREY; }

  if (s.equals(F("blue"))) { result = ADAGFX_BLUE; }

  if (s.equals(F("green"))) { result = ADAGFX_GREEN; }

  if (s.equals(F("cyan"))) { result = ADAGFX_CYAN; }

  if (s.equals(F("red"))) { result = ADAGFX_RED; }

  if (s.equals(F("magenta"))) { result = ADAGFX_MAGENTA; }

  if (s.equals(F("yellow"))) { result = ADAGFX_YELLOW; }

  if (s.equals(F("white"))) { result = ADAGFX_WHITE; }

  if (s.equals(F("orange"))) { result = ADAGFX_ORANGE; }

  if (s.equals(F("greenyellow"))) { result = ADAGFX_GREENYELLOW; }

  if (s.equals(F("pink"))) { result = ADAGFX_PINK; }

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

  if (result == -1) {
    result = ADAGFX_WHITE; // fallback value
  }

  switch (_colorDepth) {
    case ColorDepth::Monochrome:

      if ((result != ADAGFX_WHITE) && (result != ADAGFX_BLACK)) {
        result = ADAGFX_WHITE;
      }
      break;
    case ColorDepth::Duochrome:

      if ((result != ADAGFX_WHITE) && (result != ADAGFX_BLACK)) {
        result = ADAGFX_LIGHTGREY;
      }
      break;
    case ColorDepth::Quadrochrome:

      if ((result != ADAGFX_WHITE) && (result != ADAGFX_BLACK)) {
        if (result > ADAGFX_LIGHTGREY) {
          result = ADAGFX_LIGHTGREY;
        } else if (result > ADAGFX_DARKGREY) {
          result = ADAGFX_DARKGREY;
        } else {
          result = ADAGFX_BLACK;
        }
      }
      break;
    case ColorDepth::Octochrome:
      result = color565((result >> 11) / 4, (result >> 5 & 0x3F) / 4, (result & 0x1F) / 4); // reduce colors factor 4
      break;
    case ColorDepth::Quintochrome:
      result = color565((result >> 11) / 2, (result >> 5 & 0x3F) / 2, (result & 0x1F) / 2); // reduce colors factor 2
      break;
    case ColorDepth::FullColor:
      break;
  }
  return result;
}

/****************************************************************************
 * getTextMetrics: Returns the metrics related to current font
 ***************************************************************************/
void AdafruitGFX_helper::getTextMetrics(uint16_t& textcols, uint16_t& textrows, uint8_t& fontwidth, uint8_t& fontheight) {
  textcols   = _textcols;
  textrows   = _textrows;
  fontwidth  = _fontwidth;
  fontheight = _fontheight;
}

/****************************************************************************
 * calculateTextMetrix: Recalculate the text mertics based on supplied font parameters
 ***************************************************************************/
void AdafruitGFX_helper::calculateTextMetrics(uint8_t fontwidth, uint8_t fontheight) {
  _fontwidth  = fontwidth;
  _fontheight = fontheight;
  _textcols   = _res_x / (_fontwidth * _fontscaling);
  _textrows   = _res_y / (_fontheight * _fontscaling);

  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log;
    log.reserve(30);
    log  = F("AdaGFX: text columns: ");
    log += _textcols;
    log += F(" rows: ");
    log += _textrows;
    addLog(LOG_LEVEL_INFO, log);
  }
  # endif // ifndef BUILD_NO_DEBUG
}

# ifdef ADAGFX_ARGUMENT_VALIDATION

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

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
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
    addLog(LOG_LEVEL_DEBUG, log);
  }
  #  endif // ifndef BUILD_NO_DEBUG

  if (colRowMode) {
    return !((X >= 0) && (X <= _textcols) &&
             (Y >= 0) && (Y <= _textrows));
  } else {
    if (Y == 0) {
      return !((X >= 0) && (X <= std::max(_res_x, _res_y)));
    } else {
      return !((X >= 0) && (X <= _res_x) &&
               (Y >= 0) && (Y <= _res_y));
    }
  }
}

# endif // ifdef ADAGFX_ARGUMENT_VALIDATION

#endif  // ifdef PLUGIN_USES_ADAFRUITGFX
