#include "../PluginStructs/P131_data_struct.h"

#ifdef USES_P131
# include "../Helpers/AdafruitGFX_helper.h" // Use Adafruit graphics helper object

/****************************************************************************
 * P131_CommandTrigger_toString: return the command string selected
 ***************************************************************************/
const __FlashStringHelper* P131_CommandTrigger_toString(P131_CommandTrigger cmd) {
  switch (cmd) {
    case P131_CommandTrigger::neomatrix: return F("neomatrix");
    case P131_CommandTrigger::neo: return F("neo");
    case P131_CommandTrigger::MAX: break;
  }
  return F("None");
}

// **************************************************************************/
// Constructor
// **************************************************************************/
P131_data_struct::P131_data_struct(uint8_t             matrixWidth,
                                   uint8_t             matrixHeight,
                                   uint8_t             tileWidth,
                                   uint8_t             tileHeight,
                                   int8_t              pin,
                                   uint8_t             matrixType,
                                   neoPixelType        ledType,
                                   uint8_t             rotation,
                                   uint8_t             fontscaling,
                                   AdaGFXTextPrintMode textmode,
                                   String              commandTrigger,
                                   uint8_t             brightness,
                                   uint8_t             maxbright,
                                   uint16_t            fgcolor,
                                   uint16_t            bgcolor)
  :  _matrixWidth(matrixWidth),  _matrixHeight(matrixHeight),  _tileWidth(tileWidth),  _tileHeight(tileHeight),
  _pin(pin),  _matrixType(matrixType),  _ledType(ledType), _rotation(rotation), _fontscaling(fontscaling), _textmode(textmode),
  _commandTrigger(commandTrigger), _brightness(brightness), _maxbright(maxbright), _fgcolor(fgcolor), _bgcolor(bgcolor) {
  updateFontMetrics();

  _commandTrigger.toLowerCase();
  _commandTriggerCmd  = _commandTrigger;
  _commandTriggerCmd += F("cmd");
}

// **************************************************************************/
// Destructor
// **************************************************************************/
P131_data_struct::~P131_data_struct() {
  if (isInitialized()) {
    delete matrix;
    matrix = nullptr;
  }
}

/****************************************************************************
 * plugin_init: Initialize display
 ***************************************************************************/
bool P131_data_struct::plugin_init(struct EventStruct *event) {
  bool success = false;

  if (!isInitialized()) {
    addLog(LOG_LEVEL_INFO, F("NEOMATRIX: Init start."));
    matrix = new (std::nothrow) Adafruit_NeoMatrix(_matrixWidth,
                                                   _matrixHeight,
                                                   _tileWidth,
                                                   _tileHeight,
                                                   _pin,
                                                   _matrixType,
                                                   _ledType);

    if (isInitialized()) {
      _xpix = matrix->width();
      _ypix = matrix->height();
    }
    # ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log;
      log.reserve(90);
      log += F("NEOMATRIX: Init done, address: 0x");
      log += String(reinterpret_cast<ulong>(matrix), HEX);
      log += ' ';

      if (!isInitialized()) {
        log += F("in");
      }
      log += F("valid, commands: ");
      log += _commandTrigger;
      log += '/';
      log += _commandTriggerCmd;
      log += F(", size: w:");
      log += _xpix;
      log += F(", h:");
      log += _ypix;
      addLogMove(LOG_LEVEL_INFO, log);
    }
    # endif // ifndef BUILD_NO_DEBUG
  } else {
    addLog(LOG_LEVEL_INFO, F("NEOMATRIX: No init?"));
  }

  if (isInitialized()) {
    gfxHelper = new (std::nothrow) AdafruitGFX_helper(matrix,
                                                      _commandTrigger,
                                                      _xpix,
                                                      _ypix,
                                                      AdaGFXColorDepth::FullColor,
                                                      _textmode,
                                                      _fontscaling,
                                                      _fgcolor,
                                                      _bgcolor,
                                                      true,
                                                      _textBackFill);

    gfxHelper->setRotation(_rotation);
    matrix->setBrightness(std::min(_maxbright, _brightness)); // Set brightness, so we don't get blinded by the light
    matrix->fillScreen(_bgcolor);                             // fill screen with black color

    # ifdef P131_SHOW_SPLASH
    uint16_t yPos = 0;
    gfxHelper->printText(String(F("ESPEasy")).c_str(), 0, yPos, 1, ADAGFX_WHITE, ADAGFX_BLACK);
    matrix->show();
    delay(100);                               // Splash
    # endif // ifdef P131_SHOW_SPLASH

    matrix->setTextColor(_fgcolor, _bgcolor); // set text color to white and black background
    gfxHelper->setColumnRowMode(false);       // Pixel-mode
    matrix->setTextSize(_fontscaling);        // Handles 0 properly, text size, default 1 = very small
    matrix->setCursor(0, 0);                  // move cursor to position (0, 0) pixel
    updateFontMetrics();

    success = true;
  }
  return success;
}

