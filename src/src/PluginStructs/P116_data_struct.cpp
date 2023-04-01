#include "../PluginStructs/P116_data_struct.h"

#ifdef USES_P116

# include "../Helpers/Hardware.h"

/****************************************************************************
 * ST77xx_type_toString: Display-value for the device selected
 ***************************************************************************/
const __FlashStringHelper* ST77xx_type_toString(const ST77xx_type_e& device) {
  switch (device) {
    case ST77xx_type_e::ST7735s_128x128: return F("ST7735 128 x 128px");
    case ST77xx_type_e::ST7735s_128x160: return F("ST7735 128 x 160px");
    case ST77xx_type_e::ST7735s_80x160: return F("ST7735 80 x 160px");
    case ST77xx_type_e::ST7735s_80x160_M5: return F("ST7735 80 x 160px (Color inverted)");
    case ST77xx_type_e::ST7789vw_240x320: return F("ST7789 240 x 320px");
    case ST77xx_type_e::ST7789vw_240x240: return F("ST7789 240 x 240px");
    case ST77xx_type_e::ST7789vw_240x280: return F("ST7789 240 x 280px");
    case ST77xx_type_e::ST7789vw_135x240: return F("ST7789 135 x 240px");
    case ST77xx_type_e::ST7796s_320x480: return F("ST7796 320 x 480px");
  }
  return F("Unsupported type!");
}

/****************************************************************************
 * ST77xx_type_toResolution: X and Y resolution for the selected type
 ***************************************************************************/
void ST77xx_type_toResolution(const ST77xx_type_e& device,
                              uint16_t           & x,
                              uint16_t           & y) {
  switch (device) {
    case ST77xx_type_e::ST7735s_128x128:
      x = 128;
      y = 128;
      break;
    case ST77xx_type_e::ST7735s_128x160:
      x = 128;
      y = 160;
      break;
    case ST77xx_type_e::ST7735s_80x160_M5:
    case ST77xx_type_e::ST7735s_80x160:
      x = 80;
      y = 160;
      break;
    case ST77xx_type_e::ST7789vw_240x320:
      x = 240;
      y = 320;
      break;
    case ST77xx_type_e::ST7789vw_240x240:
      x = 240;
      y = 240;
      break;
    case ST77xx_type_e::ST7789vw_240x280:
      x = 240;
      y = 280;
      break;
    case ST77xx_type_e::ST7789vw_135x240:
      x = 135;
      y = 240;
      break;
    case ST77xx_type_e::ST7796s_320x480:
      x = 320;
      y = 480;
      break;
  }
}

/****************************************************************************
 * P116_CommandTrigger_toString: return the command string selected
 ***************************************************************************/
const __FlashStringHelper* P116_CommandTrigger_toString(const P116_CommandTrigger& cmd) {
  switch (cmd) {
    case P116_CommandTrigger::tft: return F("tft");
    case P116_CommandTrigger::st7735: return F("st7735");
    case P116_CommandTrigger::st7789: return F("st7789");
    case P116_CommandTrigger::st7796: return F("st7796");
    case P116_CommandTrigger::st77xx: break;
  }
  return F("st77xx"); // Default command trigger
}

/****************************************************************************
 * Constructor
 ***************************************************************************/
P116_data_struct::P116_data_struct(ST77xx_type_e       device,
                                   uint8_t             rotation,
                                   uint8_t             fontscaling,
                                   AdaGFXTextPrintMode textmode,
                                   int8_t              backlightPin,
                                   uint8_t             backlightPercentage,
                                   uint32_t            displayTimer,
                                   String              commandTrigger,
                                   uint16_t            fgcolor,
                                   uint16_t            bgcolor,
                                   bool                textBackFill)
  : _device(device), _rotation(rotation), _fontscaling(fontscaling), _textmode(textmode), _backlightPin(backlightPin),
  _backlightPercentage(backlightPercentage), _displayTimer(displayTimer), _displayTimeout(displayTimer),
  _commandTrigger(commandTrigger), _fgcolor(fgcolor), _bgcolor(bgcolor), _textBackFill(textBackFill)
{
  _commandTrigger.toLowerCase();
  _commandTriggerCmd  = _commandTrigger;
  _commandTriggerCmd += F("cmd");
}

/****************************************************************************
 * Destructor
 ***************************************************************************/
P116_data_struct::~P116_data_struct() {
  cleanup();
}

