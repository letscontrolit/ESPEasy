#ifndef PLUGINSTRUCTS_P095_DATA_STRUCT_H
#define PLUGINSTRUCTS_P095_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P095

# include <Adafruit_GFX.h>                              // include Adafruit graphics library
# include <Adafruit_ILI9341.h>                          // include Adafruit ILI9341 TFT library

# include "../Helpers/AdafruitGFX_helper.h"             // Use Adafruit graphics helper objecr

# define P095_Nlines 24                                 // The number of different lines which can be displayed
# define P095_Nchars 50

# define P095_CONFIG_ROTATION           PCONFIG(1)      // Rotation
# define P095_CONFIG_BUTTON_PIN         PCONFIG(2)      // Pin for display-button
# define P095_CONFIG_DISPLAY_TIMEOUT    PCONFIG(3)      // Time-out when display-button is enable
# define P095_CONFIG_BACKLIGHT_PIN      PCONFIG(4)      // Backlight pin
# define P095_CONFIG_BACKLIGHT_PERCENT  PCONFIG(5)      // Backlight percentage
# define P095_CONFIG_COLORS             PCONFIG_LONG(3) // 2 Colors fit in 1 long

# define P095_CONFIG_FLAGS              PCONFIG_LONG(0) // All flags
# define P095_CONFIG_FLAG_NO_WAKE       0               // Flag: Don't wake display
# define P095_CONFIG_FLAG_INVERT_BUTTON 1               // Flag: Inverted button state
# define P095_CONFIG_FLAG_CLEAR_ON_EXIT 2               // Flag: Clear display on exit
# define P095_CONFIG_FLAG_USE_COL_ROW   3               // Flag: Use Col/Row text addressing in commands
# define P095_CONFIG_FLAG_COMPAT_P095   4               // Flag: Compatibility -1 offset like original P095
# define P095_CONFIG_FLAG_CMD_TRIGGER   8               // Flag-offset to store 4 bits for Command trigger, uses bits 8, 9, 10 and 11
# define P095_CONFIG_FLAG_FONTSCALE     12              // Flag-offset to store 4 bits for Font scaling, uses bits 12, 13, 14 and 15
# define P095_CONFIG_FLAG_MODE          16              // Flag-offset to store 4 bits for Mode, uses bits 16, 17, 18 and 19
// # define P095_CONFIG_FLAG_ROTATION      8               // Flag-offset to store 4 bits for Rotation, uses bits 8, 9, 10 and 11
// # define P095_CONFIG_FLAG_TYPE          16              // Flag-offset to store 4 bits for Hardwaretype, uses bits 16, 17, 28 and 19

// // Getters
# define P095_CONFIG_GET_COLOR_FOREGROUND   (P095_CONFIG_COLORS & 0xFFFF)
# define P095_CONFIG_GET_COLOR_BACKGROUND   ((P095_CONFIG_COLORS >> 16) & 0xFFFF)
# define P095_CONFIG_FLAG_GET_CMD_TRIGGER   (get4BitFromUL(P095_CONFIG_FLAGS, P095_CONFIG_FLAG_CMD_TRIGGER))
# define P095_CONFIG_FLAG_GET_FONTSCALE     (get4BitFromUL(P095_CONFIG_FLAGS, P095_CONFIG_FLAG_FONTSCALE))
# define P095_CONFIG_FLAG_GET_MODE          (get4BitFromUL(P095_CONFIG_FLAGS, P095_CONFIG_FLAG_MODE))

// # define P095_CONFIG_FLAG_GET_ROTATION      (get4BitFromUL(P095_CONFIG_FLAGS, P095_CONFIG_FLAG_ROTATION))
// # define P095_CONFIG_FLAG_GET_TYPE          (get4BitFromUL(P095_CONFIG_FLAGS, P095_CONFIG_FLAG_TYPE))

# ifdef ESP32

// for D32 Pro with TFT connector
  #  define P095_TFT_CS        14
  #  define P095_TFT_CS_HSPI   26 // when connected to Hardware-SPI GPIO-14 is already used
  #  define P095_TFT_DC        27
  #  define P095_TFT_RST       -1 // 33
  #  define P095_BACKLIGHT_PIN -1 // 15 // D8
# else // ifdef ESP32

// Was: for D1 Mini with shield connection
  #  define P095_TFT_CS        D3 //  0   // D0
  #  define P095_TFT_DC        D2 //  4   // D8
  #  define P095_TFT_RST       -1 // D4 // -1
  #  define P095_BACKLIGHT_PIN -1 // D6 // 15 // D8 -> Blocks Wemos
# endif // ifdef ESP32

enum class P095_CommandTrigger : uint8_t {
  tft = 0u,
  ili9341,
  MAX // Keep as last item!
};

const __FlashStringHelper* P095_CommandTrigger_toString(P095_CommandTrigger cmd);

struct P095_data_struct : public PluginTaskData_base {
public:

  P095_data_struct(uint8_t  rotation,
                   uint8_t  fontscaling,
                   AdaGFXTextPrintMode textmode,
                   uint8_t  displayTimer,
                   String   commandTrigger,
                   uint16_t fgcolor = ADAGFX_WHITE,
                   uint16_t bgcolor = ADAGFX_BLACK);

  bool plugin_init(struct EventStruct *event);
  bool plugin_exit(struct EventStruct *event);
  bool plugin_read(struct EventStruct *event);
  bool plugin_write(struct EventStruct *event,
                    const String      & string);
  bool plugin_ten_per_second(struct EventStruct *event);
  bool plugin_once_a_second(struct EventStruct *event);

private:

  void displayOnOff(bool    state,
                    int8_t  backlightPin,
                    uint8_t backlightPercentage,
                    uint8_t displayTimeout);
  void updateFontMetrics();

  Adafruit_ILI9341 *tft = nullptr;

  AdafruitGFX_helper *gfxHelper = nullptr;

  uint16_t _xpix;
  uint16_t _ypix;
  uint16_t _textcols;
  uint16_t _textrows;
  uint8_t  _fontwidth  = 6; // Default font characteristics
  uint8_t  _fontheight = 10;

  uint8_t             _rotation;
  uint8_t             _fontscaling;
  AdaGFXTextPrintMode _textmode;
  uint8_t             _displayTimer;
  String              _commandTrigger;
  uint16_t            _fgcolor;
  uint16_t            _bgcolor;

  String _commandTriggerCmd;

  int8_t _leftMarginCompensation = 0; // Not settable yet
  int8_t _topMarginCompensation  = 0;
};


#endif // ifdef USES_P095
#endif // ifndef PLUGINSTRUCTS_P095_DATA_STRUCT_H
