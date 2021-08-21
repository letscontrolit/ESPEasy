#include "_Plugin_Helper.h"
#ifdef USES_P096

// #######################################################################################################
// #################################### Plugin 096: eInk display #################################
// #######################################################################################################

# define PLUGIN_096
# define PLUGIN_ID_096         96
# define PLUGIN_NAME_096       "Display - eInk with Lolin ePaper screen [TESTING]"
# define PLUGIN_VALUENAME1_096 "CursorX"
# define PLUGIN_VALUENAME2_096 "CursorY"

// #define PLUGIN_096_MAX_DISPLAY 1 // Unused

/* README.MD

 ## INTRO

   This plugin allow to control a eInk screen (ILI3897) through HTTP API

 ## Environment
   Tested with Lolin d32 pro and Wemos ePaper 2.13 shield
   Tested with ESPEasy 2.4.2  -tag mega-201902225)

   ePaper Shield : https://www.wemos.cc/en/latest/d1_mini_shiled/epd_2_13.html
   Price : ~ 10€/$ (https://fr.aliexpress.com/item/32981318996.html)


 ## Dependencies
   Plugin lib_deps = Adafruit GFX, LOLIN_EPD

 ## API Documentation

   This plugin is controlled by HTTP API, ie : http://<espeasy_ip>/control?cmd=epd,tx,HelloWorld

 | command | details | description |
 |-----|-----|-----|
 | epd | `EPD,<epd_subcommand>,....` | Draw line, rect, circle, triangle and text |
 |epdcmd | `EPDCMD,<epdcmd_subcommand>` | Control the screen (on, off, clear,..) |

   EPD Subcommands:

 | EPD Subcommands | details | description |
 |-----|-----|-----|
 | txt | txt,<text> | Write simple text (use last position, color and size) |
 | txp | txp,<X>,<Y> | Set text position (move the cursor) |
 | txc | txc,<foreColor>,<backgroundColor> | Set text color (background is transparent if not provided |
 | txz | txz,<X>,<Y>,<text> | Write text at position (move the cursor + print) |
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

   EPDCMD Subcommands:

 | EPD Subcommands | details | description |
 |-----|-----|-----|
 | clear | clear,<color> | Clear display |
 | deepsleep | deepsleep | Make screen go to sleep |
 | inv | inv,<value> | Invert the dispaly (value:0 normal display, 1 inverted display) |
 | rot | rot,<value> | Rotate display (value from 0 to 3 inclusive) |


   Examples:

        Write Text :
                http://<espeasy_ip>/control?cmd=epd,txtfull,0,0,HelloWorld

        Write Text another place:
                http://<espeasy_ip>/control?cmd=epd,txtfull,100,40,HelloWorld

        Write bigger Text :
                http://<espeasy_ip>/control?cmd=epd,txtfull,0,0,3,HelloWorld

        Write RED Text :
                http://<espeasy_ip>/control?cmd=epd,txtfull,0,0,3,HelloWorld

        Write RED Text (size is 1):
                http://<espeasy_ip>/control?cmd=epd,txtfull,0,0,1,RED,HelloWorld

        Write RED Text on YELLOW background (size is 1):
                http://<espeasy_ip>/control?cmd=epd,txtfull,0,0,1,RED,YELLOW,HelloWorld

        Clear whole display
                http://<espeasy_ip>/control?cmd=epdcmd,clear

        Deepsleep screen
                http://<espeasy_ip>/control?cmd=epdcmd,deepsleep

 */

// plugin dependency
# include "src/PluginStructs/P096_data_struct.h"

// #include <LOLIN_EPD.h>
// #include <Adafruit_GFX.h>
// #ifdef P096_USE_ADA_GRAPHICS
// #include "src/Helpers/AdafruitGFX_helper.h"
// #endif

# ifndef P096_USE_ADA_GRAPHICS

// declare functions for using default value parameters
void Plugin_096_printText(const char    *string,
                          int            X,
                          int            Y,
                          unsigned int   textSize = 1,
                          unsigned short color    = EPD_WHITE,
                          unsigned short bkcolor  = EPD_BLACK);
# endif // ifndef P096_USE_ADA_GRAPHICS

// Define the default values for both ESP32/lolin32 and D1 Mini
# ifdef ESP32

// for D32 Pro with EPD connector
  #  define EPD_CS 14
  #  define EPD_CS_HSPI 26 // when connected to Hardware-SPI GPIO-14 is already used
  #  define EPD_DC 27
  #  define EPD_RST 33     // can set to -1 and share with microcontroller Reset!
  #  define EPD_BUSY -1    // can set to -1 to not use a pin (will wait a fixed delay)
# else // ifdef ESP32

// for D1 Mini with shield connection
  #  define EPD_CS D0
  #  define EPD_DC D8
  #  define EPD_RST -1  // can set to -1 and share with microcontroller Reset!
  #  define EPD_BUSY -1 // can set to -1 to not use a pin (will wait a fixed delay)
# endif // ifdef ESP32


// // The setting structure
// struct Plugin_096_EPD_SettingStruct
// {
//   Plugin_096_EPD_SettingStruct()
//     : address_epd_cs(EPD_CS), address_epd_dc(EPD_DC), address_epd_rst(EPD_RST), address_epd_busy(EPD_BUSY), rotation(0), width(250),
// height(
//       122)
//   {}

