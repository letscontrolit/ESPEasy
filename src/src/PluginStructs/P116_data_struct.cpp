#include "../PluginStructs/P116_data_struct.h"

#ifdef USES_P116

# include "../Helpers/Hardware.h"

/****************************************************************************
 * ST77xx_type_toString: Display-value for the device selected
 ***************************************************************************/
const __FlashStringHelper* ST77xx_type_toString(ST77xx_type_e device) {
  switch (device) {
    case ST77xx_type_e::ST7735s_128x128: return F("ST7735 128 x 128px");
    case ST77xx_type_e::ST7735s_128x160: return F("ST7735 128 x 160px");
    case ST77xx_type_e::ST7735s_80x160: return F("ST7735 80 x 160px");
    case ST77xx_type_e::ST7789vw_240x320: return F("ST7789 240 x 320px");
    case ST77xx_type_e::ST7789vw_240x240: return F("ST7789 240 x 240px");
    case ST77xx_type_e::ST7789vw_240x280: return F("ST7789 240 x 280px");
    case ST77xx_type_e::ST7789vw_240x135: return F("ST7789 240 x 135px");
    case ST77xx_type_e::ST77xx_MAX: break;
  }
  return F("Unsupported type!");
}

/****************************************************************************
 * P116_CommandTrigger_toString: return the command string selected
 ***************************************************************************/
