#include "../PluginStructs/P141_data_struct.h"

#ifdef USES_P141

# include "../Helpers/Hardware.h"

/****************************************************************************
 * toString: return the command string selected
 ***************************************************************************/
const __FlashStringHelper* toString(P141_CommandTrigger cmd) {
  switch (cmd) {
    case P141_CommandTrigger::lcd: return F("lcd");
    case P141_CommandTrigger::pcd8544: break;
  }
  return F("pcd8544"); // Default command trigger
}

/****************************************************************************
 * Constructor
 ***************************************************************************/
P141_data_struct::P141_data_struct(uint8_t             rotation,
                                   uint8_t             fontscaling,
                                   AdaGFXTextPrintMode textmode,
                                   int8_t              backlightPin,
                                   uint8_t             backlightPercentage,
                                   uint8_t             contrast,
                                   uint32_t            displayTimer,
                                   String              commandTrigger,
                                   uint16_t            fgcolor,
                                   uint16_t            bgcolor,
                                   bool                textBackFill,
                                   bool                displayInverted)
  : _rotation(rotation), _fontscaling(fontscaling), _textmode(textmode), _backlightPin(backlightPin),
  _backlightPercentage(backlightPercentage), _contrast(contrast), _displayTimer(displayTimer),
  _displayTimeout(displayTimer), _commandTrigger(commandTrigger), _fgcolor(fgcolor), _bgcolor(bgcolor),
  _textBackFill(textBackFill), _displayInverted(displayInverted)
{
  updateFontMetrics();
  _commandTrigger.toLowerCase();
  _commandTriggerCmd  = _commandTrigger;
  _commandTriggerCmd += F("cmd");
}

/****************************************************************************
 * Destructor
 ***************************************************************************/
P141_data_struct::~P141_data_struct() {
  cleanup();
}

/****************************************************************************
 * plugin_init: Initialize display
 ***************************************************************************/
bool P141_data_struct::plugin_init(struct EventStruct *event) {
  bool success = false;

  ButtonState     = false; // button not touched
  ButtonLastState = 0xFF;  // Last state checked (debouncing in progress)
  DebounceCounter = 0;     // debounce counter

  if (nullptr == pcd8544) {
    addLog(LOG_LEVEL_INFO, F("PCD8544: Init start."));

    pcd8544 = new (std::nothrow) Adafruit_PCD8544(P141_DC_PIN, P141_CS_PIN, P141_RST_PIN);
    # ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log;
      log.reserve(90);
      log += F("PCD8544: Init done, address: 0x");
      log += String(reinterpret_cast<ulong>(pcd8544), HEX);
      log += ' ';

      if (nullptr == pcd8544) {
        log += F("in");
      }
      log += F("valid, commands: ");
      log += _commandTrigger;
      log += F(", display: PCD8544");
      addLogMove(LOG_LEVEL_INFO, log);
    }
    # endif // ifndef BUILD_NO_DEBUG
  } else {
    addLog(LOG_LEVEL_INFO, F("PCD8544: No init?"));
  }

  if (nullptr != pcd8544) {
    pcd8544->begin(); // Start the display
    gfxHelper = new (std::nothrow) AdafruitGFX_helper(pcd8544,
                                                      _commandTrigger,
                                                      _xpix,
                                                      _ypix,
                                                      AdaGFXColorDepth::Monochrome,
                                                      _textmode,
                                                      _fontscaling,
                                                      _fgcolor,
                                                      _bgcolor,
                                                      true,
                                                      _textBackFill);

    displayOnOff(true);

    if (nullptr != gfxHelper) {
      pcd8544->setContrast(_contrast);
      gfxHelper->invertDisplay(_displayInverted);
      gfxHelper->setRotation(_rotation);
      pcd8544->fillScreen(_bgcolor);             // fill screen with black color
      pcd8544->setTextColor(_fgcolor, _bgcolor); // set text color to white and black background

      # ifdef P141_SHOW_SPLASH
      uint16_t yPos = 0;
      gfxHelper->printText(String(F("ESPEasy")).c_str(), 0, yPos, 2, ST77XX_WHITE, ST77XX_BLUE);
      yPos += (2 * _fontheight);
      gfxHelper->printText(String(F("PCD8544")).c_str(), 0, yPos, 1, ST77XX_BLUE,  ST77XX_WHITE);
      delay(100); // Splash
      # endif // ifdef P141_SHOW_SPLASH

      gfxHelper->setColumnRowMode(bitRead(P141_CONFIG_FLAGS, P141_CONFIG_FLAG_USE_COL_ROW));
      pcd8544->setTextSize(_fontscaling); // Handles 0 properly, text size, default 1 = very small
      pcd8544->setCursor(0, 0);           // move cursor to position (0, 0) pixel
      pcd8544->display();
      updateFontMetrics();


      if (P141_CONFIG_BUTTON_PIN != -1) {
        pinMode(P141_CONFIG_BUTTON_PIN, INPUT_PULLUP);
      }
      success = true;
    }
  }
  return success;
}

/****************************************************************************
 * updateFontMetrics: recalculate x and y columns, based on font size and font scale
 ***************************************************************************/