//   uint8_t address_epd_cs;
//   uint8_t address_epd_dc;
//   uint8_t address_epd_rst;
//   uint8_t address_epd_busy;
//   uint8_t rotation;
//   int     width;
//   int     height;
// } EPD_Settings;

// // The display pointer
// LOLIN_IL3897 *eInkScreen                = nullptr;
// uint8_t plugin_096_sequence_in_progress = false;
// # ifdef P096_USE_ADA_GRAPHICS
// AdafruitGFX_helper *gfxHelper = nullptr;
// # endif // ifdef P096_USE_ADA_GRAPHICS

boolean Plugin_096(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_096;
      Device[deviceCount].Type               = DEVICE_TYPE_SPI3;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_NONE;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = false;
      # if P096_USE_EXTENDED_SETTINGS
      Device[deviceCount].ValueCount    = 2;
      Device[deviceCount].TimerOption   = true;
      Device[deviceCount].TimerOptional = true;
      # else // if P096_USE_EXTENDED_SETTINGS
      Device[deviceCount].ValueCount  = 0;
      Device[deviceCount].TimerOption = false;
      # endif // if P096_USE_EXTENDED_SETTINGS
      Device[deviceCount].SendDataOption = false;

      success = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string  = F(PLUGIN_NAME_096);
      success = true;
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_096));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_096));
      success = true;
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      event->String1 = formatGpioName_output(F("EPD CS"));
      event->String2 = formatGpioName_output(F("EPD DC"));
      event->String3 = formatGpioName_output(F("EPD RST"));
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      PIN(0) = EPD_CS;
      # ifdef ESP32

      if (Settings.InitSPI == 2) { // When using ESP32 H(ardware-)SPI
        PIN(0) = EPD_CS_HSPI;
      }
      # endif // ifdef ESP32
      PIN(1) = EPD_DC;
      PIN(2) = EPD_RST;
      PIN(3) = EPD_BUSY;
      # if P096_USE_EXTENDED_SETTINGS

      P096_CONFIG_COLORS = static_cast<uint16_t>(AdaGFXMonoDuoQuadColors::ADAGFXEPD_BLACK) | // Default to dark on white (paper) colors
                           (static_cast<uint16_t>(AdaGFXMonoDuoQuadColors::ADAGFXEPD_WHITE) << 16);

      uint32_t lSettings = 0;
      set4BitToUL(lSettings, P096_CONFIG_FLAG_COLORDEPTH, static_cast<uint8_t>(AdaGFXColorDepth::Monochrome)); // Bit 20..23 Color depth
      P096_CONFIG_FLAGS = lSettings;
      # else // if P096_USE_EXTENDED_SETTINGS
      P096_CONFIG_WIDTH  = 250;
      P096_CONFIG_HEIGHT = 122;
      # endif // if P096_USE_EXTENDED_SETTINGS
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormPinSelect(formatGpioName_output(F("EPD BUSY")), F("p096_epd_busy"), PIN(3));

      # if P096_USE_EXTENDED_SETTINGS

      if (P096_CONFIG_VERSION < 2) {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_096)); // Values introduced in V2 settings
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_096));
      }

      {
        const __FlashStringHelper *options4[] = {
          EPD_type_toString(EPD_type_e::EPD_IL3897),
          EPD_type_toString(EPD_type_e::EPD_UC8151D),
          EPD_type_toString(EPD_type_e::EPD_SSD1680) };
        const int optionValues4[] = {
          static_cast<int>(EPD_type_e::EPD_IL3897),
          static_cast<int>(EPD_type_e::EPD_UC8151D),
          static_cast<int>(EPD_type_e::EPD_SSD1680) };
        addFormSelector(F("eInk display model"),
                        F("p096_type"),
                        static_cast<int>(EPD_type_e::EPD_MAX),
                        options4,
                        optionValues4,
                        P096_CONFIG_FLAG_GET_DISPLAYTYPE);
      }

      addFormSubHeader(F("Layout"));
      # endif // if P096_USE_EXTENDED_SETTINGS

      # ifdef P096_USE_ADA_GRAPHICS
      AdaGFXFormRotation(F("p096_rotate"), P096_CONFIG_ROTATION);
      # else // ifdef P096_USE_ADA_GRAPHICS
      {
        const __FlashStringHelper *options2[4] = { F("Normal"), F("+90&deg;"), F("+180&deg;"), F("+270&deg;") };
        int optionValues2[4]                   = { 0, 1, 2, 3 };
        addFormSelector(F("Rotation"), F("p096_rotate"), 4, options2, optionValues2, P096_CONFIG_ROTATION);
      }
      # endif // ifdef P096_USE_ADA_GRAPHICS

      # if !P096_USE_EXTENDED_SETTINGS

      // Width and Height no longer needed as ythat's define by the Display Type
      uint16_t width_ = P096_CONFIG_WIDTH;

      if (width_ == 0) {
        width_ = 250; // default value
      }
      addFormNumericBox(F("Width (px)"), F("p096_width"), width_, 1, 65535);

      uint16_t height_ = P096_CONFIG_HEIGHT;

      if (height_ == 0) {
        height_ = 122; // default value
      }
      addFormNumericBox(F("Height (px)"), F("p096_height"), height_, 1, 65535);
      # endif // if !P096_USE_EXTENDED_SETTINGS

      # if P096_USE_EXTENDED_SETTINGS
      {
        const __FlashStringHelper *colorDepths[] = { // Be sure to use all options needed
          getAdaGFXColorDepth(AdaGFXColorDepth::Monochrome),
          getAdaGFXColorDepth(AdaGFXColorDepth::Duochrome),
          getAdaGFXColorDepth(AdaGFXColorDepth::Quadrochrome),
          #  if ADAGFX_SUPPORT_7COLOR
          getAdaGFXColorDepth(AdaGFXColorDepth::Septochrome)
          #  endif // if ADAGFX_SUPPORT_7COLOR
        };
        const int colorDepthOptions[] = {
          static_cast<int>(AdaGFXColorDepth::Monochrome),
          static_cast<int>(AdaGFXColorDepth::Duochrome),
          static_cast<int>(AdaGFXColorDepth::Quadrochrome),
          #  if ADAGFX_SUPPORT_7COLOR
          static_cast<int>(AdaGFXColorDepth::Septochrome)
          #  endif // if ADAGFX_SUPPORT_7COLOR
        };

        if (P096_CONFIG_FLAG_GET_COLORDEPTH == 0) { // Enum doesn't have 0
          uint32_t lSettings = 0;
          set4BitToUL(lSettings, P096_CONFIG_FLAG_COLORDEPTH, static_cast<uint8_t>(AdaGFXColorDepth::Monochrome)); // Bit 20..23 Color depth
          P096_CONFIG_FLAGS = lSettings;
        }
        addFormSelector(F("Greyscale levels"),
                        F("p096_colorDepth"),
                        ADAGFX_MONOCOLORS_COUNT,
                        colorDepths,
                        colorDepthOptions,
                        P096_CONFIG_FLAG_GET_COLORDEPTH);
      }

      AdaGFXFormTextPrintMode(F("p096_mode"), P096_CONFIG_FLAG_GET_MODE);

      addFormNumericBox(F("Font scaling"), F("p096_fontscale"), P096_CONFIG_FLAG_GET_FONTSCALE, 1, 10);
      addUnit(F("1x..10x"));

      {
        const __FlashStringHelper *commandTriggers[] = { // Be sure to use all options available in the enum (except MAX)!
          P096_CommandTrigger_toString(P096_CommandTrigger::epd),
          P096_CommandTrigger_toString(P096_CommandTrigger::eInk),
          P096_CommandTrigger_toString(P096_CommandTrigger::ePaper),
          P096_CommandTrigger_toString(P096_CommandTrigger::il3897),
          P096_CommandTrigger_toString(P096_CommandTrigger::uc8151d),
          P096_CommandTrigger_toString(P096_CommandTrigger::ssd1680)
        };
        const int commandTriggerOptions[] = {
          static_cast<int>(P096_CommandTrigger::epd),
          static_cast<int>(P096_CommandTrigger::eInk),
          static_cast<int>(P096_CommandTrigger::ePaper),
          static_cast<int>(P096_CommandTrigger::il3897),
          static_cast<int>(P096_CommandTrigger::uc8151d),
          static_cast<int>(P096_CommandTrigger::ssd1680)
        };
        addFormSelector(F("Write Command trigger"),
                        F("p096_commandtrigger"),
                        static_cast<int>(P096_CommandTrigger::MAX),
                        commandTriggers,
                        commandTriggerOptions,
                        P096_CONFIG_FLAG_GET_CMD_TRIGGER);
        addFormNote(F("Select the command that is used to handle commands for this display."));
      }

      addFormCheckBox(F("Text Coordinates in col/row"), F("p096_colrow"), bitRead(P096_CONFIG_FLAGS, P096_CONFIG_FLAG_USE_COL_ROW));
      addFormNote(F("Unchecked: Coordinates in pixels. Applies only to 'txp', 'txz' and 'txtfull' subcommands."));

      addFormCheckBox(F("Use -1px offset for txp &amp; txtfull"),
                      F("p096_compat"),
                      !bitRead(P096_CONFIG_FLAGS, P096_CONFIG_FLAG_COMPAT_P096));
      addFormNote(F("This is for compatibility with the original plugin implementation."));

      addFormSubHeader(F("Content"));

      if (P096_CONFIG_COLORS == 0) { // For migrating from older release task settings
        P096_CONFIG_COLORS = static_cast<uint16_t>(AdaGFXMonoDuoQuadColors::ADAGFXEPD_WHITE) |
                             (static_cast<uint16_t>(AdaGFXMonoDuoQuadColors::ADAGFXEPD_BLACK) << 16);
      }
      AdaGFXFormForeAndBackColors(F("p096_foregroundcolor"),
                                  P096_CONFIG_GET_COLOR_FOREGROUND,
                                  F("p096_backgroundcolor"),
                                  P096_CONFIG_GET_COLOR_BACKGROUND,
                                  static_cast<AdaGFXColorDepth>(P096_CONFIG_FLAG_GET_COLORDEPTH));

      String strings[P096_Nlines];
      LoadCustomTaskSettings(event->TaskIndex, strings, P096_Nlines, 0);

      String   line; // Default reserved length is plenty
      uint16_t remain = DAT_TASKS_CUSTOM_SIZE;

      for (uint8_t varNr = 0; varNr < P096_Nlines; varNr++) {
        line  = F("Line ");
        line += (varNr + 1);
        addFormTextBox(line, getPluginCustomArgName(varNr), strings[varNr], P096_Nchars);
        remain -= (strings[varNr].length() + 1);
      }
      String remainStr;
      remainStr.reserve(15);
      remainStr  = F("Remaining: ");
      remainStr += remain;
      addUnit(remainStr);

      # endif // if P096_USE_EXTENDED_SETTINGS

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      # if P096_USE_EXTENDED_SETTINGS
      P096_CONFIG_VERSION = 2; // mark config V2 as already saved (next time, will not use default values)
      # else // if P096_USE_EXTENDED_SETTINGS
      P096_CONFIG_VERSION = 1; // mark config as already saved (next time, will not use default values)
      # endif // if P096_USE_EXTENDED_SETTINGS

      // PIN(0)..(2) are already set
      PIN(3)               = getFormItemInt(F("p096_epd_busy"));
      P096_CONFIG_ROTATION = getFormItemInt(F("p096_rotate"));
      # if !P096_USE_EXTENDED_SETTINGS
      P096_CONFIG_WIDTH  = getFormItemInt(F("p096_width"));
      P096_CONFIG_HEIGHT = getFormItemInt(F("p096_height"));
      # endif // if !P096_USE_EXTENDED_SETTINGS

      # if P096_USE_EXTENDED_SETTINGS

      uint32_t lSettings = 0;

      bitWrite(lSettings, P096_CONFIG_FLAG_USE_COL_ROW, isFormItemChecked(F("p096_colrow")));         // Bit 3 Col/Row addressing
      bitWrite(lSettings, P096_CONFIG_FLAG_COMPAT_P096, !isFormItemChecked(F("p096_compat")));        // Bit 4 Compat_P096 (inv)

      set4BitToUL(lSettings, P096_CONFIG_FLAG_CMD_TRIGGER, getFormItemInt(F("p096_commandtrigger"))); // Bit 8..11 Command trigger
      set4BitToUL(lSettings, P096_CONFIG_FLAG_FONTSCALE,   getFormItemInt(F("p096_fontscale")));      // Bit 12..15 Font scale
      set4BitToUL(lSettings, P096_CONFIG_FLAG_MODE,        getFormItemInt(F("p096_mode")));           // Bit 16..19 Text print mode
      set4BitToUL(lSettings, P096_CONFIG_FLAG_COLORDEPTH,  getFormItemInt(F("p096_colorDepth")));     // Bit 20..23 Color depth
      set4BitToUL(lSettings, P096_CONFIG_FLAG_DISPLAYTYPE, getFormItemInt(F("p096_type")));           // Bit 24..27 Hardwaretype

      P096_CONFIG_FLAGS = lSettings;

      String   color   = web_server.arg(F("p096_foregroundcolor"));
      uint16_t fgcolor = static_cast<uint16_t>(AdaGFXMonoDuoQuadColors::ADAGFXEPD_BLACK);                  // Default to white when empty

      if (!color.isEmpty()) {
        fgcolor = AdaGFXparseColor(color, static_cast<AdaGFXColorDepth>(P096_CONFIG_FLAG_GET_COLORDEPTH)); // Reduce to rgb565
      }
      color = web_server.arg(F("p096_backgroundcolor"));
      uint16_t bgcolor = AdaGFXparseColor(color);

      P096_CONFIG_COLORS = fgcolor | (bgcolor << 16); // Store as a single setting

      String strings[P096_Nlines];
      String error;

      for (uint8_t varNr = 0; varNr < P096_Nlines; varNr++) {
        strings[varNr] = web_server.arg(getPluginCustomArgName(varNr));
      }

      error = SaveCustomTaskSettings(event->TaskIndex, strings, P096_Nlines, 0);

      if (error.length() > 0) {
        addHtmlError(error);
      }
      # endif // if P096_USE_EXTENDED_SETTINGS

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      if (Settings.InitSPI != 0) {
        initPluginTaskData(event->TaskIndex,
                           # if P096_USE_EXTENDED_SETTINGS
                           new (std::nothrow) P096_data_struct(static_cast<EPD_type_e>(P096_CONFIG_FLAG_GET_DISPLAYTYPE),
                                                               P096_CONFIG_ROTATION,
                                                               P096_CONFIG_FLAG_GET_FONTSCALE,
                                                               static_cast<AdaGFXTextPrintMode>(P096_CONFIG_FLAG_GET_MODE),
                                                               P096_CommandTrigger_toString(static_cast<P096_CommandTrigger>(
                                                                                              P096_CONFIG_FLAG_GET_CMD_TRIGGER)),
                                                               P096_CONFIG_GET_COLOR_FOREGROUND,
                                                               P096_CONFIG_GET_COLOR_BACKGROUND,
                                                               static_cast<AdaGFXColorDepth>(P096_CONFIG_FLAG_GET_COLORDEPTH))
                           # else // if P096_USE_EXTENDED_SETTINGS
                           new (std::nothrow) P096_data_struct(static_cast<EPD_type_e>(P096_CONFIG_FLAG_GET_DISPLAYTYPE),
                                                               P096_CONFIG_WIDTH,
                                                               P096_CONFIG_HEIGHT,
                                                               P096_CONFIG_ROTATION,
                                                               P096_CONFIG_FLAG_GET_FONTSCALE,
                                                               AdaGFXTextPrintMode::ContinueToNextLine,
                                                               F("epd"))
                           # endif // if P096_USE_EXTENDED_SETTINGS
                           );
        P096_data_struct *P096_data = static_cast<P096_data_struct *>(getPluginTaskData(event->TaskIndex));

        if (nullptr != P096_data) {
          success = P096_data->plugin_init(event); // Start the display
        }
      } else {
        addLog(LOG_LEVEL_ERROR, F("EPD  : SPI not enabled, init cancelled."));
      }
      break;

      // uint8_t init = PCONFIG(0);

      // // if already configured take it from settings, else use default values (only for pin values)
      // if (init != 1)
      // {
      //   PIN(0) = EPD_Settings.address_epd_cs;
      //   PIN(1) = EPD_Settings.address_epd_dc;
      //   PIN(2) = EPD_Settings.address_epd_rst;
      //   PIN(3) = EPD_Settings.address_epd_busy;
      // }

      // EPD_Settings.address_epd_cs   = PIN(0);
      // EPD_Settings.address_epd_dc   = PIN(1);
      // EPD_Settings.address_epd_rst  = PIN(2);
      // EPD_Settings.address_epd_busy = PIN(3);
      // EPD_Settings.rotation         = P096_CONFIG_ROTATION;
      // EPD_Settings.width            = P096_CONFIG_WIDTH;
      // EPD_Settings.height           = P096_CONFIG_HEIGHT;

      // eInkScreen = new LOLIN_IL3897(EPD_Settings.width,
      //                               EPD_Settings.height,
      //                               EPD_Settings.address_epd_dc,
      //                               EPD_Settings.address_epd_rst,
      //                               EPD_Settings.address_epd_cs,
      //                               EPD_Settings.address_epd_busy); // hardware SPI
      // plugin_096_sequence_in_progress = false;
      // # ifdef P096_USE_ADA_GRAPHICS

      // if (nullptr != eInkScreen) {
      //   gfxHelper = new (std::nothrow) AdafruitGFX_helper(eInkScreen,
      //                                                     F("epd"),
      //                                                     PCONFIG(2),
      //                                                     PCONFIG(3),
      //                                                     AdaGFXColorDepth::Monochrome,
      //                                                     AdaGFXTextPrintMode::ContinueToNextLine,
      //                                                     3,
      //                                                     static_cast<uint16_t>(AdaGFXMonoDuoQuadColors::ADAGFXEPD_BLACK),
      //                                                     static_cast<uint16_t>(AdaGFXMonoDuoQuadColors::ADAGFXEPD_WHITE));

      //   if (nullptr != gfxHelper) {
      //     // gfxHelper->setColumnRowMode(bitRead(P096_CONFIG_FLAGS, P096_CONFIG_FLAG_USE_COL_ROW));
      //   }
      // }
      // # endif // ifdef P096_USE_ADA_GRAPHICS
      // eInkScreen->begin();
      // eInkScreen->clearBuffer();

      // eInkScreen->setTextColor(EPD_BLACK);
      // eInkScreen->setTextSize(3);
      // eInkScreen->println("ESP Easy");
      // eInkScreen->setTextSize(2);
      // eInkScreen->println("eInk shield");
      // eInkScreen->display();
      // delay(100);

      success = true;
      break;
    }

    case PLUGIN_EXIT:
    {
      P096_data_struct *P096_data = static_cast<P096_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P096_data) {
        success = P096_data->plugin_exit(event); // Stop the display
      }
      break;
    }

    case PLUGIN_READ:
    {
      P096_data_struct *P096_data = static_cast<P096_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P096_data) {
        success = P096_data->plugin_read(event); // Read operation, redisplay the configured content
      }
      break;
    }

    case PLUGIN_WRITE:
    {
# ifndef BUILD_NO_DEBUG
      String tmpString = String(string);
# endif // ifndef BUILD_NO_DEBUG
      String arguments = String(string);

      String command;
      String subcommand;

      int argIndex = arguments.indexOf(',');

      if (argIndex)
      {
        command    = arguments.substring(0, argIndex);
        arguments  = arguments.substring(argIndex + 1);
        argIndex   = arguments.indexOf(',');
        subcommand = arguments.substring(0, argIndex);
        success    = true;

# ifndef BUILD_NO_DEBUG
        tmpString += "<br/> command= " + command;
        tmpString += "<br/> arguments= " + arguments;
        tmpString += "<br/> argIndex= " + String(argIndex);
        tmpString += "<br/> subcommand= " + subcommand;
# endif // ifndef BUILD_NO_DEBUG

        //         if (command.equalsIgnoreCase(F("EPDCMD")))
        //         {
        //           if (subcommand.equalsIgnoreCase(F("CLEAR")))
        //           {
        //             arguments = arguments.substring(argIndex + 1);
        //             eInkScreen->clearBuffer();
        //             eInkScreen->fillScreen(Plugin_096_ParseColor(arguments));
        //             eInkScreen->display();
        //             eInkScreen->clearBuffer();
        //           }
        //           else if (subcommand.equalsIgnoreCase(F("DEEPSLEEP")))
        //           {
        //             eInkScreen->deepSleep();
        //           }
        //           else if (subcommand.equalsIgnoreCase(F("SEQ_START")))
        //           {
        //             eInkScreen->clearBuffer();
        //             eInkScreen->fillScreen(Plugin_096_ParseColor(arguments));
        //             plugin_096_sequence_in_progress = true;
        //           }
        //           else if (subcommand.equalsIgnoreCase(F("SEQ_END")))
        //           {
        // # ifndef BUILD_NO_DEBUG
        //             TimingStats s;
        //             const unsigned statisticsTimerStart(micros());
        // # endif // ifndef BUILD_NO_DEBUG
        //             eInkScreen->display();

        // # ifndef BUILD_NO_DEBUG
        //             s.add(usecPassedSince(statisticsTimerStart));
        //             tmpString += "<br/> Display timings = " + String(s.getAvg());
        // # endif // ifndef BUILD_NO_DEBUG
        //             eInkScreen->clearBuffer();
        //             plugin_096_sequence_in_progress = false;
        //           }
        //           else if (subcommand.equalsIgnoreCase(F("INV")))
        //           {
        //             arguments = arguments.substring(argIndex + 1);
        //             eInkScreen->invertDisplay(arguments.toInt() == 1);
        //             eInkScreen->display();
        //           }
        //           else if (subcommand.equalsIgnoreCase(F("ROT")))
        //           {
        //             arguments = arguments.substring(argIndex + 1);
        //             eInkScreen->setRotation(arguments.toInt() % 4);
        //             eInkScreen->display();
        //           }
        //           else
        //           {
        //             success = false;
        //           }
        //         }
        //         else if (command.equalsIgnoreCase(F("EPD")))
        //         {
        //             # ifdef P096_USE_ADA_GRAPHICS
        //           String tmp = string;
        //           success = gfxHelper->processCommand(AdaGFXparseTemplate(tmp, PCONFIG(2) / 10)); // Hand it over after replacing
        // variables
        //             # else // ifdef P096_USE_ADA_GRAPHICS
        // #  ifndef BUILD_NO_DEBUG
        //           tmpString += "<br/> EPD  ";
        // #  endif // ifndef BUILD_NO_DEBUG
        //           arguments = arguments.substring(argIndex + 1);
        //           String sParams[8];
        //           int    argCount = Plugin_096_StringSplit(arguments, ',', sParams, 8);

        // #  ifndef BUILD_NO_DEBUG

        //           for (int a = 0; a < argCount && a < 8; a++)
        //           {
        //             tmpString += "<br/> ARGS[" + String(a) + "]=" + sParams[a];
        //           }
        // #  endif // ifndef BUILD_NO_DEBUG

        //           if (plugin_096_sequence_in_progress == false)
        //           {
        //             eInkScreen->clearBuffer();
        //             eInkScreen->fillScreen(EPD_WHITE);
        //           }

        //           if (subcommand.equalsIgnoreCase(F("txt")))
        //           {
        //             Plugin_096_FixText(arguments);
        //             eInkScreen->println(arguments); // write all pending cars
        //           }
        //           else if (subcommand.equalsIgnoreCase(F("txz")))
        //           {
        //             eInkScreen->setCursor(sParams[0].toInt(), sParams[1].toInt());
        //             Plugin_096_FixText(sParams[2]);
        //             eInkScreen->println(sParams[2]); // write all pending cars
        //           }
        //           else if (subcommand.equalsIgnoreCase(F("txp")) && (argCount == 2))
        //           {
        //             eInkScreen->setCursor(sParams[0].toInt(), sParams[1].toInt());
        //           }
        //           else if (subcommand.equalsIgnoreCase(F("txc")) && ((argCount == 1) || (argCount == 2)))
        //           {
        //             if (argCount == 1) {
        //               eInkScreen->setTextColor(Plugin_096_ParseColor(sParams[0]));
        //             }
        //             else { // argCount=2
        //               eInkScreen->setTextColor(Plugin_096_ParseColor(sParams[0]), Plugin_096_ParseColor(sParams[1]));
        //             }
        //           }
        //           else if (subcommand.equalsIgnoreCase(F("txs")) && (argCount == 1))
        //           {
        //             eInkScreen->setTextSize(sParams[0].toInt());
        //           }
        //           else if (subcommand.equalsIgnoreCase(F("txtfull")) && (argCount >= 3) && (argCount <= 6))
        //           {
        //             switch (argCount)
        //             {
        //               case 3: // single text
        //                 Plugin_096_printText(sParams[2].c_str(), sParams[0].toInt() - 1, sParams[1].toInt() - 1);
        //                 break;

        //               case 4: // text + size
        //                 Plugin_096_printText(sParams[3].c_str(), sParams[0].toInt() - 1, sParams[1].toInt() - 1, sParams[2].toInt());
        //                 break;

        //               case 5: // text + size + color
        //                 Plugin_096_printText(sParams[4].c_str(),
        //                                      sParams[0].toInt() - 1,
        //                                      sParams[1].toInt() - 1,
        //                                      sParams[2].toInt(),
        //                                      Plugin_096_ParseColor(sParams[3]));
        //                 break;

        //               case 6: // text + size + color
        //                 Plugin_096_printText(sParams[5].c_str(),
        //                                      sParams[0].toInt() - 1,
        //                                      sParams[1].toInt() - 1,
        //                                      sParams[2].toInt(),
        //                                      Plugin_096_ParseColor(sParams[3]),
        //                                      Plugin_096_ParseColor(sParams[4]));
        //                 break;
        //               default:
        //                 success = false;
        //                 break;
        //             }
        //           }
        //           else if (subcommand.equalsIgnoreCase(F("l")) && (argCount == 5))
        //           {
        //             eInkScreen->drawLine(sParams[0].toInt(), sParams[1].toInt(), sParams[2].toInt(), sParams[3].toInt(),
        //                                  Plugin_096_ParseColor(sParams[4]));
        //           }
        //           else if (subcommand.equalsIgnoreCase(F("lh")) && (argCount == 3))
        //           {
        //             eInkScreen->drawFastHLine(0, sParams[0].toInt(), sParams[1].toInt(), Plugin_096_ParseColor(sParams[2]));
        //           }
        //           else if (subcommand.equalsIgnoreCase(F("lv")) && (argCount == 3))
        //           {
        //             eInkScreen->drawFastVLine(sParams[0].toInt(), 0, sParams[1].toInt(), Plugin_096_ParseColor(sParams[2]));
        //           }
        //           else if (subcommand.equalsIgnoreCase(F("r")) && (argCount == 5))
        //           {
        //             eInkScreen->drawRect(sParams[0].toInt(), sParams[1].toInt(), sParams[2].toInt(), sParams[3].toInt(),
        //                                  Plugin_096_ParseColor(sParams[4]));
        //           }
        //           else if (subcommand.equalsIgnoreCase(F("rf")) && (argCount == 6))
        //           {
        //             eInkScreen->fillRect(sParams[0].toInt(), sParams[1].toInt(), sParams[2].toInt(), sParams[3].toInt(),
        //                                  Plugin_096_ParseColor(sParams[5]));
        //             eInkScreen->drawRect(sParams[0].toInt(), sParams[1].toInt(), sParams[2].toInt(), sParams[3].toInt(),
        //                                  Plugin_096_ParseColor(sParams[4]));
        //           }
        //           else if (subcommand.equalsIgnoreCase(F("c")) && (argCount == 4))
        //           {
        //             eInkScreen->drawCircle(sParams[0].toInt(), sParams[1].toInt(), sParams[2].toInt(),
        // Plugin_096_ParseColor(sParams[3]));
        //           }
        //           else if (subcommand.equalsIgnoreCase(F("cf")) && (argCount == 5))
        //           {
        //             eInkScreen->fillCircle(sParams[0].toInt(), sParams[1].toInt(), sParams[2].toInt(),
        // Plugin_096_ParseColor(sParams[4]));
        //             eInkScreen->drawCircle(sParams[0].toInt(), sParams[1].toInt(), sParams[2].toInt(),
        // Plugin_096_ParseColor(sParams[3]));
        //           }
        //           else if (subcommand.equalsIgnoreCase(F("t")) && (argCount == 7))
        //           {
        //             eInkScreen->drawTriangle(sParams[0].toInt(),
        //                                      sParams[1].toInt(),
        //                                      sParams[2].toInt(),
        //                                      sParams[3].toInt(),
        //                                      sParams[4].toInt(),
        //                                      sParams[5].toInt(),
        //                                      Plugin_096_ParseColor(sParams[6]));
        //           }
        //           else if (subcommand.equalsIgnoreCase(F("tf")) && (argCount == 8))
        //           {
        //             eInkScreen->fillTriangle(sParams[0].toInt(),
        //                                      sParams[1].toInt(),
        //                                      sParams[2].toInt(),
        //                                      sParams[3].toInt(),
        //                                      sParams[4].toInt(),
        //                                      sParams[5].toInt(),
        //                                      Plugin_096_ParseColor(sParams[7]));
        //             eInkScreen->drawTriangle(sParams[0].toInt(),
        //                                      sParams[1].toInt(),
        //                                      sParams[2].toInt(),
        //                                      sParams[3].toInt(),
        //                                      sParams[4].toInt(),
        //                                      sParams[5].toInt(),
        //                                      Plugin_096_ParseColor(sParams[6]));
        //           }
        //           else if (subcommand.equalsIgnoreCase(F("rr")) && (argCount == 6))
        //           {
        //             eInkScreen->drawRoundRect(sParams[0].toInt(),
        //                                       sParams[1].toInt(),
        //                                       sParams[2].toInt(),
        //                                       sParams[3].toInt(),
        //                                       sParams[4].toInt(),
        //                                       Plugin_096_ParseColor(sParams[5]));
        //           }
        //           else if (subcommand.equalsIgnoreCase(F("rrf")) && (argCount == 7))
        //           {
        //             eInkScreen->fillRoundRect(sParams[0].toInt(),
        //                                       sParams[1].toInt(),
        //                                       sParams[2].toInt(),
        //                                       sParams[3].toInt(),
        //                                       sParams[4].toInt(),
        //                                       Plugin_096_ParseColor(sParams[6]));
        //             eInkScreen->drawRoundRect(sParams[0].toInt(),
        //                                       sParams[1].toInt(),
        //                                       sParams[2].toInt(),
        //                                       sParams[3].toInt(),
        //                                       sParams[4].toInt(),
        //                                       Plugin_096_ParseColor(sParams[5]));
        //           }
        //           else if (subcommand.equalsIgnoreCase(F("px")) && (argCount == 3))
        //           {
        //             eInkScreen->drawPixel(sParams[0].toInt(), sParams[1].toInt(), Plugin_096_ParseColor(sParams[2]));
        //           }
        //           else
        //           {
        //             success = false;
        //           }
        //             # endif // ifdef P096_USE_ADA_GRAPHICS
        // }
        // else
        // {
        //   success = false;
        // }
      }
      else
      {
        // invalid arguments
        success = false;
      }

      // in case of command outside of sequence, then refresh screen
      //       if (success && !plugin_096_sequence_in_progress)
      //       {
      //         eInkScreen->display();
      //       }

      // # ifndef BUILD_NO_DEBUG
      //       String log;
      //       log.reserve(110);       // Prevent re-allocation
      //       log  = F("P096-eInk : WRITE = ");
      //       log += tmpString;
      //       SendStatus(event, log); // Reply (echo) to sender. This will print message on browser.
      // # endif // ifndef BUILD_NO_DEBUG
      break;
    }
  }

  return success;
}

