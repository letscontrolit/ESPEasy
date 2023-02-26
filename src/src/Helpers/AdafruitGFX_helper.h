#ifndef HELPERS_ADAFRUITGFX_HELPER_H
#define HELPERS_ADAFRUITGFX_HELPER_H

#include "../../_Plugin_Helper.h"

#ifdef PLUGIN_USES_ADAFRUITGFX

# define ADAGFX_LOG_LEVEL LOG_LEVEL_DEBUG

/****************************************************************************
 * helper class and functions for displays that use Adafruit_GFX library
 ***************************************************************************/
/************
 * Changelog:
 * 2023-02-26 tonhuisman: Use GetCommandCode() / PROGMEM for parsing of commands and colors to reduce .bin size.
 * 2022-10-05 tonhuisman: No longer trim off spaces from arguments to commands
 * 2022-09-23 tonhuisman: Allow backlight percentage from 0% instead of from 1% to be able to completely turn it off
 * 2022-09-12 tonhuisman: Add line-spacing option for Column/Row mode, default set to auto, optional 0..14 pixels line-spacing
 *                        Add line spacing form selector function
 * 2022-09-10 tonhuisman: Enable printing partial characters falling off at the right edge of the screen, only when on Window 0
 * 2022-08-25 tonhuisman: Add invertDisplay() functionality, often used for monochrome displays
 * 2022-08-23 tonhuisman: Several small improvements, and a few bugfixes
 * 2022-08-20 tonhuisman: Add txl subcommand to display text on 1 or more lines, autoincrementing the line nr,
 *                        always in row/column mode.
 *                        Improved argument parsing to allow up to 2 empty arguments between filled arguments
 * 2022-06-07 tonhuisman: Code improvements in initialization, move offset calculation to printText() function
 * 2022-06-06 tonhuisman: Process any special characters for lenght and textheight values for correct sizing
 * 2022-06-05 tonhuisman: Add support for getting config values: win (current window id), iswin (exists?), width & height (current window),
 *                        (text)length and textheight of a provided text, rot (current rotation), txs (fontscaling), tpm (textprintmode)
 * 2022-06-04 tonhuisman: Add Window support for drawing and printing within confined areas (windows)
 *                        Always use exact font calculation for determining allowable text length
 * 2022-06-02 tonhuisman: Leave out some Notes from UI to save a few bytes from size limited builds
 * 2022-05-27 tonhuisman: Change btn subcommand to split state and mode arguments, state = 0/1, -2/-1, mode = -2, -1, 0
 * 2022-05-27 tonhuisman: Fix a few character mappings in AdaGFXparseTemplate, add surrogates for chars not in font
 *                        Add support for {0xNN...} to insert any ascii character in template, supports multiple 2-digit hex values > 00
 *                        space, comma, dot, colon, semicolon or dash (' ,.:;-') as separators in hex value are allowed
 * 2022-05-23 tonhuisman: Fix cast for returned value from AdaGFXparseColor
 *                        Make 8 and 16 color support optional to squeeze a few bytes from size limited builds
 * 2022-05-23 tonhuisman: Add changelog, older changes have not been logged.
 ***************************************************************************/
# include <Arduino.h>
# include <Adafruit_GFX.h>
# include <Adafruit_SPITFT.h>
# include <FS.h>
# include <vector>

// Used for bmp support
# define BUFPIXELS 200 ///< 200 * 5 = 1000 bytes

# include "../Helpers/Numerical.h"
# include "../Helpers/ESPEasy_Storage.h"
# include "../ESPEasyCore/ESPEasy_Log.h"

# define ADAGFX_PARSE_MAX_ARGS        7 // Maximum number of arguments needed and supported (corrected)
# ifndef ADAGFX_ARGUMENT_VALIDATION
#  define ADAGFX_ARGUMENT_VALIDATION  1 // Validate command arguments
# endif // ifndef ADAGFX_ARGUMENT_VALIDATION
# ifndef ADAGFX_USE_ASCIITABLE
#  define ADAGFX_USE_ASCIITABLE       1 // Enable 'asciitable' command (useful for debugging/development)
# endif // ifndef ADAGFX_USE_ASCIITABLE
# ifndef ADAGFX_SUPPORT_7COLOR