/****************************************************************************
 * plugin_init: Initialize display
 ***************************************************************************/
bool P116_data_struct::plugin_init(struct EventStruct *event) {
  ST77xx_type_toResolution(_device, _xpix, _ypix);

  updateFontMetrics();

  bool success = false;

  ButtonState     = false; // button not touched
  ButtonLastState = 0xFF;  // Last state checked (debouncing in progress)
  DebounceCounter = 0;     // debounce counter

  if (nullptr == st77xx) {
    addLog(LOG_LEVEL_INFO, F("ST77xx: Init start."));
    uint8_t initRoptions = 0xFF;

    switch (_device) {
      case ST77xx_type_e::ST7735s_128x128:

        initRoptions = INITR_144GREENTAB; // 128x128px

      // fall through
      case ST77xx_type_e::ST7735s_128x160:

        if (initRoptions == 0xFF) {
          initRoptions = INITR_BLACKTAB; // 128x160px
        }

      // fall through
      case ST77xx_type_e::ST7735s_80x160_M5:

        if (initRoptions == 0xFF) {
          initRoptions = INITR_GREENTAB160x80; // 80x160px ST7735sv, inverted (M5Stack StickC)
        }

      // fall through
      case ST77xx_type_e::ST7735s_80x160:
      {
        if (initRoptions == 0xFF) {
          initRoptions = INITR_MINI160x80; // 80x160px
        }

        st7735 = new (std::nothrow) Adafruit_ST7735(PIN(0), PIN(1), PIN(2));

        if (nullptr != st7735) {
          st7735->initR(initRoptions); // initialize a ST7735s chip
          st77xx = st7735;             // pass pointer after initialization
        }
        break;
      }
      case ST77xx_type_e::ST7789vw_240x320: // Fall through
      case ST77xx_type_e::ST7789vw_240x240:
      case ST77xx_type_e::ST7789vw_240x280:
      case ST77xx_type_e::ST7789vw_135x240:
      {
        st7789 = new (std::nothrow) Adafruit_ST7789(PIN(0), PIN(1), PIN(2));

        if (nullptr != st7789) {
          st7789->init(_xpix, _ypix, SPI_MODE2);
          st77xx = st7789;
        }
        break;
      }
      case ST77xx_type_e::ST7796s_320x480:
      {
        st7796 = new (std::nothrow) Adafruit_ST7796S_kbv(PIN(0), PIN(1), PIN(2));

        if (nullptr != st7796) {
          st7796->begin();
          st77xx = st7796;
        }
        break;
      }
    }

    # ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log;
      log.reserve(90);
      log += F("ST77xx: Init done, address: 0x");
      log += String(reinterpret_cast<ulong>(st77xx), HEX);
      log += ' ';

      if (nullptr == st77xx) {
        log += F("in");
      }
      log += F("valid, commands: ");
      log += _commandTrigger;
      log += F(", display: ");
      log += ST77xx_type_toString(_device);
      addLogMove(LOG_LEVEL_INFO, log);
    }
    # endif // ifndef BUILD_NO_DEBUG
  } else {
    addLog(LOG_LEVEL_INFO, F("ST77xx: No init?"));
  }

  if (nullptr != st77xx) {
    gfxHelper = new (std::nothrow) AdafruitGFX_helper(st77xx,
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

    if (nullptr != gfxHelper) {
      displayOnOff(true);

      gfxHelper->initialize();
      gfxHelper->setRotation(_rotation);
      st77xx->fillScreen(_bgcolor);             // fill screen with black color
      st77xx->setTextColor(_fgcolor, _bgcolor); // set text color to white and black background

      # ifdef P116_SHOW_SPLASH
      uint16_t yPos = 0;
      gfxHelper->printText(String(F("ESPEasy")).c_str(), 0, yPos, 3, ST77XX_WHITE, ST77XX_BLUE);
      yPos += (3 * _fontheight);
      gfxHelper->printText(String(F("ST77xx")).c_str(),  0, yPos, 2, ST77XX_BLUE,  ST77XX_WHITE);
      delay(100); // Splash
      # endif // ifdef P116_SHOW_SPLASH

      gfxHelper->setColumnRowMode(bitRead(P116_CONFIG_FLAGS, P116_CONFIG_FLAG_USE_COL_ROW));
      st77xx->setTextSize(_fontscaling); // Handles 0 properly, text size, default 1 = very small
      st77xx->setCursor(0, 0);           // move cursor to position (0, 0) pixel
      updateFontMetrics();


      if (P116_CONFIG_BUTTON_PIN != -1) {
        pinMode(P116_CONFIG_BUTTON_PIN, INPUT_PULLUP);
      }

      if (!stringsLoaded) {
        LoadCustomTaskSettings(event->TaskIndex, strings, P116_Nlines, 0);
        stringsLoaded = true;

        for (uint8_t x = 0; x < P116_Nlines && !stringsHasContent; x++) {
          stringsHasContent = !strings[x].isEmpty();
        }
      }
      success = true;
    }
  }
  return success;
}

