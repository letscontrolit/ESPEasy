#include "../PluginStructs/P095_data_struct.h"

#ifdef USES_P095
# include "../Helpers/Hardware.h"

/****************************************************************************
 * ILI9xxx_type_toString: Display-value for the device selected
 ***************************************************************************/
const __FlashStringHelper* ILI9xxx_type_toString(const ILI9xxx_type_e& device) {
  switch (device) {
    case ILI9xxx_type_e::ILI9341_240x320: return F("ILI9341 240 x 320px");
    case ILI9xxx_type_e::ILI9342_240x320: return F("ILI9342 240 x 320px (M5Stack)");
    case ILI9xxx_type_e::ILI9481_320x480: return F("ILI9481 320 x 480px");
    case ILI9xxx_type_e::ILI9481_CPT29_320x480: return F("ILI9481 320 x 480px (CPT29)");
    case ILI9xxx_type_e::ILI9481_PVI35_320x480: return F("ILI9481 320 x 480px (PVI35)");
    case ILI9xxx_type_e::ILI9481_AUO317_320x480: return F("ILI9481 320 x 480px (AUO317)");
    case ILI9xxx_type_e::ILI9481_CMO35_320x480: return F("ILI9481 320 x 480px (CMO35)");
    case ILI9xxx_type_e::ILI9481_RGB_320x480: return F("ILI9481 320 x 480px (RGB)");
    case ILI9xxx_type_e::ILI9481_CMI7_320x480: return F("ILI9481 320 x 480px (CMI7)");
    case ILI9xxx_type_e::ILI9481_CMI8_320x480: return F("ILI9481 320 x 480px (CMI8)");
    # ifdef P095_ENABLE_ILI948X
    case ILI9xxx_type_e::ILI9486_320x480: return F("ILI9486 320 x 480px");
    case ILI9xxx_type_e::ILI9488_320x480: return F("ILI9488 320 x 480px");
    # endif // ifdef P095_ENABLE_ILI948X
  }
  # ifndef BUILD_NO_DEBUG
  return F("Unsupported type!");
  # else // ifndef BUILD_NO_DEBUG
  return F("");
  # endif // ifndef BUILD_NO_DEBUG
}

/****************************************************************************
 * ILI9xxx_type_toResolution: X and Y resolution for the selected type
 ***************************************************************************/
void ILI9xxx_type_toResolution(const ILI9xxx_type_e& device,
                               uint16_t            & x,
                               uint16_t            & y) {
  switch (device) {
    case ILI9xxx_type_e::ILI9341_240x320:
    case ILI9xxx_type_e::ILI9342_240x320:
      x = 240;
      y = 320;
      break;
    case ILI9xxx_type_e::ILI9481_320x480:
    case ILI9xxx_type_e::ILI9481_CPT29_320x480:
    case ILI9xxx_type_e::ILI9481_PVI35_320x480:
    case ILI9xxx_type_e::ILI9481_AUO317_320x480:
    case ILI9xxx_type_e::ILI9481_CMO35_320x480:
    case ILI9xxx_type_e::ILI9481_RGB_320x480:
    case ILI9xxx_type_e::ILI9481_CMI7_320x480:
    case ILI9xxx_type_e::ILI9481_CMI8_320x480:
    # ifdef P095_ENABLE_ILI948X
    case ILI9xxx_type_e::ILI9486_320x480:
    case ILI9xxx_type_e::ILI9488_320x480:
    # endif // ifdef P095_ENABLE_ILI948X
      x = 320;
      y = 480;
      break;
  }
}

/****************************************************************************
 * P095_CommandTrigger_toString: return the command string selected
 ***************************************************************************/
const __FlashStringHelper* P095_CommandTrigger_toString(const P095_CommandTrigger& cmd) {
  switch (cmd) {
    case P095_CommandTrigger::tft: return F("tft");
    case P095_CommandTrigger::ili9341: break;
    case P095_CommandTrigger::ili9342: return F("ili9342");
    case P095_CommandTrigger::ili9481: return F("ili9481");
    # ifdef P095_ENABLE_ILI948X
    case P095_CommandTrigger::ili9486: return F("ili9486");
    case P095_CommandTrigger::ili9488: return F("ili9488");
    # endif // ifdef P095_ENABLE_ILI948X
  }
  return F("ili9341"); // Default command trigger
}

/****************************************************************************
 * Constructor
 ***************************************************************************/