// #  define ADAGFX_SUPPORT_7COLOR       1  // Do we support 7-Color displays?
# endif // ifndef ADAGFX_SUPPORT_7COLOR
# ifndef ADAGFX_SUPPORT_8and16COLOR

// #  define ADAGFX_SUPPORT_8and16COLOR  1  // Do we support 8 and 16-Color displays?
# endif // ifndef ADAGFX_SUPPORT_8and16COLOR
# ifndef ADAGFX_FONTS_INCLUDED
#  define ADAGFX_FONTS_INCLUDED       1     // 3 extra fonts, also controls enable/disable of below 8pt/12pt fonts
# endif // ifndef ADAGFX_FONTS_INCLUDED
# ifndef ADAGFX_PARSE_SUBCOMMAND
#  define ADAGFX_PARSE_SUBCOMMAND     1     // Enable parsing of subcommands (pre/postfix below) to be executed by the helper
# endif // ifndef ADAGFX_PARSE_SUBCOMMAND
# ifndef ADAGFX_ENABLE_EXTRA_CMDS
#  define ADAGFX_ENABLE_EXTRA_CMDS    1     // Enable extra subcommands like lm (line-multi) and lmr (line-multi, relative)
# endif // ifndef ADAGFX_ENABLE_EXTRA_CMDS
# ifndef ADAGFX_ENABLE_BMP_DISPLAY
#  define ADAGFX_ENABLE_BMP_DISPLAY   1     // Enable subcommands for displaying .bmp files on supported displays (color)
# endif // ifndef ADAGFX_ENABLE_BMP_DISPLAY
# ifndef ADAGFX_ENABLE_BUTTON_DRAW
#  define ADAGFX_ENABLE_BUTTON_DRAW    1    // Enable subcommands for displaying button-like shapes
# endif // ifndef ADAGFX_ENABLE_BUTTON_DRAW
# ifndef ADAGFX_ENABLE_FRAMED_WINDOW
#  define ADAGFX_ENABLE_FRAMED_WINDOW 1     // Enable framed window features
# endif // ifndef ADAGFX_ENABLE_BUTTON_DRAW
# ifndef ADAGFX_ENABLE_GET_CONFIG_VALUE
#  define ADAGFX_ENABLE_GET_CONFIG_VALUE  1 // Enable getting values features
# endif // ifndef ADAGFX_ENABLE_GET_CONFIG_VALUE

// # define ADAGFX_FONTS_EXTRA_8PT_INCLUDED  // 8 extra 8pt fonts, should probably only be enabled in a private custom build, adds ~15.4 kB
// # define ADAGFX_FONTS_EXTRA_12PT_INCLUDED // 9 extra 12pt fonts, should probably only be enabled in a private custom build, adds ~28 kB
// # define ADAGFX_FONTS_EXTRA_16PT_INCLUDED // 5 extra 16pt fonts, should probably only be enabled in a private custom build, adds ~19.9 kB
// # define ADAGFX_FONTS_EXTRA_18PT_INCLUDED // 1 extra 18pt fonts, should probably only be enabled in a private custom build, adds ~4.3 kB
// # define ADAGFX_FONTS_EXTRA_20PT_INCLUDED // 1 extra 20pt fonts, should probably only be enabled in a private custom build, adds ~5.3 kB

// To enable/disable 8pt fonts separately: (will only be enabled if ADAGFX_FONTS_EXTRA_8PT_INCLUDED is defined)
# define ADAGFX_FONTS_EXTRA_8PT_ANGELINA // This font is proportinally spaced!
# define ADAGFX_FONTS_EXTRA_8PT_NOVAMONO
# define ADAGFX_FONTS_EXTRA_8PT_UNISPACE
# define ADAGFX_FONTS_EXTRA_8PT_UNISPACEITALIC
# define ADAGFX_FONTS_EXTRA_8PT_WHITERABBiT

