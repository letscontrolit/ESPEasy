#include "_Plugin_Helper.h"
#ifdef USES_P095

// #######################################################################################################
// #################################### Plugin 095: ILI9341 TFT 2.4inches display #################################
// #######################################################################################################

# define PLUGIN_095
# define PLUGIN_ID_095         95
# define PLUGIN_NAME_095       "Display - TFT 2.4 inches ILI9341 [TESTING]"
# define PLUGIN_VALUENAME1_095 "CursorX"
# define PLUGIN_VALUENAME2_095 "CursorY"
# define PLUGIN_095_MAX_DISPLAY 1


# if !defined(LIMIT_BUILD_SIZE) && !defined(PLUGIN_095_FONT_INCLUDED)
  #  define PLUGIN_095_FONT_INCLUDED // enable to use fonts in this plugin
# endif // if !defined(LIMIT_BUILD_SIZE) && !defined(PLUGIN_095_FONT_INCLUDED)

/**
 * Changelog:
 * 2021-11-16 tonhuisman: Add support for PLUGIN_GET_DISPLAY_PARAMETERS, removed commented old source
 * 2021-08-17 tonhuisman: Reformatted source using Uncrustify, small cleanups
 * 2021-08-16 tonhuisman: Initial refactoring into the use of AdafruitGFX_helper
 * 2020-08-29 tonhuisman: Removed TS (Touchscreen) related stuff, XPT2046 will be a separate plugin
 *                        Changed initial text from '--cdt--' to 'ESPEasy'
 * 2020-08 tonhuisman: SPI improvements
 * 2020-04 TD-er: Pull plugin into main repository and rework according to new plugin standards
 * 2019 Jean-Michel Decoret: Initial plugin work
 */
