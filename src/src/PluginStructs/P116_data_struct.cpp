#include "../PluginStructs/P116_data_struct.h"

#ifdef USES_P116

# include "../Helpers/Hardware.h"

String P116_parseTemplate(String& tmpString,
                          uint8_t lineSize); // Forward declaration

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
                                   uint16_t            bgcolor)
  : _device(device), _rotation(rotation), _fontscaling(fontscaling), _textmode(textmode), _displayTimer(displayTimer),
  _commandTrigger(commandTrigger), _fgcolor(fgcolor), _bgcolor(bgcolor)
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

  if (st77xx == nullptr) {
    addLog(LOG_LEVEL_INFO, F("ST77xx: Init start."));
    uint8_t initRoptions = 0xFF;
    int8_t  spi_MOSI_pin = -1;
    int8_t  spi_SCLK_pin = -1;
    # if defined(ESP8266) || defined(ESP8285)

    // spi_MOSI_pin = D7;
    // spi_SCLK_pin = D5;
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

        if (st7735 != nullptr) {
          st7735->initR(initRoptions); // initialize a ST7735s chip
          st77xx = st7735;             // pass pointer after initialization
        }
        break;
      }
      case ST77xx_type_e::ST7789vw_240x320: // ST7789vw 240x320
      // fall through
      case ST77xx_type_e::ST7789vw_240x240: // ST7789vw 240x240
      // fall through
      case ST77xx_type_e::ST7789vw_240x280: // ST7789vw 240x280
      {
        if (spi_MOSI_pin == -1) {
          st7789 = new (std::nothrow) Adafruit_ST7789(PIN(0), PIN(1), PIN(2));
        } else {
          st7789 = new (std::nothrow) Adafruit_ST7789(PIN(0), PIN(1), spi_MOSI_pin, spi_SCLK_pin, PIN(2));
        }

        if (st7789 != nullptr) {
          if (_device == ST77xx_type_e::ST7789vw_240x240) {
            st7789->init(240, 240, SPI_MODE2);
          }
          else if (_device == ST77xx_type_e::ST7789vw_240x280) {
            st7789->init(240, 280, SPI_MODE2);
          }
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

      if (st77xx == nullptr) {
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

  if (st77xx != nullptr) {
    # ifdef P116_USE_ADA_GRAPHICS
    gfxHelper = new (std::nothrow) AdafruitGFX_helper(st77xx,
                                                      _commandTrigger,
                                                      _xpix,
                                                      _ypix,
                                                      AdafruitGFX_helper::ColorDepth::FullColor,
                                                      _textmode,
                                                      _fontscaling,
                                                      _fgcolor,
                                                      _bgcolor);

    if (gfxHelper != nullptr) {
      gfxHelper->setColumnRowMode(bitRead(P116_CONFIG_FLAGS, P116_CONFIG_FLAG_USE_COL_ROW));
    }
    # endif // ifdef P116_USE_ADA_GRAPHICS
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
  # ifdef P116_USE_ADA_GRAPHICS

  if (gfxHelper != nullptr) {
    gfxHelper->getTextMetrics(_textcols, _textrows, _fontwidth, _fontheight);
  } else
  # endif // ifdef P116_USE_ADA_GRAPHICS
  {
    _textcols = _xpix / (_fontwidth * _fontscaling);
    _textrows = _ypix / (_fontheight * _fontscaling);
  }
}

/****************************************************************************
 * plugin_exit: De-initialize before destruction
 ***************************************************************************/
bool P116_data_struct::plugin_exit(struct EventStruct *event) {
  if ((st77xx != nullptr) && bitRead(P116_CONFIG_FLAGS, P116_CONFIG_FLAG_CLEAR_ON_EXIT)) {
    st77xx->fillScreen(ST77XX_BLACK); // fill screen with black color
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
  if (st77xx != nullptr) {
    String strings[P116_Nlines];
    LoadCustomTaskSettings(event->TaskIndex, strings, P116_Nlines, 0);

    # ifdef P116_USE_ADA_GRAPHICS
    gfxHelper->setColumnRowMode(false); // Turn off column mode
    # endif // ifdef P116_USE_ADA_GRAPHICS

    for (uint8_t x = 0; x < _textrows; x++) {
      String newString = P116_parseTemplate(strings[x], _textcols);

      if (!newString.isEmpty()) {
        # ifdef P116_USE_ADA_GRAPHICS
        gfxHelper->printText(newString.c_str(), 0, x * _fontheight * _fontscaling, _fontscaling, _fgcolor, _bgcolor);
        # else // ifdef P116_USE_ADA_GRAPHICS

        if ((_textmode != AdaGFXTextPrintMode::ContinueToNextLine) && (newString.length() > _textcols)) {
          newString = newString.substring(0, _textcols - 1);
        }

        if (_textmode == AdaGFXTextPrintMode::ClearThenTruncate) { // Clear before print
          st77xx->setCursor(_leftMarginCompensation, (x * _fontheight * _fontscaling) + _topMarginCompensation);

          for (uint16_t c = 0; c <= newString.length(); c++) {
            st77xx->print(' ');
          }
        }
        st77xx->setCursor(_leftMarginCompensation, (x * _fontheight * _fontscaling) + _topMarginCompensation);
        st77xx->print(newString);

        for (uint16_t c = newString.length(); c < _textcols && _textmode != AdaGFXTextPrintMode::ContinueToNextLine; c++) {
          st77xx->print(' ');
        }
        delay(0);
        # endif // ifdef P116_USE_ADA_GRAPHICS
      }
    }
    # ifdef P116_USE_ADA_GRAPHICS
    gfxHelper->setColumnRowMode(bitRead(P116_CONFIG_FLAGS, P116_CONFIG_FLAG_USE_COL_ROW)); // Restore column mode
    int16_t curX, curY;
    gfxHelper->getCursorXY(curX, curY);                                                    // Get current X and Y coordinates,
    UserVar[event->BaseVarIndex]     = curX;                                               // and put into Values
    UserVar[event->BaseVarIndex + 1] = curY;
    # endif // ifdef P116_USE_ADA_GRAPHICS
  }
  return false; // Always return false, so no attempt to send to Controllers or generate events is started
}

/****************************************************************************
 * plugin_ten_per_second: check button, if any, that wakes up the display
 ***************************************************************************/
bool P116_data_struct::plugin_ten_per_second(struct EventStruct *event) {
  if ((P116_CONFIG_BUTTON_PIN != -1) && (st77xx != nullptr)) {
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

    if ((st77xx != nullptr) && (_displayTimer == 0)) {
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

  if ((st77xx != nullptr) && cmd.equals(_commandTriggerCmd)) {
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
    # ifdef P116_USE_ADA_GRAPHICS

    if (gfxHelper != nullptr) {
      String tmp = string;
      success = gfxHelper->processCommand(P116_parseTemplate(tmp, _textcols)); // Hand it over after replacing variables

      if (success) {
        updateFontMetrics();                                                   // Font may have changed
        int16_t curX, curY;
        gfxHelper->getCursorXY(curX, curY);                                    // Get current X and Y coordinates, and put into Values
        UserVar[event->BaseVarIndex]     = curX;
        UserVar[event->BaseVarIndex + 1] = curY;
      }
    }
    # else // ifdef P116_USE_ADA_GRAPHICS
    int colPos  = (event->Par2 - 1) + _leftMarginCompensation;
    int rowPos  = ((event->Par1 - 1) * 10 * _fontscaling) + _topMarginCompensation;
    String text = parseStringKeepCase(string, 4);
    text = P116_parseTemplate(text, _textcols);

    // clear line before writing new string
    if (_textmode == 2) {
      st77xx->setCursor(colPos, rowPos);

      for (uint8_t i = colPos; i < _textcols; i++) {
        st77xx->print(" ");
      }
    }

    // truncate message exceeding cols
    st77xx->setCursor(colPos, rowPos);

    if ((_textmode == 1) || (_textmode == 2)) {
      st77xx->setCursor(colPos, rowPos);

      for (uint8_t i = 0; i < _textcols - colPos; i++) {
        if (text[i]) {
          st77xx->print(text[i]);
        }
      }
    }

    // message exceeding cols will continue to next line
    // FIXME: tonhuisman: this is probably outdated or not applicable
    else {
      bool stillProcessing = 1;
      uint8_t charCount    = 1;

      while (stillProcessing) {
        if (++colPos > _textcols) {                           // have we printed 20 characters yet (+1 for the logic)
          rowPos += (10 * _fontscaling);
          st77xx->setCursor(_leftMarginCompensation, rowPos); // move cursor down
          colPos = 1;
        }

        // dont print if "lower" than the lcd
        if ((rowPos * _fontscaling) < _textrows * _fontscaling) {
          st77xx->print(text[charCount - 1]);
        }

        if (!text[charCount]) { // no more chars to process?
          stillProcessing = 0;
        }
        charCount += 1;
      }
    }
    # endif // ifdef P116_USE_ADA_GRAPHICS
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

/****************************************************************************
 * P116_parseTemplate: Replace variables and adjust unicode special characters to Adafruit font
 ***************************************************************************/

// Änderung WDS: Tabelle vorerst Abgeschaltet !!!!
// Perform some specific changes for LCD display
// https://www.letscontrolit.com/forum/viewtopic.php?t=2368
String P116_parseTemplate(String& tmpString, uint8_t lineSize) {
  String result            = parseTemplate_padded(tmpString, lineSize);
  const char degree[3]     = { 0xc2, 0xb0, 0 }; // Unicode degree symbol
  const char degree_tft[2] = { 0xdf, 0 };       // P116_LCD degree symbol

  result.replace(degree, degree_tft);

  char unicodePrefix = 0xc3;

  if (result.indexOf(unicodePrefix) != -1) {
    // See: https://github.com/letscontrolit/ESPEasy/issues/2081

    const char umlautAE_uni[3] = { 0xc3, 0x84, 0 };  // Unicode Umlaute AE
    const char umlautAE_tft[2] = { 0x8e, 0 };        // P116_LCD Umlaute
    result.replace(umlautAE_uni, umlautAE_tft);

    const char umlaut_ae_uni[3] = { 0xc3, 0xa4, 0 }; // Unicode Umlaute ae
    const char umlautae_tft[2]  = { 0x84, 0 };       // P116_LCD Umlaute
    result.replace(umlaut_ae_uni, umlautae_tft);

    const char umlautOE_uni[3] = { 0xc3, 0x96, 0 };  // Unicode Umlaute OE
    const char umlautOE_tft[2] = { 0x99, 0 };        // P116_LCD Umlaute
    result.replace(umlautOE_uni, umlautOE_tft);

    const char umlaut_oe_uni[3] = { 0xc3, 0xb6, 0 }; // Unicode Umlaute oe
    const char umlautoe_tft[2]  = { 0x98, 0 };       // P116_LCD Umlaute
    result.replace(umlaut_oe_uni, umlautoe_tft);

    const char umlautUE_uni[3] = { 0xc3, 0x9c, 0 };  // Unicode Umlaute UE
    const char umlautUE_tft[2] = { 0x9a, 0 };        // P116_LCD Umlaute
    result.replace(umlautUE_uni, umlautUE_tft);

    const char umlaut_ue_uni[3] = { 0xc3, 0xbc, 0 }; // Unicode Umlaute ue
    const char umlautue_tft[2]  = { 0x81, 0 };       // P116_LCD Umlaute
    result.replace(umlaut_ue_uni, umlautue_tft);

    //    const char umlaut_sz_uni[3] = {0xc3, 0x9f, 0}; // Unicode Umlaute sz
    //    const char umlaut_sz_tft[2] = {0xe2, 0}; // P116_LCD Umlaute
    //    result.replace(umlaut_sz_uni, umlaut_sz_tft);
    delay(0);
  }
  return result;
}

#endif // ifdef USES_P116
