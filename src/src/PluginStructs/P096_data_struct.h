#ifndef PLUGINSTRUCTS_P096_DATA_STRUCT_H
#define PLUGINSTRUCTS_P096_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P096

# include <Adafruit_GFX.h>                         // include Adafruit graphics library
# include <LOLIN_EPD.h>                            // include Adafruit Lolin eInk/ePaper library

# define P096_USE_ADA_GRAPHICS                     // Use AdafruitGFX_helper

# ifndef P096_USE_EXTENDED_SETTINGS
#  define P096_USE_EXTENDED_SETTINGS 1             // Allow more settings/options, made available by the AdaGFX helper
# endif // ifndef P096_USE_EXTENDED_SETTINGS

# include "../Helpers/AdafruitGFX_helper.h"        // Use Adafruit graphics helper objecr
# include "../CustomBuild/StorageLayout.h"

# define P096_Nlines 24                            // The number of different lines which can be displayed
# define P096_Nchars 60

# define P096_CONFIG_VERSION            PCONFIG(0) // Settings version
# define P096_CONFIG_ROTATION           PCONFIG(1) // Rotation
# define P096_CONFIG_WIDTH              PCONFIG(2) // Display width
# define P096_CONFIG_HEIGHT             PCONFIG(3) // Display height
// Extended setting definitions
// # define P096_CONFIG_BUTTON_PIN         PCONFIG(4)      // Pin for display-button
// # define P096_CONFIG_DISPLAY_TIMEOUT    PCONFIG(5)      // Time-out when display-button is enable
// # define P096_CONFIG_BACKLIGHT_PIN      PCONFIG(6)      // Backlight pin
// # define P096_CONFIG_BACKLIGHT_PERCENT  PCONFIG(7)      // Backlight percentage
# define P096_CONFIG_COLORS             PCONFIG_LONG(3) // 2 Colors fit in 1 long

# define P096_CONFIG_FLAGS              PCONFIG_LONG(0) // All flags, 32 bits available
// # define P096_CONFIG_FLAG_NO_WAKE       0               // Flag: Don't wake display
// # define P096_CONFIG_FLAG_INVERT_BUTTON 1               // Flag: Inverted button state
// # define P096_CONFIG_FLAG_CLEAR_ON_EXIT 2               // Flag: Clear display on exit
# define P096_CONFIG_FLAG_USE_COL_ROW   3  // Flag: Use Col/Row text addressing in commands
# define P096_CONFIG_FLAG_COMPAT_P096   4  // Flag: Compatibility -1 offset like original P096
# define P096_CONFIG_FLAG_BACK_FILL     5  // Flag: Background fill when printing text
# define P096_CONFIG_FLAG_CMD_TRIGGER   8  // Flag-offset to store 4 bits for Command trigger, uses bits 8, 9, 10 and 11
# define P096_CONFIG_FLAG_FONTSCALE     12 // Flag-offset to store 4 bits for Font scaling, uses bits 12, 13, 14 and 15
# define P096_CONFIG_FLAG_MODE          16 // Flag-offset to store 4 bits for Mode, uses bits 16, 17, 18 and 19
# define P096_CONFIG_FLAG_COLORDEPTH    20 // Flag-offset to store 4 bits for Color depth, uses bits 20, 21, 22 and 23
# define P096_CONFIG_FLAG_DISPLAYTYPE   24 // Flag-offset to store 4 bits for Display type, uses bits 24, 25, 26 and 27

// // Getters
# define P096_CONFIG_GET_COLOR_FOREGROUND   (P096_CONFIG_COLORS & 0xFFFF)
# define P096_CONFIG_GET_COLOR_BACKGROUND   ((P096_CONFIG_COLORS >> 16) & 0xFFFF)
# define P096_CONFIG_FLAG_GET_CMD_TRIGGER   (get4BitFromUL(P096_CONFIG_FLAGS, P096_CONFIG_FLAG_CMD_TRIGGER))
# define P096_CONFIG_FLAG_GET_FONTSCALE     (get4BitFromUL(P096_CONFIG_FLAGS, P096_CONFIG_FLAG_FONTSCALE))
# define P096_CONFIG_FLAG_GET_MODE          (get4BitFromUL(P096_CONFIG_FLAGS, P096_CONFIG_FLAG_MODE))
# define P096_CONFIG_FLAG_GET_COLORDEPTH    (get4BitFromUL(P096_CONFIG_FLAGS, P096_CONFIG_FLAG_COLORDEPTH))
# define P096_CONFIG_FLAG_GET_DISPLAYTYPE   (get4BitFromUL(P096_CONFIG_FLAGS, P096_CONFIG_FLAG_DISPLAYTYPE))