/****************************************************************************
 * updateFontMetrics: recalculate x and y columns, based on font size and font scale
 ***************************************************************************/
void P116_data_struct::updateFontMetrics() {
  if (nullptr != gfxHelper) {
    gfxHelper->getTextMetrics(_textcols, _textrows, _fontwidth, _fontheight, _fontscaling, _heightOffset, _xpix, _ypix);
    gfxHelper->getColors(_fgcolor, _bgcolor);
  } else {
    _textcols = _xpix / (_fontwidth * _fontscaling);
    _textrows = _ypix / (_fontheight * _fontscaling);
  }
}

/****************************************************************************
 * plugin_exit: De-initialize before destruction
 ***************************************************************************/
bool P116_data_struct::plugin_exit(struct EventStruct *event) {
  # ifndef BUILD_NO_DEBUG
  addLog(LOG_LEVEL_INFO, F("ST77xx: Exit."));
  # endif // ifndef BUILD_NO_DEBUG

  if ((nullptr != st77xx) && bitRead(P116_CONFIG_FLAGS, P116_CONFIG_FLAG_CLEAR_ON_EXIT)) {
    st77xx->fillScreen(ADAGFX_BLACK); // fill screen with black color
    displayOnOff(false);
  }
  cleanup();
  return true;
}

/****************************************************************************
 * cleanup: De-initialize pointers
 ***************************************************************************/
void P116_data_struct::cleanup() {
  delete gfxHelper;
  gfxHelper = nullptr;

  delete st7735;
  st7735 = nullptr;

  delete st7789;
  st7789 = nullptr;
  st77xx = nullptr; // Only used as a proxy
}

/****************************************************************************
 * plugin_read: Re-draw the default content
 ***************************************************************************/
bool P116_data_struct::plugin_read(struct EventStruct *event) {
  if (nullptr != st77xx) {
    if (stringsHasContent) {
      gfxHelper->setColumnRowMode(false); // Turn off column mode

      int yPos = 0;

      for (uint8_t x = 0; x < P116_Nlines; x++) {
        String newString = AdaGFXparseTemplate(strings[x], _textcols, gfxHelper);

        # if ADAGFX_PARSE_SUBCOMMAND
        updateFontMetrics();
        # endif // if ADAGFX_PARSE_SUBCOMMAND

        if (yPos < _ypix) {
          gfxHelper->printText(newString.c_str(), 0, yPos, _fontscaling, _fgcolor, _bgcolor);
        }
        delay(0);
        yPos += (_fontheight * _fontscaling);
      }
      gfxHelper->setColumnRowMode(bitRead(P116_CONFIG_FLAGS, P116_CONFIG_FLAG_USE_COL_ROW)); // Restore column mode
      int16_t curX, curY;
      gfxHelper->getCursorXY(curX, curY);                                                    // Get current X and Y coordinates,
      UserVar[event->BaseVarIndex]     = curX;                                               // and put into Values
      UserVar[event->BaseVarIndex + 1] = curY;
    }
  }
  return false; // Always return false, so no attempt to send to
                // Controllers or generate events is started
}

/****************************************************************************
 * plugin_ten_per_second: check button, if any, that wakes up the display
 ***************************************************************************/
bool P116_data_struct::plugin_ten_per_second(struct EventStruct *event) {
  if ((P116_CONFIG_BUTTON_PIN != -1) && (getButtonState()) && (nullptr != st77xx)) {
    displayOnOff(true);
    markButtonStateProcessed();
  }
  return true;
}

/****************************************************************************
 * plugin_once_a_second: Count down display timer, if any, and turn display off if countdown reached
 ***************************************************************************/
bool P116_data_struct::plugin_once_a_second(struct EventStruct *event) {
  if (_displayTimer > 0) {
    _displayTimer--;

    if ((nullptr != st77xx) && (_displayTimer == 0)) {
      displayOnOff(false);
    }
  }
  return true;
}