/****************************************************************************
 * plugin_exit: De-initialize before destruction
 ***************************************************************************/
bool P131_data_struct::plugin_exit(struct EventStruct *event) {
  addLog(LOG_LEVEL_INFO, F("NEOMATRIX: Exit."));

  if ((nullptr != matrix) && bitRead(P131_CONFIG_FLAGS, P131_CONFIG_FLAG_CLEAR_ON_EXIT)) {
    matrix->setTextColor(ADAGFX_WHITE, ADAGFX_BLACK);
    matrix->fillScreen(ADAGFX_BLACK); // fill screen with black color
  }
  cleanup();
  return true;
}

/****************************************************************************
 * cleanup: De-initialize pointers
 ***************************************************************************/
void P131_data_struct::cleanup() {
  if (nullptr != gfxHelper) { delete gfxHelper; }
  gfxHelper = nullptr;

  if (nullptr != matrix) { delete matrix; }
  matrix = nullptr;
}

/****************************************************************************
 * plugin_read: Re-draw the default content
 ***************************************************************************/
bool P131_data_struct::plugin_read(struct EventStruct *event) {
  if (isInitialized()) {
    if (!stringsInitialized) {
      LoadCustomTaskSettings(event->TaskIndex, strings, P131_Nlines, 0);
      stringsInitialized = true;
    }

    if (!contentInitialized && stringsInitialized) {
      content.clear();
      content.reserve(P131_CONFIG_TILE_HEIGHT);

      for (uint8_t x = 0; x < P131_CONFIG_TILE_HEIGHT; x++) {
        content[x] = P131_content_struct();
        String opts    = parseString(strings[x], 2);
        int    optBits = 0;
        validIntFromString(opts, optBits);
        content[x].active      = bitRead(optBits, P131_OPTBITS_SCROLL);
        content[x].rightScroll = bitRead(optBits, P131_OPTBITS_RIGHTSCROLL);
        content[x].pixelMode   = bitRead(optBits, P131_OPTBITS_PIXELSCROLL);
        opts                   = parseString(strings[x], 3);
        int speed = 0;
        validIntFromString(opts, speed);
        content[x].speed = speed;
      }
      contentInitialized = true;
    }

    bool hasContent = false;

    for (uint8_t x = 0; x < P131_CONFIG_TILE_HEIGHT && !hasContent; x++) {
      hasContent = !parseStringKeepCase(strings[x], 1).isEmpty();
    }

    if (hasContent) {
      display_content(event);
    }
  }
  return false; // Always return false, so no attempt to send to
                // Controllers or generate events is started
}

/****************************************************************************
 * display_content: Re-display the text, and apply any scrolling offset
 ***************************************************************************/
