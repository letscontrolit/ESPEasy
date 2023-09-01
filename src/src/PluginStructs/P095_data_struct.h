#ifndef PLUGINSTRUCTS_P095_DATA_STRUCT_H
#define PLUGINSTRUCTS_P095_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P095

# include <Adafruit_GFX.h>                  // include Adafruit graphics library
# include <Adafruit_ILI9341.h>              // include Adafruit ILI9341 TFT library

# include "../Helpers/AdafruitGFX_helper.h" // Use Adafruit graphics helper object
# include "../CustomBuild/StorageLayout.h"

# define P095_Nlines           24           // The number of different lines which can be displayed
# define P095_Nchars           60
# define P095_DebounceTreshold  5           // number of 20 msec (fifty per second) ticks before the button has settled

// # define P095_ENABLE_ILI948X                            // Enable or disable support for ILI9486 and ILI9488.
// MUST reflect similar #define in Adafruit_ILI9341.h !

# ifndef LIMIT_BUILD_SIZE
#  define P095_SHOW_SPLASH                              // Enable to show initial splash (text)
# endif // ifndef LIMIT_BUILD_SIZE
# define P095_SPLASH_DURATION           (3000 / 100)    // 3 seconds in 100 millisecond chunks

# define P095_CONFIG_VERSION            PCONFIG(0)      // Settings version
# define P095_CONFIG_ROTATION           PCONFIG(1)      // Rotation
# define P095_CONFIG_BUTTON_PIN         PCONFIG(2)      // Pin for display-button
# define P095_CONFIG_DISPLAY_TIMEOUT    PCONFIG(3)      // Time-out when display-button is enable
# define P095_CONFIG_BACKLIGHT_PIN      PCONFIG(4)      // Backlight pin
# define P095_CONFIG_BACKLIGHT_PERCENT  PCONFIG(5)      // Backlight percentage
# define P095_CONFIG_COLORS            PCONFIG_ULONG(3) // 2 Colors fit in 1 long

# define P095_CONFIG_FLAGS             PCONFIG_ULONG(0) // All flags
# define P095_CONFIG_FLAG_NO_WAKE       0               // Flag: Don't wake display
# define P095_CONFIG_FLAG_INVERT_BUTTON 1               // Flag: Inverted button state
# define P095_CONFIG_FLAG_CLEAR_ON_EXIT 2               // Flag: Clear display on exit
# define P095_CONFIG_FLAG_USE_COL_ROW   3               // Flag: Use Col/Row text addressing in commands
# define P095_CONFIG_FLAG_COMPAT_P095   4               // Flag: Compatibility -1 offset like original P095
# define P095_CONFIG_FLAG_BACK_FILL     5               // Flag: Background fill when printing text
# define P095_CONFIG_FLAG_SHOW_SPLASH   6               // Flag: Show splash during startup of the plugin
# define P095_CONFIG_FLAG_INVERTDISPLAY 7               // Flag: Swap foreground and background colors/invertDisplay() (M5Stack Core2)
# define P095_CONFIG_FLAG_CMD_TRIGGER   8               // Flag-offset to store 4 bits for Command trigger, uses bits 8, 9, 10 and 11
# define P095_CONFIG_FLAG_FONTSCALE     12              // Flag-offset to store 4 bits for Font scaling, uses bits 12, 13, 14 and 15
# define P095_CONFIG_FLAG_MODE          16              // Flag-offset to store 4 bits for Mode, uses bits 16, 17, 18 and 19
# define P095_CONFIG_FLAG_TYPE          20              // Flag-offset to store 4 bits for Display type, uses bits 20..24

// // Getters
# define P095_CONFIG_GET_COLOR_FOREGROUND   (P095_CONFIG_COLORS & 0xFFFF)
# define P095_CONFIG_GET_COLOR_BACKGROUND   ((P095_CONFIG_COLORS >> 16) & 0xFFFF)
# define P095_CONFIG_FLAG_GET_CMD_TRIGGER   (get4BitFromUL(P095_CONFIG_FLAGS, P095_CONFIG_FLAG_CMD_TRIGGER))
# define P095_CONFIG_FLAG_GET_FONTSCALE     (get4BitFromUL(P095_CONFIG_FLAGS, P095_CONFIG_FLAG_FONTSCALE))
# define P095_CONFIG_FLAG_GET_MODE          (get4BitFromUL(P095_CONFIG_FLAGS, P095_CONFIG_FLAG_MODE))
# define P095_CONFIG_FLAG_GET_TYPE          (get4BitFromUL(P095_CONFIG_FLAGS, P095_CONFIG_FLAG_TYPE))
# define P095_CONFIG_FLAG_GET_SHOW_SPLASH   (!bitRead(P095_CONFIG_FLAGS, P095_CONFIG_FLAG_SHOW_SPLASH)) // Inverted setting, default on
# define P095_CONFIG_FLAG_GET_INVERTDISPLAY (bitRead(P095_CONFIG_FLAGS, P095_CONFIG_FLAG_INVERTDISPLAY))

# ifdef ESP32

// for D32 Pro with TFT connector
  #  define P095_TFT_CS        14
  #  define P095_TFT_CS_HSPI   26 // when connected to Hardware-SPI GPIO-14 is already used
  #  define P095_TFT_DC        27
  #  define P095_TFT_RST       -1 // 33
  #  define P095_BACKLIGHT_PIN -1 // 15 // D8
