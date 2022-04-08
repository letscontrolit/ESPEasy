#include "../PluginStructs/P095_data_struct.h"

#ifdef USES_P095

# include "../Helpers/Hardware.h"

/****************************************************************************
 * P095_CommandTrigger_toString: return the command string selected
 ***************************************************************************/
const __FlashStringHelper* P095_CommandTrigger_toString(P095_CommandTrigger cmd) {
  switch (cmd) {
    case P095_CommandTrigger::tft: return F("tft");
    case P095_CommandTrigger::ili9341: return F("ili9341");
    case P095_CommandTrigger::MAX: return F("None");
  }
  return F("ili9341"); // Default command trigger
}

/****************************************************************************
 * Constructor
 ***************************************************************************/
P095_data_struct::P095_data_struct(uint8_t             rotation,
                                   uint8_t             fontscaling,
                                   AdaGFXTextPrintMode textmode,
                                   int8_t              backlightPin,
                                   uint8_t             backlightPercentage,
                                   uint32_t            displayTimer,
                                   String              commandTrigger,
                                   uint16_t            fgcolor,
                                   uint16_t            bgcolor,
                                   bool                textBackFill)
  : _rotation(rotation), _fontscaling(fontscaling), _textmode(textmode), _backlightPin(backlightPin),
  _backlightPercentage(backlightPercentage), _displayTimer(displayTimer), _displayTimeout(displayTimer),
  _commandTrigger(commandTrigger), _fgcolor(fgcolor), _bgcolor(bgcolor), _textBackFill(textBackFill)
{
  _xpix = 240;
  _ypix = 320;

  updateFontMetrics();
  _commandTrigger.toLowerCase();
  _commandTriggerCmd  = _commandTrigger;
  _commandTriggerCmd += F("cmd");
}

/****************************************************************************
 * Destructor
 ***************************************************************************/
P095_data_struct::~P095_data_struct() {
  if (nullptr != gfxHelper) {
    delete gfxHelper;
    gfxHelper = nullptr;
  }

  if (nullptr != tft) {
    delete tft;
    tft = nullptr;
  }
}

/****************************************************************************
 * plugin_init: Initialize display
 ***************************************************************************/
bool P095_data_struct::plugin_init(struct EventStruct *event) {
  bool success = false;

  if (nullptr == tft) {
    addLog(LOG_LEVEL_INFO, F("ILI9341: Init start."));

    tft = new (std::nothrow) Adafruit_ILI9341(PIN(0), PIN(1), PIN(2));

    # ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log;
      log.reserve(65);
      log += F("ILI9341: Init done, address: 0x");
      log += String(reinterpret_cast<ulong>(tft), HEX);
      log += ' ';

      if (nullptr == tft) {
        log += F("in");
      }
      log += F("valid, commands: ");
      log += _commandTrigger;
      log += '/';
      log += _commandTriggerCmd;
      addLogMove(LOG_LEVEL_INFO, log);
    }
    # endif // ifndef BUILD_NO_DEBUG
  } else {
    addLog(LOG_LEVEL_INFO, F("ILI9341: No init?"));
  }

  if (nullptr != tft) {
    gfxHelper = new (std::nothrow) AdafruitGFX_helper(tft,
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
      gfxHelper->setRotation(_rotation);
      gfxHelper->setColumnRowMode(bitRead(P095_CONFIG_FLAGS, P095_CONFIG_FLAG_USE_COL_ROW));
      gfxHelper->setTxtfullCompensation(!bitRead(P095_CONFIG_FLAGS, P095_CONFIG_FLAG_COMPAT_P095) ? 0 : 1);
    }
    updateFontMetrics();
    tft->fillScreen(_bgcolor);             // fill screen with black color
    tft->setTextColor(_fgcolor, _bgcolor); // set text color to white and black background
    tft->setTextSize(_fontscaling);        // Handles 0 properly, text size, default 1 = very small
    tft->setCursor(0, 0);                  // move cursor to position (0, 0) pixel
    displayOnOff(true);
    # ifdef P095_SHOW_SPLASH
    uint16_t yPos = 0;
    gfxHelper->printText(String(F("ESPEasy")).c_str(), 0, yPos, 3, ST77XX_WHITE, ST77XX_BLUE);
    yPos += (3 * _fontheight);
    gfxHelper->printText(String(F("ILI9341")).c_str(), 0, yPos, 2, ST77XX_BLUE,  ST77XX_WHITE);
    delay(100); // Splash
    # endif // ifdef P095_SHOW_SPLASH
    updateFontMetrics();


    if (P095_CONFIG_BUTTON_PIN != -1) {
      pinMode(P095_CONFIG_BUTTON_PIN, INPUT_PULLUP);
    }
    success = true;
  }
  return success;
}

/****************************************************************************
 * updateFontMetrics: recalculate x and y columns, based on font size and font scale
 ***************************************************************************/
