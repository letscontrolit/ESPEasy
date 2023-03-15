#include "_Plugin_Helper.h"

#ifdef USES_P131

// #######################################################################################################
// ########################### Plugin 131 NeoPixel Matrix display ########################################
// #######################################################################################################

/** Changelog:
 * 2023-02-27 tonhuisman: Implement support for getting config values, see AdafruitGFX_Helper.h changelog for details
 * 2022-07-30 tonhuisman: Add commands to set scroll-options (settext, setscroll, setstep, setspeed, setempty, setright)
 *                        Fix issue that on startup the display wasn't cleared (unit reset should turn off the display)
 *                        Changed initial Text print mode to 'Truncate exceeding message' to enable Scroll mode
 *                        Removed Testing tag from the plugin name. It has been tested and proven stable.
 * 2022-07-15 tonhuisman: Implement horizontal scrolling (left/right, opt. empty start) for predefined content
 * 2022-07-11 tonhuisman: Fix Clear on Exit issue
 * 2022-06-13 tonhuisman: Improved Splash handling, non-blocking delay, default 3 seconds
 * 2022-06-12 tonhuisman: Fix reading settings before plugin_ten_per_second() is executed
 *                        Implement PCONFIG_ULONG(n) macro
 * 2022-06-11 tonhuisman: Cleanup and optimize, make startup-splash configurable
 *                        Removed old setting for Brightness, check settings if older version has been used!
 * 2022-03-16 tonhuisman: Add textmode option, start scrolling implementation
 * 2022-03-12 tonhuisman: Add Max. Brightness setting. Bugfixing.
 * 2022-03-11 tonhuisman: Add Initial Brightness setting
 * 2022-03-05 tonhuisman: Initial plugin development.
 * Forum request: https://www.letscontrolit.com/forum/viewtopic.php?p=38095&hilit=matrix#p38095
 */

# define PLUGIN_131
# define PLUGIN_ID_131      131
# define PLUGIN_NAME_131    "Display - NeoPixel Matrix"