const __FlashStringHelper* P116_CommandTrigger_toString(P116_CommandTrigger cmd) {
  switch (cmd) {
    case P116_CommandTrigger::tft: return F("tft");
    case P116_CommandTrigger::st7735: return F("st7735");
    case P116_CommandTrigger::st7789: return F("st7789");
    case P116_CommandTrigger::MAX: return F("None");
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
                                   uint8_t             displayTimer,
                                   String              commandTrigger,
                                   uint16_t            fgcolor,
                                   uint16_t            bgcolor,
                                   bool                textBackFill)
  : _device(device), _rotation(rotation), _fontscaling(fontscaling), _textmode(textmode), _displayTimer(displayTimer),
  _commandTrigger(commandTrigger), _fgcolor(fgcolor), _bgcolor(bgcolor), _textBackFill(textBackFill)
{
  switch (_device) {
    case ST77xx_type_e::ST7735s_128x128:
      _xpix = 128;
      _ypix = 128;
      break;
    case ST77xx_type_e::ST7735s_128x160:
      _xpix = 128;
      _ypix = 160;
      break;
    case ST77xx_type_e::ST7735s_80x160:
      _xpix = 80;
      _ypix = 160;
      break;
    case ST77xx_type_e::ST7789vw_240x320:
      _xpix = 240;
      _ypix = 320;
      break;
    case ST77xx_type_e::ST7789vw_240x240:
      _xpix = 240;
      _ypix = 240;
      break;
    case ST77xx_type_e::ST7789vw_240x280:
      _xpix = 240;
      _ypix = 280;
      break;
    case ST77xx_type_e::ST7789vw_240x135:
      _xpix = 240;
      _ypix = 135;
      break;
    case ST77xx_type_e::ST77xx_MAX:
      break;
  }

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
bool P116_data_struct::plugin_init(struct EventStruct *event) {
  bool success = false;

  if (nullptr == st77xx) {
    addLog(LOG_LEVEL_INFO, F("ST77xx: Init start."));
    uint8_t initRoptions = 0xFF;
    int8_t  spi_MOSI_pin = -1;
    int8_t  spi_SCLK_pin = -1;
    # if defined(ESP8266) || defined(ESP8285)

    # endif // if defined(ESP8266) || defined(ESP8285)
    # if defined(ESP32)

    switch (Settings.InitSPI) {
      case 1: // VSPI
        spi_MOSI_pin = 23;
        spi_SCLK_pin = 18;
        break;
      case 2: // HSPI
        // spi_MOSI_pin = 13;
        // spi_SCLK_pin = 14;
        break;
    }
    # endif // if defined(ESP32)

    switch (_device) {
      case ST77xx_type_e::ST7735s_128x128:  // ST7735s 128x128

        if (initRoptions == 0xFF) {
          initRoptions = INITR_144GREENTAB; // 128x128px
        }

      // fall through
      case ST77xx_type_e::ST7735s_128x160: // ST7735s 128x160

        if (initRoptions == 0xFF) {
          initRoptions = INITR_BLACKTAB;   // 128x160px
        }

      // fall through
      case ST77xx_type_e::ST7735s_80x160:  // ST7735s 80x160
      {
        if (initRoptions == 0xFF) {
          initRoptions = INITR_MINI160x80; // 80x160px
        }

        if (spi_MOSI_pin == -1) {
          st7735 = new (std::nothrow) Adafruit_ST7735(PIN(0), PIN(1), PIN(2));
        } else {
          st7735 = new (std::nothrow) Adafruit_ST7735(PIN(0), PIN(1), spi_MOSI_pin, spi_SCLK_pin, PIN(2));
        }

        if (nullptr != st7735) {
          st7735->initR(initRoptions); // initialize a ST7735s chip
          st77xx = st7735;             // pass pointer after initialization
        }
        break;
      }
      case ST77xx_type_e::ST7789vw_240x320: // ST7789vw 240x320 fall through
      case ST77xx_type_e::ST7789vw_240x240: // ST7789vw 240x240
      case ST77xx_type_e::ST7789vw_240x280: // ST7789vw 240x280
      case ST77xx_type_e::ST7789vw_240x135: // ST7789vw 240x135
      {
        if (spi_MOSI_pin == -1) {
          st7789 = new (std::nothrow) Adafruit_ST7789(PIN(0), PIN(1), PIN(2));
        } else {
          st7789 = new (std::nothrow) Adafruit_ST7789(PIN(0), PIN(1), spi_MOSI_pin, spi_SCLK_pin, PIN(2));
        }

        if (nullptr != st7789) {
          st7789->init(_xpix, _ypix, SPI_MODE2);
          st77xx = st7789;
        }
        break;
      }
      case ST77xx_type_e::ST77xx_MAX:
        break;
    }

    # ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log;
      log.reserve(90);
      log  = F("ST77xx: Init done, address: 0x");
      log += String(reinterpret_cast<ulong>(st77xx), HEX);
      log += ' ';

      if (nullptr == st77xx) {
        log += F("in");
      }
      log += F("valid, device: ");
      log += ST77xx_type_toString(static_cast<ST77xx_type_e>(P116_CONFIG_FLAG_GET_TYPE));
      log += F(", commands: ");
      log += _commandTrigger;
      addLog(LOG_LEVEL_INFO, log);
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
      gfxHelper->setColumnRowMode(bitRead(P116_CONFIG_FLAGS, P116_CONFIG_FLAG_USE_COL_ROW));
    }
    updateFontMetrics();
    st77xx->enableDisplay(true);              // Display on
    st77xx->setRotation(_rotation);           // Set rotation 0/1/2/3
    st77xx->fillScreen(_bgcolor);             // fill screen with black color
    st77xx->setTextColor(_fgcolor, _bgcolor); // set text color to white and black background
    st77xx->setTextSize(_fontscaling);        // Handles 0 properly, text size, default 1 = very small
    st77xx->setCursor(0, 0);                  // move cursor to position (0, 0) pixel

    displayOnOff(true, P116_CONFIG_BACKLIGHT_PIN, P116_CONFIG_BACKLIGHT_PERCENT, P116_CONFIG_DISPLAY_TIMEOUT);

    if (P116_CONFIG_BUTTON_PIN != -1) {
      pinMode(P116_CONFIG_BUTTON_PIN, INPUT_PULLUP);
    }
    success = true;
  }
  return success;
}

/****************************************************************************
 * updateFontMetrics: recalculate x and y columns, based on font size and font scale
 ***************************************************************************/
void P116_data_struct::updateFontMetrics() {
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
bool P116_data_struct::plugin_exit(struct EventStruct *event) {
  if ((nullptr != st77xx) && bitRead(P116_CONFIG_FLAGS, P116_CONFIG_FLAG_CLEAR_ON_EXIT)) {
    st77xx->fillScreen(ADAGFX_BLACK); // fill screen with black color
    displayOnOff(false, P116_CONFIG_BACKLIGHT_PIN, P116_CONFIG_BACKLIGHT_PERCENT, P116_CONFIG_DISPLAY_TIMEOUT);
  }
  st7735 = nullptr;
  st7789 = nullptr;
  st77xx = nullptr;
  return true;
}

/****************************************************************************
 * plugin_read: Re-draw the default content
 ***************************************************************************/
bool P116_data_struct::plugin_read(struct EventStruct *event) {
  if (nullptr != st77xx) {
    String strings[P116_Nlines];
    LoadCustomTaskSettings(event->TaskIndex, strings, P116_Nlines, 0);

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
  return false;                                                                            // Always return false, so no attempt to send to
                                                                                           // Controllers or generate events is started
}

/****************************************************************************
 * plugin_ten_per_second: check button, if any, that wakes up the display
 ***************************************************************************/
bool P116_data_struct::plugin_ten_per_second(struct EventStruct *event) {
  if ((P116_CONFIG_BUTTON_PIN != -1) && (nullptr != st77xx)) {
    if (digitalRead(P116_CONFIG_BUTTON_PIN) == (bitRead(P116_CONFIG_FLAGS, P116_CONFIG_FLAG_INVERT_BUTTON) ? 1 : 0)) { // Invert state
      displayOnOff(true, P116_CONFIG_BACKLIGHT_PIN, P116_CONFIG_BACKLIGHT_PERCENT, P116_CONFIG_DISPLAY_TIMEOUT);
    }
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
      displayOnOff(false, P116_CONFIG_BACKLIGHT_PIN, P116_CONFIG_BACKLIGHT_PERCENT, P116_CONFIG_DISPLAY_TIMEOUT);
    }
  }
  return true;
}

/****************************************************************************
 * plugin_write: Handle commands
 ***************************************************************************/
bool P116_data_struct::plugin_write(struct EventStruct *event, const String& string) {
  bool   success = false;
  String cmd     = parseString(string, 1);

  if ((nullptr != st77xx) && cmd.equals(_commandTriggerCmd)) {
    String arg1 = parseString(string, 2);
    success = true;

    if (arg1.equals(F("off"))) {
      displayOnOff(false, P116_CONFIG_BACKLIGHT_PIN, P116_CONFIG_BACKLIGHT_PERCENT, P116_CONFIG_DISPLAY_TIMEOUT);
    }
    else if (arg1.equals(F("on"))) {
      displayOnOff(true, P116_CONFIG_BACKLIGHT_PIN, P116_CONFIG_BACKLIGHT_PERCENT, P116_CONFIG_DISPLAY_TIMEOUT);
    }
    else if (arg1.equals(F("clear"))) {
      st77xx->fillScreen(_bgcolor);
    }
    else if (arg1.equals(F("backlight"))) {
      String arg2 = parseString(string, 3);
      int    nArg2;

      if ((P116_CONFIG_BACKLIGHT_PIN != -1) && // All is valid?
          validIntFromString(arg2, nArg2) &&
          (nArg2 > 0) &&
          (nArg2 <= 100)) {
        P116_CONFIG_BACKLIGHT_PERCENT = nArg2; // Set but don't store
        displayOnOff(true, P116_CONFIG_BACKLIGHT_PIN, P116_CONFIG_BACKLIGHT_PERCENT, P116_CONFIG_DISPLAY_TIMEOUT);
      } else {
        success = false;
      }
    } else {
      success = false;
    }
  }
  else if (st77xx && cmd.equals(_commandTrigger)) {
    success = true;

    if (!bitRead(P116_CONFIG_FLAGS, P116_CONFIG_FLAG_NO_WAKE)) { // Wake display?
      displayOnOff(true, P116_CONFIG_BACKLIGHT_PIN, P116_CONFIG_BACKLIGHT_PERCENT, P116_CONFIG_DISPLAY_TIMEOUT);
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
void P116_data_struct::displayOnOff(bool    state,
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
  st77xx->enableDisplay(state);           // Display on
  _displayTimer = (state ? displayTimeout : 0);
}

#endif // ifdef USES_P116