P095_data_struct::P095_data_struct(ILI9xxx_type_e      displayType,
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
  : _displayType(displayType), _rotation(rotation), _fontscaling(fontscaling), _textmode(textmode),
  _backlightPin(backlightPin), _backlightPercentage(backlightPercentage), _displayTimer(displayTimer),
  _displayTimeout(displayTimer), _commandTrigger(commandTrigger), _fgcolor(fgcolor), _bgcolor(bgcolor),
  _textBackFill(textBackFill)
{
  _commandTrigger.toLowerCase();
  _commandTriggerCmd  = _commandTrigger;
  _commandTriggerCmd += F("cmd");
}

/****************************************************************************
 * Destructor
 ***************************************************************************/
P095_data_struct::~P095_data_struct() {
  delete gfxHelper;
  delete tft;
}

void P095_data_struct::init() {
  _xpix = 240;
  _ypix = 320;
  ILI9xxx_type_toResolution(_displayType, _xpix, _ypix);

  updateFontMetrics();
}

/****************************************************************************
 * plugin_init: Initialize display
 ***************************************************************************/
bool P095_data_struct::plugin_init(struct EventStruct *event) {
  init();
  bool success = false;

  if (nullptr == tft) {
    # ifndef BUILD_NO_DEBUG
    addLog(LOG_LEVEL_INFO, F("ILI9341: Init start."));
    # endif // ifndef BUILD_NO_DEBUG

    tft = new (std::nothrow) Adafruit_ILI9341(PIN(0), PIN(1), PIN(2), static_cast<uint8_t>(_displayType), _xpix, _ypix);

    if (nullptr != tft) {
      tft->begin();
    }

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
      log += F("valid, display: ");
      log += ILI9xxx_type_toString(static_cast<ILI9xxx_type_e>(P095_CONFIG_FLAG_GET_TYPE));
      log += F(", commands: ");
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
      gfxHelper->initialize();
      gfxHelper->setRotation(_rotation);
      gfxHelper->setColumnRowMode(bitRead(P095_CONFIG_FLAGS, P095_CONFIG_FLAG_USE_COL_ROW));
      gfxHelper->setTxtfullCompensation(!bitRead(P095_CONFIG_FLAGS, P095_CONFIG_FLAG_COMPAT_P095) ? 0 : 1);
      gfxHelper->invertDisplay(P095_CONFIG_FLAG_GET_INVERTDISPLAY);
    }
    updateFontMetrics();
    tft->fillScreen(_bgcolor);             // fill screen with background color
    tft->setTextColor(_fgcolor, _bgcolor); // set text color to white and configured background
    tft->setTextSize(_fontscaling);        // Handles 0 properly, text size, default 1 = very small
    tft->setCursor(0, 0);                  // move cursor to position (0, 0) pixel
    displayOnOff(true);
    # ifdef P095_SHOW_SPLASH

    if (P095_CONFIG_FLAG_GET_SHOW_SPLASH) {
      uint16_t yPos = 0;
      gfxHelper->printText(String(F("ESPEasy")).c_str(),         0, yPos, 3, ADAGFX_WHITE, ADAGFX_BLUE);
      yPos += (3 * _fontheight);
      gfxHelper->printText(String(F("ILI934x/ILI948x")).c_str(), 0, yPos, 2, ADAGFX_BLUE,  ADAGFX_WHITE);
      _splashState   = true; // Splash
      _splashCounter = P095_SPLASH_DURATION;
      #  ifndef BUILD_NO_DEBUG
      addLog(LOG_LEVEL_INFO, F("P095 Splash start"));
      #  endif // ifndef BUILD_NO_DEBUG
    }
    # endif // ifdef P095_SHOW_SPLASH
    updateFontMetrics();


    if (P095_CONFIG_BUTTON_PIN != -1) {
      pinMode(P095_CONFIG_BUTTON_PIN, INPUT_PULLUP);
    }

    if (!stringsLoaded) {
      LoadCustomTaskSettings(event->TaskIndex, strings, P095_Nlines, 0);
      stringsLoaded = true;

      for (uint8_t x = 0; x < P095_Nlines && !stringsHasContent; x++) {
        stringsHasContent = !strings[x].isEmpty();
      }
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
  # ifndef BUILD_NO_DEBUG
  addLog(LOG_LEVEL_INFO, F("ILI9341: Exit."));
  # endif // ifndef BUILD_NO_DEBUG

  if ((nullptr != tft) && bitRead(P095_CONFIG_FLAGS, P095_CONFIG_FLAG_CLEAR_ON_EXIT)) {
    tft->fillScreen(ADAGFX_BLACK); // fill screen with black color
    displayOnOff(false);
  }

  delete gfxHelper;
  gfxHelper = nullptr;

  delete tft;
  tft = nullptr;
  return true;
}

/****************************************************************************
 * plugin_read: Re-draw the default content
 ***************************************************************************/
bool P095_data_struct::plugin_read(struct EventStruct *event) {
  if ((nullptr != tft) && !_splashState) {
    if (stringsHasContent) {
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
  # ifdef P095_SHOW_SPLASH

  if (_splashState) { // Decrement splash counter
    _splashCounter--;
    _splashState = _splashCounter != 0;

    if (!_splashState) {
      #  ifndef BUILD_NO_DEBUG
      addLog(LOG_LEVEL_INFO, F("P095 Splash finished."));
      #  endif // ifndef BUILD_NO_DEBUG

      if (nullptr != tft) {
        tft->fillScreen(_bgcolor); // fill screen with background color
      }

      // Schedule the surrogate initial PLUGIN_READ that has been suppressed by the splash
      Scheduler.schedule_task_device_timer(event->TaskIndex, millis() + 10);
    }
  }
  # endif // ifdef P095_SHOW_SPLASH

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
  if ((_displayTimer > 0) && !_splashState) {
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

  if ((nullptr != tft) && cmd.equals(_commandTriggerCmd) && !_splashState) {
    String arg1 = parseString(string, 2);
    success = true;

    if (equals(arg1, F("off"))) {
      displayOnOff(false);
    }
    else if (equals(arg1, F("on"))) {
      displayOnOff(true);
    }
    else if (equals(arg1, F("clear")))
    {
      String arg2 = parseString(string, 3);

      if (!arg2.isEmpty()) {
        tft->fillScreen(AdaGFXparseColor(arg2));
      } else {
        tft->fillScreen(_bgcolor);
      }
    }
    else if (equals(arg1, F("backlight"))) {
      if ((P095_CONFIG_BACKLIGHT_PIN != -1) &&       // All is valid?
          (event->Par2 > 0) &&
          (event->Par2 <= 100)) {
        P095_CONFIG_BACKLIGHT_PERCENT = event->Par2; // Set but don't store
        displayOnOff(true);
      } else {
        success = false;
      }
    }
    else if (equals(arg1, F("inv"))) {
      if ((event->Par2 >= 0) &&
          (event->Par2 <= 1)) {
        tft->invertDisplay(event->Par2);
      } else {
        success = false;
      }
    }
    else if (equals(arg1, F("rot"))) {
      if ((event->Par2 >= 0)) {
        if (nullptr != gfxHelper) {
          gfxHelper->setRotation(event->Par2 % 4);
        } else {
          tft->setRotation(event->Par2 % 4);
        }
      } else {
        success = false;
      }
    } else {
      success = false;
    }
  }
  else if (tft && (cmd.equals(_commandTrigger) ||
                   (gfxHelper && gfxHelper->isAdaGFXTrigger(cmd))) && !_splashState) {
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

# if ADAGFX_ENABLE_GET_CONFIG_VALUE

/****************************************************************************
 * plugin_get_config_value: Retrieve values like [<taskname>#<valuename>]
 ***************************************************************************/
bool P095_data_struct::plugin_get_config_value(struct EventStruct *event,
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
void P095_data_struct::displayOnOff(bool state) {
  if (_backlightPin != -1) {
    # if defined(ESP8266)
    analogWrite(_backlightPin, state ? ((1024 / 100) * _backlightPercentage) : 0);
    # endif // if defined(ESP8266)
    # if defined(ESP32)
    analogWriteESP32(_backlightPin, state ? ((1024 / 100) * _backlightPercentage) : 0, 0);
    # endif // if defined(ESP32)
  }

  tft->sendCommand(state ? ILI9341_DISPON : ILI9341_DISPOFF);
  _displayTimer = (state ? _displayTimeout : 0);
}

/****************************************************************************
 * registerButtonState: the button has been pressed, apply some debouncing
 ***************************************************************************/
void P095_data_struct::registerButtonState(const uint8_t& newButtonState,
                                           const bool   & bPin3Invers) {
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
