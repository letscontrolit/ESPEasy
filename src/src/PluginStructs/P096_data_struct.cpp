#include "../PluginStructs/P096_data_struct.h"

#ifdef USES_P096

# include "../Helpers/Hardware.h"

/****************************************************************************
 * toString: Display-value for the EPD/eInk device selected
 ***************************************************************************/
const __FlashStringHelper* toString(EPD_type_e device) {
  switch (device) {
    case EPD_type_e::EPD_IL3897: return F("IL3897 (Lolin 250 x 122px)");
    case EPD_type_e::EPD_UC8151D: return F("UC8151D (white/black/red 212 x 104px)");
    case EPD_type_e::EPD_SSD1680: return F("SSD1680 (250 x 212px)");
    # if P096_USE_WAVESHARE_2IN7
    case EPD_type_e::EPD_WS2IN7: return F("Waveshare 2.7\" (264 x 176px)");
    # endif // if P096_USE_WAVESHARE_2IN7
    # if P096_USE_WAVESHARE_1IN54B
    case EPD_type_e::EPD_WS1IN54B: return F("Waveshare 1.54\" (200 x 200px)");
    # endif // if P096_USE_WAVESHARE_1IN54B
    # if P096_USE_MH_ET_LIVE_1IN54
    case EPD_type_e::EPD_MHET1IN54: return F("MH-ET Live 1.54\" (white/black/red 200 x 200px)");
    # endif // if P096_USE_MH_ET_LIVE_1IN54
  }
  return F("Unsupported type!");
}

/****************************************************************************
 * toString: return the command string selected
 ***************************************************************************/
const __FlashStringHelper* toString(P096_CommandTrigger cmd) {
  switch (cmd) {
    case P096_CommandTrigger::eInk: return F("eink");
    case P096_CommandTrigger::ePaper: return F("epaper");
    case P096_CommandTrigger::il3897: return F("il3897");
    case P096_CommandTrigger::uc8151d: return F("uc8151d");
    case P096_CommandTrigger::ssd1680: return F("ssd1680");
    # if P096_USE_WAVESHARE_2IN7
    case P096_CommandTrigger::ws2in7: return F("ws2in7");
    # endif // if P096_USE_WAVESHARE_2IN7
    # if P096_USE_WAVESHARE_1IN54B
    case P096_CommandTrigger::ws1in54b: return F("ws1in54");
    # endif // if P096_USE_WAVESHARE_1IN54B
    # if P096_USE_MH_ET_LIVE_1IN54
    case P096_CommandTrigger::mhet1in54: return F("mhet1in54");
    # endif // if P096_USE_MH_ET_LIVE_1IN54
    case P096_CommandTrigger::epd: break;
  }
  return F("epd"); // Default command trigger
}

/****************************************************************************
 * EPD_type_toResolution: X and Y resolution for the selected type
 ***************************************************************************/
void EPD_type_toResolution(EPD_type_e device, uint16_t& x, uint16_t& y) {
  switch (device) {
    case EPD_type_e::EPD_IL3897:
    case EPD_type_e::EPD_SSD1680:
      x = 250;
      y = 122;
      break;
    case EPD_type_e::EPD_UC8151D:
      x = 212;
      y = 104;
      break;
    # if P096_USE_WAVESHARE_2IN7
    case EPD_type_e::EPD_WS2IN7:
      x = 264;
      y = 176;
      break;
    # endif // if P096_USE_WAVESHARE_2IN7
    # if P096_USE_WAVESHARE_1IN54B
    case EPD_type_e::EPD_WS1IN54B:
      x = 200;
      y = 200;
      break;
    # endif // if P096_USE_WAVESHARE_1IN54B
    # if P096_USE_MH_ET_LIVE_1IN54
    case EPD_type_e::EPD_MHET1IN54:
      x = 200;
      y = 200;
      break;
    # endif // if P096_USE_MH_ET_LIVE_1IN54
      // case EPD_type_e::EPD_MAX:
      //   break;
  }
}

/****************************************************************************
 * Constructor
 ***************************************************************************/