void P141_data_struct::updateFontMetrics() {
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
bool P141_data_struct::plugin_exit(struct EventStruct *event) {
  addLog(LOG_LEVEL_INFO, F("PCD8544: Exit."));

  if ((nullptr != pcd8544) && bitRead(P141_CONFIG_FLAGS, P141_CONFIG_FLAG_CLEAR_ON_EXIT)) {
    pcd8544->fillScreen(_displayInverted
                        ? ADAGFX_WHITE
                        : ADAGFX_BLACK); // fill screen with black or white color
    pcd8544->display();
    displayOnOff(false);
  }
  cleanup();
  return true;
}

/****************************************************************************
 * cleanup: De-initialize pointers
 ***************************************************************************/
void P141_data_struct::cleanup() {
  if (nullptr != gfxHelper) { delete gfxHelper; }
  gfxHelper = nullptr;

  if (nullptr != pcd8544) { delete pcd8544; }
  pcd8544 = nullptr;
}

/****************************************************************************
 * plugin_read: Re-draw the default content
 ***************************************************************************/
bool P141_data_struct::plugin_read(struct EventStruct *event) {
  if (nullptr != pcd8544) {
    String strings[P141_Nlines];
    LoadCustomTaskSettings(event->TaskIndex, strings, P141_Nlines, 0);

    bool hasContent = false;

    for (uint8_t x = 0; x < P141_Nlines && !hasContent; x++) {
      hasContent = !strings[x].isEmpty();
    }

    if (hasContent) {
      gfxHelper->setColumnRowMode(false); // Turn off column mode

      int yPos = 1;                       // Bound to the display

      for (uint8_t x = 0; x < P141_Nlines; x++) {
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
      pcd8544->display();
      gfxHelper->setColumnRowMode(bitRead(P141_CONFIG_FLAGS, P141_CONFIG_FLAG_USE_COL_ROW)); // Restore column mode
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
bool P141_data_struct::plugin_ten_per_second(struct EventStruct *event) {
  if ((P141_CONFIG_BUTTON_PIN != -1) && (getButtonState()) && (nullptr != pcd8544)) {
    displayOnOff(true);
    markButtonStateProcessed();
  }
  return true;
}

/****************************************************************************
 * plugin_once_a_second: Count down display timer, if any, and turn display off if countdown reached
 ***************************************************************************/
bool P141_data_struct::plugin_once_a_second(struct EventStruct *event) {
  if (_displayTimer > 0) {
    _displayTimer--;

    if ((nullptr != pcd8544) && (_displayTimer == 0)) {
      displayOnOff(false);
    }
  }
  return true;
}

/****************************************************************************
 * plugin_write: Handle commands
 ***************************************************************************/
bool P141_data_struct::plugin_write(struct EventStruct *event,
                                    const String      & string) {
  bool   success = false;
  String cmd     = parseString(string, 1);

  if ((nullptr != pcd8544) && cmd.equals(_commandTriggerCmd)) {
    String arg1 = parseString(string, 2);
    success = true;

    if (arg1.equals(F("off"))) {               // Screen off
      displayOnOff(false);
    }
    else if (arg1.equals(F("on"))) {           // Screen on
      displayOnOff(true);
    }
    else if (arg1.equals(F("clear"))) {        // Empty screen
      pcd8544->fillScreen(_bgcolor);
      pcd8544->display();                      // Put on display
    }
    else if (arg1.equals(F("inv")) &&          // Invert display
             (event->Par2 >= 0) && (event->Par2 <= 1)) {
      if (parseString(string, 3).isEmpty()) {  // No argument: flip previous state
        _displayInverted != _displayInverted;
      } else {
        _displayInverted = (event->Par2 == 1); // Set state
      }

      if (nullptr != gfxHelper) {
        gfxHelper->invertDisplay(_displayInverted);
      } else {
        pcd8544->invertDisplay(_displayInverted);
      }
      pcd8544->display();                            // Put on display
    }
    else if (arg1.equals(F("backlight"))) {          // Backlight percentage
      if ((P141_CONFIG_BACKLIGHT_PIN != -1) &&       // All is valid?
          (event->Par2 > 0) &&
          (event->Par2 <= 100)) {
        P141_CONFIG_BACKLIGHT_PERCENT = event->Par2; // Set but don't store
        _backlightPercentage          = event->Par2; // Also set to current
        displayOnOff(true);
      } else {
        success = false;
      }
    } else {
      success = false;
    }
  }
  else if (pcd8544 && (cmd.equals(_commandTrigger) ||
                       (gfxHelper && gfxHelper->isAdaGFXTrigger(cmd)))) {
    success = true;

    if (!bitRead(P141_CONFIG_FLAGS, P141_CONFIG_FLAG_NO_WAKE)) { // Wake display?
      displayOnOff(true);
    }

    if (nullptr != gfxHelper) {
      String tmp = string;

      // Hand it over after replacing variables
      success = gfxHelper->processCommand(AdaGFXparseTemplate(tmp, _textcols, gfxHelper));

      updateFontMetrics();                  // Font or color may have changed

      if (success) {
        pcd8544->display();                 // Put on display
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
void P141_data_struct::displayOnOff(bool state) {
  if (_backlightPin != -1) {
    # if defined(ESP8266)
    analogWrite(_backlightPin, state ? ((1024 / 100) * _backlightPercentage) : 0);
    # endif // if defined(ESP8266)
    # if defined(ESP32)
    analogWriteESP32(_backlightPin, state ? ((1024 / 100) * _backlightPercentage) : 0, 0);
    # endif // if defined(ESP32)
  }

  // pcd8544->enableDisplay(state); // Display on // Not supported for PCD8544
  _displayTimer = (state ? _displayTimeout : 0);
}

/****************************************************************************
 * registerButtonState: the button has been pressed, apply some debouncing
 ***************************************************************************/
void P141_data_struct::registerButtonState(uint8_t newButtonState,
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
      (DebounceCounter >= P141_DebounceTreshold)) {
    ButtonState = true;
  }
}

/****************************************************************************
 * markButtonStateProcessed: reset the button state
 ***************************************************************************/
void P141_data_struct::markButtonStateProcessed() {
  ButtonState     = false;
  DebounceCounter = 0;
}

#endif // ifdef USES_P141