// # define ADAGFX_FONTS_EXTRA_8PT_ROBOTO          // This font is proportinally spaced!
// # define ADAGFX_FONTS_EXTRA_8PT_ROBOTOCONDENSED // This font is proportinally spaced!
# define ADAGFX_FONTS_EXTRA_8PT_ROBOTOMONO

// To enable/disable 12pt fonts separately: (will only be enabled if ADAGFX_FONTS_EXTRA_12PT_INCLUDED is defined)
# define ADAGFX_FONTS_EXTRA_12PT_ANGELINA // This font is proportinally spaced!
# define ADAGFX_FONTS_EXTRA_12PT_NOVAMONO
# define ADAGFX_FONTS_EXTRA_12PT_REPETITIONSCROLLiNG
# define ADAGFX_FONTS_EXTRA_12PT_UNISPACE
# define ADAGFX_FONTS_EXTRA_12PT_UNISPACEITALIC
# define ADAGFX_FONTS_EXTRA_12PT_WHITERABBiT

// # define ADAGFX_FONTS_EXTRA_12PT_ROBOTO          // This font is proportinally spaced!
// # define ADAGFX_FONTS_EXTRA_12PT_ROBOTOCONDENSED // This font is proportinally spaced!
# define ADAGFX_FONTS_EXTRA_12PT_ROBOTOMONO

// To enable/disable 16pt fonts separately: (will only be enabled if ADAGFX_FONTS_EXTRA_16PT_INCLUDED is defined)
# define ADAGFX_FONTS_EXTRA_16PT_AMERIKASANS // This font is proportinally spaced!
# define ADAGFX_FONTS_EXTRA_16PT_WHITERABBiT

// # define ADAGFX_FONTS_EXTRA_16PT_ROBOTO          // This font is proportinally spaced!
// # define ADAGFX_FONTS_EXTRA_16PT_ROBOTOCONDENSED // This font is proportinally spaced!
# define ADAGFX_FONTS_EXTRA_16PT_ROBOTOMONO

// To enable/disable 18pt fonts separately: (will only be enabled if ADAGFX_FONTS_EXTRA_18PT_INCLUDED is defined)
# define ADAGFX_FONTS_EXTRA_18PT_WHITERABBiT

// To enable/disable 20pt fonts separately: (will only be enabled if ADAGFX_FONTS_EXTRA_20PT_INCLUDED is defined)
# define ADAGFX_FONTS_EXTRA_20PT_WHITERABBiT

# ifdef LIMIT_BUILD_SIZE
#  ifdef ADAGFX_FONTS_INCLUDED
#   undef ADAGFX_FONTS_INCLUDED
#  endif // ifdef ADAGFX_FONTS_INCLUDED
#  ifdef ADAGFX_ARGUMENT_VALIDATION
#   undef ADAGFX_ARGUMENT_VALIDATION
#  endif // ifdef ADAGFX_ARGUMENT_VALIDATION
#  ifdef ADAGFX_USE_ASCIITABLE
#   undef ADAGFX_USE_ASCIITABLE
#  endif // ifdef ADAGFX_USE_ASCIITABLE
#  ifdef ADAGFX_SUPPORT_8and16COLOR
#   undef ADAGFX_SUPPORT_8and16COLOR
#  endif // ifdef ADAGFX_SUPPORT_8and16COLOR
// #  ifdef ADAGFX_ENABLE_BMP_DISPLAY
// #   undef ADAGFX_ENABLE_BMP_DISPLAY
// #  endif // ifdef ADAGFX_ENABLE_BMP_DISPLAY
// #  ifdef ADAGFX_ENABLE_BUTTON_DRAW
// #   undef ADAGFX_ENABLE_BUTTON_DRAW
// #  endif // ifdef ADAGFX_ENABLE_BUTTON_DRAW
// #  ifdef ADAGFX_ENABLE_FRAMED_WINDOW
// #   undef ADAGFX_ENABLE_FRAMED_WINDOW
// #  endif // ifdef ADAGFX_ENABLE_FRAMED_WINDOW
// #  ifdef ADAGFX_ENABLE_GET_CONFIG_VALUE
// #   undef ADAGFX_ENABLE_GET_CONFIG_VALUE
// #  endif // ifdef ADAGFX_ENABLE_GET_CONFIG_VALUE
# endif  // ifdef LIMIT_BUILD_SIZE