/* README.MD

 ## INTRO

   This plugin allow to control a TFT screen (ILI9341) through HTTP API

 ## Environment
   Tested with WEMOS D1 Mini Pro and Wemos TDFT 2.4
   Tested with ESPEasy 2.4.2  -tag mega-201902225)

   TFT Shield : https://docs.wemos.cc/en/latest/d1_mini_shiled/tft_2_4.html
   Price : ~ 5.40€/$ (https://fr.aliexpress.com/item/32919729730.html)


 ## Dependencies
   Plugin lib_deps = Adafruit GFX, Adafruit ILI9341

 ## API Documentation

   This plugin is controlled by HTTP API, ie : http://<espeasy_ip>/control?cmd=tft,tx,HelloWorld

 | command | details | description |
 |-----|-----|-----|
 | tft | `TFT,<tft_subcommand>,....` | Draw line, rect, circle, triangle and text |
 |tftcmd | `TFTCMD,<tftcmd_subcommand>` | Control the screen (on, off, clear,..) |

   TFT Subcommands:

 | TFT Subcommands | details | description |
 |-----|-----|-----|
 | txt | txt,<text> | Write simple text (use last position, color and size) |
 | txp | txp,<X>,<Y> | Set text position (move the cursor) |
 | txc | txc,<foreColor>,<backgroundColor> | Set text color (background is transparent if not provided |
 | txs | txs,<SIZE> | Set text size |
 | txtfull | txtfull,<row>,<col>,<size=1>,<foreColor=white>,<backColor=black>,<text> | Write text with all options |
 | l | l,<x1>,<y1>,<2>,<y2>,<color> | Draw a simple line |
 | lh | lh,<y>,<width>,<color> | Draw an horizontal line (width = Line width in pixels (positive = right of first point, negative = point of
 |first corner). |
 | lv | lv,<x>,<height>,<color> | Draw a vertical line (height= Line height in pixels (positive = below first point, negative = above first
 |point).|
 | r | r,<x>,<y>,<width>,<height>,<color> | Draw a rectangle |
 | rf | rf,<x>,<y>,<width>,<height>,<bordercolor>,<innercolor> | Draw a filled rectangle |
 | c | c,<x>,<y>,<radius>,<color> | Draw a circle |
 | cf | cf,<x>,<y>,<radius>,<bordercolor>,<innercolor> | Draw a filled circle |
 | t | t,<x1>,<y1>,<x2>,<y2>,<x3>,<y3>,<color>| Draw a triangle |
 | tf | tf,<x1>,<y1>,<x2>,<y2>,<x3>,<y3>,<bordercolor>,<innercolor> | Draw a filled triangle |
 | rr | rr,<x>,<y>,<width>,<height>,<corner_radius>,<color> | Draw a round rectangle |
 | rrf | rrf,<x>,<y>,<width>,<height>,<corner_radius>,<bordercolor>,<innercolor> | Draw a filled round rectangle |
 | px | px,<x>,<y>,<color> | Print a single pixel |
 | font| font,<fontname>  | Switch to font - SEVENSEG24, SEVENSEG18, FREESANS, DEFAULT |

   TFTCMD Subcommands:

 | TFT Subcommands | details | description |
 |-----|-----|-----|
 | on | on | Display ON |
 | off | off | Display OFF |
 | clear | clear,<color> | Clear display |
 | inv | inv,<value> | Invert the dispaly (value:0 normal display, 1 inverted display) |
 | rot | rot,<value> | Rotate display (value from 0 to 3 inclusive) |


   Examples:

        Write Text :
                http://<espeasy_ip>/control?cmd=tft,txtfull,0,0,HelloWorld

        Write Text another place:
                http://<espeasy_ip>/control?cmd=tft,txtfull,100,40,HelloWorld

        Write bigger Text :
                http://<espeasy_ip>/control?cmd=tft,txtfull,0,0,3,HelloWorld

        Write RED Text :
                http://<espeasy_ip>/control?cmd=tft,txtfull,0,0,3,HelloWorld

        Write RED Text (size is 1):
                http://<espeasy_ip>/control?cmd=tft,txtfull,0,0,1,RED,HelloWorld

        Write RED Text on YELLOW background (size is 1):
                http://<espeasy_ip>/control?cmd=tft,txtfull,0,0,1,RED,YELLOW,HelloWorld

        Switch display ON
                http://<espeasy_ip>/control?cmd=tftcmd,on

        Switch display OFF
                http://<espeasy_ip>/control?cmd=tftcmd,off

        Clear whole display
                http://<espeasy_ip>/control?cmd=tftcmd,clear

        Clear GREEN whole display
                http://<espeasy_ip>/control?cmd=tftcmd,clear,green
 */

# include "src/PluginStructs/P095_data_struct.h"


