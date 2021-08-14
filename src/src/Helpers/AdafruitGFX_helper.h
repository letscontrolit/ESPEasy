#ifndef HELPERS_ADAFRUITGFX_HELPER_H
#define HELPERS_ADAFRUITGFX_HELPER_H

#ifdef PLUGIN_USES_ADAFRUITGFX

/****************************************************************************
 * helper class and functions for displays that use Adafruit_GFX library
 ***************************************************************************/
# include <Arduino.h>
# include <Adafruit_GFX.h>

# include "../Helpers/Numerical.h"
# include "../ESPEasyCore/ESPEasy_Log.h"

# define ADAGFX_PARSE_MAX_ARGS  8 // Maximum number of arguments needed and supported
# define ADAGFX_FONTS_INCLUDED    // 3 extra fonts, also controls enable/disable of below 8pt/12pt fonts
// #define ADAGFX_FONTS_EXTRA_8PT_INCLUDED  // 6 extra 8pt fonts, should probably only be enabled in a private custom build, adds ~11,8 kB
// #define ADAGFX_FONTS_EXTRA_12PT_INCLUDED // 6 extra 12pt fonts, should probably only be enabled in a private custom build, adds ~19,8 kB

// To enable/disable 8pt fonts separately: (will only be enabled if ADAGFX_FONTS_EXTRA_8PT_INCLUDED is defined)
# define ADAGFX_FONTS_EXTRA_8PT_ANGELINA // This font is proportinally spaced!
# define ADAGFX_FONTS_EXTRA_8PT_NOVAMONO
# define ADAGFX_FONTS_EXTRA_8PT_REPETITIONSCROLLiNG
# define ADAGFX_FONTS_EXTRA_8PT_UNISPACE
# define ADAGFX_FONTS_EXTRA_8PT_UNISPACEITALIC
# define ADAGFX_FONTS_EXTRA_8PT_WHITERABBiT

// To enable/disable 12pt fonts separately: (will only be enabled if ADAGFX_FONTS_EXTRA_12PT_INCLUDED is defined)
# define ADAGFX_FONTS_EXTRA_12PT_ANGELINA // This font is proportinally spaced!
# define ADAGFX_FONTS_EXTRA_12PT_NOVAMONO
# define ADAGFX_FONTS_EXTRA_12PT_REPETITIONSCROLLiNG
# define ADAGFX_FONTS_EXTRA_12PT_UNISPACE
# define ADAGFX_FONTS_EXTRA_12PT_UNISPACEITALIC
# define ADAGFX_FONTS_EXTRA_12PT_WHITERABBiT

# if defined(LIMIT_BUILD_SIZE) && defined(ADAGFX_FONTS_INCLUDED)
#  undef ADAGFX_FONTS_INCLUDED
# endif // if defined(LIMIT_BUILD_SIZE) && defined(ADAGFX_FONTS_INCLUDED)

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

enum class AdaGFXTextPrintMode : uint8_t {
  ContinueToNextLine       = 0,
  TruncateExceedingMessage = 1,
  ClearThenTruncate        = 2, // Should have max. 16 options

  MAX                           // Keep as last
};

const __FlashStringHelper* getAdaGFXTextPrintMode(AdaGFXTextPrintMode mode);
void                       AdaGFXFormTextPrintMode(const __FlashStringHelper *id,
                                                   uint8_t                    selectedIndex);

class AdafruitGFX_helper {
public:

  enum class ColorDepth : uint16_t {
    Monochrome   = 2,     // Black & white
    Duochrome    = 3,     // Black, white & grey
    Quadrochrome = 4,     // Black, white, lightgrey & darkgrey
    Octochrome   = 8,     // 8 regular colors
    Quintochrome = 16,    // 16 colors
    FullColor    = 0xFFFF // 65535 colors (max. supported by RGB565)
  };

  AdafruitGFX_helper(Adafruit_GFX       *display,
                     const String      & trigger,
                     uint16_t            res_x,
                     uint16_t            res_y,
                     ColorDepth          colorDepth    = ColorDepth::FullColor,
                     AdaGFXTextPrintMode textPrintMode = AdaGFXTextPrintMode::ContinueToNextLine,
                     uint8_t             fontscaling   = 1);

  bool     processCommand(const String& string); // Parse the string for recognized commands and apply them on the graphics display
  uint16_t parseColor(String& s);                // Parse either a color by name, 6 digit hex rrggbb color, or 1..4 digit #rgb565 color
                                                 // (hex with # prefix)

  void     printText(const char    *string,
                     int            X,
                     int            Y,
                     unsigned int   textSize = 0,
                     unsigned short color    = ADAGFX_WHITE,
                     unsigned short bkcolor  = ADAGFX_BLACK);
  void calculateTextMetrics(uint8_t fontwidth,
                            uint8_t fontheight);
  void getTextMetrics(uint16_t& textcols,
                      uint16_t& textrows,
                      uint8_t & fontwidth,
                      uint8_t & fontheight);

  void setP095TxtfullCompensation(uint8_t compensation) { // Set to 1 for backward comp. with P095 txtfull subcommands, uses offset -1
    _p095_compensation = compensation;
  }

  void setColumnRowMode(bool state) { // When true, addressing for txp, txtfull commands is in columns/rows, default in pixels
    _columnRowMode = state;           // NOT compatible with _p095_compensation!
  }

private:

  Adafruit_GFX *_display;
  String _trigger;
  uint16_t _res_x;
  uint16_t _res_y;
  ColorDepth _colorDepth;
  AdaGFXTextPrintMode _textPrintMode;
  uint16_t _textcols;
  uint16_t _textrows;
  uint16_t _fgcolor          = ADAGFX_WHITE;
  uint16_t _bgcolor          = ADAGFX_BLACK;
  uint8_t _fontscaling       = 1;
  uint8_t _fontwidth         = 8; // Default font characteristics
  uint8_t _fontheight        = 10;
  uint8_t _p095_compensation = 0;
  bool _columnRowMode        = false;
};
#endif // ifdef PLUGIN_USES_ADAFRUITGFX

#endif // ifndef HELPERS_ADAFRUITGFX_HELPER_H