# ifdef PLUGIN_SET_MAX // Include all fonts in MAX builds
#  ifndef ADAGFX_FONTS_EXTRA_8PT_INCLUDED
#   define ADAGFX_FONTS_EXTRA_8PT_INCLUDED
#  endif // ifndef ADAGFX_FONTS_EXTRA_8PT_INCLUDED
#  ifndef ADAGFX_FONTS_EXTRA_12PT_INCLUDED
#   define ADAGFX_FONTS_EXTRA_12PT_INCLUDED
#  endif // ifndef ADAGFX_FONTS_EXTRA_12PT_INCLUDED
#  ifndef ADAGFX_FONTS_EXTRA_16PT_INCLUDED
#   define ADAGFX_FONTS_EXTRA_16PT_INCLUDED
#  endif // ifndef ADAGFX_FONTS_EXTRA_16PT_INCLUDED
#  ifndef ADAGFX_FONTS_EXTRA_18PT_INCLUDED
#   define ADAGFX_FONTS_EXTRA_18PT_INCLUDED
#  endif // ifndef ADAGFX_FONTS_EXTRA_18PT_INCLUDED
#  ifndef ADAGFX_FONTS_EXTRA_20PT_INCLUDED
#   define ADAGFX_FONTS_EXTRA_20PT_INCLUDED
#  endif // ifndef ADAGFX_FONTS_EXTRA_20PT_INCLUDED
#  ifndef ADAGFX_SUPPORT_7COLOR
#   define ADAGFX_SUPPORT_7COLOR       1
#  endif // ifndef ADAGFX_SUPPORT_7COLOR
#  ifndef ADAGFX_SUPPORT_8and16COLOR
#   define ADAGFX_SUPPORT_8and16COLOR  1
#  endif // ifndef ADAGFX_SUPPORT_8and16COLOR
# endif  // ifdef PLUGIN_SET_MAX

# define ADAGFX_PARSE_PREFIX      F("~")              // Subcommand-trigger prefix and postfix strings
# define ADAGFX_PARSE_PREFIX_LEN  1
# define ADAGFX_PARSE_POSTFIX     F("~")              // Will be removed before the normal template parsing is done
# define ADAGFX_PARSE_POSTFIX_LEN 1

# define ADAGFX_UNIVERSAL_TRIGGER F("adagfx_trigger") // Universal command trigger

// Color definitions, borrowed from Adafruit_ILI9341.h

# define ADAGFX_BLACK        0x0000 ///<   0,   0,   0
# define ADAGFX_NAVY         0x000F ///<   0,   0, 123
# define ADAGFX_DARKGREEN    0x03E0 ///<   0, 125,   0
# define ADAGFX_DARKCYAN     0x03EF ///<   0, 125, 123
# define ADAGFX_MAROON       0x7800 ///< 123,   0,   0
# define ADAGFX_PURPLE       0x780F ///< 123,   0, 123
# define ADAGFX_OLIVE        0x7BE0 ///< 123, 125,   0
# define ADAGFX_LIGHTGREY    0xC618 ///< 198, 195, 198
# define ADAGFX_DARKGREY     0x7BEF ///< 123, 125, 123
# define ADAGFX_BLUE         0x001F ///<   0,   0, 255
# define ADAGFX_GREEN        0x07E0 ///<   0, 255,   0
# define ADAGFX_CYAN         0x07FF ///<   0, 255, 255
# define ADAGFX_RED          0xF800 ///< 255,   0,   0
# define ADAGFX_MAGENTA      0xF81F ///< 255,   0, 255
# define ADAGFX_YELLOW       0xFFE0 ///< 255, 255,   0
# define ADAGFX_WHITE        0xFFFF ///< 255, 255, 255
# define ADAGFX_ORANGE       0xFD20 ///< 255, 165,   0
# define ADAGFX_GREENYELLOW  0xAFE5 ///< 173, 255,  41
# define ADAGFX_PINK         0xFC18 ///< 255, 130, 198