boolean Plugin_095(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_095;
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
      success                                = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string  = F(PLUGIN_NAME_095);
      success = true;
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_095));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_095));
      success = true;
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      event->String1 = formatGpioName_output(F("TFT CS"));
      event->String2 = formatGpioName_output(F("TFT DC"));
      event->String3 = formatGpioName_output(F("TFT RST"));
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      # ifdef ESP32

      if (Settings.InitSPI == 2) { // When using ESP32 H(ardware-)SPI
        PIN(0) = P095_TFT_CS_HSPI;
      } else {
        PIN(0) = P095_TFT_CS;
      }
      # else // ifdef ESP32
      PIN(0) = P095_TFT_CS;
      # endif // ifdef ESP32
      PIN(1)                        = P095_TFT_DC;
      PIN(2)                        = P095_TFT_RST;
      P095_CONFIG_BUTTON_PIN        = -1;  // No button connected
      P095_CONFIG_BACKLIGHT_PIN     = P095_BACKLIGHT_PIN;
      P095_CONFIG_BACKLIGHT_PERCENT = 100; // Percentage backlight

      uint32_t lSettings = 0;

      set4BitToUL(lSettings, P095_CONFIG_FLAG_FONTSCALE, 1);

      // set4BitToUL(lSettings, P095_CONFIG_FLAG_CMD_TRIGGER, 0); // Default trigger on tft
      P095_CONFIG_FLAGS = lSettings;

      P095_CONFIG_COLORS = ADAGFX_WHITE | (ADAGFX_BLACK << 16);

      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      if (P095_CONFIG_VERSION < 2) {
        P095_CONFIG_BUTTON_PIN    = -1;                                                   // No button connected
        P095_CONFIG_BACKLIGHT_PIN = P095_BACKLIGHT_PIN;
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_095)); // Values introduced in V2 settings
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_095));
      }

      AdaGFXFormBacklight(F("p095_backlight"), P095_CONFIG_BACKLIGHT_PIN,
                          F("p095_backpercentage"), P095_CONFIG_BACKLIGHT_PERCENT);

      AdaGFXFormDisplayButton(F("p095_button"), P095_CONFIG_BUTTON_PIN,
                              F("p095_buttonInverse"), bitRead(P095_CONFIG_FLAGS, P095_CONFIG_FLAG_INVERT_BUTTON),
                              F("p095_timer"), P095_CONFIG_DISPLAY_TIMEOUT);

      addFormSubHeader(F("Layout"));

      AdaGFXFormRotation(F("p095_rotate"), P095_CONFIG_ROTATION);

      AdaGFXFormTextPrintMode(F("p095_mode"), P095_CONFIG_FLAG_GET_MODE);

      AdaGFXFormFontScaling(F("p095_fontscale"), P095_CONFIG_FLAG_GET_FONTSCALE);

      addFormCheckBox(F("Clear display on exit"), F("p095_clearOnExit"), bitRead(P095_CONFIG_FLAGS, P095_CONFIG_FLAG_CLEAR_ON_EXIT));

      {
        const __FlashStringHelper *commandTriggers[] = { // Be sure to use all options available in the enum (except MAX)!
          P095_CommandTrigger_toString(P095_CommandTrigger::tft),
          P095_CommandTrigger_toString(P095_CommandTrigger::ili9341)
        };
        const int commandTriggerOptions[] = {
          static_cast<int>(P095_CommandTrigger::tft),
          static_cast<int>(P095_CommandTrigger::ili9341)
        };
        addFormSelector(F("Write Command trigger"),
                        F("p095_commandtrigger"),
                        static_cast<int>(P095_CommandTrigger::MAX),
                        commandTriggers,
                        commandTriggerOptions,
                        P095_CONFIG_FLAG_GET_CMD_TRIGGER);
        addFormNote(F("Select the command that is used to handle commands for this display."));
      }

      // Inverted state!
      addFormCheckBox(F("Wake display on receiving text"), F("p095_NoDisplay"), !bitRead(P095_CONFIG_FLAGS, P095_CONFIG_FLAG_NO_WAKE));
      addFormNote(F("When checked, the display wakes up at receiving remote updates."));

      AdaGFXFormTextColRowMode(F("p095_colrow"), bitRead(P095_CONFIG_FLAGS, P095_CONFIG_FLAG_USE_COL_ROW) == 1);

      AdaGFXFormOnePixelCompatibilityOption(F("p095_compat"), !bitRead(P095_CONFIG_FLAGS, P095_CONFIG_FLAG_COMPAT_P095)); // Inverse

      AdaGFXFormTextBackgroundFill(F("p095_backfill"), bitRead(P095_CONFIG_FLAGS, P095_CONFIG_FLAG_BACK_FILL) == 0);      // Inverse

      addFormSubHeader(F("Content"));

      if (P095_CONFIG_COLORS == 0) { // For migrating from older release task settings
        P095_CONFIG_COLORS = ADAGFX_WHITE | (ADAGFX_BLACK << 16);
      }
      AdaGFXFormForeAndBackColors(F("p095_foregroundcolor"),
                                  P095_CONFIG_GET_COLOR_FOREGROUND,
                                  F("p095_backgroundcolor"),
                                  P095_CONFIG_GET_COLOR_BACKGROUND);

      String strings[P095_Nlines];
      LoadCustomTaskSettings(event->TaskIndex, strings, P095_Nlines, 0);

      String   line; // Default reserved length is plenty
      uint16_t remain = DAT_TASKS_CUSTOM_SIZE;

      for (uint8_t varNr = 0; varNr < P095_Nlines; varNr++) {
        line  = F("Line ");
        line += (varNr + 1);
        addFormTextBox(line, getPluginCustomArgName(varNr), strings[varNr], P095_Nchars);
        remain -= (strings[varNr].length() + 1);
      }
      String remainStr;
      remainStr.reserve(15);
      remainStr  = F("Remaining: ");
      remainStr += remain;
      addUnit(remainStr);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P095_CONFIG_VERSION = 2; // mark config V2 as already saved (next time, will not convert 'invalid' values)
      // PIN(0)..(2) are already set

      P095_CONFIG_ROTATION          = getFormItemInt(F("p095_rotate"));
      P095_CONFIG_BUTTON_PIN        = getFormItemInt(F("p095_button"));
      P095_CONFIG_DISPLAY_TIMEOUT   = getFormItemInt(F("p095_timer"));
      P095_CONFIG_BACKLIGHT_PIN     = getFormItemInt(F("p095_backlight"));
      P095_CONFIG_BACKLIGHT_PERCENT = getFormItemInt(F("p095_backpercentage"));

      uint32_t lSettings = 0;
      bitWrite(lSettings, P095_CONFIG_FLAG_NO_WAKE,       !isFormItemChecked(F("p095_NoDisplay")));    // Bit 0 NoDisplayOnReceivingText,
                                                                                                       // reverse logic, default=checked!
      bitWrite(lSettings, P095_CONFIG_FLAG_INVERT_BUTTON, isFormItemChecked(F("p095_buttonInverse"))); // Bit 1 buttonInverse
      bitWrite(lSettings, P095_CONFIG_FLAG_CLEAR_ON_EXIT, isFormItemChecked(F("p095_clearOnExit")));   // Bit 2 ClearOnExit
      bitWrite(lSettings, P095_CONFIG_FLAG_USE_COL_ROW,   isFormItemChecked(F("p095_colrow")));        // Bit 3 Col/Row addressing
      bitWrite(lSettings, P095_CONFIG_FLAG_COMPAT_P095,   !isFormItemChecked(F("p095_compat")));       // Bit 4 Compat_P095 (inv)
      bitWrite(lSettings, P095_CONFIG_FLAG_BACK_FILL,     !isFormItemChecked(F("p095_backfill")));     // Bit 5 Back fill text (inv)

      set4BitToUL(lSettings, P095_CONFIG_FLAG_CMD_TRIGGER, getFormItemInt(F("p095_commandtrigger")));  // Bit 8..11 Command trigger
      set4BitToUL(lSettings, P095_CONFIG_FLAG_FONTSCALE,   getFormItemInt(F("p095_fontscale")));       // Bit 12..15 Font scale
      set4BitToUL(lSettings, P095_CONFIG_FLAG_MODE,        getFormItemInt(F("p095_mode")));            // Bit 16..19 Text print mode
      P095_CONFIG_FLAGS = lSettings;

      String   color   = web_server.arg(F("p095_foregroundcolor"));
      uint16_t fgcolor = ADAGFX_WHITE;     // Default to white when empty

      if (!color.isEmpty()) {
        fgcolor = AdaGFXparseColor(color); // Reduce to rgb565
      }
      color = web_server.arg(F("p095_backgroundcolor"));
      uint16_t bgcolor = AdaGFXparseColor(color);

      P095_CONFIG_COLORS = fgcolor | (bgcolor << 16); // Store as a single setting

      String strings[P095_Nlines];
      String error;

      for (uint8_t varNr = 0; varNr < P095_Nlines; varNr++) {
        strings[varNr] = web_server.arg(getPluginCustomArgName(varNr));
      }

      error = SaveCustomTaskSettings(event->TaskIndex, strings, P095_Nlines, 0);

      if (error.length() > 0) {
        addHtmlError(error);
      }

      success = true;
      break;
    }

    case PLUGIN_GET_DISPLAY_PARAMETERS:
    {
      event->Par1 = 240;                                           // X-resolution in pixels
      event->Par2 = 320;                                           // Y-resolution in pixels
      event->Par3 = P095_CONFIG_ROTATION;                          // Rotation (0..3: 0, 90, 180, 270 degrees)
      event->Par4 = static_cast<int>(AdaGFXColorDepth::FullColor); // Color depth

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      if (Settings.InitSPI != 0) {
        initPluginTaskData(event->TaskIndex,
                           new (std::nothrow) P095_data_struct(P095_CONFIG_ROTATION,
                                                               P095_CONFIG_FLAG_GET_FONTSCALE,
                                                               static_cast<AdaGFXTextPrintMode>(P095_CONFIG_FLAG_GET_MODE),
                                                               P095_CONFIG_BACKLIGHT_PIN,
                                                               P095_CONFIG_BACKLIGHT_PERCENT,
                                                               P095_CONFIG_DISPLAY_TIMEOUT,
                                                               P095_CommandTrigger_toString(static_cast<P095_CommandTrigger>(
                                                                                              P095_CONFIG_FLAG_GET_CMD_TRIGGER)),
                                                               P095_CONFIG_GET_COLOR_FOREGROUND,
                                                               P095_CONFIG_GET_COLOR_BACKGROUND,
                                                               bitRead(P095_CONFIG_FLAGS, P095_CONFIG_FLAG_BACK_FILL) == 0));
        P095_data_struct *P095_data = static_cast<P095_data_struct *>(getPluginTaskData(event->TaskIndex));

        if (nullptr != P095_data) {
          success = P095_data->plugin_init(event); // Start the display
        }
      } else {
        addLog(LOG_LEVEL_ERROR, F("ILI9341: SPI not enabled, init cancelled."));
      }
      break;
    }

    case PLUGIN_EXIT:
    {
      P095_data_struct *P095_data = static_cast<P095_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P095_data) {
        success = P095_data->plugin_exit(event); // Stop the display
      }
      break;
    }

    // Check more often for debouncing the button, when enabled
    case PLUGIN_FIFTY_PER_SECOND:
    {
      if (P095_CONFIG_BUTTON_PIN != -1) {
        P095_data_struct *P095_data = static_cast<P095_data_struct *>(getPluginTaskData(event->TaskIndex));

        if (nullptr == P095_data) {
          return success;
        }

        P095_data->registerButtonState(digitalRead(P095_CONFIG_BUTTON_PIN), bitRead(P095_CONFIG_FLAGS, P095_CONFIG_FLAG_INVERT_BUTTON));
        success = true;
      }
      break;
    }

    case PLUGIN_TEN_PER_SECOND:
    {
      P095_data_struct *P095_data = static_cast<P095_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P095_data) {
        success = P095_data->plugin_ten_per_second(event); // 10 per second actions
      }
      break;
    }

    case PLUGIN_ONCE_A_SECOND:
    {
      P095_data_struct *P095_data = static_cast<P095_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P095_data) {
        success = P095_data->plugin_once_a_second(event); // Once a second actions
      }
      break;
    }

    case PLUGIN_READ:
    {
      P095_data_struct *P095_data = static_cast<P095_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P095_data) {
        success = P095_data->plugin_read(event); // Read operation, redisplay the configured content
      }
      break;
    }

    case PLUGIN_WRITE:
    {
      P095_data_struct *P095_data = static_cast<P095_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P095_data) {
        success = P095_data->plugin_write(event, string); // Write operation, handle commands, mostly delegated to AdafruitGFX_helper
      }
      break;
    }
  }


  return success;
}

#endif // USES_P095