# ifndef P096_USE_ADA_GRAPHICS

// Print some text
// param [in] string : The text to display
// param [in] X : The left position (X)
// param [in] Y : The top position (Y)
// param [in] textSize : The text size (default 1)
// param [in] color : The fore color (default ILI9341_WHITE)
// param [in] bkcolor : The background color (default ILI9341_BLACK)
void Plugin_096_printText(const char *string, int X, int Y, unsigned int textSize, unsigned short color, unsigned short bkcolor)
{
  eInkScreen->clearBuffer();
  eInkScreen->clearDisplay();
  eInkScreen->setCursor(X, Y);
  eInkScreen->setTextColor(color, bkcolor);
  eInkScreen->setTextSize(textSize);
  String fixString = string;

  Plugin_096_FixText(fixString);
  eInkScreen->println(fixString);
  eInkScreen->display();
}

# endif // ifndef P096_USE_ADA_GRAPHICS

// Parse color string to color
// param [in] colorString : The color string (white, red, ...)
// return : color (default EPD_WHITE)
unsigned short Plugin_096_ParseColor(const String& colorString)
{
  // copy to local var and ensure lowercase
  // this optimise the next equlaity checks
  String s = colorString;

  s.toLowerCase();

  if (s.equals(F("black"))) {
    return EPD_BLACK;
  }

  if (s.equals(F("white"))) {
    return EPD_WHITE;
  }

  if (s.equals(F("inverse"))) {
    return EPD_INVERSE;
  }

  if (s.equals(F("red"))) {
    return EPD_RED;
  }

  if (s.equals(F("dark"))) {
    return EPD_DARK;
  }

  if (s.equals(F("light"))) {
    return EPD_LIGHT;
  }
  return EPD_WHITE;
}

