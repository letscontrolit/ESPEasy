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

      // uint8_t init = PCONFIG(0);

      // // if already configured take it from settings, else use default values (only for pin values)
      // if (init != 1)
      // {
      //   # ifdef ESP32

      //   if (Settings.InitSPI == 2) { // When using ESP32 H(ardware-)SPI
      //     TFT_Settings.address_tft_cs = TFT_CS_HSPI;
      //   }
      //   # endif // ifdef ESP32
      //   PIN(0) = TFT_Settings.address_tft_cs;
      //   PIN(1) = TFT_Settings.address_tft_dc;
      //   PIN(2) = TFT_Settings.address_tft_rst;
      // }
      // break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      // uint8_t init = PCONFIG(0);

      // // if already configured take it from settings, else use default values (only for pin values)
      // if (init == 1)
      // {
      //   TFT_Settings.address_tft_cs  = PIN(0);
      //   TFT_Settings.address_tft_dc  = PIN(1);
      //   TFT_Settings.address_tft_rst = PIN(2);
      // }

      if (PCONFIG(0) < 2) {
        P095_CONFIG_BUTTON_PIN    = -1;                                                   // No button connected
        P095_CONFIG_BACKLIGHT_PIN = P095_BACKLIGHT_PIN;
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_095)); // Values introduced in V2 settings
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_095));
      }

      addFormPinSelect(formatGpioName_output_optional(F("Backlight ")), F("p095_backlight"), P095_CONFIG_BACKLIGHT_PIN);
      addFormNumericBox(F("Backlight percentage"), F("p095_backpercentage"), P095_CONFIG_BACKLIGHT_PERCENT, 1, 100);
      addUnit(F("1-100%"));

      addFormPinSelect(F("Display button"), F("p095_button"), P095_CONFIG_BUTTON_PIN);

      addFormCheckBox(F("Inversed Logic"), F("p095_buttonInverse"), bitRead(P095_CONFIG_FLAGS, P095_CONFIG_FLAG_INVERT_BUTTON)); // Bit 1

      addFormNumericBox(F("Display Timeout"), F("p095_timer"), P095_CONFIG_DISPLAY_TIMEOUT);


      addFormSubHeader(F("Layout"));

      AdaGFXFormRotation(F("p095_rotate"), P095_CONFIG_ROTATION);

      AdaGFXFormTextPrintMode(F("p095_mode"), P095_CONFIG_FLAG_GET_MODE);

      addFormNumericBox(F("Font scaling"), F("p095_fontscale"), P095_CONFIG_FLAG_GET_FONTSCALE, 1, 10);
      addUnit(F("1x..10x"));

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

      AdaGFXFormTextColRowMode(F("p095_colrow"), bitRead(P095_CONFIG_FLAGS, P095_CONFIG_FLAG_USE_COL_ROW));

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
      PCONFIG(0) = 2; // mark config V2 as already saved (next time, will not convert 'invalid' values)
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

    case PLUGIN_INIT:
    {
      if (Settings.InitSPI != 0) {
        initPluginTaskData(event->TaskIndex,
                           new (std::nothrow) P095_data_struct(P095_CONFIG_ROTATION,
                                                               P095_CONFIG_FLAG_GET_FONTSCALE,
                                                               static_cast<AdaGFXTextPrintMode>(P095_CONFIG_FLAG_GET_MODE),
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

      // case PLUGIN_WRITE:
      // {
      // String tmpString = string;
      // String arguments = string;

      // String command;
      // String subcommand;

      // int argIndex = arguments.indexOf(',');

      // if (argIndex)
      // {
      //   command    = arguments.substring(0, argIndex);
      //   arguments  = arguments.substring(argIndex + 1);
      //   argIndex   = arguments.indexOf(',');
      //   subcommand = arguments.substring(0, argIndex);
      //   success    = true;

      //   tmpString += "<br/> command= " + command;
      //   tmpString += "<br/> arguments= " + arguments;
      //   tmpString += "<br/> argIndex= " + argIndex;
      //   tmpString += "<br/> subcommand= " + subcommand;


      //   if (command.equalsIgnoreCase(F("TFTCMD")))
      //   {
      //     if (subcommand.equalsIgnoreCase(F("ON")))
      //     {
      //       tft->sendCommand(ILI9341_DISPON);
      //     }
      //     else if (subcommand.equalsIgnoreCase(F("OFF")))
      //     {
      //       tft->sendCommand(ILI9341_DISPOFF);
      //     }
      //     else if (subcommand.equalsIgnoreCase(F("CLEAR")))
      //     {
      //       arguments = arguments.substring(argIndex + 1);
      //       tft->fillScreen(Plugin_095_ParseColor(arguments));
      //     }
      //     else if (subcommand.equalsIgnoreCase(F("INV")))
      //     {
      //       arguments = arguments.substring(argIndex + 1);
      //       tft->invertDisplay(arguments.toInt() == 1);
      //     }
      //     else if (subcommand.equalsIgnoreCase(F("ROT")))
      //     {
      //       ///control?cmd=tftcmd,rot,0
      //       // not working to verify
      //       arguments = arguments.substring(argIndex + 1);
      //       tft->setRotation(arguments.toInt() % 4);
      //     }
      //     else
      //     {
      //       success = false;
      //     }
      //   }
      //   else if (command.equalsIgnoreCase(F("TFT")))
      //   {
      //     # ifdef P095_USE_ADA_GRAPHICS
      //     success = gfxHelper->processCommand(AdaGFXparseTemplate(string, 128)); // Hand it over after replacing variables
      //     # else // ifdef P095_USE_ADA_GRAPHICS
      //     tmpString += "<br/> TFT  ";

      //     arguments = arguments.substring(argIndex + 1);
      //     String sParams[8];
      //     int    argCount = Plugin_095_StringSplit(arguments, ',', sParams, 8);

      //     for (int a = 0; a < argCount && a < 8; a++)
      //     {
      //       tmpString += "<br/> ARGS[" + String(a) + "]=" + sParams[a];
      //     }

      //     if (subcommand.equalsIgnoreCase(F("txt")))
      //     {
      //       tft->println(arguments); // write all pending cars
      //     }
      //     else if (subcommand.equalsIgnoreCase(F("txp")) && (argCount == 2))
      //     {
      //       tft->setCursor(sParams[0].toInt(), sParams[1].toInt());
      //     }
      //     else if (subcommand.equalsIgnoreCase(F("txc")) && ((argCount == 1) || (argCount == 2)))
      //     {
      //       if (argCount == 1) {
      //         tft->setTextColor(Plugin_095_ParseColor(sParams[0]));
      //       }
      //       else { // argCount=2
      //         tft->setTextColor(Plugin_095_ParseColor(sParams[0]), Plugin_095_ParseColor(sParams[1]));
      //       }
      //     }
      //     else if (subcommand.equalsIgnoreCase(F("txs")) && (argCount == 1))
      //     {
      //       tft->setTextSize(sParams[0].toInt());
      //     }
      //     #  ifdef PLUGIN_095_FONT_INCLUDED
      //     else if (subcommand.equalsIgnoreCase(F("font")) && (argCount == 1)) {
      //       if (sParams[0].equalsIgnoreCase(F("SEVENSEG24"))) {
      //         tft->setFont(&Seven_Segment24pt7b);
      //       } else if (sParams[0].equalsIgnoreCase(F("SEVENSEG18"))) {
      //         tft->setFont(&Seven_Segment18pt7b);
      //       } else if (sParams[0].equalsIgnoreCase(F("FREESANS"))) {
      //         tft->setFont(&FreeSans9pt7b);
      //       } else if (sParams[0].equalsIgnoreCase(F("DEFAULT"))) {
      //         tft->setFont();
      //       } else {
      //         success = false;
      //       }
      //     }
      //     #  endif // ifdef PLUGIN_095_FONT_INCLUDED
      //     else if (subcommand.equalsIgnoreCase(F("txtfull")) && (argCount >= 3) && (argCount <= 6))
      //     {
      //       switch (argCount)
      //       {
      //         case 3: // single text
      //           Plugin_095_printText(sParams[2].c_str(), sParams[0].toInt() - 1, sParams[1].toInt() - 1);
      //           break;

      //         case 4: // text + size
      //           Plugin_095_printText(sParams[3].c_str(), sParams[0].toInt() - 1, sParams[1].toInt() - 1, sParams[2].toInt());
      //           break;

      //         case 5: // text + size + color
      //           Plugin_095_printText(sParams[4].c_str(),
      //                                sParams[0].toInt() - 1,
      //                                sParams[1].toInt() - 1,
      //                                sParams[2].toInt(),
      //                                Plugin_095_ParseColor(sParams[3]));
      //           break;

      //         case 6: // text + size + color
      //           Plugin_095_printText(sParams[5].c_str(),
      //                                sParams[0].toInt() - 1,
      //                                sParams[1].toInt() - 1,
      //                                sParams[2].toInt(),
      //                                Plugin_095_ParseColor(sParams[3]),
      //                                Plugin_095_ParseColor(sParams[4]));
      //           break;
      //         default:
      //           success = false;
      //           break;
      //       }
      //     }
      //     else if (subcommand.equalsIgnoreCase(F("l")) && (argCount == 5))
      //     {
      //       tft->drawLine(sParams[0].toInt(), sParams[1].toInt(), sParams[2].toInt(), sParams[3].toInt(),
      // Plugin_095_ParseColor(sParams[4]));
      //     }
      //     else if (subcommand.equalsIgnoreCase(F("lh")) && (argCount == 3))
      //     {
      //       tft->drawFastHLine(0, sParams[0].toInt(), sParams[1].toInt(), Plugin_095_ParseColor(sParams[2]));
      //     }
      //     else if (subcommand.equalsIgnoreCase(F("lv")) && (argCount == 3))
      //     {
      //       tft->drawFastVLine(sParams[0].toInt(), 0, sParams[1].toInt(), Plugin_095_ParseColor(sParams[2]));
      //     }
      //     else if (subcommand.equalsIgnoreCase(F("r")) && (argCount == 5))
      //     {
      //       tft->drawRect(sParams[0].toInt(), sParams[1].toInt(), sParams[2].toInt(), sParams[3].toInt(),
      // Plugin_095_ParseColor(sParams[4]));
      //     }
      //     else if (subcommand.equalsIgnoreCase(F("rf")) && (argCount == 6))
      //     {
      //       tft->fillRect(sParams[0].toInt(), sParams[1].toInt(), sParams[2].toInt(), sParams[3].toInt(),
      // Plugin_095_ParseColor(sParams[5]));
      //       tft->drawRect(sParams[0].toInt(), sParams[1].toInt(), sParams[2].toInt(), sParams[3].toInt(),
      // Plugin_095_ParseColor(sParams[4]));
      //     }
      //     else if (subcommand.equalsIgnoreCase(F("c")) && (argCount == 4))
      //     {
      //       tft->drawCircle(sParams[0].toInt(), sParams[1].toInt(), sParams[2].toInt(), Plugin_095_ParseColor(sParams[3]));
      //     }
      //     else if (subcommand.equalsIgnoreCase(F("cf")) && (argCount == 5))
      //     {
      //       tft->fillCircle(sParams[0].toInt(), sParams[1].toInt(), sParams[2].toInt(), Plugin_095_ParseColor(sParams[4]));
      //       tft->drawCircle(sParams[0].toInt(), sParams[1].toInt(), sParams[2].toInt(), Plugin_095_ParseColor(sParams[3]));
      //     }
      //     else if (subcommand.equalsIgnoreCase(F("t")) && (argCount == 7))
      //     {
      //       tft->drawTriangle(sParams[0].toInt(),
      //                         sParams[1].toInt(),
      //                         sParams[2].toInt(),
      //                         sParams[3].toInt(),
      //                         sParams[4].toInt(),
      //                         sParams[5].toInt(),
      //                         Plugin_095_ParseColor(sParams[6]));
      //     }
      //     else if (subcommand.equalsIgnoreCase(F("tf")) && (argCount == 8))
      //     {
      //       tft->fillTriangle(sParams[0].toInt(),
      //                         sParams[1].toInt(),
      //                         sParams[2].toInt(),
      //                         sParams[3].toInt(),
      //                         sParams[4].toInt(),
      //                         sParams[5].toInt(),
      //                         Plugin_095_ParseColor(sParams[7]));
      //       tft->drawTriangle(sParams[0].toInt(),
      //                         sParams[1].toInt(),
      //                         sParams[2].toInt(),
      //                         sParams[3].toInt(),
      //                         sParams[4].toInt(),
      //                         sParams[5].toInt(),
      //                         Plugin_095_ParseColor(sParams[6]));
      //     }
      //     else if (subcommand.equalsIgnoreCase(F("rr")) && (argCount == 6))
      //     {
      //       tft->drawRoundRect(sParams[0].toInt(),
      //                          sParams[1].toInt(),
      //                          sParams[2].toInt(),
      //                          sParams[3].toInt(),
      //                          sParams[4].toInt(),
      //                          Plugin_095_ParseColor(sParams[5]));
      //     }
      //     else if (subcommand.equalsIgnoreCase(F("rrf")) && (argCount == 7))
      //     {
      //       tft->fillRoundRect(sParams[0].toInt(),
      //                          sParams[1].toInt(),
      //                          sParams[2].toInt(),
      //                          sParams[3].toInt(),
      //                          sParams[4].toInt(),
      //                          Plugin_095_ParseColor(sParams[6]));
      //       tft->drawRoundRect(sParams[0].toInt(),
      //                          sParams[1].toInt(),
      //                          sParams[2].toInt(),
      //                          sParams[3].toInt(),
      //                          sParams[4].toInt(),
      //                          Plugin_095_ParseColor(sParams[5]));
      //     }
      //     else if (subcommand.equalsIgnoreCase(F("px")) && (argCount == 3))
      //     {
      //       tft->drawPixel(sParams[0].toInt(), sParams[1].toInt(), Plugin_095_ParseColor(sParams[2]));
      //     }
      //     else
      //     {
      //       success = false;
      //     }
      //     # endif // ifdef P095_USE_ADA_GRAPHICS
      //   }
      //   else {
      //     success = false;
      //   }
      // }
      // else
      // {
      //   // invalid arguments
      //   success = false;
      // }

      // if (!success)
      // {
      //   if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      //     addLog(LOG_LEVEL_INFO, F("Fail to parse command correctly; please check API documentation"));
      //     String log2 = F("Parsed command = \"");
      //     log2 += string;
      //     log2 += F("\"");
      //     addLog(LOG_LEVEL_INFO, log2);
      //   }
      // }
      // else
      // {
      //   String log;
      //   log.reserve(110);             // Prevent re-allocation
      //   # ifdef P095_USE_ADA_GRAPHICS // TODO Restore original message!
      //   log = F("P095-AdaGFX : WRITE = ");
      //   # else // ifdef P095_USE_ADA_GRAPHICS
      //   log = F("P095-ILI9341 : WRITE = ");
      //   # endif // ifdef P095_USE_ADA_GRAPHICS
      //   log += tmpString;
      //   SendStatus(event, log); // Reply (echo) to sender. This will print message on browser.
      // }
      //   break;
      // }
  }


  return success;
}

#endif // USES_P095
