#include "_Plugin_Helper.h"

#ifdef USES_P131

// #######################################################################################################
// ########################### Plugin 131 NeoPixel Matrix display ########################################
// #######################################################################################################

/** Changelog:
 * 2022-03-16 tonhuisman: Add textmode option, start scrolling implementation
 * 2022-03-12 tonhuisman: Add Max. Brightness setting. Bugfixing.
 * 2022-03-11 tonhuisman: Add Initial Brightness setting
 * 2022-03-05 tonhuisman: Initial plugin development.
 * Forum request: https://www.letscontrolit.com/forum/viewtopic.php?p=38095&hilit=matrix#p38095
 */

# define PLUGIN_131
# define PLUGIN_ID_131      131
# define PLUGIN_NAME_131    "Display - NeoPixel Matrix [TESTING]"

# include "./src/PluginStructs/P131_data_struct.h"

/******************************************
 * enquoteString wrap in ", ' or ` unless all 3 quote types are used
 *****************************************/
String P131enquoteString(const String& input) {
  char quoteChar = '"';

  if (input.indexOf(quoteChar) > -1) {
    quoteChar = '\'';

    if (input.indexOf(quoteChar) > -1) {
      quoteChar = '`';

      if (input.indexOf(quoteChar) > -1) {
        return input; // All types of supported quotes used, return original string
      }
    }
  }
  String result;

  result.reserve(input.length() + 2);
  result  = quoteChar;
  result += input;
  result += quoteChar;

  return result;
}

