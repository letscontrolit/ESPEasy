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
    delete matrix; // Doesn't have a virtual destructor (yet)
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
    addLog(LOG_LEVEL_INFO, F("NEOMATRIX: Init failed."));
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

    success = (nullptr != gfxHelper);

    if (success) {
      gfxHelper->setRotation(_rotation);
      matrix->setBrightness(std::min(_maxbright, _brightness)); // Set brightness, so we don't get blinded by the light
      matrix->fillScreen(_bgcolor);                             // fill screen with black color
      matrix->show();                                           // Update the display

      # ifdef P131_SHOW_SPLASH

      if (P131_CONFIG_FLAG_GET_SHOW_SPLASH) {
        uint16_t yPos = 0;
        gfxHelper->printText(String(F("ESPEasy")).c_str(), 0, yPos, 1, ADAGFX_WHITE, ADAGFX_BLACK);
        matrix->show();
        _splashState   = true; // Splash
        _splashCounter = P131_SPLASH_DURATION;
        #  ifndef BUILD_NO_DEBUG
        addLog(LOG_LEVEL_INFO, F("P131 Splash start."));
        #  endif // ifndef BUILD_NO_DEBUG
      }
      # endif // ifdef P131_SHOW_SPLASH

      matrix->setTextColor(_fgcolor, _bgcolor); // set text color to white and black background
      gfxHelper->setColumnRowMode(false);       // Pixel-mode
      matrix->setTextSize(_fontscaling);        // Handles 0 properly, text size, default 1
      matrix->setCursor(0, 0);                  // move cursor to position (0, 0) pixel
      updateFontMetrics();

      // Load
      loadContent(event);

      // Setup initial scroll position
      for (uint8_t x = 0; x < P131_CONFIG_TILE_HEIGHT; x++) {
        content[x].pixelPos = 0;

        if (content[x].active) {
          String   tmpString = parseStringKeepCase(strings[x], 1);
          String   newString = AdaGFXparseTemplate(tmpString, _textcols, gfxHelper);
          uint16_t h;
          content[x].length = gfxHelper->getTextSize(newString, h);

          if (content[x].startBlank && (content[x].length > _xpix)) {
            if (content[x].rightScroll) {
              content[x].pixelPos = -1 * content[x].length;
            } else {
              content[x].pixelPos = _xpix;
            }
          }
        }
      }
    }
  }
  return success;
}

/****************************************************************************
 * plugin_exit: De-initialize before destruction
 ***************************************************************************/
bool P131_data_struct::plugin_exit(struct EventStruct *event) {
  # ifndef BUILD_NO_DEBUG
  addLog(LOG_LEVEL_INFO, F("NEOMATRIX: Exit."));
  # endif // ifndef BUILD_NO_DEBUG

  if ((nullptr != matrix) && bitRead(P131_CONFIG_FLAGS, P131_CONFIG_FLAG_CLEAR_ON_EXIT)) {
    matrix->fillScreen(ADAGFX_BLACK); // fill screen with black color
    matrix->show();
  }
  cleanup();
  return true;
}

/****************************************************************************
 * cleanup: De-initialize pointers
 ***************************************************************************/
void P131_data_struct::cleanup() {
  delete gfxHelper;
  gfxHelper = nullptr;

  delete matrix; // Doesn't have a virtual destructor (yet)
  matrix = nullptr;
}

/****************************************************************************
 * loadContent: load the default content if not yet loaded
 ***************************************************************************/
void P131_data_struct::loadContent(struct EventStruct *event) {
  if (!stringsInitialized) {
    LoadCustomTaskSettings(event->TaskIndex, strings, P131_Nlines, 0);
    stringsInitialized = true;
  }

  if (!contentInitialized && stringsInitialized) {
    content.clear();
    content.resize(P131_CONFIG_TILE_HEIGHT);

    for (uint8_t x = 0; x < P131_CONFIG_TILE_HEIGHT; x++) {
      content[x] = P131_content_struct();
      initialize_content(event, x);
    }
    contentInitialized = true;
  }
}