# else // ifdef ESP32

// Was: for D1 Mini with shield connection
  #  define P095_TFT_CS        0  // D3
  #  define P095_TFT_DC        4  // D2
  #  define P095_TFT_RST       -1 // D4 // -1
  #  define P095_BACKLIGHT_PIN -1 // D6 // 15 // D8 -> Blocks Wemos
# endif // ifdef ESP32

enum class ILI9xxx_type_e : uint8_t {
  ILI9341_240x320        = 0u,
  ILI9342_240x320        = 1u,
  ILI9481_320x480        = 2u,
  ILI9481_CPT29_320x480  = 3u,
  ILI9481_PVI35_320x480  = 4u,
  ILI9481_AUO317_320x480 = 5u,
  ILI9481_CMO35_320x480  = 6u,
  ILI9481_RGB_320x480    = 7u,
  ILI9481_CMI7_320x480   = 8u,
  ILI9481_CMI8_320x480   = 9u,
  # ifdef P095_ENABLE_ILI948X
  ILI9486_320x480 = 10u,
  ILI9488_320x480 = 11u,
  # endif // ifdef P095_ENABLE_ILI948X
};

enum class P095_CommandTrigger : uint8_t {
  tft = 0u,
  ili9341,
  ili9342,
  ili9481,
  # ifdef P095_ENABLE_ILI948X
  ili9486,
  ili9488,
  # endif // ifdef P095_ENABLE_ILI948X
};

const __FlashStringHelper* ILI9xxx_type_toString(const ILI9xxx_type_e& device);
const __FlashStringHelper* P095_CommandTrigger_toString(const P095_CommandTrigger& cmd);
void                       ILI9xxx_type_toResolution(const ILI9xxx_type_e& device,
                                                     uint16_t            & x,
                                                     uint16_t            & y);

struct P095_data_struct : public PluginTaskData_base {
public:

  P095_data_struct(ILI9xxx_type_e      displayType,
                   uint8_t             rotation,
                   uint8_t             fontscaling,
                   AdaGFXTextPrintMode textmode,
                   int8_t              backlightPin,
                   uint8_t             backlightPercentage,
                   uint32_t            displayTimer,
                   String              commandTrigger,
                   uint16_t            fgcolor      = ADAGFX_WHITE,
                   uint16_t            bgcolor      = ADAGFX_BLACK,
                   bool                textBackFill = true);
  P095_data_struct()                                = delete;
  virtual ~P095_data_struct();

  void init();

  bool plugin_init(struct EventStruct *event);
  bool plugin_exit(struct EventStruct *event);
  bool plugin_read(struct EventStruct *event);
  bool plugin_write(struct EventStruct *event,
                    const String      & string);
  # if ADAGFX_ENABLE_GET_CONFIG_VALUE
  bool plugin_get_config_value(struct EventStruct *event,
                               String            & string);
  # endif // if ADAGFX_ENABLE_GET_CONFIG_VALUE
  bool plugin_ten_per_second(struct EventStruct *event);
  bool plugin_once_a_second(struct EventStruct *event);

  void registerButtonState(const uint8_t& newButtonState,
                           const bool   & bPin3Invers);
  void markButtonStateProcessed();
  bool getButtonState() {
    return ButtonState;
  }

private:

  void displayOnOff(bool state);
  void updateFontMetrics();

  Adafruit_ILI9341 *tft = nullptr;

  AdafruitGFX_helper *gfxHelper = nullptr;

  ILI9xxx_type_e _displayType  = ILI9xxx_type_e::ILI9341_240x320;
  uint16_t       _xpix         = 240;
  uint16_t       _ypix         = 320;
  uint16_t       _textcols     = 0;
  uint16_t       _textrows     = 0;
  uint8_t        _fontwidth    = 6; // Default font characteristics
  uint8_t        _fontheight   = 10;
  uint8_t        _heightOffset = 0;

  uint8_t             _rotation            = 0;
  uint8_t             _fontscaling         = 1;
  AdaGFXTextPrintMode _textmode            = AdaGFXTextPrintMode::ClearThenTruncate;
  int8_t              _backlightPin        = -1;
  uint8_t             _backlightPercentage = 100;
  uint32_t            _displayTimer        = 0;
  uint32_t            _displayTimeout      = 0;
  String              _commandTrigger;
  uint16_t            _fgcolor      = ADAGFX_WHITE;
  uint16_t            _bgcolor      = ADAGFX_BLACK;
  bool                _textBackFill = false;

  String _commandTriggerCmd;

  // Display button
  bool    ButtonState     = false;    // button not touched
  uint8_t ButtonLastState = 0;        // Last state checked (debouncing in progress)
  uint8_t DebounceCounter = 0;        // debounce counter

  int8_t _leftMarginCompensation = 0; // Not settable yet
  int8_t _topMarginCompensation  = 0;

  bool _splashState = false;          // Have this always available to avoid 'many' #ifdefs in the code
  # ifdef P095_SHOW_SPLASH
  uint8_t _splashCounter = P095_SPLASH_DURATION;
  # endif // ifdef P095_SHOW_SPLASH

  String strings[P095_Nlines];
  bool   stringsLoaded     = false;
  bool   stringsHasContent = false;
};


#endif // ifdef USES_P095
#endif // ifndef PLUGINSTRUCTS_P095_DATA_STRUCT_H