boolean Plugin_131(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_131;
      Device[deviceCount].Type               = DEVICE_TYPE_SINGLE;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_NONE;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = false;
      Device[deviceCount].ValueCount         = 0;
      Device[deviceCount].SendDataOption     = false;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].TimerOptional      = true;

      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_131);
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      P131_CONFIG_MATRIX_WIDTH  = 8; // Default matrix width
      P131_CONFIG_MATRIX_HEIGHT = 8; // Default matrix height
      P131_CONFIG_TILE_WIDTH    = 1; // Default tile width
      P131_CONFIG_TILE_HEIGHT   = 1; // Default tile height

      uint32_t lSettings = 0;
      set8BitToUL(lSettings, P131_FLAGS_MATRIX_TYPE, NEO_MATRIX_TOP | NEO_MATRIX_LEFT | NEO_MATRIX_ROWS | NEO_MATRIX_PROGRESSIVE);
      set8BitToUL(lSettings, P131_FLAGS_TILE_TYPE,   NEO_TILE_TOP | NEO_TILE_LEFT | NEO_TILE_ROWS | NEO_TILE_PROGRESSIVE);
      P131_CONFIG_FLAGS = lSettings;

      lSettings = 0;
      set8BitToUL(lSettings, P131_CONFIG_FLAG_B_BRIGHTNESS, 40);  // Default brightness
      set8BitToUL(lSettings, P131_CONFIG_FLAG_B_MAXBRIGHT,  255); // Default max brightness
      P131_CONFIG_FLAGS_B = lSettings;

      P131_CONFIG_COLORS = ADAGFX_WHITE | (ADAGFX_BLACK << 16);

      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES: {
      event->String1 = formatGpioName_output(F("DIN"));
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      {
        const __FlashStringHelper *stripOptions[] = { F("GRB"), F("GRBW") }; // Selection copied from P038
        int stripIndices[]                        = { 0, 1 };
        addFormSelector(F("Strip Type"), F("p131_strip"), 2, stripOptions, stripIndices, P131_CONFIG_FLAGS_GET_STRIP_TYPE);
      }

      {
        const __FlashStringHelper *optionsTop[] = {
          F("Top/Left"),
          F("Top/Right"),
          F("Bottom/Left"),
          F("Bottom/Right") };
        int optionValuesTop[] { 0, 2, 1, 3 };

        const __FlashStringHelper *optionsRowCol[] = {
          F("Rows"),
          F("Columns") };
        int optionValuesRowCol[] { 0, 1 };

        const __FlashStringHelper *optionsProZig[] = {
          F("Progressive"),
          F("Zigzag") };
        int optionValuesProZig[] { 0, 1 };

        addFormSubHeader(F("Matrix"));

        addFormNumericBox(F("Matrix width"),  F("p131_matrixwidth"),
                          P131_CONFIG_MATRIX_WIDTH, 1, 100);
        addFormNumericBox(F("Matrix height"), F("p131_matrixheight"),
                          P131_CONFIG_MATRIX_HEIGHT, 1, 100);

        addFormSelector(F("Matrix start-pixel"), F("p131_matrixstart"), 4, optionsTop, optionValuesTop,
                        get2BitFromUL(P131_CONFIG_FLAGS, P131_FLAGS_MATRIX_TYPE_TOP));

        addFormSelector(F("Matrix Rows/Columns mode"), F("p131_matrixrowcol"), 2, optionsRowCol, optionValuesRowCol,
                        bitRead(P131_CONFIG_FLAGS, P131_FLAGS_MATRIX_TYPE_RC));

        addFormSelector(F("Matrix flow direction"), F("p131_matrixprozig"), 2, optionsProZig, optionValuesProZig,
                        bitRead(P131_CONFIG_FLAGS, P131_FLAGS_MATRIX_TYPE_PZ));

        addFormSubHeader(F("Multiple matrices: Tiles"));

        addFormNumericBox(F("Tile matrix width"),  F("p131_tilewidth"),
                          P131_CONFIG_TILE_WIDTH, 1, 32);
        addFormNumericBox(F("Tile matrix height"), F("p131_tileheight"),
                          P131_CONFIG_TILE_HEIGHT, 1, P131_Nlines);

        addFormSelector(F("Tile start-matrix"), F("p131_tilestart"), 4, optionsTop, optionValuesTop,
                        get2BitFromUL(P131_CONFIG_FLAGS, P131_FLAGS_TILE_TYPE_TOP));

        addFormSelector(F("Tile Rows/Columns mode"), F("p131_tilerowcol"), 2, optionsRowCol, optionValuesRowCol,
                        bitRead(P131_CONFIG_FLAGS, P131_FLAGS_TILE_TYPE_RC));

        addFormSelector(F("Tile flow direction"), F("p131_tileprozig"), 2, optionsProZig, optionValuesProZig,
                        bitRead(P131_CONFIG_FLAGS, P131_FLAGS_TILE_TYPE_PZ));
      }

      addFormSubHeader(F("Display"));

      AdaGFXFormRotation(F("p131_rotate"), P131_CONFIG_FLAG_GET_ROTATION);

      AdaGFXFormTextPrintMode(F("p131_mode"), P131_CONFIG_FLAG_GET_MODE);

      if ((P131_CONFIG_FLAG_GET_MAXBRIGHT == 0) && (P131_CONFIG_FLAG_GET_BRIGHTNESS == 0)) { // TODO TO BE REMOVED!
        uint32_t lSettings = 0;
        set8BitToUL(lSettings, P131_CONFIG_FLAG_B_BRIGHTNESS, P131_CONFIG_BRIGHTNESS_OLD);
        set8BitToUL(lSettings, P131_CONFIG_FLAG_B_MAXBRIGHT,  255);
        P131_CONFIG_FLAGS_B = lSettings;
      }
      addFormNumericBox(F("Initial brightness"), F("p131_brightness"), P131_CONFIG_FLAG_GET_BRIGHTNESS, 0, 255);
      addUnit(F("0..255"));
      addFormNumericBox(F("Maximum allowed brightness"), F("p131_maxbright"), P131_CONFIG_FLAG_GET_MAXBRIGHT, 1, 255);
      addUnit(F("1..255"));

      AdaGFXFormFontScaling(F("p131_fontscale"), P131_CONFIG_FLAG_GET_FONTSCALE, 4);

      addFormCheckBox(F("Clear display on exit"), F("p131_clearOnExit"), bitRead(P131_CONFIG_FLAGS, P131_CONFIG_FLAG_CLEAR_ON_EXIT));

      {
        const __FlashStringHelper *commandTriggers[] = { // Be sure to use all options available in the enum (except MAX)!
          P131_CommandTrigger_toString(P131_CommandTrigger::neomatrix),
          P131_CommandTrigger_toString(P131_CommandTrigger::neo)
        };
        const int commandTriggerOptions[] = {
          static_cast<int>(P131_CommandTrigger::neomatrix),
          static_cast<int>(P131_CommandTrigger::neo)
        };
        addFormSelector(F("Write Command trigger"),
                        F("p131_commandtrigger"),
                        static_cast<int>(P131_CommandTrigger::MAX),
                        commandTriggers,
                        commandTriggerOptions,
                        P131_CONFIG_FLAG_GET_CMD_TRIGGER);
        addFormNote(F("Select the command that is used to handle commands for this display."));
      }

      addFormSubHeader(F("Content"));

      AdaGFXFormForeAndBackColors(F("p131_foregroundcolor"),
                                  P131_CONFIG_GET_COLOR_FOREGROUND,
                                  F("p131_backgroundcolor"),
                                  P131_CONFIG_GET_COLOR_BACKGROUND);

      {
        String strings[P131_Nlines];
        LoadCustomTaskSettings(event->TaskIndex, strings, P131_Nlines, 0);

        String   line; // Default reserved length is plenty
        uint16_t remain = DAT_TASKS_CUSTOM_SIZE;

        addFormSubHeader(F("Lines"));
        addRowLabel(F("Lines"));

        html_table(EMPTY_STRING); // Sub-table
        html_table_header(F("Line #&nbsp;"));
        html_table_header(F("Text"));
        html_table_header(F("Scroll"));
        html_table_header(F("Right"));
        html_table_header(F("/Pixel"));
        html_table_header(F("Speed"));

        for (uint8_t varNr = 0; varNr < P131_CONFIG_TILE_HEIGHT; varNr++) {
          html_TR_TD(); // All columns use max. width available
          addHtml(F("&nbsp;"));
          addHtmlInt(varNr + 1);

          // line  = F("Line ");
          // line += (varNr + 1);
          html_TD(); // Text
          addTextBox(getPluginCustomArgName(varNr),
                     parseStringKeepCase(strings[varNr], 1),
                     P131_Nchars,
                     false,
                     false,
                     EMPTY_STRING,
                     EMPTY_STRING);

          String opts    = parseString(strings[varNr], 2);
          int    optBits = 0;
          validIntFromString(opts, optBits);

          html_TD(); // Scroll
          addCheckBox(getPluginCustomArgName(varNr + 100), bitRead(optBits, P131_OPTBITS_SCROLL) == 1, false
                      # ifdef ENABLE_TOOLTIPS
                      , F("Scroll text if length > display-width")
                      # endif // ifdef ENABLE_TOOLTIPS
                      );
          html_TD(); // Scroll from right
          addCheckBox(getPluginCustomArgName(varNr + 200), bitRead(optBits, P131_OPTBITS_RIGHTSCROLL) == 1, false
                      # ifdef ENABLE_TOOLTIPS
                      , F("Scroll in from right")
                      # endif // ifdef ENABLE_TOOLTIPS
                      );
          html_TD(); // Scroll per character or per pixel
          addCheckBox(getPluginCustomArgName(varNr + 300), bitRead(optBits, P131_OPTBITS_PIXELSCROLL) == 1, false
                      # ifdef ENABLE_TOOLTIPS
                      , F("Scroll per pixel")
                      # endif // ifdef ENABLE_TOOLTIPS
                      );

          opts = parseString(strings[varNr], 3);
          int scrollSpeed = 0;
          validIntFromString(opts, scrollSpeed);

          if (scrollSpeed == 0) { scrollSpeed = 10; }
          html_TD(); // Speed 0.1 steps per second
          addNumericBox(getPluginCustomArgName(varNr + 400), scrollSpeed, 1, 600
                        # ifdef ENABLE_TOOLTIPS
                        , F(""), F("Scroll-speed in 0.1 steps / second.")
                        # endif // ifdef ENABLE_TOOLTIPS
                        );

          remain -= (strings[varNr].length() + 1);

          if ((P131_CONFIG_TILE_HEIGHT > 1) && (varNr == P131_CONFIG_TILE_HEIGHT - 1)) {
            String remainStr;
            remainStr.reserve(15);
            remainStr  = F("Remaining: ");
            remainStr += remain;
            html_TD();
            addUnit(remainStr);
          }
        }
        html_end_table();
      }

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      const uint8_t prevLines = P131_CONFIG_TILE_HEIGHT;
      P131_CONFIG_MATRIX_WIDTH  = getFormItemInt(F("p131_matrixwidth"));
      P131_CONFIG_MATRIX_HEIGHT = getFormItemInt(F("p131_matrixheight"));
      P131_CONFIG_TILE_WIDTH    = getFormItemInt(F("p131_tilewidth"));
      P131_CONFIG_TILE_HEIGHT   = getFormItemInt(F("p131_tileheight"));

      uint32_t lSettings = 0;

      // Bits are already in the correct order/configuration to be passed on to the constructor
      // Matrix bits
      set2BitToUL(lSettings, P131_FLAGS_MATRIX_TYPE_TOP, getFormItemInt(F("p131_matrixstart")) & 0x03);
      bitWrite(lSettings, P131_FLAGS_MATRIX_TYPE_RC, getFormItemInt(F("p131_matrixrowcol")));
      bitWrite(lSettings, P131_FLAGS_MATRIX_TYPE_PZ, getFormItemInt(F("p131_matrixprozig")));

      // Tile bits
      set2BitToUL(lSettings, P131_FLAGS_TILE_TYPE_TOP, getFormItemInt(F("p131_tilestart")) & 0x03);
      bitWrite(lSettings, P131_FLAGS_TILE_TYPE_RC, getFormItemInt(F("p131_tilerowcol")));
      bitWrite(lSettings, P131_FLAGS_TILE_TYPE_PZ, getFormItemInt(F("p131_tileprozig")));

      // Other settings
      set4BitToUL(lSettings, P131_CONFIG_FLAG_MODE,        getFormItemInt(F("p131_mode")));
      set4BitToUL(lSettings, P131_CONFIG_FLAG_ROTATION,    getFormItemInt(F("p131_rotate")));
      set4BitToUL(lSettings, P131_CONFIG_FLAG_FONTSCALE,   getFormItemInt(F("p131_fontscale")));
      set4BitToUL(lSettings, P131_CONFIG_FLAG_CMD_TRIGGER, getFormItemInt(F("p131_commandtrigger")));

      bitWrite(lSettings, P131_CONFIG_FLAG_CLEAR_ON_EXIT, isFormItemChecked(F("p131_clearOnExit")));
      bitWrite(lSettings, P131_CONFIG_FLAG_STRIP_TYPE,    getFormItemInt(F("p131_strip")) == 1);

      P131_CONFIG_FLAGS = lSettings;

      lSettings = 0;
      set8BitToUL(lSettings, P131_CONFIG_FLAG_B_BRIGHTNESS, getFormItemInt(F("p131_brightness")));
      set8BitToUL(lSettings, P131_CONFIG_FLAG_B_MAXBRIGHT,  getFormItemInt(F("p131_maxbright")));
      P131_CONFIG_FLAGS_B = lSettings;

      String   color   = web_server.arg(F("p131_foregroundcolor"));
      uint16_t fgcolor = ADAGFX_WHITE;     // Default to white when empty

      if (!color.isEmpty()) {
        fgcolor = AdaGFXparseColor(color); // Reduce to rgb565
      }
      color = web_server.arg(F("p131_backgroundcolor"));
      const uint16_t bgcolor = AdaGFXparseColor(color);

      P131_CONFIG_COLORS = fgcolor | (bgcolor << 16); // Store as a single setting

      String strings[P131_Nlines];
      String error;
      error.reserve(70);

      for (uint8_t varNr = 0; varNr < prevLines; varNr++) {
        error.clear();
        error += P131enquoteString(web_server.arg(getPluginCustomArgName(varNr)));
        error += ',';
        uint32_t optBits = 0;
        bitWrite(optBits, P131_OPTBITS_SCROLL,      isFormItemChecked(getPluginCustomArgName(varNr + 100)));
        bitWrite(optBits, P131_OPTBITS_RIGHTSCROLL, isFormItemChecked(getPluginCustomArgName(varNr + 200)));
        bitWrite(optBits, P131_OPTBITS_PIXELSCROLL, isFormItemChecked(getPluginCustomArgName(varNr + 300)));
        error         += optBits;
        error         += ',';
        error         += getFormItemInt(getPluginCustomArgName(varNr + 400));
        strings[varNr] = error;
      }
      error.clear();

      error += SaveCustomTaskSettings(event->TaskIndex, strings, P131_Nlines, 0);

      if (error.length() > 0) {
        addHtmlError(error);
      }

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      if (PIN(0) != -1) {
        String log = F("neomatrix: INIT, matrixType: 0b");
        log += String(P131_CONFIG_FLAGS_GET_MATRIX_TYPE, BIN);
        addLogMove(LOG_LEVEL_INFO, log);
        initPluginTaskData(event->TaskIndex, new (std::nothrow) P131_data_struct(P131_CONFIG_MATRIX_WIDTH,
                                                                                 P131_CONFIG_MATRIX_HEIGHT,
                                                                                 P131_CONFIG_TILE_WIDTH,
                                                                                 P131_CONFIG_TILE_HEIGHT,
                                                                                 PIN(0),
                                                                                 P131_CONFIG_FLAGS_GET_MATRIX_TYPE,
                                                                                 P131_CONFIG_FLAGS_GET_STRIP_TYPE
                                                                                    ? NEO_GRBW + NEO_KHZ800
                                                                                    : NEO_GRB + NEO_KHZ800,
                                                                                 P131_CONFIG_FLAG_GET_ROTATION,
                                                                                 P131_CONFIG_FLAG_GET_FONTSCALE,
                                                                                 static_cast<AdaGFXTextPrintMode>(P131_CONFIG_FLAG_GET_MODE),
                                                                                 P131_CommandTrigger_toString(static_cast<P131_CommandTrigger>(
                                                                                                                P131_CONFIG_FLAG_GET_CMD_TRIGGER)),
                                                                                 P131_CONFIG_FLAG_GET_BRIGHTNESS,
                                                                                 P131_CONFIG_FLAG_GET_MAXBRIGHT,
                                                                                 P131_CONFIG_GET_COLOR_FOREGROUND,
                                                                                 P131_CONFIG_GET_COLOR_BACKGROUND));
        P131_data_struct *P131_data = static_cast<P131_data_struct *>(getPluginTaskData(event->TaskIndex));

        if (nullptr != P131_data) {
          success = P131_data->plugin_init(event); // Start the display
        }
      } else {
        addLog(LOG_LEVEL_ERROR, F("NEOMATRIX: No GPIO pin configured, init cancelled."));
      }

      break;
    }

    case PLUGIN_EXIT:
    {
      P131_data_struct *P131_data = static_cast<P131_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P131_data) {
        success = P131_data->plugin_exit(event);
      }
      break;
    }

    case PLUGIN_READ:
    {
      P131_data_struct *P131_data = static_cast<P131_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P131_data) {
        success = P131_data->plugin_read(event); // Read operation, redisplay the configured content
      }
      break;
    }

    case PLUGIN_WRITE:
    {
      P131_data_struct *P131_data = static_cast<P131_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P131_data) {
        success = P131_data->plugin_write(event, string); // Write operation, handle commands, mostly delegated to AdafruitGFX_helper
      }

      break;
    }

    case PLUGIN_TEN_PER_SECOND:
    {
      P131_data_struct *P131_data = static_cast<P131_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P131_data) {
        success = P131_data->plugin_ten_per_second(event); // 10 per second operation, update scrolling
      }

      break;
    }
  }
  return success;
}

#endif // ifdef USES_P131