P096_data_struct::P096_data_struct(EPD_type_e          display,
                                   # if !P096_USE_EXTENDED_SETTINGS
                                   uint16_t            width,
                                   uint16_t            height,
                                   # endif // if P096_USE_EXTENDED_SETTINGS
                                   uint8_t             rotation,
                                   uint8_t             fontscaling,
                                   AdaGFXTextPrintMode textmode,
                                   String              commandTrigger,
                                   uint16_t            fgcolor,
                                   uint16_t            bgcolor,
                                   AdaGFXColorDepth    colorDepth,
                                   bool                textBackFill)
  : _display(display),
  # if !P096_USE_EXTENDED_SETTINGS
  _xpix(width), _ypix(height),
  # endif // if !P096_USE_EXTENDED_SETTINGS
  _rotation(rotation), _fontscaling(fontscaling), _textmode(textmode), _commandTrigger(commandTrigger),
  _fgcolor(fgcolor), _bgcolor(bgcolor), _colorDepth(colorDepth), _textBackFill(textBackFill)
{
  _commandTrigger.toLowerCase();
  _commandTriggerCmd  = _commandTrigger;
  _commandTriggerCmd += F("cmd");
}

/****************************************************************************
 * Destructor
 ***************************************************************************/
P096_data_struct::~P096_data_struct() {
  if (nullptr != gfxHelper) {
    delete gfxHelper;
    gfxHelper = nullptr;
  }

  if (nullptr != eInkScreen) {
    delete eInkScreen;
    eInkScreen = nullptr;
  }
}

/****************************************************************************
 * plugin_init: Initialize display
 ***************************************************************************/
