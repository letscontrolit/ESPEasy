#include "_Plugin_Helper.h"

#ifdef USES_P116

// #######################################################################################################
// ########################### Plugin 116: ST77xx TFT displays ###########################################
// #######################################################################################################


// History:
// 2023-02-27 tonhuisman: Implement support for getting config values, see AdafruitGFX_Helper.h changelog for details
// 2022-07-06 tonhuisman: Add support for ST7735sv M5Stack StickC (Inverted colors)
// 2021-11-16 tonhuisman: P116: Change state from Development to Testing
// 2021-11-08 tonhuisman: Add support for function PLUGIN_GET_DISPLAY_PARAMETERS for retrieving the display parameters
//                        as implemented by FT6206 touchscreen plugin. Added ST77xx_type_toResolution
// 2021-11-06 tonhuisman: P116: Add support for ST7796s 320x480 displays
//                        Changed name of plugin to 'Display - ST77xx TFT' (was 'Display - ST7735/ST7789 TFT')
// 2021-08-16 tonhuisman: P116: Add default color settings
// 2021-08-16 tonhuisman: P116: Reorder some device configuration options, add backlight command (triggerCmd option)
// 2021-08-15 tonhuisman: P116: Make CursorX/CursorY coordinates available as Values (no events are generated!)
//                        P116: Use more features of AdafruitGFX_helper
//                        AdafruitGFX: Apply 'Text Print Mode' options
// 2021-08 tonhuisman: Refactor into AdafruitGFX_helper
// 2021-08 tonhuisman: Continue development, added new features, font scaling, display limits, extra text lines
//                     update to current ESPEasy state/style of development, make multi-instance possible
// 2020-08 tonhuisman: Adaptations for multiple ST77xx chips, ST7735s, ST7789vw (shelved temporarily)
//                     Added several features like display button, rotation
// 2020-04 WDS (Wolfdieter): initial plugin for ST7735, based on P012

# define PLUGIN_116
# define PLUGIN_ID_116         116
# define PLUGIN_NAME_116       "Display - ST77xx TFT"
# define PLUGIN_VALUENAME1_116 "CursorX"
# define PLUGIN_VALUENAME2_116 "CursorY"

# include "src/PluginStructs/P116_data_struct.h"