enum class AdaGFXMonoRedGreyscaleColors: uint16_t {
  ADAGFXEPD_BLACK,                  ///< black color
  ADAGFXEPD_WHITE,                  ///< white color
  ADAGFXEPD_INVERSE,                ///< invert color
  ADAGFXEPD_RED,                    ///< red color
  ADAGFXEPD_DARK,                   ///< darker color
  ADAGFXEPD_LIGHT                   ///< lighter color
};

# if ADAGFX_SUPPORT_7COLOR
enum class AdaGFX7Colors: uint16_t {
  ADAGFX7C_BLACK,  ///< black color
  ADAGFX7C_WHITE,  ///< white color
  ADAGFX7C_GREEN,  ///< green color
  ADAGFX7C_BLUE,   ///< blue color
  ADAGFX7C_RED,    ///< red color
  ADAGFX7C_YELLOW, ///< yellow color
  ADAGFX7C_ORANGE  ///< orange color
};
# endif // if ADAGFX_SUPPORT_7COLOR

enum class AdaGFXTextPrintMode : uint8_t {
  ContinueToNextLine        = 0u,
  TruncateExceedingMessage  = 1u,
  ClearThenTruncate         = 2u,
  TruncateExceedingCentered = 3u, // Should have max. 16 options

  MAX                             // Keep as last
};

# if ADAGFX_SUPPORT_7COLOR
#  if ADAGFX_SUPPORT_8and16COLOR
#   define ADAGFX_COLORDEPTH_COUNT 7
#   define ADAGFX_MONOCOLORS_COUNT 4
#  else // if ADAGFX_SUPPORT_8and16COLOR
#   define ADAGFX_COLORDEPTH_COUNT 5
#   define ADAGFX_MONOCOLORS_COUNT 4
#  endif // if ADAGFX_SUPPORT_8and16COLOR
# else // if ADAGFX_SUPPORT_7COLOR
#  if ADAGFX_SUPPORT_8and16COLOR
#   define ADAGFX_COLORDEPTH_COUNT 6
#   define ADAGFX_MONOCOLORS_COUNT 3
#  else // if ADAGFX_SUPPORT_8and16COLOR
#   define ADAGFX_COLORDEPTH_COUNT 4
#   define ADAGFX_MONOCOLORS_COUNT 3
#  endif // if ADAGFX_SUPPORT_8and16COLOR
# endif // if ADAGFX_SUPPORT_7COLOR
enum class AdaGFXColorDepth : uint16_t {
  Monochrome            = 2u, // Black & white
  BlackWhiteRed         = 3u, // Black, white & red (or yellow)
  BlackWhite2Greyscales = 4u, // Black, white, lightgrey & darkgrey
  # if ADAGFX_SUPPORT_7COLOR
  SevenColor = 7u,            // Black, white, red, yellow, blue, green, orange
  # endif // if ADAGFX_SUPPORT_7COLOR
  # if ADAGFX_SUPPORT_8and16COLOR
  EightColor   = 8u,          // 8 regular colors
  SixteenColor = 16u,         // 16 colors
  # endif // if ADAGFX_SUPPORT_8and16COLOR
  FullColor = 65535u          // 65535 colors (max. supported by RGB565)
};

# if ADAGFX_ENABLE_BUTTON_DRAW

// Only bits 0..3 can be used, masked with: 0x0F
// stored combined with Button_layout_e value
enum class Button_type_e : uint8_t {
  None       = 0x00,
  Square     = 0x01,
  Rounded    = 0x02,
  Circle     = 0x03,
  ArrowLeft  = 0x04,
  ArrowUp    = 0x05,
  ArrowRight = 0x06,
  ArrowDown  = 0x07,
  Button_MAX = 8u // must be last value in enum, max possible values: 16
};

