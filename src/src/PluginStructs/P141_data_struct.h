#ifndef PLUGINSTRUCTS_P141_DATA_STRUCT_H
#define PLUGINSTRUCTS_P141_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P141

# include <Adafruit_GFX.h>                  // include Adafruit graphics library
# include <Adafruit_PCD8544.h>              // include Adafruit PCD8544 LCD library

# include "../Helpers/AdafruitGFX_helper.h" // Use Adafruit graphics helper object

# define P141_FEATURE_CURSOR_XY_VALUES  1   // Enable availability of CursorX and CursorY values

# ifdef LIMIT_BUILD_SIZE
#  if P141_FEATURE_CURSOR_XY_VALUES
#   undef P141_FEATURE_CURSOR_XY_VALUES
#   define P141_FEATURE_CURSOR_XY_VALUES  0
#  endif // if P141_FEATURE_CURSOR_XY_VALUES
# endif // ifdef LIMIT_BUILD_SIZE

# define P141_Nlines            9                        // The number of different lines which can be displayed
# define P141_Nchars           60                        // Allow space for variables and formatting
# define P141_DebounceTreshold  5                        // number of 20 msec (fifty per second) ticks before the button has settled

# define P141_CS_PIN                    PIN(0)           // CS pin
# define P141_DC_PIN                    PIN(1)           // DC pin
# define P141_RST_PIN                   PIN(2)           // RST pin
# define P141_CONFIG_BUTTON_PIN         PIN(3)           // Pin for display-button
# define P141_CONFIG_DISPLAY_TIMEOUT    PCONFIG(1)       // Time-out when display-button is enabled
# define P141_CONFIG_CONTRAST           PCONFIG(2)       // Contrast
# define P141_CONFIG_BACKLIGHT_PIN      PCONFIG(3)       // Backlight pin
# define P141_CONFIG_BACKLIGHT_PERCENT  PCONFIG(4)       // Backlight percentage

# define P141_CONFIG_FLAGS              PCONFIG_ULONG(0) // All flags
# define P141_CONFIG_FLAG_NO_WAKE       0                // Flag: Don't wake display
# define P141_CONFIG_FLAG_INVERT_BUTTON 1                // Flag: Inverted button state
# define P141_CONFIG_FLAG_CLEAR_ON_EXIT 2                // Flag: Clear display on exit
# define P141_CONFIG_FLAG_USE_COL_ROW   3                // Flag: Use Col/Row text addressing in commands
# define P141_CONFIG_FLAG_MODE          4                // Flag-offset to store 4 bits for Mode, uses bits 4, 5, 6 and 7
# define P141_CONFIG_FLAG_ROTATION      8                // Flag-offset to store 4 bits for Rotation, uses bits 8, 9, 10 and 11
# define P141_CONFIG_FLAG_FONTSCALE     12               // Flag-offset to store 4 bits for Font scaling, uses bits 12, 13, 14 and 15
# define P141_CONFIG_FLAG_LINESPACING   16               // Flag-offset to store 4 bits for Linespacing, uses bits 16, 17, 18 and 19
# define P141_CONFIG_FLAG_CMD_TRIGGER   20               // Flag-offset to store 4 bits for Command trigger, uses bits 20, 21, 22 and 23
# define P141_CONFIG_FLAG_BACK_FILL     28               // Flag: Background fill when printing text
# define P141_CONFIG_FLAG_INVERTED      29               // Flag: Invert display content

// Getters
# define P141_CONFIG_FLAG_GET_MODE          (get4BitFromUL(P141_CONFIG_FLAGS, P141_CONFIG_FLAG_MODE))
# define P141_CONFIG_FLAG_GET_ROTATION      (get4BitFromUL(P141_CONFIG_FLAGS, P141_CONFIG_FLAG_ROTATION))
# define P141_CONFIG_FLAG_GET_FONTSCALE     (get4BitFromUL(P141_CONFIG_FLAGS, P141_CONFIG_FLAG_FONTSCALE))

# define P141_CONFIG_FLAG_GET_LINESPACING   (get4BitFromUL(P141_CONFIG_FLAGS, P141_CONFIG_FLAG_LINESPACING))
# define P141_CONFIG_FLAG_GET_CMD_TRIGGER   (get4BitFromUL(P141_CONFIG_FLAGS, P141_CONFIG_FLAG_CMD_TRIGGER))