void P131_data_struct::display_content(struct EventStruct *event,
                                       bool                scrollOnly) {
  int16_t yPos = 0;

  for (uint8_t x = 0; x < P131_CONFIG_TILE_HEIGHT; x++) {
    String tmpString = parseStringKeepCase(strings[x], 1);

    if (!scrollOnly ||
        (scrollOnly && content[x].active)) {
      String newString = AdaGFXparseTemplate(tmpString, _textcols, gfxHelper);

      # if ADAGFX_PARSE_SUBCOMMAND
      updateFontMetrics();
      # endif // if ADAGFX_PARSE_SUBCOMMAND

      // TODO apply scrolling offset
      if (yPos < _ypix) {
        gfxHelper->printText(newString.c_str(), 0, yPos, _fontscaling, _fgcolor, _bgcolor);
      }
    }
    delay(0);
    yPos += (_fontheight * _fontscaling);
  }
  matrix->show();
}

/****************************************************************************
 * plugin_write: Handle commands
 ***************************************************************************/
bool P131_data_struct::plugin_write(struct EventStruct *event, const String& string) {
  bool   success = false;
  String cmd     = parseString(string, 1);

  if ((nullptr != matrix) && cmd.equals(_commandTriggerCmd)) {
    String sub = parseString(string, 2);
    success = true;

    if (sub.equals(F("clear"))) {
      matrix->fillScreen(_bgcolor);
    }
    else if (sub.startsWith(F("bright")) && (event->Par2 >= 0) && (event->Par2 <= 255)) {
      if (parseString(string, 3).isEmpty()) {                     // No argument, then
        matrix->setBrightness(std::min(_maxbright, _brightness)); // use initial brightness
      } else {
        matrix->setBrightness(std::min(_maxbright, static_cast<uint8_t>(event->Par2)));
      }
    } else {
      success = false;
    }
  }
  else if (matrix && (cmd.equals(_commandTrigger) ||
                      (gfxHelper && gfxHelper->isAdaGFXTrigger(cmd)))) {
    success = true;

    if (nullptr != gfxHelper) {
      String tmp(string);

      // Hand it over after replacing variables
      success = gfxHelper->processCommand(AdaGFXparseTemplate(tmp, _textcols, gfxHelper));

      updateFontMetrics(); // Font or color may have changed
    }
  }

  if (success) {
    matrix->show();
  }
  return success;
}

/****************************************************************************
 * plugin_ten_per_second: Re-draw the default content that should be scrolled
 ***************************************************************************/
bool P131_data_struct::plugin_ten_per_second(struct EventStruct *event) {
  bool success = false;

  if (isInitialized()) {
    success = true;

    for (uint8_t x = 0; x < P131_CONFIG_TILE_HEIGHT; x++) {
      if (content[x].active) {
        if (content[x].loop == -1) { content[x].loop = content[x].speed; } // Initialize

        if (!content[x].loop--) {
          if (content[x].pixelMode) {
            if (content[x].pixelPos > 0) {
              // TODO
            }
          }

          // TODO
          // display_content(event, true);
        }
      }
    }
  }
  return success;
}

/****************************************************************************
 * updateFontMetrics: recalculate x and y columns, based on font size and font scale
 ***************************************************************************/
void P131_data_struct::updateFontMetrics() {
  if (nullptr != gfxHelper) {
    gfxHelper->getTextMetrics(_textcols, _textrows, _fontwidth, _fontheight, _fontscaling, _heightOffset, _xpix, _ypix);
    gfxHelper->getColors(_fgcolor, _bgcolor);
  } else {
    _textcols = _xpix / (_fontwidth * _fontscaling);
    _textrows = _ypix / (_fontheight * _fontscaling);
  }
  # ifdef P131_DEBUG_LOG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("updateFontMetrics: size: ");
    log += _fontscaling;
    log += F(", fg: ");
    log += String(_fgcolor, HEX); // AdaGFXcolorToString(_fgcolor);
    log += F(", bg: ");
    log += String(_bgcolor, HEX); // AdaGFXcolorToString(_bgcolor);

    if (nullptr != matrix) {
      log += F(", xp: ");
      log += matrix->getCursorX();
      log += F(", yp: ");
      log += matrix->getCursorY();
    }
    addLog(LOG_LEVEL_INFO, log);
  }
  # endif // ifdef P131_DEBUG_LOG
}

#endif // ifdef USES_P131
