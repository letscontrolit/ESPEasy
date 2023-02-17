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
  _commandTrigger.toLowerCase();
  _commandTriggerCmd = concat(_commandTrigger, F("cmd"));
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
  updateFontMetrics();
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
      gfxHelper->initialize();
      pcd8544->setContrast(_contrast);
      gfxHelper->invertDisplay(_displayInverted);
      gfxHelper->setRotation(_rotation);
      pcd8544->fillScreen(_bgcolor);             // fill screen with black color
      pcd8544->setTextColor(_fgcolor, _bgcolor); // set text color to white and black background

      gfxHelper->setColumnRowMode(bitRead(P141_CONFIG_FLAGS, P141_CONFIG_FLAG_USE_COL_ROW));
      gfxHelper->setLineSpacing(P141_CONFIG_FLAG_GET_LINESPACING);
      pcd8544->setTextSize(_fontscaling); // Handles 0 properly, text size, default 1 = very small
      pcd8544->setCursor(0, 0);           // move cursor to position (0, 0) pixel
      pcd8544->display();
      updateFontMetrics();


      if (P141_CONFIG_BUTTON_PIN != -1) {
        pinMode(P141_CONFIG_BUTTON_PIN, INPUT_PULLUP);
      }

      if (!stringsLoaded) {
        LoadCustomTaskSettings(event->TaskIndex, strings, P141_Nlines, 0);
        stringsLoaded = true;

        for (uint8_t x = 0; x < P141_Nlines && !stringsHasContent; x++) {
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
  # ifdef BUILD_NO_DEBUG
  addLog(LOG_LEVEL_INFO, F("PCD8544: Exit."));
  # endif // ifdef BUILD_NO_DEBUG

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
    if (stringsHasContent) {
      gfxHelper->setColumnRowMode(false); // Turn off column mode
      uint8_t  yPos  = 0;                 // Bound to the display
      int16_t  dum   = 0;
      uint16_t udum  = 0;
      uint16_t hText = 0;

      for (uint8_t x = 0; x < P141_Nlines; x++) {
        String newString = AdaGFXparseTemplate(strings[x], _textcols, gfxHelper);

        # if ADAGFX_PARSE_SUBCOMMAND
        updateFontMetrics();
        # endif // if ADAGFX_PARSE_SUBCOMMAND

        if ((yPos < _ypix) && !newString.isEmpty()) {
          gfxHelper->printText(newString.c_str(), 0, yPos, _fontscaling, _fgcolor, _bgcolor);
        }
        delay(0);

        if (15 == P141_CONFIG_FLAG_GET_LINESPACING) {
          yPos += (_fontheight * _fontscaling);                             // Auto, using font-height and scaling
        } else {
          pcd8544->getTextBounds(F("Ay"), 0, 0, &dum, &dum, &udum, &hText); // Measure current font-height
          yPos += hText + P141_CONFIG_FLAG_GET_LINESPACING;                 // Explicit distance
        }
      }
      pcd8544->display();
      gfxHelper->setColumnRowMode(bitRead(P141_CONFIG_FLAGS, P141_CONFIG_FLAG_USE_COL_ROW)); // Restore column mode
      # if P141_FEATURE_CURSOR_XY_VALUES
      updateValues(event);
      # endif // if P141_FEATURE_CURSOR_XY_VALUES
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

    if (equals(arg1, F("off"))) {               // Screen off
      displayOnOff(false);
    }
    else if (equals(arg1, F("on"))) {           // Screen on
      displayOnOff(true);
    }
    else if (equals(arg1, F("clear"))) {        // Empty screen
      pcd8544->fillScreen(_bgcolor);
      pcd8544->display();                      // Put on display
    }
    else if (equals(arg1, F("inv")) &&          // Invert display
             (event->Par2 >= 0) && (event->Par2 <= 1)) {
      if (parseString(string, 3).isEmpty()) {  // No argument: flip previous state
        _displayInverted = !_displayInverted;
      } else {
        _displayInverted = (event->Par2 == 1); // Set state
      }

      // Store in settings, but do not save
      bitWrite(P141_CONFIG_FLAGS, P141_CONFIG_FLAG_INVERTED, _displayInverted);

      if (nullptr != gfxHelper) {
        gfxHelper->invertDisplay(_displayInverted);
      } else {
        pcd8544->invertDisplay(_displayInverted);
      }
      pcd8544->display();                            // Put on display
    }
    else if (equals(arg1, F("backlight"))) {          // Backlight percentage
      if ((P141_CONFIG_BACKLIGHT_PIN != -1) &&       // All is valid?
          (event->Par2 >= 0) &&
          (event->Par2 <= 100)) {
        P141_CONFIG_BACKLIGHT_PERCENT = event->Par2; // Set but don't store
        _backlightPercentage          = event->Par2; // Also set to current
        displayOnOff(true);
      } else {
        success = false;
      }
    }
    else if (equals(arg1, F("contrast")) && // Display contrast
             (event->Par2 >= 0) &&
             (event->Par2 <= 100)) {
      P141_CONFIG_CONTRAST = event->Par2;  // Set but don't store
      _contrast            = event->Par2;  // Also set to current
      pcd8544->setContrast(_contrast);
    } else {
      success = false;
    }
  }
  else if (pcd8544 && (cmd.equals(_commandTrigger) ||
                       (gfxHelper && gfxHelper->isAdaGFXTrigger(cmd)))) {
    if (!bitRead(P141_CONFIG_FLAGS, P141_CONFIG_FLAG_NO_WAKE)) { // Wake display?
      displayOnOff(true);
    }

    if (nullptr != gfxHelper) {
      success = gfxHelper->processCommand(string);

      updateFontMetrics();  // Font or color may have changed

      if (success) {
        pcd8544->display(); // Put on display
        # if P141_FEATURE_CURSOR_XY_VALUES
        updateValues(event);
        # endif // if P141_FEATURE_CURSOR_XY_VALUES
      }
    }
  }
  return success;
}

# if P141_FEATURE_CURSOR_XY_VALUES

/****************************************************************************
 * updateValues: put x and y coordinates in Values 0 and 1
 ***************************************************************************/
void P141_data_struct::updateValues(struct EventStruct *event) {
  int16_t curX = 0;
  int16_t curY = 0;

  if (nullptr != gfxHelper) {
    gfxHelper->getCursorXY(curX, curY); // Get current X and Y coordinates, and put into Values
  }
  UserVar[event->BaseVarIndex]     = curX;
  UserVar[event->BaseVarIndex + 1] = curY;
}

# endif // if P141_FEATURE_CURSOR_XY_VALUES

# if ADAGFX_ENABLE_GET_CONFIG_VALUE

/****************************************************************************
 * plugin_get_config_value: Retrieve values like [<taskname>#<valuename>]
 ***************************************************************************/
bool P141_data_struct::plugin_get_config_value(struct EventStruct *event,
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
void P141_data_struct::displayOnOff(bool state) {
  if (validGpio(_backlightPin)) {
    # if defined(ESP8266)
    analogWrite(_backlightPin, state ? ((1024 / 100) * _backlightPercentage) : 0);
    # endif // if defined(ESP8266)
    # if defined(ESP32)
    analogWriteESP32(_backlightPin, state ? ((1024 / 100) * _backlightPercentage) : 0, 0);
    # endif // if defined(ESP32)
  }

  if (!state && pcd8544) { pcd8544->fillScreen(ADAGFX_BLACK); } // Can't turn off, clearing the display is the least bad alternative
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
    markButtonStateProcessed();
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