/****************************************************************************
 * initialize_content: set the content[x] flags from arguments provided
 ***************************************************************************/
void P131_data_struct::initialize_content(struct EventStruct *event,
                                          uint8_t             x) {
  String   opts    = parseString(strings[x], 2);
  uint32_t optBits = 0;

  validUIntFromString(opts, optBits);
  content[x].active      = bitRead(optBits, P131_OPTBITS_SCROLL);
  content[x].rightScroll = bitRead(optBits, P131_OPTBITS_RIGHTSCROLL);
  content[x].startBlank  = bitRead(optBits, P131_OPTBITS_STARTBLANK) == 0;      // Inverted
  content[x].stepWidth   = get4BitFromUL(optBits, P131_OPTBITS_SCROLLSTEP) + 1; // Add offset once
  opts                   = parseString(strings[x], 3);
  int speed = 0;

  validIntFromString(opts, speed);
  content[x].speed = speed;
}

/****************************************************************************
 * plugin_read: Re-draw the default content
 ***************************************************************************/
bool P131_data_struct::plugin_read(struct EventStruct *event) {
  if (isInitialized() && !_splashState) {
    loadContent(event);

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
                                       bool                scrollOnly,
                                       uint8_t             line) {
  if (isInitialized() && (nullptr != gfxHelper)) {
    int16_t yPos   = 0;
    bool    useVal = gfxHelper->getValidation();
    gfxHelper->setValidation(false); // Ignore validation to enable scrolling

    uint8_t x     = 0;
    uint8_t x_end = P131_CONFIG_TILE_HEIGHT;

    if (line != 255) {
      x     = line;
      x_end = line + 1;
    }

    for (; x < x_end; x++) {
      if (!scrollOnly ||
          (scrollOnly && content[x].active)) {
        String   tmpString = parseStringKeepCase(strings[x], 1);
        String   newString = AdaGFXparseTemplate(tmpString, _textcols, gfxHelper);
        uint16_t h;
        content[x].length = gfxHelper->getTextSize(newString, h);

        # if ADAGFX_PARSE_SUBCOMMAND
        updateFontMetrics();
        # endif // if ADAGFX_PARSE_SUBCOMMAND

        if (yPos < _ypix) {
          gfxHelper->printText(newString.c_str(),
                               content[x].pixelPos,
                               yPos,
                               _fontscaling,
                               _fgcolor,
                               _bgcolor);

          if (scrollOnly && content[x].active && (content[x].length > _xpix))  {
            if (content[x].rightScroll && (content[x].pixelPos > 0)) {
              // Clear left from text
              matrix->fillRect(content[x].pixelPos - content[x].stepWidth,
                               yPos,
                               content[x].stepWidth,
                               h,
                               _bgcolor);
            }

            if (!content[x].rightScroll && (content[x].pixelPos + content[x].length < _xpix) && (content[x].stepWidth > 1)) {
              // Clear right from text
              matrix->fillRect(content[x].pixelPos + content[x].length + 1,
                               yPos,
                               content[x].stepWidth - 1,
                               h,
                               _bgcolor);
            }
          }
        }

        if (scrollOnly && content[x].active) {
          if (content[x].rightScroll) {
            // Fully scrolled? then reset, starting left of the screen or with right side aligned right if not startBlank
            if (content[x].pixelPos > (content[x].startBlank ? _xpix : 0 - content[x].stepWidth)) {
              if (content[x].startBlank) {
                content[x].pixelPos = -1 * content[x].length;
              } else {
                content[x].pixelPos = (-1 * content[x].length) + _xpix - content[x].stepWidth;
              }
            }
          } else {
            // Fully scrolled? then reset, starting at right of the screen or left if not startBlank
            if (content[x].pixelPos + content[x].length < (content[x].startBlank ? 0 : _xpix + content[x].stepWidth)) {
              if (content[x].startBlank) {
                content[x].pixelPos = _xpix;
              } else {
                content[x].pixelPos = content[x].stepWidth;
              }
            }
          }

          // Logging used only during development
          // String log = F("display_content: x=");
          // log += x;
          // log += F(", pxPos=");
          // log += content[x].pixelPos;
          // log += F(", len=");
          // log += content[x].length;
          // log += F(", stp=");
          // log += content[x].stepWidth;
          // log += F(", xpix=");
          // log += _xpix;
          // addLogMove(LOG_LEVEL_INFO, log);
        }
      }
      delay(0);
      yPos += (_fontheight * _fontscaling);
    }
    gfxHelper->setValidation(useVal);
    matrix->show();
  }
}

/****************************************************************************
 * plugin_write: Handle commands
 ***************************************************************************/
bool P131_data_struct::plugin_write(struct EventStruct *event, const String& string) {
  bool success = false;

  if (isInitialized()  && !_splashState) {
    const String cmd = parseString(string, 1);

    if ((nullptr != matrix) && cmd.equals(_commandTriggerCmd)) {
      String  sub = parseString(string, 2);
      int16_t x   = event->Par2 - 1;
      success = true;

      if (sub.equals(F("clear"))) {
        matrix->fillScreen(_bgcolor);
      } else if (sub.startsWith(F("bright")) && (event->Par2 >= 0) && (event->Par2 <= 255)) {
        if (parseString(string, 3).isEmpty()) {                     // No argument, then
          matrix->setBrightness(std::min(_maxbright, _brightness)); // use initial brightness
        } else {
          matrix->setBrightness(std::min(_maxbright, static_cast<uint8_t>(event->Par2)));
        }
      } else if (sub.equals(F("settext"))
                 && ((event->Par2 > 0) && (event->Par2 <= P131_CONFIG_TILE_HEIGHT))) { // line
        String tmpString = parseStringToEnd(strings[x], 2);                            // settings to be transferred
        strings[x]  = wrapWithQuotesIfContainsParameterSeparatorChar(parseStringToEndKeepCase(string, 4));
        strings[x] += ',';
        strings[x] += tmpString;
      } else if ((sub.equals(F("setscroll")) ||                                     // neomatrixcmd,setscroll,<line>,0|1
                  sub.equals(F("setempty")) ||                                      // neomatrixcmd,setempty,<line>,0|1
                  sub.equals(F("setright"))                                         // neomatrixcmd,setright,<line>,0|1
                  )
                 && ((event->Par2 > 0) && (event->Par2 <= P131_CONFIG_TILE_HEIGHT)) // line
                 && ((event->Par3 >= 0) && (event->Par3 <= 1))) {                   // on/off
        String   tmpString1 = parseStringKeepCase(strings[x], 1);                   // settings to be transferred
        String   tmpString3 = parseString(strings[x], 3);                           // settings to be transferred
        String   opts       = parseString(strings[x], 2);
        uint32_t optBits    = 0;

        validUIntFromString(opts, optBits);

        if (sub[3] == 's') {                                            // setscroll
          bitWrite(optBits, P131_OPTBITS_SCROLL, event->Par3 == 1);
        } else if (sub[3] == 'e') {                                     // setempty
          bitWrite(optBits, P131_OPTBITS_STARTBLANK, event->Par3 == 0); // Inverted value
        } else if (sub[3] == 'r') {                                     // setright
          bitWrite(optBits, P131_OPTBITS_RIGHTSCROLL, event->Par3 == 1);
        }
        strings[x]  = wrapWithQuotesIfContainsParameterSeparatorChar(tmpString1);
        strings[x] += ',';
        strings[x] += optBits;
        strings[x] += ',';
        strings[x] += tmpString3;
      } else if (sub.equals(F("setstep"))                                            // neomatrixcmd,setstep,<line>,1..16
                 && ((event->Par2 > 0) && (event->Par2 <= P131_CONFIG_TILE_HEIGHT))  // line
                 && ((event->Par3 > 0) && (event->Par3 <= P131_MAX_SCROLL_STEPS))) { // 1..16
        String   tmpString1 = parseStringKeepCase(strings[x], 1);                    // settings to be transferred
        String   tmpString3 = parseString(strings[x], 3);                            // settings to be transferred
        String   opts       = parseString(strings[x], 2);
        uint32_t optBits    = 0;

        validUIntFromString(opts, optBits);
        set4BitToUL(optBits, P131_OPTBITS_SCROLLSTEP, event->Par3 - 1); // Use offset of -1
        strings[x]  = wrapWithQuotesIfContainsParameterSeparatorChar(tmpString1);
        strings[x] += ',';
        strings[x] += optBits;
        strings[x] += ',';
        strings[x] += tmpString3;
      } else if (sub.equals(F("setspeed"))                                           // neomatrixcmd,setspeed,<line>,1..600
                 && ((event->Par2 > 0) && (event->Par2 <= P131_CONFIG_TILE_HEIGHT))  // line
                 && ((event->Par3 > 0) && (event->Par3 <= P131_MAX_SCROLL_SPEED))) { // 1..600
        String tmpString1 = parseStringKeepCase(strings[x], 1);                      // settings to be transferred
        String tmpString2 = parseString(strings[x], 2);                              // settings to be transferred
        strings[x]  = wrapWithQuotesIfContainsParameterSeparatorChar(tmpString1);
        strings[x] += ',';
        strings[x] += tmpString2;
        strings[x] += ',';
        strings[x] += event->Par3;
      } else {
        success = false;
      }

      if (success && sub.startsWith(F("set"))) {
        initialize_content(event, x);     // Set up line parameters
        content[x].loop = -1;             // Restart loop
        display_content(event, false, x); // (re-)display line
        # ifndef BUILD_NO_DEBUG

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log = F("NEOMATRIX: set line ");
          log += event->Par2;
          log += F(": ");
          log += strings[x];
          addLogMove(LOG_LEVEL_INFO, log);
        }
        # endif // ifndef BUILD_NO_DEBUG
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
  }
  return success;
}

/****************************************************************************
 * plugin_ten_per_second: Re-draw the default content that should be scrolled
 ***************************************************************************/
bool P131_data_struct::plugin_ten_per_second(struct EventStruct *event) {
  bool success = false;

  # ifdef P131_SHOW_SPLASH

  if (_splashState) { // Decrement splash counter
    _splashCounter--;
    _splashState = _splashCounter != 0;

    if (!_splashState) {
      #  ifndef BUILD_NO_DEBUG
      addLog(LOG_LEVEL_INFO, F("P131 Splash finished."));
      #  endif // ifndef BUILD_NO_DEBUG

      if (nullptr != matrix) {
        matrix->fillScreen(_bgcolor); // fill screen with black color
      }

      // Schedule the surrogate initial PLUGIN_READ that has been suppressed by the splash
      Scheduler.schedule_task_device_timer(event->TaskIndex, millis() + 10);
    }
  }
  # endif // ifdef P131_SHOW_SPLASH

  if (isInitialized() && !_splashState) {
    loadContent(event);
    success = true;

    for (uint8_t x = 0; x < P131_CONFIG_TILE_HEIGHT; x++) {
      if (content[x].active && (content[x].length > _xpix)) {
        if (content[x].loop == -1) { content[x].loop = content[x].speed; } // Initialize

        if (!content[x].loop--) {
          content[x].pixelPos += (content[x].rightScroll ? 1 : -1) * content[x].stepWidth;

          display_content(event, true);
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
  if (_fontscaling == 0) { _fontscaling = 1; } // Sanity checks

  if (_fontwidth == 0) { _fontwidth = 6; }

  if (_fontheight == 0) { _fontheight = 10; }

  if (nullptr != gfxHelper) {
    gfxHelper->getTextMetrics(_textcols, _textrows, _fontwidth, _fontheight, _fontscaling, _heightOffset, _xpix, _ypix);
    gfxHelper->getColors(_fgcolor, _bgcolor);
  } else {
    _textcols = _xpix / (_fontwidth * _fontscaling);
    _textrows = _ypix / (_fontheight * _fontscaling);
  }
  # ifdef P131_DEBUG_LOG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
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
    addLogMove(LOG_LEVEL_DEBUG, log);
  }
  # endif // ifdef P131_DEBUG_LOG
}

#endif // ifdef USES_P131