/****************************************************************************
 * plugin_write: Handle commands
 ***************************************************************************/
bool P116_data_struct::plugin_write(struct EventStruct *event,
                                    const String      & string) {
  bool   success = false;
  String cmd     = parseString(string, 1);

  if ((nullptr != st77xx) && cmd.equals(_commandTriggerCmd)) {
    String arg1 = parseString(string, 2);
    success = true;

    if (equals(arg1, F("off"))) {
      displayOnOff(false);
    }
    else if (equals(arg1, F("on"))) {
      displayOnOff(true);
    }
    else if (equals(arg1, F("clear"))) {
      st77xx->fillScreen(_bgcolor);
    }
    else if (equals(arg1, F("backlight"))) {
      String arg2 = parseString(string, 3);
      int    nArg2;

      if ((P116_CONFIG_BACKLIGHT_PIN != -1) && // All is valid?
          validIntFromString(arg2, nArg2) &&
          (nArg2 > 0) &&
          (nArg2 <= 100)) {
        P116_CONFIG_BACKLIGHT_PERCENT = nArg2; // Set but don't store
        displayOnOff(true);
      } else {
        success = false;
      }
    } else {
      success = false;
    }
  }
  else if (st77xx && (cmd.equals(_commandTrigger) ||
                      (gfxHelper && gfxHelper->isAdaGFXTrigger(cmd)))) {
    success = true;

    if (!bitRead(P116_CONFIG_FLAGS, P116_CONFIG_FLAG_NO_WAKE)) { // Wake display?
      displayOnOff(true);
    }

    if (nullptr != gfxHelper) {
      String tmp = string;

      // Hand it over after replacing variables
      success = gfxHelper->processCommand(AdaGFXparseTemplate(tmp, _textcols, gfxHelper));

      updateFontMetrics(); // Font or color may have changed

      if (success) {
        int16_t curX, curY;
        gfxHelper->getCursorXY(curX, curY); // Get current X and Y coordinates, and put into Values
        UserVar[event->BaseVarIndex]     = curX;
        UserVar[event->BaseVarIndex + 1] = curY;
      }
    }
  }
  return success;
}

# if ADAGFX_ENABLE_GET_CONFIG_VALUE

/****************************************************************************
 * plugin_get_config_value: Retrieve values like [<taskname>#<valuename>]
 ***************************************************************************/
bool P116_data_struct::plugin_get_config_value(struct EventStruct *event,
                                               String            & string) {
  bool success = false;

  if (gfxHelper != nullptr) {
    success = gfxHelper->pluginGetConfigValue(string);
  }
  return success;
}

# endif // if ADAGFX_ENABLE_GET_CONFIG_VALUE

/****************************************************************************
 * displayOnOff: Turn display on or off
 ***************************************************************************/
void P116_data_struct::displayOnOff(bool state) {
  if (_backlightPin != -1) {
    # if defined(ESP8266)
    analogWrite(_backlightPin, state ? ((1024 / 100) * _backlightPercentage) : 0);
    # endif // if defined(ESP8266)
    # if defined(ESP32)
    analogWriteESP32(_backlightPin, state ? ((1024 / 100) * _backlightPercentage) : 0, 0);
    # endif // if defined(ESP32)
  }
  st77xx->enableDisplay(state); // Display on
  _displayTimer = (state ? _displayTimeout : 0);
}

/****************************************************************************
 * registerButtonState: the button has been pressed, apply some debouncing
 ***************************************************************************/
void P116_data_struct::registerButtonState(uint8_t newButtonState,
                                           bool    bPin3Invers) {
  if ((ButtonLastState == 0xFF) || (bPin3Invers != (!!newButtonState))) {
    ButtonLastState = newButtonState;
    DebounceCounter++;
  } else {
    ButtonLastState = 0xFF; // Reset
    DebounceCounter = 0;
    ButtonState     = false;
  }

  if ((ButtonLastState == newButtonState) &&
      (DebounceCounter >= P116_DebounceTreshold)) {
    ButtonState = true;
  }
}

/****************************************************************************
 * markButtonStateProcessed: reset the button state
 ***************************************************************************/
void P116_data_struct::markButtonStateProcessed() {
  ButtonState     = false;
  DebounceCounter = 0;
}

#endif // ifdef USES_P116