// Only bits 4..7 can be used, masked with: 0xF0
// stored combined with Button_type_e value
enum class Button_layout_e : uint8_t {
  CenterAligned      = 0x00,
  LeftAligned        = 0x10,
  TopAligned         = 0x20,
  RightAligned       = 0x30,
  BottomAligned      = 0x40,
  LeftTopAligned     = 0x50,
  RightTopAligned    = 0x60,
  RightBottomAligned = 0x70,
  LeftBottomAligned  = 0x80,
  NoCaption          = 0x90,
  Bitmap             = 0xA0,
  Alignment_MAX      = 11u // options-count, max possible values: 16
};

const __FlashStringHelper* toString(const Button_type_e button);
const __FlashStringHelper* toString(const Button_layout_e layout);

# endif // if ADAGFX_ENABLE_BUTTON_DRAW

# if ADAGFX_ENABLE_FRAMED_WINDOW

struct tWindowPoint {
  uint16_t x = 0;
  uint16_t y = 0;
};
struct tWindowObject {
  tWindowPoint top_left;
  tWindowPoint width_height;
  tWindowPoint org_top_left;
  tWindowPoint org_width_height;
  uint8_t      id       = 0u;
  int8_t       rotation = 0;
};
# endif // if ADAGFX_ENABLE_FRAMED_WINDOW

class AdafruitGFX_helper; // Forward declaration

// Some generic AdafruitGFX_helper support functions
const __FlashStringHelper* toString(const AdaGFXTextPrintMode& mode);
const __FlashStringHelper* toString(const AdaGFXColorDepth& colorDepth);
void                       AdaGFXFormTextPrintMode(const __FlashStringHelper *id,
                                                   uint8_t                    selectedIndex);
void                       AdaGFXFormColorDepth(const __FlashStringHelper *id,
                                                uint16_t                   selectedIndex,
                                                bool                       enabled = true);
void                       AdaGFXFormRotation(const __FlashStringHelper *id,
                                              uint8_t                    selectedIndex);
void                       AdaGFXFormTextBackgroundFill(const __FlashStringHelper *id,
                                                        uint8_t                    selectedIndex);
void                       AdaGFXFormTextColRowMode(const __FlashStringHelper *id,
                                                    bool                       selectedState);
void                       AdaGFXFormOnePixelCompatibilityOption(const __FlashStringHelper *id,
                                                                 uint8_t                    selectedIndex);
void                       AdaGFXFormForeAndBackColors(const __FlashStringHelper *foregroundId,
                                                       uint16_t                   foregroundColor,
                                                       const __FlashStringHelper *backgroundId,
                                                       uint16_t                   backgroundColor,
                                                       AdaGFXColorDepth           colorDepth = AdaGFXColorDepth::FullColor);
void AdaGFXFormBacklight(const __FlashStringHelper *backlightPinId,
                         int8_t                     backlightPin,
                         const __FlashStringHelper *backlightPercentageId,
                         uint16_t                   backlightPercentage);
void AdaGFXFormDisplayButton(const __FlashStringHelper *buttonPinId,
                             int8_t                     buttonPin,
                             const __FlashStringHelper *buttonInverseId,
                             bool                       buttonInverse,
                             const __FlashStringHelper *displayTimeoutId,
                             int                        displayTimeout);
void     AdaGFXFormFontScaling(const __FlashStringHelper *fontScalingId,
                               uint8_t                    fontScaling,
                               uint8_t                    maxScale = 10);
String   AdaGFXparseTemplate(const String      & tmpString,
                             const uint8_t       lineSize,
                             AdafruitGFX_helper *gfxHelper = nullptr);
uint16_t AdaGFXparseColor(String                & s,
                          const AdaGFXColorDepth& colorDepth   = AdaGFXColorDepth::FullColor,
                          const bool              emptyIsBlack = false); // Parse either a color by name, 6 digit hex rrggbb color,
                                                                         // or 1..4 digit
                                                                         // #rgb565 color (hex with # prefix)
void   AdaGFXHtmlColorDepthDataList(const __FlashStringHelper *id,
                                    const AdaGFXColorDepth   & colorDepth);
String AdaGFXcolorToString(const uint16_t        & color,
                           const AdaGFXColorDepth& colorDepth   = AdaGFXColorDepth::FullColor,
                           bool                    blackIsEmpty = false);
