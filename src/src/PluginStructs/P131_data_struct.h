#ifndef PLUGINSTRUCTS_P131_DATA_STRUCT_H
#define PLUGINSTRUCTS_P131_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P131

# include <Adafruit_NeoMatrix.h>
# include "../Helpers/AdafruitGFX_helper.h" // Use Adafruit graphics helper object
# include "../CustomBuild/StorageLayout.h"  // Available custom storage

# include <vector>

# ifndef BUILD_NO_DEBUG
#  define P131_DEBUG_LOG // Enable for some (extra) logging
# endif // ifndef BUILD_NO_DEBUG

# define P131_Nlines  16 // The number of different lines which can be displayed
# define P131_Nchars  50

// # define P131_SHOW_SPLASH                        // Enable to show splash (text)
# define P131_SPLASH_DURATION       (3000 / 100) // 3 seconds in 100 millisecond chunks

# define P131_CONFIG_MATRIX_WIDTH   PCONFIG(0)
# define P131_CONFIG_MATRIX_HEIGHT  PCONFIG(1)
# define P131_CONFIG_TILE_WIDTH     PCONFIG(2)
# define P131_CONFIG_TILE_HEIGHT    PCONFIG(3)

# define P131_CONFIG_FLAGS          PCONFIG_ULONG(0)
# define P131_CONFIG_FLAGS_B        PCONFIG_ULONG(1)
# define P131_CONFIG_COLORS         PCONFIG_ULONG(3) // 2 Colors fit in 1 long

# define P131_MAX_SCROLL_STEPS 16                    // Max. stepsize
# define P131_MAX_SCROLL_SPEED 600                   // Max. speed

# define P131_FLAGS_MATRIX_TYPE             0        // MatrixType flags
# define P131_FLAGS_MATRIX_TYPE_TOP         0        // MatrixType flags Top/Bottom/Left/Right
# define P131_FLAGS_MATRIX_TYPE_RC          2        // MatrixType Row/Col
# define P131_FLAGS_MATRIX_TYPE_PZ          3        // MatrixType Progressive/Zigzag
# define P131_FLAGS_TILE_TYPE               4        // TileType flags
# define P131_FLAGS_TILE_TYPE_TOP           4        // TileType Top/Bottom/Left/Right
# define P131_FLAGS_TILE_TYPE_RC            6        // TileType RowCol
# define P131_FLAGS_TILE_TYPE_PZ            7        // TileType Progressive/Zigzag
# define P131_CONFIG_FLAG_ROTATION          8        // Flag-offset to store 4 bits for Rotation, uses bits 8, 9, 10, 11
# define P131_CONFIG_FLAG_MODE              12       // Flag-offset to store 4 bits for Mode, uses bits 12, 13, 14 and 15
# define P131_CONFIG_FLAG_FONTSCALE         20       // Flag-offset to store 4 bits for Font scaling, uses bits 20, 21, 22, 23
# define P131_CONFIG_FLAG_CMD_TRIGGER       24       // Flag-offset to store 4 bits for Command trigger, uses bits 24, 25, 26, 27
# define P131_CONFIG_FLAG_CLEAR_ON_EXIT     28       // Flag: Clear display on exit
# define P131_CONFIG_FLAG_STRIP_TYPE        29       // Flag: Strip type
# define P131_CONFIG_FLAG_SHOW_SPLASH       30       // Flag: Show splash

# define P131_CONFIG_FLAG_B_BRIGHTNESS      0        // Brightness, 8 bits
# define P131_CONFIG_FLAG_B_MAXBRIGHT       8        // Maximum Brightness, 8 bits

# define P131_CONFIG_FLAGS_GET_MATRIX_TYPE  (get8BitFromUL(P131_CONFIG_FLAGS, P131_FLAGS_MATRIX_TYPE))
# define P131_CONFIG_FLAG_GET_ROTATION      (get4BitFromUL(P131_CONFIG_FLAGS, P131_CONFIG_FLAG_ROTATION))
# define P131_CONFIG_FLAG_GET_FONTSCALE     (get4BitFromUL(P131_CONFIG_FLAGS, P131_CONFIG_FLAG_FONTSCALE))
# define P131_CONFIG_FLAG_GET_CMD_TRIGGER   (get4BitFromUL(P131_CONFIG_FLAGS, P131_CONFIG_FLAG_CMD_TRIGGER))
# define P131_CONFIG_FLAGS_GET_STRIP_TYPE   (bitRead(P131_CONFIG_FLAGS, P131_CONFIG_FLAG_STRIP_TYPE))

# define P131_CONFIG_FLAG_GET_MODE          (get4BitFromUL(P131_CONFIG_FLAGS, P131_CONFIG_FLAG_MODE))
# define P131_CONFIG_FLAG_GET_BRIGHTNESS    (get8BitFromUL(P131_CONFIG_FLAGS_B, P131_CONFIG_FLAG_B_BRIGHTNESS))
# define P131_CONFIG_FLAG_GET_MAXBRIGHT     (get8BitFromUL(P131_CONFIG_FLAGS_B, P131_CONFIG_FLAG_B_MAXBRIGHT))
# define P131_CONFIG_FLAG_GET_SHOW_SPLASH   (!bitRead(P131_CONFIG_FLAGS, P131_CONFIG_FLAG_SHOW_SPLASH)) // Inverted setting, default on

