#ifndef PLUGINSTRUCTS_P116_DATA_STRUCT_H
#define PLUGINSTRUCTS_P116_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P116

# include <Adafruit_GFX.h>                              // include Adafruit graphics library
# include <Adafruit_ST7735.h>                           // include Adafruit ST7735 TFT library
# include <Adafruit_ST7789.h>                           // include Adafruit ST7789 TFT library

# include "../Helpers/AdafruitGFX_helper.h"             // Use Adafruit graphics helper objecr

# define P116_Nlines 24                                 // The number of different lines which can be displayed
# define P116_Nchars 50

# define P116_USE_ADA_GRAPHICS                          // Use AdaGFX_Helper for graphics support

# define P116_CONFIG_BUTTON_PIN         PCONFIG(0)      // Pin for display-button
# define P116_CONFIG_DISPLAY_TIMEOUT    PCONFIG(1)      // Time-out when display-button is enable
# define P116_CONFIG_TYPE               PCONFIG(2)      // Type of device
# define P116_CONFIG_BACKLIGHT_PIN      PCONFIG(3)      // Backlight pin
# define P116_CONFIG_BACKLIGHT_PERCENT  PCONFIG(4)      // Backlight percentage

# define P116_CONFIG_FLAGS              PCONFIG_LONG(0) // All flags
# define P116_CONFIG_FLAG_NO_WAKE       0               // Flag: Don't wake display
# define P116_CONFIG_FLAG_INVERT_BUTTON 1               // Flag: Inverted button state
# define P116_CONFIG_FLAG_CLEAR_ON_EXIT 2               // Flag: Clear display on exit
# define P116_CONFIG_FLAG_USE_COL_ROW   3               // Flag: Use Col/Row text addressing in commands
# define P116_CONFIG_FLAG_MODE          4               // Flag-offset to store 4 bits for Mode, uses bits 4, 5, 6 and 7
# define P116_CONFIG_FLAG_ROTATION      8               // Flag-offset to store 4 bits for Rotation, uses bits 8, 9, 10 and 11
# define P116_CONFIG_FLAG_FONTSCALE     12              // Flag-offset to store 4 bits for Font scaling, uses bits 12, 13, 14 and 15
# define P116_CONFIG_FLAG_TYPE          16              // Flag-offset to store 4 bits for Hardwaretype, uses bits 16, 17, 28 and 19
# define P116_CONFIG_FLAG_CMD_TRIGGER   20              // Flag-offset to store 4 bits for Command trigger, uses bits 20, 21, 22 and 23

// Getters
# define P116_CONFIG_FLAG_GET_MODE        (get4BitFromUL(P116_CONFIG_FLAGS, P116_CONFIG_FLAG_MODE))
# define P116_CONFIG_FLAG_GET_ROTATION    (get4BitFromUL(P116_CONFIG_FLAGS, P116_CONFIG_FLAG_ROTATION))
# define P116_CONFIG_FLAG_GET_FONTSCALE   (get4BitFromUL(P116_CONFIG_FLAGS, P116_CONFIG_FLAG_FONTSCALE))
# define P116_CONFIG_FLAG_GET_TYPE        (get4BitFromUL(P116_CONFIG_FLAGS, P116_CONFIG_FLAG_TYPE))
# define P116_CONFIG_FLAG_GET_CMD_TRIGGER (get4BitFromUL(P116_CONFIG_FLAGS, P116_CONFIG_FLAG_CMD_TRIGGER))

# define P116_WRITE_PREFIX                        "tft"
# define P116_WRITE_CMD_PREFIX P116_WRITE_PREFIX  "cmd" // results in "tftcmd"

# ifdef ESP32

// for D32 Pro with TFT connector
  #  define P116_TFT_CS        14
  #  define P116_TFT_CS_HSPI   26 // when connected to Hardware-SPI GPIO-14 is already used
  #  define P116_TFT_DC        27
  #  define P116_TFT_RST       -1 // 33
  #  define P116_BACKLIGHT_PIN -1 // 15 // D8
# else // ifdef ESP32

// Was: for D1 Mini with shield connection
  #  define P116_TFT_CS        D3 //  0   // D0
  #  define P116_TFT_DC        D2 //  4   // D8
  #  define P116_TFT_RST       -1 // D4 // -1
  #  define P116_BACKLIGHT_PIN -1 // D6 // 15 // D8 -> Blocks Wemos
# endif // ifdef ESP32

enum class ST77xx_type_e : uint8_t {
  ST7735s_128x128 = 0,
  ST7735s_128x160,
  ST7735s_80x160,
  ST7789vw_240x320,
  ST7789vw_240x240,
  ST7789vw_240x280,
  ST77xx_MAX // must be last value in enum
};

enum class P116_CommandTrigger : uint8_t {
  tft = 0u,
  st77xx,
  st7735,
  st7789
};

const __FlashStringHelper* ST77xx_type_toString(ST77xx_type_e device);
const __FlashStringHelper* P116_CommandTrigger_toString(P116_CommandTrigger cmd);

struct P116_data_struct : public PluginTaskData_base {
public:

  P116_data_struct(ST77xx_type_e       device,
                   uint8_t             rotation,
                   uint8_t             fontscaling,
                   AdaGFXTextPrintMode textmode,
                   uint8_t             displayTimer,
                   String              commandTrigger);

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

  Adafruit_ST77xx *st77xx = nullptr;
  Adafruit_ST7735 *st7735 = nullptr;
  Adafruit_ST7789 *st7789 = nullptr;

  # ifdef P116_USE_ADA_GRAPHICS
  AdafruitGFX_helper *gfxHelper = nullptr;
  # endif // ifdef P116_USE_GRAPHICS
  enum ST77xx_type_e _device;

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

  String _commandTriggerCmd;

  int8_t _leftMarginCompensation = 0; // Not settable yet
  int8_t _topMarginCompensation  = 0;
};


#endif // ifdef USES_P116
#endif // ifndef PLUGINSTRUCTS_P116_DATA_STRUCT_H