# if ADAGFX_SUPPORT_7COLOR
uint16_t AdaGFXrgb565ToColor7(const uint16_t& color); // Convert rgb565 color to 7-color
# endif // if ADAGFX_SUPPORT_7COLOR
void     AdaGFXFormLineSpacing(const __FlashStringHelper *id,
                               uint8_t                    selectedIndex);

class AdafruitGFX_helper {
public:

  AdafruitGFX_helper(Adafruit_GFX              *display,
                     const String             & trigger,
                     const uint16_t             res_x,
                     const uint16_t             res_y,
                     const AdaGFXColorDepth   & colorDepth    = AdaGFXColorDepth::FullColor,
                     const AdaGFXTextPrintMode& textPrintMode = AdaGFXTextPrintMode::ContinueToNextLine,
                     const uint8_t              fontscaling   = 1,
                     const uint16_t             fgcolor       = ADAGFX_WHITE,
                     const uint16_t             bgcolor       = ADAGFX_BLACK,
                     const bool                 useValidation = true,
                     const bool                 textBackFill  = false);
  # if ADAGFX_ENABLE_BMP_DISPLAY
  AdafruitGFX_helper(Adafruit_SPITFT           *display,
                     const String             & trigger,
                     const uint16_t             res_x,
                     const uint16_t             res_y,
                     const AdaGFXColorDepth   & colorDepth    = AdaGFXColorDepth::FullColor,
                     const AdaGFXTextPrintMode& textPrintMode = AdaGFXTextPrintMode::ContinueToNextLine,
                     const uint8_t              fontscaling   = 1,
                     const uint16_t             fgcolor       = ADAGFX_WHITE,
                     const uint16_t             bgcolor       = ADAGFX_BLACK,
                     const bool                 useValidation = true,
                     const bool                 textBackFill  = false);
  # endif // if ADAGFX_ENABLE_BMP_DISPLAY
  virtual ~AdafruitGFX_helper() {}

  String getFeatures();

  bool   processCommand(const String& string); // Parse the string for recognized commands and apply them on the graphics display

  # if ADAGFX_ENABLE_GET_CONFIG_VALUE
  bool   pluginGetConfigValue(String& string); // Get a config value from the plugin
  # endif // if ADAGFX_ENABLE_GET_CONFIG_VALUE

  void   printText(const char     *string,
                   const int16_t & X,
                   const int16_t & Y,
                   const uint8_t & textSize = 0,
                   const uint16_t& color    = ADAGFX_WHITE,
                   uint16_t        bkcolor  = ADAGFX_BLACK,
                   const uint16_t& maxWidth = 0);
  void calculateTextMetrics(const uint8_t fontwidth,
                            const uint8_t fontheight,
                            const int8_t  heightOffset   = 0,
                            const bool    isProportional = false);
  void getTextMetrics(uint16_t& textcols,
                      uint16_t& textrows,
                      uint8_t & fontwidth,
                      uint8_t & fontheight,
                      uint8_t & fontscaling,
                      uint8_t & heightOffset,
                      uint16_t& xpix,
                      uint16_t& ypix);
  void getColors(uint16_t& fgcolor,
                 uint16_t& bgcolor);
  void getCursorXY(int16_t& currentX,                // Get last known (text)cursor position, recalculates to col/row if that
                   int16_t& currentY);               // setting is acive

  void setTxtfullCompensation(uint8_t compensation); // Set to 1 for backward comp. with P095/P096 txtfull subcommands, uses offset -1
                                                     // Set to 2 for extra offset of +1 on y axis
                                                     // Set to 3 for extra offset of +1 on x axis

  void setRotation(uint8_t m);                       // Set the helper-rotation the same as the display object rotation

  void setColumnRowMode(bool state) {                // When true, addressing for txp, txtfull commands is in columns/rows, default in
    _columnRowMode = state;                          // pixels NOT compatible with _x_compensation!
  }

  void setLineSpacing(int8_t lineSpacing) {          // Set inter-line spacing in Column/Row mode
    _lineSpacing = lineSpacing & 0xF;                // Limited to 0..14 px, 15 = auto, based on fontheight * fontsize
  }