# ifdef ESP32

// for D32 Pro with TFT connector
  #  define P096_TFT_CS        14
  #  define P096_TFT_CS_HSPI   26 // when connected to Hardware-SPI GPIO-14 is already used
  #  define P096_TFT_DC        27
  #  define P096_TFT_RST       -1 // 33
// #  define P096_BACKLIGHT_PIN -1 // 15 // D8
# else // ifdef ESP32

// Was: for D1 Mini with shield connection
  #  define P096_TFT_CS        D3 //  0   // D0
  #  define P096_TFT_DC        D2 //  4   // D8
  #  define P096_TFT_RST       -1 // D4 // -1
// #  define P096_BACKLIGHT_PIN -1 // D6 // 15 // D8 -> Blocks Wemos
# endif // ifdef ESP32

enum class EPD_type_e : uint8_t {
  EPD_IL3897 = 0,
  EPD_UC8151D,
  EPD_SSD1680,
  EPD_MAX // must be last value in enum
};

enum class P096_CommandTrigger : uint8_t {
  epd = 0u,
  eInk,
  ePaper,
  il3897,
  uc8151d,
  ssd1680,

  MAX // Keep as last item!
};

const __FlashStringHelper* EPD_type_toString(EPD_type_e device);
const __FlashStringHelper* P096_CommandTrigger_toString(P096_CommandTrigger cmd);

struct P096_data_struct : public PluginTaskData_base {
public:

  P096_data_struct(EPD_type_e          display,
                   # if !P096_USE_EXTENDED_SETTINGS
                   uint16_t            width,
                   uint16_t            height,
                   # endif // if !P096_USE_EXTENDED_SETTINGS
                   uint8_t             rotation,
                   uint8_t             fontscaling,
                   AdaGFXTextPrintMode textmode,
                   String              commandTrigger,
                   uint16_t            fgcolor      = ADAGFX_WHITE,
                   uint16_t            bgcolor      = ADAGFX_BLACK,
                   AdaGFXColorDepth    colorDepth   = AdaGFXColorDepth::Monochrome,
                   bool                textBackFill = true);

  bool plugin_init(struct EventStruct *event);
  bool plugin_exit(struct EventStruct *event);
  bool plugin_read(struct EventStruct *event);
  bool plugin_write(struct EventStruct *event,
                    const String      & string);

  // bool plugin_ten_per_second(struct EventStruct *event);
  // bool plugin_once_a_second(struct EventStruct *event);

private:

  // void displayOnOff(bool    state,
  //                   int8_t  backlightPin,
  //                   uint8_t backlightPercentage,
  //                   uint8_t displayTimeout);
  void updateFontMetrics();

  LOLIN_EPD *eInkScreen                      = nullptr;
  uint8_t    plugin_096_sequence_in_progress = false;

  AdafruitGFX_helper *gfxHelper = nullptr;

  EPD_type_e _display;
  uint16_t   _xpix;
  uint16_t   _ypix;
  uint16_t   _textcols;
  uint16_t   _textrows;
  uint8_t    _fontwidth  = 6; // Default font characteristics
  uint8_t    _fontheight = 10;

  uint8_t             _rotation;
  uint8_t             _fontscaling;
  AdaGFXTextPrintMode _textmode;
  String              _commandTrigger;
  uint16_t            _fgcolor;
  uint16_t            _bgcolor;
  AdaGFXColorDepth    _colorDepth;
  bool                _textBackFill;

  String _commandTriggerCmd;

  int8_t _leftMarginCompensation = 0; // Not settable yet
  int8_t _topMarginCompensation  = 0;
};


#endif // ifdef USES_P096
#endif // ifndef PLUGINSTRUCTS_P096_DATA_STRUCT_H