// Define the default values for both ESP32/lolin32 and D1 Mini
# ifdef ESP32
  #  define P141_LCD_CS      14
  #  define P141_LCD_CS_HSPI 26 // when connected to Hardware-SPI GPIO-14 is already used
  #  define P141_LCD_DC      27
  #  define P141_LCD_RST     33
# else // ifdef ESP32

// for D1 Mini
  #  define P141_LCD_CS  16 // D0
  #  define P141_LCD_DC  15 // D8
  #  define P141_LCD_RST -1
# endif // ifdef ESP32

enum class P141_CommandTrigger : uint8_t {
  pcd8544 = 0u,
  lcd     = 1u,
};

const __FlashStringHelper* toString(P141_CommandTrigger cmd);

struct P141_data_struct : public PluginTaskData_base {
public:

  P141_data_struct(uint8_t             rotation,
                   uint8_t             fontscaling,
                   AdaGFXTextPrintMode textmode,
                   int8_t              backlightPin,
                   uint8_t             backlightPercentage,
                   uint8_t             contrast,
                   uint32_t            displayTimer,
                   String              commandTrigger,
                   uint16_t            fgcolor         = ADAGFX_WHITE,
                   uint16_t            bgcolor         = ADAGFX_BLACK,
                   bool                textBackFill    = true,
                   bool                displayInverted = false);
  P141_data_struct() = delete;
  virtual ~P141_data_struct();

  bool plugin_init(struct EventStruct *event);
  bool plugin_exit(struct EventStruct *event);
  bool plugin_read(struct EventStruct *event);
  bool plugin_write(struct EventStruct *event,
                    const String      & string);
  bool plugin_ten_per_second(struct EventStruct *event);
  bool plugin_once_a_second(struct EventStruct *event);
  # if ADAGFX_ENABLE_GET_CONFIG_VALUE
  bool plugin_get_config_value(struct EventStruct *event,
                               String            & string);
  # endif // if ADAGFX_ENABLE_GET_CONFIG_VALUE

  void registerButtonState(uint8_t newButtonState,
                           bool    bPin3Invers);
  void markButtonStateProcessed();
  bool getButtonState() {
    return ButtonState;
  }

private:

  void displayOnOff(bool state);
  void updateFontMetrics();
  void cleanup();
  # if P141_FEATURE_CURSOR_XY_VALUES
  void updateValues(struct EventStruct *event);
  # endif // if P141_FEATURE_CURSOR_XY_VALUES

  Adafruit_PCD8544   *pcd8544   = nullptr;
  AdafruitGFX_helper *gfxHelper = nullptr;

  uint16_t _xpix = 84; // Fixed size
  uint16_t _ypix = 48;
  uint16_t _textcols = 0;
  uint16_t _textrows = 0;
  uint8_t  _fontwidth    = 6; // Default font characteristics
  uint8_t  _fontheight   = 10;
  uint8_t  _heightOffset = 0;

  uint8_t             _rotation;
  uint8_t             _fontscaling;
  AdaGFXTextPrintMode _textmode;
  int8_t              _backlightPin;
  uint8_t             _backlightPercentage;
  uint8_t             _contrast;
  uint32_t            _displayTimer;
  uint32_t            _displayTimeout;
  String              _commandTrigger;
  uint16_t            _fgcolor;
  uint16_t            _bgcolor;
  bool                _textBackFill;
  bool                _displayInverted;

  String _commandTriggerCmd;

  // Display button
  bool    ButtonState     = false;    // button not touched
  uint8_t ButtonLastState = 0;        // Last state checked (debouncing in progress)
  uint8_t DebounceCounter = 0;        // debounce counter

  int8_t _leftMarginCompensation = 0; // Not settable yet
  int8_t _topMarginCompensation  = 0;

  String strings[P141_Nlines];
  bool   stringsLoaded     = false;
  bool   stringsHasContent = false;
};


#endif // ifdef USES_P141
#endif // ifndef PLUGINSTRUCTS_P141_DATA_STRUCT_H