void P095_data_struct::updateFontMetrics() {
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
bool P095_data_struct::plugin_exit(struct EventStruct *event) {
  addLog(LOG_LEVEL_INFO, F("ILI9341: Exit."));

  if ((nullptr != tft) && bitRead(P095_CONFIG_FLAGS, P095_CONFIG_FLAG_CLEAR_ON_EXIT)) {
    tft->fillScreen(ADAGFX_BLACK); // fill screen with black color
    displayOnOff(false);
  }

  if (nullptr != gfxHelper) { delete gfxHelper; }
  gfxHelper = nullptr;

  if (nullptr != tft) {
    // delete tft; // Library is not properly inherited so no destructor called
    free(tft); // Free up some memory without calling the destructor chain
  }
  tft = nullptr;
  return true;
}

/****************************************************************************
 * plugin_read: Re-draw the default content
 ***************************************************************************/
bool P095_data_struct::plugin_read(struct EventStruct *event) {
  if (nullptr != tft) {
    String strings[P095_Nlines];
    LoadCustomTaskSettings(event->TaskIndex, strings, P095_Nlines, 0);

    bool hasContent = false;

    for (uint8_t x = 0; x < P095_Nlines && !hasContent; x++) {
      hasContent = !strings[x].isEmpty();
    }

    if (hasContent) {
      gfxHelper->setColumnRowMode(false); // Turn off column mode

      int yPos = 0;

      for (uint8_t x = 0; x < P095_Nlines; x++) {
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
      gfxHelper->setColumnRowMode(bitRead(P095_CONFIG_FLAGS, P095_CONFIG_FLAG_USE_COL_ROW)); // Restore column mode
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
bool P095_data_struct::plugin_ten_per_second(struct EventStruct *event) {
  if ((P095_CONFIG_BUTTON_PIN != -1) && (getButtonState()) && (nullptr != tft)) {
    displayOnOff(true);
    markButtonStateProcessed();
  }
  return true;
}

/****************************************************************************
 * plugin_once_a_second: Count down display timer, if any, and turn display off if countdown reached
 ***************************************************************************/
bool P095_data_struct::plugin_once_a_second(struct EventStruct *event) {
  if (_displayTimer > 0) {
    _displayTimer--;

    if ((nullptr != tft) && (_displayTimer == 0)) {
      displayOnOff(false);
    }
  }
  return true;
}

/****************************************************************************
 * plugin_write: Handle commands
 ***************************************************************************/
bool P095_data_struct::plugin_write(struct EventStruct *event, const String& string) {
  bool   success = false;
  String cmd     = parseString(string, 1);

  if ((nullptr != tft) && cmd.equals(_commandTriggerCmd)) {
    String arg1 = parseString(string, 2);
    success = true;

    if (arg1.equals(F("off"))) {
      displayOnOff(false);
    }
    else if (arg1.equals(F("on"))) {
      displayOnOff(true);
    }
    else if (arg1.equals(F("clear")))
    {
      String arg2 = parseString(string, 3);

      if (!arg2.isEmpty()) {
        tft->fillScreen(AdaGFXparseColor(arg2));
      } else {
        tft->fillScreen(_bgcolor);
      }
    }
    else if (arg1.equals(F("backlight"))) {
      String arg2 = parseString(string, 3);
      int    nArg2;

      if ((P095_CONFIG_BACKLIGHT_PIN != -1) && // All is valid?
          validIntFromString(arg2, nArg2) &&
          (nArg2 > 0) &&
          (nArg2 <= 100)) {
        P095_CONFIG_BACKLIGHT_PERCENT = nArg2; // Set but don't store
        displayOnOff(true);
      } else {
        success = false;
      }
    }
    else if (arg1.equals(F("inv")))
    {
      String arg2 = parseString(string, 3);
      int    nArg2;

      if (validIntFromString(arg2, nArg2) &&
          (nArg2 >= 0) &&
          (nArg2 <= 1)) {
        tft->invertDisplay(nArg2);
      } else {
        success = false;
      }
    }
    else if (arg1.equals(F("rot")))
    {
      ///control?cmd=tftcmd,rot,0
      // not working to verify
      String arg2 = parseString(string, 3);
      int    nArg2;

      if (validIntFromString(arg2, nArg2) &&
          (nArg2 >= 0)) {
        tft->setRotation(nArg2 % 4);
      } else {
        success = false;
      }
    } else {
      success = false;
    }
  }
  else if (tft && (cmd.equals(_commandTrigger) ||
                   (gfxHelper && gfxHelper->isAdaGFXTrigger(cmd)))) {
    success = true;

    if (!bitRead(P095_CONFIG_FLAGS, P095_CONFIG_FLAG_NO_WAKE)) { // Wake display?
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

/****************************************************************************
 * displayOnOff: Turn display on or off
 ***************************************************************************/
void P095_data_struct::displayOnOff(bool state) {
  if (_backlightPin != -1) {
    # if defined(ESP8266)
    analogWrite(_backlightPin, state ? ((1024 / 100) * _backlightPercentage) : 0);
    # endif // if defined(ESP8266)
    # if defined(ESP32)
    analogWriteESP32(_backlightPin, state ? ((1024 / 100) * _backlightPercentage) : 0, 0);
    # endif // if defined(ESP32)
  }

  if (state) {
    tft->sendCommand(ILI9341_DISPON);
  } else {
    tft->sendCommand(ILI9341_DISPOFF);
  }
  _displayTimer = (state ? _displayTimeout : 0);
}

/****************************************************************************
 * registerButtonState: the button has been pressed, apply some debouncing
 ***************************************************************************/
void P095_data_struct::registerButtonState(uint8_t newButtonState, bool bPin3Invers) {
  if ((ButtonLastState == 0xFF) || (bPin3Invers != (!!newButtonState))) {
    ButtonLastState = newButtonState;
    DebounceCounter++;
  } else {
    ButtonLastState = 0xFF; // Reset
    DebounceCounter = 0;
    ButtonState     = false;
  }

  if ((ButtonLastState == newButtonState) &&
      (DebounceCounter >= P095_DebounceTreshold)) {
    ButtonState = true;
  }
}

/****************************************************************************
 * markButtonStateProcessed: reset the button state
 ***************************************************************************/
void P095_data_struct::markButtonStateProcessed() {
  ButtonState     = false;
  DebounceCounter = 0;
}

#endif // ifdef USES_P095
