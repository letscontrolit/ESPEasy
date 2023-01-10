#include "_Plugin_Helper.h"

#ifdef USES_P141

// #######################################################################################################
// ########################### Plugin 141: PCD8544 Nokia 5110 LCD display ################################
// #######################################################################################################


/** Changelog:
 * 2022-10-08 tonhuisman: Enable PLUGIN_GET_CONFIG_VALUE event to get runtime info from plugin
 * 2022-09-24 tonhuisman: Store inverted setting when changed via inv subcommand (not saved)
 * 2022-09-23 tonhuisman: Allow backlight form 0% instead of from 1% to be able to completely turn it off
 *                        Allow contrast setting from 0..100%.
 * 2022-09-21 tonhuisman: Allow contrast setting from 1..100%, add contrast subcommand
 * 2022-09-19 tonhuisman: Skip empty Lines (don't clear display line) on TaskRun/PLUGIN_READ
 * 2022-09-12 tonhuisman: Remove unneeded color settings, as we have the Inverted display option
 * 2022-09-11 tonhuisman: Disable some less needed features to reduce .bin size, remove Cursor X/Y Values for challenged builds
 *                        remove unused Splash feature code (was already disabled)
 * 2022-09-10 tonhuisman: Add configurable line spacing for configured Lines (user request), clear screen when using the 'off' subcommand
 * 2022-08-25 tonhuisman: Remove strange option 'tft' for command trigger, as this has nothing to with a tft display
 *                        Clear screen with correct color on exit when inverted is active
 * 2022-08-23 tonhuisman: Add <trigger>cmd,inv[,0|1] subcommand for inverting the display, also an extra Config option
 *                        is added to set 'Invert display' as the default.
 * 2022-08-20 tonhuisman: Migrate/rewrite from ESPEasy PluginPlayground _P208_Nokia_LCD_5110.ino, now based
 *                        on Plugin 116 ST77xx
 */

# define PLUGIN_141
# define PLUGIN_ID_141         141
# define PLUGIN_NAME_141       "Display - PCD8544 Nokia 5110 LCD"
# define PLUGIN_VALUENAME1_141 "CursorX"
# define PLUGIN_VALUENAME2_141 "CursorY"

# include "src/PluginStructs/P141_data_struct.h"