# include "./src/PluginStructs/P131_data_struct.h"

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

      set8BitToUL(P131_CONFIG_FLAGS, P131_FLAGS_MATRIX_TYPE,
                  NEO_MATRIX_TOP | NEO_MATRIX_LEFT | NEO_MATRIX_ROWS | NEO_MATRIX_PROGRESSIVE |
                  NEO_TILE_TOP | NEO_TILE_LEFT | NEO_TILE_ROWS | NEO_TILE_PROGRESSIVE);

      set4BitToUL(P131_CONFIG_FLAGS, P131_CONFIG_FLAG_MODE, static_cast<int>(AdaGFXTextPrintMode::TruncateExceedingMessage));

      set8BitToUL(P131_CONFIG_FLAGS_B, P131_CONFIG_FLAG_B_BRIGHTNESS, 40);  // Default brightness
      set8BitToUL(P131_CONFIG_FLAGS_B, P131_CONFIG_FLAG_B_MAXBRIGHT,  255); // Default max brightness

      P131_CONFIG_COLORS = ADAGFX_WHITE | (ADAGFX_BLACK << 16);

      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      event->String1 = formatGpioName_output(F("DIN"));
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      const int optionValuesZeroOne[] { 0, 1 };

      {
        const __FlashStringHelper *stripOptions[] = { F("GRB"), F("GRBW") }; // Selection copied from P038
        addFormSelector(F("Strip Type"), F("striptype"), 2, stripOptions, optionValuesZeroOne, P131_CONFIG_FLAGS_GET_STRIP_TYPE);
      }

      {
        const __FlashStringHelper *optionsTop[] = {
          F("Top/Left"),
          F("Top/Right"),
          F("Bottom/Left"),
          F("Bottom/Right") };
        const int optionValuesTop[] { 0, 2, 1, 3 };

        const __FlashStringHelper *optionsRowCol[] = {
          F("Rows"),
          F("Columns") };

        const __FlashStringHelper *optionsProZig[] = {
          F("Progressive"),
          F("Zigzag") };

        addFormSubHeader(F("Matrix"));

        addFormNumericBox(F("Matrix width"),  F("mxwidth"),
                          P131_CONFIG_MATRIX_WIDTH, 1, 100);
        addFormNumericBox(F("Matrix height"), F("mxheight"),
                          P131_CONFIG_MATRIX_HEIGHT, 1, 100);

        addFormSelector(F("Matrix start-pixel"), F("mxstart"), 4, optionsTop, optionValuesTop,
                        get2BitFromUL(P131_CONFIG_FLAGS, P131_FLAGS_MATRIX_TYPE_TOP));

        addFormSelector(F("Matrix Rows/Columns mode"), F("mxrowcol"), 2, optionsRowCol, optionValuesZeroOne,
                        bitRead(P131_CONFIG_FLAGS, P131_FLAGS_MATRIX_TYPE_RC));

        addFormSelector(F("Matrix flow direction"), F("mxprozig"), 2, optionsProZig, optionValuesZeroOne,
                        bitRead(P131_CONFIG_FLAGS, P131_FLAGS_MATRIX_TYPE_PZ));

        addFormSubHeader(F("Multiple matrices: Tiles"));

        addFormNumericBox(F("Tile matrix width"),  F("tlwidth"),
                          P131_CONFIG_TILE_WIDTH, 1, 32);
        addFormNumericBox(F("Tile matrix height"), F("tlheight"),
                          P131_CONFIG_TILE_HEIGHT, 1, P131_Nlines);

        addFormSelector(F("Tile start-matrix"), F("tlstart"), 4, optionsTop, optionValuesTop,
                        get2BitFromUL(P131_CONFIG_FLAGS, P131_FLAGS_TILE_TYPE_TOP));

        addFormSelector(F("Tile Rows/Columns mode"), F("tlrowcol"), 2, optionsRowCol, optionValuesZeroOne,
                        bitRead(P131_CONFIG_FLAGS, P131_FLAGS_TILE_TYPE_RC));

        addFormSelector(F("Tile flow direction"), F("tlprozig"), 2, optionsProZig, optionValuesZeroOne,
                        bitRead(P131_CONFIG_FLAGS, P131_FLAGS_TILE_TYPE_PZ));
      }

      addFormSubHeader(F("Display"));

      AdaGFXFormRotation(F("rotate"), P131_CONFIG_FLAG_GET_ROTATION);

      AdaGFXFormTextPrintMode(F("tpmode"), P131_CONFIG_FLAG_GET_MODE);

      addFormNumericBox(F("Initial brightness"), F("brightness"), P131_CONFIG_FLAG_GET_BRIGHTNESS, 0, 255);
      addUnit(F("0..255"));
      addFormNumericBox(F("Maximum allowed brightness"), F("maxbright"), P131_CONFIG_FLAG_GET_MAXBRIGHT, 1, 255);
      addUnit(F("1..255"));

      AdaGFXFormFontScaling(F("fontscale"), P131_CONFIG_FLAG_GET_FONTSCALE, 4);

      # ifdef P131_SHOW_SPLASH
      addFormCheckBox(F("Show splash on start"),  F("splash"),      P131_CONFIG_FLAG_GET_SHOW_SPLASH);
      # endif // ifdef P131_SHOW_SPLASH

      addFormCheckBox(F("Clear display on exit"), F("clearOnExit"), bitRead(P131_CONFIG_FLAGS, P131_CONFIG_FLAG_CLEAR_ON_EXIT));

      {
        const __FlashStringHelper *commandTriggers[] = { // Be sure to use all options available in the enum (except MAX)!
          P131_CommandTrigger_toString(P131_CommandTrigger::neomatrix),
          P131_CommandTrigger_toString(P131_CommandTrigger::neo)
        };
        const int commandTriggerOptions[] = {
          static_cast<int>(P131_CommandTrigger::neomatrix),
          static_cast<int>(P131_CommandTrigger::neo)
        };
        constexpr int cmdCount = sizeof(commandTriggerOptions) / sizeof(commandTriggerOptions[0]);
        addFormSelector(F("Write Command trigger"),
                        F("cmdtrigger"),
                        cmdCount,
                        commandTriggers,
                        commandTriggerOptions,
                        P131_CONFIG_FLAG_GET_CMD_TRIGGER);
        # ifndef LIMIT_BUILD_SIZE
        addFormNote(F("Select the command that is used to handle commands for this display."));
        # endif // ifndef LIMIT_BUILD_SIZE
      }

      addFormSubHeader(F("Content"));

      AdaGFXFormForeAndBackColors(F("fgcolor"),
                                  P131_CONFIG_GET_COLOR_FOREGROUND,
                                  F("bgcolor"),
                                  P131_CONFIG_GET_COLOR_BACKGROUND);

      {
        String strings[P131_Nlines];
        LoadCustomTaskSettings(event->TaskIndex, strings, P131_Nlines, 0);

        uint16_t remain = DAT_TASKS_CUSTOM_SIZE;

        addFormSubHeader(F("Lines"));
        addRowLabel(F("Lines"));

        html_table(EMPTY_STRING); // Sub-table
        html_table_header(F("Line #&nbsp;"));
        html_table_header(F("Text"));
        html_table_header(F("Scroll"));
        html_table_header(F("Empty start"));
        html_table_header(F("Scroll right"));
        html_table_header(F("Stepsize"));
        html_table_header(F("Speed"));

        for (uint8_t varNr = 0; varNr < P131_CONFIG_TILE_HEIGHT; varNr++) {
          html_TR_TD(); // All columns use max. width available
          addHtml(F("&nbsp;"));
          addHtmlInt(varNr + 1);

          html_TD(); // Text
          addTextBox(getPluginCustomArgName(varNr),
                     parseStringKeepCase(strings[varNr], 1),
                     P131_Nchars,
                     false,
                     false,
                     EMPTY_STRING,
                     F(""));

          String   opts    = parseString(strings[varNr], 2);
          uint32_t optBits = 0;
          validUIntFromString(opts, optBits);

          html_TD(); // Scroll
          addCheckBox(getPluginCustomArgName(varNr + 100), bitRead(optBits, P131_OPTBITS_SCROLL) == 1, false
                      # if FEATURE_TOOLTIPS
                      , F("Scroll text")
                      # endif // if FEATURE_TOOLTIPS
                      );
          html_TD(); // Start with empty display, inverted setting
          addCheckBox(getPluginCustomArgName(varNr + 200), bitRead(optBits, P131_OPTBITS_STARTBLANK) == 0, false
                      # if FEATURE_TOOLTIPS
                      , F("Start and end scroll with empty display")
                      # endif // if FEATURE_TOOLTIPS
                      );
          html_TD(); // Scroll from right
          addCheckBox(getPluginCustomArgName(varNr + 300), bitRead(optBits, P131_OPTBITS_RIGHTSCROLL) == 1, false
                      # if FEATURE_TOOLTIPS
                      , F("Scroll from left to right")
                      # endif // if FEATURE_TOOLTIPS
                      );
          html_TD(); // Pixels per step, offset with -1
          addNumericBox(getPluginCustomArgName(varNr + 400), get4BitFromUL(optBits, P131_OPTBITS_SCROLLSTEP) + 1, 1, P131_MAX_SCROLL_STEPS
                        # if FEATURE_TOOLTIPS
                        , F(""), F("Scroll 1..16 pixels / step")
                        # endif // if FEATURE_TOOLTIPS
                        );

          opts = parseString(strings[varNr], 3);
          int scrollSpeed = 0;
          validIntFromString(opts, scrollSpeed);

          if (scrollSpeed == 0) { scrollSpeed = 10; }
          html_TD(); // Speed 0.1 seconds per step
          addNumericBox(getPluginCustomArgName(varNr + 500), scrollSpeed, 1, P131_MAX_SCROLL_SPEED
                        # if FEATURE_TOOLTIPS
                        , F(""), F("Scroll-speed in 0.1 seconds / step")
                        # endif // if FEATURE_TOOLTIPS
                        );

          remain -= (strings[varNr].length() + 1);

          if ((P131_CONFIG_TILE_HEIGHT > 1) && (varNr == P131_CONFIG_TILE_HEIGHT - 1)) {
            html_TD();
            addUnit(concat(F("Remaining: "), static_cast<int>(remain)));
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
      P131_CONFIG_MATRIX_WIDTH  = getFormItemInt(F("mxwidth"));
      P131_CONFIG_MATRIX_HEIGHT = getFormItemInt(F("mxheight"));
      P131_CONFIG_TILE_WIDTH    = getFormItemInt(F("tlwidth"));
      P131_CONFIG_TILE_HEIGHT   = getFormItemInt(F("tlheight"));

      // Bits are already in the correct order/configuration to be passed on to the constructor
      // Matrix bits
      set2BitToUL(P131_CONFIG_FLAGS, P131_FLAGS_MATRIX_TYPE_TOP, getFormItemInt(F("mxstart")) & 0x03);
      bitWrite(P131_CONFIG_FLAGS, P131_FLAGS_MATRIX_TYPE_RC, getFormItemInt(F("mxrowcol")));
      bitWrite(P131_CONFIG_FLAGS, P131_FLAGS_MATRIX_TYPE_PZ, getFormItemInt(F("mxprozig")));

      // Tile bits
      set2BitToUL(P131_CONFIG_FLAGS, P131_FLAGS_TILE_TYPE_TOP, getFormItemInt(F("tlstart")) & 0x03);
      bitWrite(P131_CONFIG_FLAGS, P131_FLAGS_TILE_TYPE_RC, getFormItemInt(F("tlrowcol")));
      bitWrite(P131_CONFIG_FLAGS, P131_FLAGS_TILE_TYPE_PZ, getFormItemInt(F("tlprozig")));

      // Other settings
      set4BitToUL(P131_CONFIG_FLAGS, P131_CONFIG_FLAG_MODE,        getFormItemInt(F("tpmode")));
      set4BitToUL(P131_CONFIG_FLAGS, P131_CONFIG_FLAG_ROTATION,    getFormItemInt(F("rotate")));
      set4BitToUL(P131_CONFIG_FLAGS, P131_CONFIG_FLAG_FONTSCALE,   getFormItemInt(F("fontscale")));
      set4BitToUL(P131_CONFIG_FLAGS, P131_CONFIG_FLAG_CMD_TRIGGER, getFormItemInt(F("cmdtrigger")));

      bitWrite(P131_CONFIG_FLAGS, P131_CONFIG_FLAG_CLEAR_ON_EXIT, isFormItemChecked(F("clearOnExit")));
      bitWrite(P131_CONFIG_FLAGS, P131_CONFIG_FLAG_STRIP_TYPE,    getFormItemInt(F("striptype")) == 1);
      # ifdef P131_SHOW_SPLASH
      bitWrite(P131_CONFIG_FLAGS, P131_CONFIG_FLAG_SHOW_SPLASH,   !isFormItemChecked(F("splash")));
      # endif // ifdef P131_SHOW_SPLASH


      set8BitToUL(P131_CONFIG_FLAGS_B, P131_CONFIG_FLAG_B_BRIGHTNESS, getFormItemInt(F("brightness")));
      set8BitToUL(P131_CONFIG_FLAGS_B, P131_CONFIG_FLAG_B_MAXBRIGHT,  getFormItemInt(F("maxbright")));

      String   color   = webArg(F("fgcolor"));
      uint16_t fgcolor = ADAGFX_WHITE;     // Default to white when empty

      if (!color.isEmpty()) {
        fgcolor = AdaGFXparseColor(color); // Reduce to rgb565
      }
      color = webArg(F("bgcolor"));
      const uint16_t bgcolor = AdaGFXparseColor(color);

      P131_CONFIG_COLORS = fgcolor | (bgcolor << 16); // Store as a single setting

      String strings[P131_Nlines];
      String error;
      error.reserve(70);

      for (uint8_t varNr = 0; varNr < prevLines; varNr++) {
        error.clear();
        error += wrapWithQuotes(webArg(getPluginCustomArgName(varNr)));
        error += ',';
        uint32_t optBits = 0;
        bitWrite(optBits, P131_OPTBITS_SCROLL,      isFormItemChecked(getPluginCustomArgName(varNr + 100)));
        bitWrite(optBits, P131_OPTBITS_STARTBLANK,  !isFormItemChecked(getPluginCustomArgName(varNr + 200))); // Inverted
        bitWrite(optBits, P131_OPTBITS_RIGHTSCROLL, isFormItemChecked(getPluginCustomArgName(varNr + 300)));
        set4BitToUL(optBits, P131_OPTBITS_SCROLLSTEP, getFormItemIntCustomArgName(varNr + 400) - 1);          // Offset -1
        error         += optBits;
        error         += ',';
        error         += getFormItemIntCustomArgName(varNr + 500);
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
        # ifndef BUILD_NO_DEBUG
        String log = F("neomatrix: INIT, matrixType: 0b");
        log += String(P131_CONFIG_FLAGS_GET_MATRIX_TYPE, BIN);
        addLogMove(LOG_LEVEL_INFO, log);
        # endif // ifndef BUILD_NO_DEBUG
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

        success = (nullptr != P131_data) && P131_data->plugin_init(event); // Start the display
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
        success = P131_data->plugin_ten_per_second(event); // 10 per second operation, update scrolling, handle splash counter
      }

      break;
    }

    # if ADAGFX_ENABLE_GET_CONFIG_VALUE
    case PLUGIN_GET_CONFIG_VALUE:
    {
      P131_data_struct *P131_data = static_cast<P131_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P131_data) {
        success = P131_data->plugin_get_config_value(event, string); // GetConfig operation, handle variables, fully delegated to
                                                                     // AdafruitGFX_helper
      }
      break;
    }
    # endif // if ADAGFX_ENABLE_GET_CONFIG_VALUE
  }
  return success;
}

#endif // ifdef USES_P131
