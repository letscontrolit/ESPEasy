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
                                   uint8_t             displayTimer,
                                   String              commandTrigger,
                                   uint16_t            fgcolor,
                                   uint16_t            bgcolor,
                                   bool                textBackFill)
  : _rotation(rotation), _fontscaling(fontscaling), _textmode(textmode), _displayTimer(displayTimer),
  _commandTrigger(commandTrigger), _fgcolor(fgcolor), _bgcolor(bgcolor), _textBackFill(textBackFill)
{
  _xpix = 240;
  _ypix = 320;

  if (_rotation & 0x01) {
    std::swap(_xpix, _ypix);
  }
  updateFontMetrics();
  _commandTrigger.toLowerCase();
  _commandTriggerCmd  = _commandTrigger;
  _commandTriggerCmd += F("cmd");
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
      log.reserve(50);
      log  = F("ILI9341: Init done, address: 0x");
      log += String(reinterpret_cast<ulong>(tft), HEX);
      log += ' ';

      if (nullptr == tft) {
        log += F("in");
      }
      log += F("valid, commands: ");
      log += _commandTrigger;
      addLog(LOG_LEVEL_INFO, log);
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
      gfxHelper->setColumnRowMode(bitRead(P095_CONFIG_FLAGS, P095_CONFIG_FLAG_USE_COL_ROW));
      gfxHelper->setTxtfullCompensation(!bitRead(P095_CONFIG_FLAGS, P095_CONFIG_FLAG_COMPAT_P095) ? 0 : 1);
    }
    updateFontMetrics();
    tft->setRotation(_rotation);           // Set rotation 0/1/2/3
    tft->fillScreen(_bgcolor);             // fill screen with black color
    tft->setTextColor(_fgcolor, _bgcolor); // set text color to white and black background
    tft->setTextSize(_fontscaling);        // Handles 0 properly, text size, default 1 = very small
    tft->setCursor(0, 0);                  // move cursor to position (0, 0) pixel
    displayOnOff(true, P095_CONFIG_BACKLIGHT_PIN, P095_CONFIG_BACKLIGHT_PERCENT, P095_CONFIG_DISPLAY_TIMEOUT);
    gfxHelper->printText(String(F("ESPEasy")).c_str(), 1, 1);


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
    gfxHelper->getTextMetrics(_textcols, _textrows, _fontwidth, _fontheight, _fontscaling);
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
  if ((nullptr != tft) && bitRead(P095_CONFIG_FLAGS, P095_CONFIG_FLAG_CLEAR_ON_EXIT)) {
    tft->fillScreen(ADAGFX_BLACK); // fill screen with black color
    displayOnOff(false, P095_CONFIG_BACKLIGHT_PIN, P095_CONFIG_BACKLIGHT_PERCENT, P095_CONFIG_DISPLAY_TIMEOUT);
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
  return false;                                                                            // Always return false, so no attempt to send to
                                                                                           // Controllers or generate events is started
}

/****************************************************************************
 * plugin_ten_per_second: check button, if any, that wakes up the display
 ***************************************************************************/
bool P095_data_struct::plugin_ten_per_second(struct EventStruct *event) {
  if ((P095_CONFIG_BUTTON_PIN != -1) && (nullptr != tft)) {
    if (digitalRead(P095_CONFIG_BUTTON_PIN) == (bitRead(P095_CONFIG_FLAGS, P095_CONFIG_FLAG_INVERT_BUTTON) ? 1 : 0)) { // Invert state
      displayOnOff(true, P095_CONFIG_BACKLIGHT_PIN, P095_CONFIG_BACKLIGHT_PERCENT, P095_CONFIG_DISPLAY_TIMEOUT);
    }
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
      displayOnOff(false, P095_CONFIG_BACKLIGHT_PIN, P095_CONFIG_BACKLIGHT_PERCENT, P095_CONFIG_DISPLAY_TIMEOUT);
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
      displayOnOff(false, P095_CONFIG_BACKLIGHT_PIN, P095_CONFIG_BACKLIGHT_PERCENT, P095_CONFIG_DISPLAY_TIMEOUT);
    }
    else if (arg1.equals(F("on"))) {
      displayOnOff(true, P095_CONFIG_BACKLIGHT_PIN, P095_CONFIG_BACKLIGHT_PERCENT, P095_CONFIG_DISPLAY_TIMEOUT);
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
        displayOnOff(true, P095_CONFIG_BACKLIGHT_PIN, P095_CONFIG_BACKLIGHT_PERCENT, P095_CONFIG_DISPLAY_TIMEOUT);
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
  else if (tft && cmd.equals(_commandTrigger)) {
    success = true;

    if (!bitRead(P095_CONFIG_FLAGS, P095_CONFIG_FLAG_NO_WAKE)) { // Wake display?
      displayOnOff(true, P095_CONFIG_BACKLIGHT_PIN, P095_CONFIG_BACKLIGHT_PERCENT, P095_CONFIG_DISPLAY_TIMEOUT);
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
void P095_data_struct::displayOnOff(bool    state,
                                    int8_t  backlightPin,
                                    uint8_t backlightPercentage,
                                    uint8_t displayTimeout) {
  if (backlightPin != -1) {
    # if defined(ESP8266)
    analogWrite(backlightPin, state ? ((1024 / 100) * backlightPercentage) : 0);
    # endif // if defined(ESP8266)
    # if defined(ESP32)
    analogWriteESP32(backlightPin, state ? ((1024 / 100) * backlightPercentage) : 0, 0);
    # endif // if defined(ESP32)
  }

  if (state) {
    tft->sendCommand(ILI9341_DISPON);
  } else {
    tft->sendCommand(ILI9341_DISPOFF);
  }
  _displayTimer = (state ? displayTimeout : 0);
}

#endif // ifdef USES_P095