# define P131_CONFIG_GET_COLOR_FOREGROUND   (P131_CONFIG_COLORS & 0xFFFF)
# define P131_CONFIG_GET_COLOR_BACKGROUND   ((P131_CONFIG_COLORS >> 16) & 0xFFFF)

// Line options
# define P131_OPTBITS_SCROLL            0 // Scrolling enabled
# define P131_OPTBITS_RIGHTSCROLL       1 // Scroll in from right
# define P131_OPTBITS_STARTBLANK        3 // Start with blank display (1) or not (0)
# define P131_OPTBITS_SCROLLSTEP        4 // Number of pixels to scroll

enum class P131_CommandTrigger : uint8_t {
  neomatrix = 0u,
  neo       = 1u,
};

const __FlashStringHelper* P131_CommandTrigger_toString(P131_CommandTrigger cmd);

struct P131_content_struct {
  int16_t length      = 0;  // Intermediate storage for current length
  int16_t position    = 0;  // position within the string to display (characters)
  int16_t pixelPos    = 0;  // current left-offset on display
  int16_t speed       = 0;  // 0.1 sec. steps
  int16_t loop        = -1; // steps before we go, -1 = restart from speed
  int8_t  stepWidth   = 0;  // Nr. of pixels to scroll - 1
  bool    active      = false;
  bool    rightScroll = false;
  bool    startBlank  = false;
};

struct P131_data_struct : public PluginTaskData_base {
public:

  P131_data_struct(uint8_t             matrixWidth,
                   uint8_t             matrixHeight,
                   uint8_t             tileWidth,
                   uint8_t             tileHeight,
                   int8_t              pin,
                   uint8_t             matrixType,
                   neoPixelType        ledType,
                   uint8_t             rotation,
                   uint8_t             fontscaling,
                   AdaGFXTextPrintMode textmode,
                   String              commandTrigger,
                   uint8_t             brightness,
                   uint8_t             maxbright,
                   uint16_t            fgcolor = ADAGFX_WHITE,
                   uint16_t            bgcolor = ADAGFX_BLACK);

  P131_data_struct() = delete;
  virtual ~P131_data_struct();

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

  bool isInitialized() {
    return matrix != nullptr;
  }

private:

  void updateFontMetrics();
  void cleanup();
  void display_content(struct EventStruct *event,
                       bool                scrollOnly = false,
                       uint8_t             line       = 255);
  void loadContent(struct EventStruct *event);
  void initialize_content(struct EventStruct *event,
                          uint8_t             x);

  Adafruit_NeoMatrix *matrix    = nullptr;
  AdafruitGFX_helper *gfxHelper = nullptr;

  uint8_t             _matrixWidth  = 8;
  uint8_t             _matrixHeight = 8;
  uint8_t             _tileWidth    = 1;
  uint8_t             _tileHeight   = 1;
  int8_t              _pin          = -1;
  uint8_t             _matrixType   = NEO_MATRIX_TOP | NEO_MATRIX_LEFT | NEO_MATRIX_ROWS | NEO_MATRIX_PROGRESSIVE;
  uint8_t             _ledType      = NEO_TILE_TOP | NEO_TILE_LEFT | NEO_TILE_ROWS | NEO_TILE_PROGRESSIVE;
  uint8_t             _rotation     = 0;
  uint8_t             _fontscaling  = 1;
  AdaGFXTextPrintMode _textmode     = AdaGFXTextPrintMode::ContinueToNextLine;
  String              _commandTrigger;
  uint8_t             _brightness = 40;
  uint8_t             _maxbright  = 255;
  uint16_t            _fgcolor    = ADAGFX_WHITE;
  uint16_t            _bgcolor    = ADAGFX_BLACK;

  uint16_t _textcols     = 0;
  uint16_t _textrows     = 0;
  uint16_t _xpix         = 0u;
  uint16_t _ypix         = 0u;
  uint8_t  _fontwidth    = 6; // Default font characteristics
  uint8_t  _fontheight   = 10;
  uint8_t  _heightOffset = 0;
  bool     _textBackFill = false;
  String   _commandTriggerCmd;

  String strings[P131_Nlines]; // Read once
  bool   stringsInitialized = false;
  bool   stringsHasContent  = false;

  std::vector<P131_content_struct>content;
  bool                            contentInitialized = false;

  bool _splashState = false; // Have this always available to avoid 'many' #ifdefs in the code
  # ifdef P131_SHOW_SPLASH
  uint8_t _splashCounter = P131_SPLASH_DURATION;
  # endif // ifdef P131_SHOW_SPLASH
};
#endif // ifdef USES_P131
#endif // ifndef PLUGINSTRUCTS_P131_DATA_STRUCT_H