  String getTrigger() {                              // Returns the current trigger
    return _trigger;
  }

  bool isAdaGFXTrigger(const String& trigger) {
    return trigger.equalsIgnoreCase(ADAGFX_UNIVERSAL_TRIGGER);
  }

  # if ADAGFX_ENABLE_BMP_DISPLAY
  bool showBmp(const String& filename,
               int16_t       x,
               int16_t       y);
  # endif // if ADAGFX_ENABLE_BMP_DISPLAY

  # if ADAGFX_ENABLE_FRAMED_WINDOW
  uint8_t getWindow() {
    return _window;
  }

  bool    validWindow(const uint8_t& windowId);
  bool    selectWindow(const uint8_t& windowId,
                       const int8_t & rotation = -1);
  uint8_t defineWindow(const int16_t& x,
                       const int16_t& y,
                       const int16_t& w,
                       const int16_t& h,
                       int16_t        windowId = -1,
                       const int8_t & rotation = -1);
  bool     deleteWindow(const uint8_t& windowId);
  # endif // if ADAGFX_ENABLE_FRAMED_WINDOW

  uint16_t getTextSize(const String& text,
                       uint16_t    & h); // return length and height in pixels using current font

  void     setValidation(const bool& state);
  bool     getValidation() const {
    return _useValidation;
  }

  void invertDisplay(bool i);
  void initialize();

private:

  # if ADAGFX_ARGUMENT_VALIDATION
  bool invalidCoordinates(const int  X,
                          const int  Y,
                          const bool colRowMode = false);
  # endif // if ADAGFX_ARGUMENT_VALIDATION
  # if ADAGFX_ENABLE_BUTTON_DRAW
  void drawButtonShape(const Button_type_e& buttonType,
                       const int          & x,
                       const int          & y,
                       const int          & w,
                       const int          & h,
                       const uint16_t     & fillColor,
                       const uint16_t     & borderColor);
  # endif // if ADAGFX_ENABLE_BUTTON_DRAW

  Adafruit_GFX *_display = nullptr;
  Adafruit_SPITFT *_tft = nullptr;
  String _trigger;
  uint16_t _res_x;
  uint16_t _res_y;
  AdaGFXColorDepth _colorDepth;
  AdaGFXTextPrintMode _textPrintMode;
  uint8_t _fontscaling;
  uint16_t _fgcolor;
  uint16_t _bgcolor;
  bool _useValidation;
  bool _textBackFill;
  uint16_t _textcols     = 0;
  uint16_t _textrows     = 0;
  int16_t _lastX         = 0;
  int16_t _lastY         = 0;
  uint8_t _fontwidth     = 6; // Default font characteristics
  uint8_t _fontheight    = 10;
  int8_t _heightOffset   = 0;
  bool _isProportional   = false;
  int8_t _x_compensation = 0;
  int8_t _y_compensation = 0;
  bool _columnRowMode    = false;
  int8_t _rotation       = 0;
  bool _displayInverted  = false;
  int8_t _lineSpacing    = 15; // Default fontheight * fontsize

  uint16_t _display_x = 0;
  uint16_t _display_y = 0;
  # if ADAGFX_ENABLE_BMP_DISPLAY
  uint16_t readLE16(void);
  uint32_t readLE32(void);
  fs::File file;
  # endif // if ADAGFX_ENABLE_BMP_DISPLAY
  # if ADAGFX_ENABLE_FRAMED_WINDOW
  int16_t getWindowIndex(const int16_t& windowId);
  void    logWindows(const String& prefix = EMPTY_STRING);
  void    getWindowOffsets(uint16_t& xOffset,
                           uint16_t& yOffset);
  void    getWindowLimits(uint16_t& xLimit,
                          uint16_t& yLimit);
  std::vector<tWindowObject>_windows;
  uint8_t _window      = 0; // current window
  uint8_t _windowIndex = 0; // current window Index
  # endif // if ADAGFX_ENABLE_FRAMED_WINDOW
};
#endif // ifdef PLUGIN_USES_ADAFRUITGFX

#endif // ifndef HELPERS_ADAFRUITGFX_HELPER_H