bool P096_data_struct::plugin_init(struct EventStruct *event) {
  # if P096_USE_EXTENDED_SETTINGS

  EPD_type_toResolution(_display, _xpix, _ypix);
  # endif // if P096_USE_EXTENDED_SETTINGS

  updateFontMetrics();

  bool success = false;

  addLog(LOG_LEVEL_INFO, F("EPD  : Init start."));

  switch (_display) {
    case EPD_type_e::EPD_IL3897:
      eInkScreen = new (std::nothrow) LOLIN_IL3897(_xpix, _ypix, PIN(1), PIN(2), PIN(0), PIN(3));  // HSPI
      break;
    case EPD_type_e::EPD_UC8151D:
      eInkScreen = new (std::nothrow) LOLIN_UC8151D(_xpix, _ypix, PIN(1), PIN(2), PIN(0), PIN(3)); // HSPI
      break;
    case EPD_type_e::EPD_SSD1680:
      eInkScreen = new (std::nothrow) LOLIN_SSD1680(_xpix, _ypix, PIN(1), PIN(2), PIN(0), PIN(3)); // HSPI
      break;
      # if P096_USE_WAVESHARE_2IN7
    case EPD_type_e::EPD_WS2IN7:
      eInkScreen = new (std::nothrow) Waveshare_2in7(_xpix, _ypix, PIN(1), PIN(2), PIN(0), PIN(3)); // HSPI
      break;
      # endif // if P096_USE_WAVESHARE_2IN7
      # if P096_USE_WAVESHARE_1IN54B
    case EPD_type_e::EPD_WS1IN54B:
      eInkScreen = new (std::nothrow) Waveshare_1in54b(_xpix, _ypix, PIN(1), PIN(2), PIN(0), PIN(3)); // HSPI
      break;
      # endif // if P096_USE_WAVESHARE_1IN54B
      # if P096_USE_MH_ET_LIVE_1IN54
    case EPD_type_e::EPD_MHET1IN54:
      eInkScreen = new (std::nothrow) MH_ET_Live_1in54(_xpix, _ypix, PIN(1), PIN(2), PIN(0), PIN(3)); // HSPI
      break;
      # endif // if P096_USE_MH_ET_LIVE_1IN54
      // case EPD_type_e::EPD_MAX:
      //   break;
  }
  _sequence_in_progress = false;
  # ifdef P096_USE_ADA_GRAPHICS

  if (nullptr != eInkScreen) {
    eInkScreen->begin(); // Start the device
    gfxHelper = new (std::nothrow) AdafruitGFX_helper(eInkScreen,
                                                      _commandTrigger,
                                                      _xpix,
                                                      _ypix,
                                                      _colorDepth,
                                                      _textmode,
                                                      _fontscaling,
                                                      _fgcolor,
                                                      _bgcolor,
                                                      true,
                                                      _textBackFill);
    #  if P096_USE_EXTENDED_SETTINGS

    if (nullptr != gfxHelper) {
      gfxHelper->initialize();
      gfxHelper->setRotation(_rotation);
      gfxHelper->setColumnRowMode(bitRead(P096_CONFIG_FLAGS, P096_CONFIG_FLAG_USE_COL_ROW));
      gfxHelper->setTxtfullCompensation(!bitRead(P096_CONFIG_FLAGS, P096_CONFIG_FLAG_COMPAT_P096) ? 0 : 1); // Inverted
    }
    #  endif // if P096_USE_EXTENDED_SETTINGS
  }
  updateFontMetrics();
  # endif // ifdef P096_USE_ADA_GRAPHICS

  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log;
    log.reserve(50);
    log += F("EPD  : Init done, address: ");
    log += formatToHex(reinterpret_cast<ulong>(eInkScreen));
    log += ' ';

    if (nullptr == eInkScreen) {
      log += F("in");
    }
    log += F("valid, commands: ");
    log += _commandTrigger;
    log += F(", display: ");
    log += toString(_display);
    addLog(LOG_LEVEL_INFO, log);
    log.clear();
    log += F("EPD  : Foreground: ");
    log += AdaGFXcolorToString(_fgcolor, static_cast<AdaGFXColorDepth>(P096_CONFIG_FLAG_GET_COLORDEPTH));
    log += F(", background: ");
    log += AdaGFXcolorToString(_bgcolor, static_cast<AdaGFXColorDepth>(P096_CONFIG_FLAG_GET_COLORDEPTH));
    addLogMove(LOG_LEVEL_INFO, log);
  }
  # endif // ifndef BUILD_NO_DEBUG

  if (nullptr != eInkScreen) {
    eInkScreen->begin(); // Start the device
    eInkScreen->clearBuffer();

    eInkScreen->setRotation(_rotation);
    eInkScreen->setTextColor(_fgcolor);
    eInkScreen->setTextSize(_fontscaling); // Handles 0 properly, text size, default 1 = very small
    eInkScreen->setCursor(0, 0);           // move cursor to position (0, 0) pixel

    if (!stringsLoaded) {
      LoadCustomTaskSettings(event->TaskIndex, strings, P096_Nlines, 0);
      stringsLoaded = true;

      for (uint8_t x = 0; x < P096_Nlines && !stringsHasContent; x++) {
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
void P096_data_struct::updateFontMetrics() {
  if (nullptr != gfxHelper) {
    gfxHelper->getTextMetrics(_textcols, _textrows, _fontwidth, _fontheight, _fontscaling, _heightOffset, _xpix, _ypix);
    gfxHelper->getColors(_fgcolor, _bgcolor);
  } else {
    if (_fontscaling == 0) { _fontscaling = 1; }
    _textcols = _xpix / (_fontwidth * _fontscaling);
    _textrows = _ypix / (_fontheight * _fontscaling);
  }
}

/****************************************************************************
 * plugin_exit: De-initialize before destruction
 ***************************************************************************/
bool P096_data_struct::plugin_exit(struct EventStruct *event) {
  addLog(LOG_LEVEL_INFO, F("EPD  : Exit."));

  # if P096_USE_EXTENDED_SETTINGS

  if (nullptr != gfxHelper) { delete gfxHelper; }
  gfxHelper = nullptr;

  # endif // if P096_USE_EXTENDED_SETTINGS

  if (nullptr != eInkScreen) { delete eInkScreen; }
  eInkScreen = nullptr; // Is used as a proxy only
  return true;
}

/****************************************************************************
 * plugin_read: Re-draw the default content
 ***************************************************************************/
bool P096_data_struct::plugin_read(struct EventStruct *event) {
  # if P096_USE_EXTENDED_SETTINGS

  if (nullptr != eInkScreen) {
    if (stringsHasContent) {
      gfxHelper->setColumnRowMode(false); // Turn off column mode

      eInkScreen->clearBuffer();

      int yPos = 0;

      for (uint8_t x = 0; x < P096_Nlines; x++) {
        String newString = AdaGFXparseTemplate(strings[x], _textcols, gfxHelper);

        #  if ADAGFX_PARSE_SUBCOMMAND
        updateFontMetrics();
        #  endif // if ADAGFX_PARSE_SUBCOMMAND

        if (yPos < _ypix) {
          gfxHelper->printText(newString.c_str(), 0, yPos, _fontscaling, _fgcolor, _bgcolor);
        }
        delay(0);
        yPos += (_fontheight * _fontscaling);
      }
      gfxHelper->setColumnRowMode(bitRead(P096_CONFIG_FLAGS, P096_CONFIG_FLAG_USE_COL_ROW)); // Restore column mode
      int16_t curX, curY;
      gfxHelper->getCursorXY(curX, curY);                                                    // Get current X and Y coordinates,
      UserVar[event->BaseVarIndex]     = curX;                                               // and put into Values
      UserVar[event->BaseVarIndex + 1] = curY;

      eInkScreen->display();
      eInkScreen->clearBuffer();
    }
  }
  # endif // if P096_USE_EXTENDED_SETTINGS
  return false; // Always return false, so no attempt to send to Controllers or generate events is started
}

/****************************************************************************
 * plugin_write: Handle commands
 ***************************************************************************/
bool P096_data_struct::plugin_write(struct EventStruct *event, const String& string) {
  bool   success = false;
  String cmd     = parseString(string, 1);

  if ((nullptr != eInkScreen) && cmd.equals(_commandTriggerCmd)) {
    String arg1 = parseString(string, 2);

    if (arg1.equals(F("clear"))) {
      String arg2 = parseString(string, 3);

      eInkScreen->clearBuffer();

      if (!arg2.isEmpty()) {
        eInkScreen->fillScreen(AdaGFXparseColor(arg2, _colorDepth));
      } else {
        eInkScreen->fillScreen(_bgcolor);
      }
      eInkScreen->display();
      eInkScreen->clearBuffer();
      success = true;
    }
    else if (arg1.equals(F("deepsleep"))) {
      eInkScreen->deepSleep();
      success = true;
    }
    else if (arg1.equals(F("seq_start"))) {
      String arg2 = parseString(string, 3);

      eInkScreen->clearBuffer();
      const uint16_t fillColor =
        (arg2.isEmpty() ? static_cast<uint16_t>(AdaGFXMonoRedGreyscaleColors::ADAGFXEPD_WHITE)
        : AdaGFXparseColor(arg2, _colorDepth));
      eInkScreen->fillScreen(fillColor);
      _sequence_in_progress = true;
      success               = true;
    }
    else if (arg1.equals(F("seq_end"))) {
      eInkScreen->display();
      eInkScreen->clearBuffer();
      _sequence_in_progress = false;
      success               = true;
    }
    else if (arg1.equals(F("inv"))) {
      if ((event->Par2 >= 0) &&
          (event->Par2 <= 1)) {
        eInkScreen->invertDisplay(event->Par2);
        eInkScreen->display();
        success = true;
      }
    }
    else if (arg1.equals(F("rot"))) {
      ///control?cmd=epdcmd,rot,0
      // not working to verify
      if ((event->Par2 >= 0)) {
        eInkScreen->setRotation(event->Par2 % 4);
        eInkScreen->display();
        success = true;
      }
    } else {
      success = false;
    }
  }
  else if (eInkScreen && (cmd.equals(_commandTrigger) ||
                          (gfxHelper && gfxHelper->isAdaGFXTrigger(cmd)))) {
    success = true;

    if (nullptr != gfxHelper) {
      String tmp = string;

      if (!_sequence_in_progress) {
        eInkScreen->clearBuffer();
        eInkScreen->fillScreen(EPD_WHITE);
      }

      // Hand it over after replacing variables
      success = gfxHelper->processCommand(AdaGFXparseTemplate(tmp, _textcols, gfxHelper));

      if (success && !_sequence_in_progress) {
        eInkScreen->display();
      }

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

#endif // ifdef USES_P096