boolean Plugin_116(uint8_t function, struct EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_116;
      Device[deviceCount].Type               = DEVICE_TYPE_SPI3;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_NONE;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = false;
      Device[deviceCount].ValueCount         = 2;
      Device[deviceCount].SendDataOption     = false;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].TimerOptional      = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_116);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_116));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_116));
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      event->String1 = formatGpioName_output_optional(F("CS "));
      event->String2 = formatGpioName_output(F("DC"));
      event->String3 = formatGpioName_output_optional(F("RES "));
      break;
    }

    case PLUGIN_WEBFORM_SHOW_GPIO_DESCR:
    {
      string  = F("CS: ");
      string += formatGpioLabel(PIN(0), false);
      string += event->String1; // contains the NewLine sequence
      string += F("DC: ");
      string += formatGpioLabel(PIN(1), false);
      string += event->String1;
      string += F("RES: ");
      string += formatGpioLabel(PIN(2), false);
      string += event->String1;
      string += F("Btn: ");
      string += formatGpioLabel(P116_CONFIG_BUTTON_PIN, false);
      string += event->String1;
      string += F("Bckl: ");
      string += formatGpioLabel(P116_CONFIG_BACKLIGHT_PIN, false);
      success = true;
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      # ifdef ESP32

      if (Settings.InitSPI == 2) { // When using ESP32 H(ardware-)SPI
        PIN(0) = P116_TFT_CS_HSPI;
      } else {
        PIN(0) = P116_TFT_CS;
      }
      # else // ifdef ESP32
      PIN(0) = P116_TFT_CS;
      # endif // ifdef ESP32
      PIN(1)                        = P116_TFT_DC;
      PIN(2)                        = P116_TFT_RST;
      P116_CONFIG_BUTTON_PIN        = -1;  // No button connected
      P116_CONFIG_BACKLIGHT_PIN     = P116_BACKLIGHT_PIN;
      P116_CONFIG_BACKLIGHT_PERCENT = 100; // Percentage backlight

      uint32_t lSettings = 0;

      // Truncate exceeding message
      set4BitToUL(lSettings, P116_CONFIG_FLAG_MODE,        static_cast<int>(AdaGFXTextPrintMode::TruncateExceedingMessage));
      set4BitToUL(lSettings, P116_CONFIG_FLAG_FONTSCALE,   1);
      set4BitToUL(lSettings, P116_CONFIG_FLAG_CMD_TRIGGER, 1); // Default trigger on st77xx
      P116_CONFIG_FLAGS = lSettings;

      P116_CONFIG_COLORS = ADAGFX_WHITE | (ADAGFX_BLACK << 16);

      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      AdaGFXFormBacklight(F("backlight"), P116_CONFIG_BACKLIGHT_PIN,
                          F("backpercentage"), P116_CONFIG_BACKLIGHT_PERCENT);

      AdaGFXFormDisplayButton(F("button"), P116_CONFIG_BUTTON_PIN,
                              F("buttonInverse"), bitRead(P116_CONFIG_FLAGS, P116_CONFIG_FLAG_INVERT_BUTTON),
                              F("timer"), P116_CONFIG_DISPLAY_TIMEOUT);

      {
        const __FlashStringHelper *options4[] = {
          ST77xx_type_toString(ST77xx_type_e::ST7735s_128x128),
          ST77xx_type_toString(ST77xx_type_e::ST7735s_128x160),
          ST77xx_type_toString(ST77xx_type_e::ST7735s_80x160),
          ST77xx_type_toString(ST77xx_type_e::ST7735s_80x160_M5),
          ST77xx_type_toString(ST77xx_type_e::ST7789vw_240x320),
          ST77xx_type_toString(ST77xx_type_e::ST7789vw_240x240),
          ST77xx_type_toString(ST77xx_type_e::ST7789vw_240x280),
          ST77xx_type_toString(ST77xx_type_e::ST7789vw_135x240),
          ST77xx_type_toString(ST77xx_type_e::ST7796s_320x480)
        };
        const int optionValues4[] = {
          static_cast<int>(ST77xx_type_e::ST7735s_128x128),
          static_cast<int>(ST77xx_type_e::ST7735s_128x160),
          static_cast<int>(ST77xx_type_e::ST7735s_80x160),
          static_cast<int>(ST77xx_type_e::ST7735s_80x160_M5),
          static_cast<int>(ST77xx_type_e::ST7789vw_240x320),
          static_cast<int>(ST77xx_type_e::ST7789vw_240x240),
          static_cast<int>(ST77xx_type_e::ST7789vw_240x280),
          static_cast<int>(ST77xx_type_e::ST7789vw_135x240),
          static_cast<int>(ST77xx_type_e::ST7796s_320x480)
        };
        constexpr int optCount4 = sizeof(optionValues4) / sizeof(optionValues4[0]);
        addFormSelector(F("TFT display model"),
                        F("type"),
                        optCount4,
                        options4,
                        optionValues4,
                        P116_CONFIG_FLAG_GET_TYPE);
      }

      addFormSubHeader(F("Layout"));

      AdaGFXFormRotation(F("rotate"), P116_CONFIG_FLAG_GET_ROTATION);

      AdaGFXFormTextPrintMode(F("mode"), P116_CONFIG_FLAG_GET_MODE);

      AdaGFXFormFontScaling(F("fontscale"), P116_CONFIG_FLAG_GET_FONTSCALE);

      addFormCheckBox(F("Clear display on exit"), F("clearOnExit"), bitRead(P116_CONFIG_FLAGS, P116_CONFIG_FLAG_CLEAR_ON_EXIT));

      {
        const __FlashStringHelper *commandTriggers[] = { // Be sure to use all options available in the enum (except MAX)!
          P116_CommandTrigger_toString(P116_CommandTrigger::tft),
          P116_CommandTrigger_toString(P116_CommandTrigger::st77xx),
          P116_CommandTrigger_toString(P116_CommandTrigger::st7735),
          P116_CommandTrigger_toString(P116_CommandTrigger::st7789),
          P116_CommandTrigger_toString(P116_CommandTrigger::st7796)
        };
        const int commandTriggerOptions[] = {
          static_cast<int>(P116_CommandTrigger::tft),
          static_cast<int>(P116_CommandTrigger::st77xx),
          static_cast<int>(P116_CommandTrigger::st7735),
          static_cast<int>(P116_CommandTrigger::st7789),
          static_cast<int>(P116_CommandTrigger::st7796)
        };
        constexpr int cmdCount = sizeof(commandTriggerOptions) / sizeof(commandTriggerOptions[0]);
        addFormSelector(F("Write Command trigger"),
                        F("commandtrigger"),
                        cmdCount,
                        commandTriggers,
                        commandTriggerOptions,
                        P116_CONFIG_FLAG_GET_CMD_TRIGGER);
        # ifndef LIMIT_BUILD_SIZE
        addFormNote(F("Select the command that is used to handle commands for this display."));
        # endif // ifndef LIMIT_BUILD_SIZE
      }

      // Inverted state!
      addFormCheckBox(F("Wake display on receiving text"), F("NoDisplay"), !bitRead(P116_CONFIG_FLAGS, P116_CONFIG_FLAG_NO_WAKE));
      # ifndef LIMIT_BUILD_SIZE
      addFormNote(F("When checked, the display wakes up at receiving remote updates."));
      # endif // ifndef LIMIT_BUILD_SIZE

      AdaGFXFormTextColRowMode(F("colrow"), bitRead(P116_CONFIG_FLAGS, P116_CONFIG_FLAG_USE_COL_ROW) == 1);

      AdaGFXFormTextBackgroundFill(F("backfill"), bitRead(P116_CONFIG_FLAGS, P116_CONFIG_FLAG_BACK_FILL) == 0); // Inverse

      addFormSubHeader(F("Content"));

      AdaGFXFormForeAndBackColors(F("foregroundcolor"),
                                  P116_CONFIG_GET_COLOR_FOREGROUND,
                                  F("backgroundcolor"),
                                  P116_CONFIG_GET_COLOR_BACKGROUND);

      String strings[P116_Nlines];
      LoadCustomTaskSettings(event->TaskIndex, strings, P116_Nlines, 0);

      uint16_t remain = DAT_TASKS_CUSTOM_SIZE;

      for (uint8_t varNr = 0; varNr < P116_Nlines; varNr++) {
        addFormTextBox(concat(F("Line "), varNr + 1), getPluginCustomArgName(varNr), strings[varNr], P116_Nchars);
        remain -= (strings[varNr].length() + 1);
      }
      addUnit(concat(F("Remaining: "), remain));

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P116_CONFIG_BUTTON_PIN        = getFormItemInt(F("button"));
      P116_CONFIG_DISPLAY_TIMEOUT   = getFormItemInt(F("timer"));
      P116_CONFIG_BACKLIGHT_PIN     = getFormItemInt(F("backlight"));
      P116_CONFIG_BACKLIGHT_PERCENT = getFormItemInt(F("backpercentage"));

      uint32_t lSettings = 0;
      bitWrite(lSettings, P116_CONFIG_FLAG_NO_WAKE,       !isFormItemChecked(F("NoDisplay")));    // Bit 0 NoDisplayOnReceivingText,
                                                                                                  // reverse logic, default=checked!
      bitWrite(lSettings, P116_CONFIG_FLAG_INVERT_BUTTON, isFormItemChecked(F("buttonInverse"))); // Bit 1 buttonInverse
      bitWrite(lSettings, P116_CONFIG_FLAG_CLEAR_ON_EXIT, isFormItemChecked(F("clearOnExit")));   // Bit 2 ClearOnExit
      bitWrite(lSettings, P116_CONFIG_FLAG_USE_COL_ROW,   isFormItemChecked(F("colrow")));        // Bit 3 Col/Row addressing

      set4BitToUL(lSettings, P116_CONFIG_FLAG_MODE,        getFormItemInt(F("mode")));            // Bit 4..7 Text print mode
      set4BitToUL(lSettings, P116_CONFIG_FLAG_ROTATION,    getFormItemInt(F("rotate")));          // Bit 8..11 Rotation
      set4BitToUL(lSettings, P116_CONFIG_FLAG_FONTSCALE,   getFormItemInt(F("fontscale")));       // Bit 12..15 Font scale
      set4BitToUL(lSettings, P116_CONFIG_FLAG_TYPE,        getFormItemInt(F("type")));            // Bit 16..19 Hardwaretype
      set4BitToUL(lSettings, P116_CONFIG_FLAG_CMD_TRIGGER, getFormItemInt(F("commandtrigger")));  // Bit 20..23 Command trigger

      bitWrite(lSettings, P116_CONFIG_FLAG_BACK_FILL, !isFormItemChecked(F("backfill")));         // Bit 28 Back fill text (inv)
      P116_CONFIG_FLAGS = lSettings;

      String   color   = webArg(F("foregroundcolor"));
      uint16_t fgcolor = ADAGFX_WHITE;     // Default to white when empty

      if (!color.isEmpty()) {
        fgcolor = AdaGFXparseColor(color); // Reduce to rgb565
      }
      color = webArg(F("backgroundcolor"));
      uint16_t bgcolor = AdaGFXparseColor(color);

      P116_CONFIG_COLORS = fgcolor | (bgcolor << 16); // Store as a single setting

      String strings[P116_Nlines];

      for (uint8_t varNr = 0; varNr < P116_Nlines; varNr++) {
        strings[varNr] = webArg(getPluginCustomArgName(varNr));
      }

      const String error = SaveCustomTaskSettings(event->TaskIndex, strings, P116_Nlines, 0);

      if (!error.isEmpty()) {
        addHtmlError(error);
      }

      success = true;
      break;
    }

    case PLUGIN_GET_DISPLAY_PARAMETERS:
    {
      uint16_t x, y;
      ST77xx_type_toResolution(static_cast<ST77xx_type_e>(P116_CONFIG_FLAG_GET_TYPE), x, y);

      event->Par1 = x;                                             // X-resolution in pixels
      event->Par2 = y;                                             // Y-resolution in pixels
      event->Par3 = P116_CONFIG_FLAG_GET_ROTATION;                 // Rotation (0..3: 0, 90, 180, 270 degrees)
      event->Par4 = static_cast<int>(AdaGFXColorDepth::FullColor); // Color depth

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      if (Settings.InitSPI != 0) {
        initPluginTaskData(event->TaskIndex,
                           new (std::nothrow) P116_data_struct(static_cast<ST77xx_type_e>(P116_CONFIG_FLAG_GET_TYPE),
                                                               P116_CONFIG_FLAG_GET_ROTATION,
                                                               P116_CONFIG_FLAG_GET_FONTSCALE,
                                                               static_cast<AdaGFXTextPrintMode>(P116_CONFIG_FLAG_GET_MODE),
                                                               P116_CONFIG_BACKLIGHT_PIN,
                                                               P116_CONFIG_BACKLIGHT_PERCENT,
                                                               P116_CONFIG_DISPLAY_TIMEOUT,
                                                               P116_CommandTrigger_toString(static_cast<P116_CommandTrigger>(
                                                                                              P116_CONFIG_FLAG_GET_CMD_TRIGGER)),
                                                               P116_CONFIG_GET_COLOR_FOREGROUND,
                                                               P116_CONFIG_GET_COLOR_BACKGROUND,
                                                               bitRead(P116_CONFIG_FLAGS, P116_CONFIG_FLAG_BACK_FILL) == 0));
        P116_data_struct *P116_data = static_cast<P116_data_struct *>(getPluginTaskData(event->TaskIndex));

        success = (nullptr != P116_data) && P116_data->plugin_init(event); // Start the display
      } else {
        addLog(LOG_LEVEL_ERROR, F("ST77xx: SPI not enabled, init cancelled."));
      }
      break;
    }

    case PLUGIN_EXIT:
    {
      P116_data_struct *P116_data = static_cast<P116_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P116_data) {
        success = P116_data->plugin_exit(event); // Stop the display
      }
      break;
    }

    // Check more often for debouncing the button, when enabled
    case PLUGIN_FIFTY_PER_SECOND:
    {
      if (P116_CONFIG_BUTTON_PIN != -1) {
        P116_data_struct *P116_data = static_cast<P116_data_struct *>(getPluginTaskData(event->TaskIndex));

        if (nullptr != P116_data) {
          P116_data->registerButtonState(digitalRead(P116_CONFIG_BUTTON_PIN), bitRead(P116_CONFIG_FLAGS, P116_CONFIG_FLAG_INVERT_BUTTON));
          success = true;
        }
      }
      break;
    }

    case PLUGIN_TEN_PER_SECOND:
    {
      P116_data_struct *P116_data = static_cast<P116_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P116_data) {
        success = P116_data->plugin_ten_per_second(event); // 10 per second actions
      }
      break;
    }

    case PLUGIN_ONCE_A_SECOND:
    {
      P116_data_struct *P116_data = static_cast<P116_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P116_data) {
        success = P116_data->plugin_once_a_second(event); // Once a second actions
      }
      break;
    }

    case PLUGIN_READ:
    {
      P116_data_struct *P116_data = static_cast<P116_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P116_data) {
        success = P116_data->plugin_read(event); // Read operation, redisplay the configured content
      }
      break;
    }

    case PLUGIN_WRITE:
    {
      P116_data_struct *P116_data = static_cast<P116_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P116_data) {
        success = P116_data->plugin_write(event, string); // Write operation, handle commands, mostly delegated to AdafruitGFX_helper
      }
      break;
    }

    # if ADAGFX_ENABLE_GET_CONFIG_VALUE
    case PLUGIN_GET_CONFIG_VALUE:
    {
      P116_data_struct *P116_data = static_cast<P116_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P116_data) {
        success = P116_data->plugin_get_config_value(event, string); // GetConfig operation, handle variables, fully delegated to
                                                                     // AdafruitGFX_helper
      }
      break;
    }
    # endif // if ADAGFX_ENABLE_GET_CONFIG_VALUE
  }
  return success;
}

#endif // USES_P116