# ifndef P096_USE_ADA_GRAPHICS

// Fix text with handling special characters (degrees and main monetary symbols)
// This is specific case for current AdafruitGfx standard fontused for eink screen
// param [in/out] s : The string to fix
void Plugin_096_FixText(String& s)
{
  const char degree[3]      = { 0xc2, 0xb0, 0 }; // Unicode degree symbol
  const char degree_eink[2] = { 0xf7, 0 };       // eink degree symbol

  s.replace(degree,     degree_eink);
  s.replace(F("{D}"),   degree_eink);
  s.replace(F("&deg;"), degree_eink);

  const char euro[4]      = { 0xe2, 0x82, 0xac, 0 }; // Unicode euro symbol
  const char euro_eink[2] = { 0xED, 0 };             // eink degree symbol

  s.replace(euro,        euro_eink);
  s.replace(F("{E}"),    euro_eink);
  s.replace(F("&euro;"), euro_eink);

  const char pound[3]      = { 0xc2, 0xa3, 0 }; // Unicode pound symbol
  const char pound_eink[2] = { 0x9C, 0 };       // eink pound symbol

  s.replace(pound,        pound_eink);
  s.replace(F("{P}"),     pound_eink);
  s.replace(F("&pound;"), pound_eink);

  const char yen[3]      = { 0xc2, 0xa5, 0 }; // Unicode yen symbol
  const char yen_eink[2] = { 0x9D, 0 };       // eink yen symbol

  s.replace(yen,        yen_eink);
  s.replace(F("{Y}"),   yen_eink);
  s.replace(F("&yen;"), yen_eink);

  const char cent[3]      = { 0xc2, 0xa2, 0 }; // Unicode yen symbol
  const char cent_eink[2] = { 0x9B, 0 };       // eink cent symbol

  s.replace(cent,        cent_eink);
  s.replace(F("{c}"),    cent_eink);
  s.replace(F("&cent;"), cent_eink);
}

// Split a string by delimiter
// param [in] s : The input string
// param [in] c : The delimiter
// param [out] op : The resulting string array
// param [in] limit : The maximum strings to find
// return : The string count
int Plugin_096_StringSplit(const String& s, char c, String op[], int limit)
{
  int    count = 0;
  char  *pch;
  String d = String(c);

  pch = strtok((char *)(s.c_str()), d.c_str());

  while (pch != NULL && count < limit)
  {
    op[count] = String(pch);
    count++;
    pch = strtok(NULL, ",");
  }
  return count;
}

# endif // ifndef P096_USE_ADA_GRAPHICS

#endif  // USES_P096