boolean Plugin_141(uint8_t function, struct EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_141;
      Device[deviceCount].Type               = DEVICE_TYPE_SPI3;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_NONE;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = false;
      # if P141_FEATURE_CURSOR_XY_VALUES
      Device[deviceCount].ValueCount = 2;
      # endif // if P141_FEATURE_CURSOR_XY_VALUES
      Device[deviceCount].SendDataOption = false;
      Device[deviceCount].TimerOption    = true;
      Device[deviceCount].TimerOptional  = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_141);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      # if P141_FEATURE_CURSOR_XY_VALUES
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_141));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_141));
      # endif // if P141_FEATURE_CURSOR_XY_VALUES
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      event->String1 = formatGpioName_output(F("SCE"));
      event->String2 = formatGpioName_output(F("D/C"));
      event->String3 = formatGpioName_output_optional(F("RST "));
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      # ifdef ESP32

      if (Settings.InitSPI == 2) { // When using ESP32 H(ardware-)SPI
        P141_CS_PIN = P141_LCD_CS_HSPI;
      } else
      # endif // ifdef ESP32
      {
        P141_CS_PIN = P141_LCD_CS;
      }
      P141_DC_PIN                   = P141_LCD_DC;
      P141_RST_PIN                  = P141_LCD_RST;
      P141_CONFIG_BUTTON_PIN        = -1;
      P141_CONFIG_BACKLIGHT_PIN     = -1;
      P141_CONFIG_BACKLIGHT_PERCENT = 50; // Percentage backlight
      P141_CONFIG_CONTRAST          = 60; // 60% seems to be properly visible, but may depend on the actual display

      uint32_t lSettings = 0;

      // Clear then Truncate exceeding message
      set4BitToUL(lSettings, P141_CONFIG_FLAG_MODE,        static_cast<int>(AdaGFXTextPrintMode::ClearThenTruncate));
      set4BitToUL(lSettings, P141_CONFIG_FLAG_FONTSCALE,   1);
      set4BitToUL(lSettings, P141_CONFIG_FLAG_LINESPACING, 15); // Auto
      set4BitToUL(lSettings, P141_CONFIG_FLAG_CMD_TRIGGER, static_cast<int>(P141_CommandTrigger::pcd8544));
      bitWrite(lSettings, P141_CONFIG_FLAG_BACK_FILL, 1);       // No back-fill
      P141_CONFIG_FLAGS = lSettings;

      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      AdaGFXFormBacklight(F("pbacklight"), P141_CONFIG_BACKLIGHT_PIN,
                          F("pbackpercent"), P141_CONFIG_BACKLIGHT_PERCENT);

      addFormNumericBox(F("Display Contrast"), F("pcontrast"), P141_CONFIG_CONTRAST, 0, 100);
      addUnit(F("0-100%"));

      AdaGFXFormDisplayButton(F("pbutton"), P141_CONFIG_BUTTON_PIN,
                              F("pbtnInverse"), bitRead(P141_CONFIG_FLAGS, P141_CONFIG_FLAG_INVERT_BUTTON),
                              F("ptimer"), P141_CONFIG_DISPLAY_TIMEOUT);

      addFormSubHeader(F("Layout"));

      AdaGFXFormRotation(F("protate"), P141_CONFIG_FLAG_GET_ROTATION);

      AdaGFXFormTextPrintMode(F("pmode"), P141_CONFIG_FLAG_GET_MODE);

      AdaGFXFormFontScaling(F("pfontscale"), P141_CONFIG_FLAG_GET_FONTSCALE);

      addFormCheckBox(F("Invert display"),        F("pinvert"),      bitRead(P141_CONFIG_FLAGS, P141_CONFIG_FLAG_INVERTED));

      addFormCheckBox(F("Clear display on exit"), F("pclearOnExit"), bitRead(P141_CONFIG_FLAGS, P141_CONFIG_FLAG_CLEAR_ON_EXIT));

      {
        const __FlashStringHelper *commandTriggers[] = { // Be sure to use all options available in the enum
          toString(P141_CommandTrigger::pcd8544),
          toString(P141_CommandTrigger::lcd),
        };
        const int commandTriggerOptions[] = {
          static_cast<int>(P141_CommandTrigger::pcd8544),
          static_cast<int>(P141_CommandTrigger::lcd),
        };
        addFormSelector(F("Write Command trigger"),
                        F("pcmdtrigger"),
                        sizeof(commandTriggerOptions) / sizeof(int),
                        commandTriggers,
                        commandTriggerOptions,
                        P141_CONFIG_FLAG_GET_CMD_TRIGGER);
        # ifndef LIMIT_BUILD_SIZE
        addFormNote(F("Select the command that is used to handle commands for this display."));
        # endif // ifndef LIMIT_BUILD_SIZE
      }

      // Inverted state!
      addFormCheckBox(F("Wake display on receiving text"), F("pNoDisplay"), !bitRead(P141_CONFIG_FLAGS, P141_CONFIG_FLAG_NO_WAKE));
      # ifndef LIMIT_BUILD_SIZE
      addFormNote(F("When checked, the display wakes up at receiving remote updates."));
      # endif // ifndef LIMIT_BUILD_SIZE

      AdaGFXFormTextColRowMode(F("pcolrow"), bitRead(P141_CONFIG_FLAGS, P141_CONFIG_FLAG_USE_COL_ROW) == 1);

      AdaGFXFormTextBackgroundFill(F("pbackfill"), bitRead(P141_CONFIG_FLAGS, P141_CONFIG_FLAG_BACK_FILL) == 0); // Inverse

      addFormSubHeader(F("Content"));

      String strings[P141_Nlines];
      LoadCustomTaskSettings(event->TaskIndex, strings, P141_Nlines, 0);

      # ifndef LIMIT_BUILD_SIZE
      uint16_t remain = P141_Nlines * (P141_Nchars + 1); // DAT_TASKS_CUSTOM_SIZE;
      # endif // ifndef LIMIT_BUILD_SIZE

      for (uint8_t varNr = 0; varNr < P141_Nlines; varNr++) {
        addFormTextBox(concat(F("Line "), varNr + 1), getPluginCustomArgName(varNr), strings[varNr], P141_Nchars);
        # ifndef LIMIT_BUILD_SIZE
        remain -= (strings[varNr].length() + 1);
        # endif // ifndef LIMIT_BUILD_SIZE
      }
      # ifndef LIMIT_BUILD_SIZE
      addUnit(concat(F("Remaining: "), remain));
      # endif // ifndef LIMIT_BUILD_SIZE

      AdaGFXFormLineSpacing(F("linespc"), P141_CONFIG_FLAG_GET_LINESPACING);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P141_CONFIG_BUTTON_PIN        = getFormItemInt(F("pbutton"));
      P141_CONFIG_DISPLAY_TIMEOUT   = getFormItemInt(F("ptimer"));
      P141_CONFIG_BACKLIGHT_PIN     = getFormItemInt(F("pbacklight"));
      P141_CONFIG_BACKLIGHT_PERCENT = getFormItemInt(F("pbackpercent"));
      P141_CONFIG_CONTRAST          = getFormItemInt(F("pcontrast"));

      uint32_t lSettings = 0;
      bitWrite(lSettings, P141_CONFIG_FLAG_NO_WAKE,       !isFormItemChecked(F("pNoDisplay")));  // Bit 0 NoDisplayOnReceivingText,
      // reverse logic, default=checked!
      bitWrite(lSettings, P141_CONFIG_FLAG_INVERT_BUTTON, isFormItemChecked(F("pbtnInverse")));  // Bit 1 buttonInverse
      bitWrite(lSettings, P141_CONFIG_FLAG_CLEAR_ON_EXIT, isFormItemChecked(F("pclearOnExit"))); // Bit 2 ClearOnExit
      bitWrite(lSettings, P141_CONFIG_FLAG_USE_COL_ROW,   isFormItemChecked(F("pcolrow")));      // Bit 3 Col/Row addressing

      set4BitToUL(lSettings, P141_CONFIG_FLAG_MODE,        getFormItemInt(F("pmode")));          // Bit 4..7 Text print mode
      set4BitToUL(lSettings, P141_CONFIG_FLAG_ROTATION,    getFormItemInt(F("protate")));        // Bit 8..11 Rotation
      set4BitToUL(lSettings, P141_CONFIG_FLAG_FONTSCALE,   getFormItemInt(F("pfontscale")));     // Bit 12..15 Font scale
      set4BitToUL(lSettings, P141_CONFIG_FLAG_LINESPACING, getFormItemInt(F("linespc")));        // Bit 16..19 Line spacing
      set4BitToUL(lSettings, P141_CONFIG_FLAG_CMD_TRIGGER, getFormItemInt(F("pcmdtrigger")));    // Bit 20..23 Command trigger

      bitWrite(lSettings, P141_CONFIG_FLAG_BACK_FILL, !isFormItemChecked(F("pbackfill")));       // Bit 28 Back fill text (inv)
      bitWrite(lSettings, P141_CONFIG_FLAG_INVERTED,  isFormItemChecked(F("pinvert")));          // Bit 29 Invert display
      P141_CONFIG_FLAGS = lSettings;

      String strings[P141_Nlines];
      String error;

      for (uint8_t varNr = 0; varNr < P141_Nlines; varNr++) {
        strings[varNr] = web_server.arg(getPluginCustomArgName(varNr));
      }

      error = SaveCustomTaskSettings(event->TaskIndex, strings, P141_Nlines, 0);

      if (!error.isEmpty()) {
        addHtmlError(error);
      }

      success = true;
      break;
    }

    case PLUGIN_GET_DISPLAY_PARAMETERS:
    {
      event->Par1 = 84;                                             // X-resolution in pixels
      event->Par2 = 48;                                             // Y-resolution in pixels
      event->Par3 = P141_CONFIG_FLAG_GET_ROTATION;                  // Rotation (0..3: 0, 90, 180, 270 degrees)
      event->Par4 = static_cast<int>(AdaGFXColorDepth::Monochrome); // Color depth

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      if (Settings.InitSPI != 0) {
        initPluginTaskData(event->TaskIndex,
                           new (std::nothrow) P141_data_struct(P141_CONFIG_FLAG_GET_ROTATION,
                                                               P141_CONFIG_FLAG_GET_FONTSCALE,
                                                               static_cast<AdaGFXTextPrintMode>(P141_CONFIG_FLAG_GET_MODE),
                                                               P141_CONFIG_BACKLIGHT_PIN,
                                                               P141_CONFIG_BACKLIGHT_PERCENT,
                                                               P141_CONFIG_CONTRAST,
                                                               P141_CONFIG_DISPLAY_TIMEOUT,
                                                               toString(static_cast<P141_CommandTrigger>(
                                                                          P141_CONFIG_FLAG_GET_CMD_TRIGGER)),
                                                               ADAGFX_WHITE,
                                                               ADAGFX_BLACK,
                                                               bitRead(P141_CONFIG_FLAGS, P141_CONFIG_FLAG_BACK_FILL) == 0,
                                                               bitRead(P141_CONFIG_FLAGS, P141_CONFIG_FLAG_INVERTED) == 1));
        P141_data_struct *P141_data = static_cast<P141_data_struct *>(getPluginTaskData(event->TaskIndex));

        success = (nullptr != P141_data) && P141_data->plugin_init(event); // Start the display
      # ifndef LIMIT_BUILD_SIZE
      } else {
        addLog(LOG_LEVEL_ERROR, F("PCD8544: SPI not enabled."));
      # endif // ifndef LIMIT_BUILD_SIZE
      }
      break;
    }

    case PLUGIN_EXIT:
    {
      P141_data_struct *P141_data = static_cast<P141_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P141_data) {
        success = P141_data->plugin_exit(event); // Stop the display
      }
      break;
    }

    // Check more often for debouncing the button, when enabled
    case PLUGIN_FIFTY_PER_SECOND:
    {
      if (validGpio(P141_CONFIG_BUTTON_PIN)) {
        P141_data_struct *P141_data = static_cast<P141_data_struct *>(getPluginTaskData(event->TaskIndex));

        if (nullptr != P141_data) {
          P141_data->registerButtonState(digitalRead(P141_CONFIG_BUTTON_PIN), bitRead(P141_CONFIG_FLAGS, P141_CONFIG_FLAG_INVERT_BUTTON));
          success = true;
        }
      }
      break;
    }

    case PLUGIN_TEN_PER_SECOND:
    {
      P141_data_struct *P141_data = static_cast<P141_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P141_data) {
        success = P141_data->plugin_ten_per_second(event); // 10 per second actions
      }
      break;
    }

    case PLUGIN_ONCE_A_SECOND:
    {
      P141_data_struct *P141_data = static_cast<P141_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P141_data) {
        success = P141_data->plugin_once_a_second(event); // Once a second actions
      }
      break;
    }

    case PLUGIN_READ:
    {
      P141_data_struct *P141_data = static_cast<P141_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P141_data) {
        success = P141_data->plugin_read(event); // Read operation, redisplay the configured content
      }
      break;
    }

    case PLUGIN_WRITE:
    {
      P141_data_struct *P141_data = static_cast<P141_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P141_data) {
        success = P141_data->plugin_write(event, string); // Write operation, handle commands, mostly delegated to AdafruitGFX_helper
      }
      break;
    }
    # if ADAGFX_ENABLE_GET_CONFIG_VALUE
    case PLUGIN_GET_CONFIG_VALUE:
    {
      P141_data_struct *P141_data = static_cast<P141_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P141_data) {
        success = P141_data->plugin_get_config_value(event, string); // GetConfig operation, handle variables, fully delegated to
                                                                     // AdafruitGFX_helper
      }
      break;
    }
    # endif // if ADAGFX_ENABLE_GET_CONFIG_VALUE
  }
  return success;
}

#endif // USES_P141
