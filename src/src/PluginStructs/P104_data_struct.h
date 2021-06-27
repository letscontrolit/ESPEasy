#ifndef PLUGINSTRUCTS_P104_DATA_STRUCT_H
#define PLUGINSTRUCTS_P104_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P104

# define P104_DEBUG     // Log some extra (tech) data, also useful during development
# define P104_DEBUG_DEV // Log some extra development info

# include "../CustomBuild/StorageLayout.h"
# include "src/Globals/EventQueue.h"
# include "src/Globals/MQTT.h"
# include "src/Globals/CPlugins.h"
# include "src/Globals/Plugins.h"
# include "src/Helpers/ESPEasy_Storage.h"
# include "src/Helpers/Hardware.h"
# include "src/Helpers/Misc.h"
# include "src/Helpers/StringParser.h"

// # define P104_USE_NUMERIC_DOUBLEHEIGHT_FONT                 // Enables double height numeric font for double-height time/date
# define P104_USE_FULL_DOUBLEHEIGHT_FONT                    // Enables the use of a full (lower ascii only) set double height font
# define P104_USE_VERTICAL_FONT                             // Enables the use of a vertical font
# define P104_USE_EXT_ASCII_FONT                            // Enables the use of an extended ascii font
# define P104_USE_ARABIC_FONT                               // Enables the use of a Arabic font (see usage in MD_Parola examples)
# define P104_USE_GREEK_FONT                                // Enables the use of a Greek font (see usage in MD_Parola examples)
# define P104_USE_KATAKANA_FONT                             // Enables the use of a Katakana font (see usage in MD_Parola examples)
# define P104_USE_COMMANDS                                  // Enables the use of all commands, not just clear, txt, settxt and update
# define P104_USE_DATETIME_OPTIONS                          // Enables extra date/time options
# define P104_USE_BAR_GRAPH                                 // Enables the use of Bar-graph feature

# define P104_ADD_SETTINGS_NOTES                            // Adds some notes on the Settings page

# if defined(PLUGIN_DISPLAY_COLLECTION) && defined(ESP8266) // To make it fit in the ESP8266 display build
#  ifdef P104_USE_FULL_DOUBLEHEIGHT_FONT
#   undef P104_USE_FULL_DOUBLEHEIGHT_FONT
#   ifndef P104_USE_NUMERIC_DOUBLEHEIGHT_FONT
#    define P104_USE_NUMERIC_DOUBLEHEIGHT_FONT
#   endif // ifndef P104_USE_NUMERIC_DOUBLEHEIGHT_FONT
#  endif  // ifdef P104_USE_FULL_DOUBLEHEIGHT_FONT
#  ifdef P104_USE_VERTICAL_FONT
#   undef P104_USE_VERTICAL_FONT
#  endif  // ifdef P104_USE_VERTICAL_FONT
#  ifdef P104_USE_EXT_ASCII_FONT
#   undef P104_USE_EXT_ASCII_FONT
#  endif  // ifdef P104_USE_EXT_ASCII_FONT
#  ifdef P104_USE_ARABIC_FONT
#   undef P104_USE_ARABIC_FONT
#  endif  // ifdef P104_USE_ARABIC_FONT
#  ifdef P104_USE_GREEK_FONT
#   undef P104_USE_GREEK_FONT
#  endif  // ifdef P104_USE_GREEK_FONT
#  ifdef P104_USE_KATAKANA_FONT
#   undef P104_USE_KATAKANA_FONT
#  endif  // ifdef P104_USE_KATAKANA_FONT
#  ifdef P104_USE_COMMANDS
#   undef P104_USE_COMMANDS
#  endif  // ifdef P104_USE_COMMANDS
#  ifdef P104_USE_DATETIME_OPTIONS
#   undef P104_USE_DATETIME_OPTIONS
#  endif  // ifdef P104_USE_DATETIME_OPTIONS
#  ifdef P104_ADD_SETTINGS_NOTES
#   undef P104_ADD_SETTINGS_NOTES
#  endif  // ifdef P104_ADD_SETTINGS_NOTES
#  ifdef P104_DEBUG
#   undef P104_DEBUG
#  endif  // ifdef P104_DEBUG
#  ifdef P104_DEBUG_DEV
#   undef P104_DEBUG_DEV
#  endif  // ifdef P104_DEBUG_DEV
#  define P104_MEDIUM_ANIMATIONS
# endif   // if defined(PLUGIN_DISPLAY_COLLECTION) && defined(ESP8266)

// # define P104_MINIMAL_ANIMATIONS            // disable most animations
// # define P104_MEDIUM_ANIMATIONS             // disable some complex animations


# define P104_MAX_MESG  15           // Message size for time/date

# ifdef ESP32
#  define P104_MAX_ZONES        16u  // 1..P104_MAX_ZONES zones selectable
#  define P104_SETTINGS_BUFFER  1020 // Bigger buffer possible on ESP32
# else // ifdef ESP32
#  define P104_MAX_ZONES        8u   // 1..P104_MAX_ZONES zones selectable
#  define P104_SETTINGS_BUFFER  512
# endif // ifdef ESP32

# define P104_MAX_MODULES_PER_ZONE     64    // Maximum allowed modules per zone
# define P104_MAX_TEXT_LENGTH_PER_ZONE 100   // Limit the Text content length
# define P104_MAX_SPEED_PAUSE_VALUE    65535 // Value is in milliseconds
# define P104_MAX_REPEATDELAY_VALUE    86400 // Value is in seconds

# define P104_USE_TOOLTIPS                   // Enable tooltips in UI

# ifdef LIMIT_BUILD_SIZE
#  ifdef P104_DEBUG
#   undef P104_DEBUG
#  endif // ifdef P104_DEBUG
#  ifdef P104_DEBUG_DEV
#   undef P104_DEBUG_DEV
#  endif // ifdef P104_DEBUG_DEV
#  ifdef P104_USE_TOOLTIPS
#   undef P104_USE_TOOLTIPS
#  endif // ifdef P104_USE_TOOLTIPS
// Disable all fonts
#  ifdef P104_USE_FULL_DOUBLEHEIGHT_FONT
#   undef P104_USE_FULL_DOUBLEHEIGHT_FONT
#  endif // ifdef P104_USE_FULL_DOUBLEHEIGHT_FONT
#  ifdef P104_USE_VERTICAL_FONT
#   undef P104_USE_VERTICAL_FONT
#  endif // ifdef P104_USE_VERTICAL_FONT
#  ifdef P104_USE_EXT_ASCII_FONT
#   undef P104_USE_EXT_ASCII_FONT
#  endif // ifdef P104_USE_EXT_ASCII_FONT
#  ifdef P104_USE_ARABIC_FONT
#   undef P104_USE_ARABIC_FONT
#  endif // ifdef P104_USE_ARABIC_FONT
#  ifdef P104_USE_GREEK_FONT
#   undef P104_USE_GREEK_FONT
#  endif // ifdef P104_USE_GREEK_FONT
#  ifdef P104_USE_KATAKANA_FONT
#   undef P104_USE_KATAKANA_FONT
#  endif // ifdef P104_USE_KATAKANA_FONT
# endif    // ifdef LIMIT_BUILD_SIZE

# if defined(P104_USE_TOOLTIPS) && !defined(ENABLE_TOOLTIPS)
#  undef P104_USE_TOOLTIPS
# endif // if defined(P104_USE_TOOLTIPS) && !defined(ENABLE_TOOLTIPS)


# ifdef P104_MINIMAL_ANIMATIONS
#  define ENA_MISC 0 // Disabling some MD_Parola features
#  define ENA_WIPE 0
#  define ENA_SCAN 0
#  ifndef P104_MEDIUM_ANIMATIONS
#   define P104_MEDIUM_ANIMATIONS
#  endif // ifndef P104_MEDIUM_ANIMATIONS
# endif  // ifdef P104_MINIMAL_ANIMATIONS

# ifdef P104_MEDIUM_ANIMATIONS
#  define ENA_SPRITE  0 // Disabling more MD_Parola features
#  define ENA_OPNCLS  0
#  define ENA_SCR_DIA 0
#  define ENA_GROW    0
# endif // ifdef P104_MEDIUM_ANIMATIONS

# include <MD_Parola.h>
# include <MD_MAX72xx.h>

// WARNING: Order of values should match the numeric order of P104_OFFSET_* values
# define P104_OFFSET_SIZE         0u
# define P104_OFFSET_TEXT         1u
# define P104_OFFSET_CONTENT      2u
# define P104_OFFSET_ALIGNMENT    3u
# define P104_OFFSET_ANIM_IN      4u
# define P104_OFFSET_SPEED        5u
# define P104_OFFSET_ANIM_OUT     6u
# define P104_OFFSET_PAUSE        7u
# define P104_OFFSET_FONT         8u
# define P104_OFFSET_LAYOUT       9u
# define P104_OFFSET_SPEC_EFFECT  10u
# define P104_OFFSET_OFFSET       11u
# define P104_OFFSET_BRIGHTNESS   12u
# define P104_OFFSET_REPEATDELAY  13u

# define P104_OFFSET_COUNT        14u // Highest P104_OFFSET_* defined + 1

# define P104_CONFIG_ZONE_COUNT   PCONFIG(0)
# define P104_CONFIG_TOTAL_UNITS  PCONFIG(1)
# define P104_CONFIG_HARDWARETYPE PCONFIG(2)
# define P104_CONFIG_FLAGS        PCONFIG_LONG(0)
# define P104_CONFIG_DATETIME     PCONFIG_LONG(1)

# define P104_CONFIG_FLAG_CLEAR_DISABLE 0
# define P104_CONFIG_FLAG_LOG_ALL_TEXT  1

# define P104_CONFIG_DATETIME_FLASH     0
# define P104_CONFIG_DATETIME_12H       1
# define P104_CONFIG_DATETIME_AMPM      2
# define P104_CONFIG_DATETIME_YEAR4DGT  3
# define P104_CONFIG_DATETIME_FORMAT    4 // Uses 4 bits, leave some space for a few options
# define P104_CONFIG_DATETIME_SEP_CHAR  8 // Uses 4 bits

# define P104_DATE_FORMAT_EU        0
# define P104_DATE_FORMAT_US        1
# define P104_DATE_FORMAT_JP        2

# define P104_DATE_SEPARATOR_SPACE  0
# define P104_DATE_SEPARATOR_SLASH  1
# define P104_DATE_SEPARATOR_DASH   2
# define P104_DATE_SEPARATOR_DOT    3

# define P104_CONTENT_TEXT        0
# define P104_CONTENT_TIME        1
# define P104_CONTENT_TIME_SEC    2
# define P104_CONTENT_DATE4       3
# define P104_CONTENT_DATE6       4
# define P104_CONTENT_DATE_TIME   5
# define P104_CONTENT_BAR_GRAPH   6
# ifdef P104_USE_BAR_GRAPH
#  define P104_CONTENT_count       7 // The number of content type options
# else // ifdef P104_USE_BAR_GRAPH
#  define P104_CONTENT_count       6 // The number of content type options
# endif // ifdef P104_USE_BAR_GRAPH

# define P104_SPECIAL_EFFECT_NONE       0
# define P104_SPECIAL_EFFECT_UP_DOWN    1
# define P104_SPECIAL_EFFECT_LEFT_RIGHT 2
# define P104_SPECIAL_EFFECT_BOTH       P104_SPECIAL_EFFECT_UP_DOWN + P104_SPECIAL_EFFECT_LEFT_RIGHT // Used as a bitmap

# define P104_LAYOUT_STANDARD     0
# define P104_LAYOUT_DOUBLE_UPPER 1
# define P104_LAYOUT_DOUBLE_LOWER 2

# define P104_BRIGHTNESS_MAX      15 // Brightness levels range from 0 .. 15

# define P104_BARTYPE_STANDARD    0  // Solid line, with zero-mark if width > 2
# define P104_BARTYPE_SINGLE      1  // Solid single-line, even if width allows more
# define P104_BARTYPE_ALT_DOT     2  // Dotted line, alternating odd/even, only if bar width > 1

// Font related stuff

// To add a font:
// - generate the font using one of the font tools (or obtain externally, or craft it manually...)
// - add a new font define, like #define P104_USE_MY_FANCY_FONT
// - select an unused new numeric font ID define, like #define P104_MY_FANCY_FONT_ID n
// - include the .h file guarded by #ifdef P104_USE_MY_FANCY_FONT
// - extend in P104_data_struct::webform_load the fontTypes, fontOptions arrays like the P104_USE_NUMERIC_DOUBLEHEIGHT_FONT example
// - extend in P104_data_struct::handlePluginWrite the list of supported font id's for the "font" command
// - extend in P104_data_struct::configureZones the switch/case statement to conditionaly support the new font
// - update documentation

// This is the default font id
# define P104_DEFAULT_FONT_ID     0

// These fonts are copied from the MD_Parola examples
# ifdef P104_USE_NUMERIC_DOUBLEHEIGHT_FONT

#  define P104_DOUBLE_HEIGHT_FONT_ID 1

#  include "../Static/Fonts/P104_font_numeric7SegDouble.h"
# endif // ifdef P104_USE_NUMERIC_DOUBLEHEIGHT_FONT

# ifdef P104_USE_FULL_DOUBLEHEIGHT_FONT

#  define P104_FULL_DOUBLEHEIGHT_FONT_ID 2

#  include "../Static/Fonts/P104_font_BigFont.h"
# endif // ifdef P104_USE_FULL_DOUBLEHEIGHT_FONT

# ifdef P104_USE_VERTICAL_FONT

#  define P104_VERTICAL_FONT_ID 3

#  include "../Static/Fonts/P104_font_vertical.h"
# endif // ifdef P104_USE_VERTICAL_FONT

# ifdef P104_USE_EXT_ASCII_FONT

#  define P104_EXT_ASCII_FONT_ID 4

#  include "../Static/Fonts/P104_font_ExtASCII.h"
# endif // ifdef P104_USE_EXT_ASCII_FONT

# ifdef P104_USE_ARABIC_FONT

#  define P104_ARABIC_FONT_ID 5

#  include "../Static/Fonts/P104_font_arabic.h"
# endif // ifdef P104_USE_ARABIC_FONT

# ifdef P104_USE_GREEK_FONT

#  define P104_GREEK_FONT_ID 6

#  include "../Static/Fonts/P104_font_greek.h"
# endif // ifdef P104_USE_GREEK_FONT

# ifdef P104_USE_KATAKANA_FONT

#  define P104_KATAKANA_FONT_ID 7

#  include "../Static/Fonts/P104_font_katakana.h"
# endif // ifdef P104_USE_KATAKANA_FONT

struct P104_zone_struct {
  P104_zone_struct(uint8_t _zone) : zone(_zone) {}

  uint8_t  zone;
  uint8_t  size;
  String   text;
  uint8_t  alignment;
  uint8_t  animationIn, animationOut;
  uint8_t  font;
  uint8_t  content;
  uint8_t  layout;
  uint8_t  specialEffect;
  uint8_t  offset;
  uint8_t  brightness;
  uint16_t speed, pause;
  int32_t  repeatDelay;
  uint32_t _repeatTimer;
  int8_t   _lastChecked = -1;
  # ifdef P104_USE_BAR_GRAPH
  uint8_t  _startModule;   // starting module, end module is _startModule + size - 1
  uint16_t _lower, _upper; // lower and upper pixel numbers
  # endif // ifdef P104_USE_BAR_GRAPH
};

# ifdef P104_USE_BAR_GRAPH
struct P104_bargraph_struct {
  P104_bargraph_struct(uint8_t _graph) : graph(_graph) {}

  uint8_t graph;
  double  value;
  double  max;
  double  min;
  uint8_t barType;
  uint8_t direction;
};
# endif // ifdef P104_USE_BAR_GRAPH

struct tP104_StoredSettings {
  uint16_t bufferSize = 0u;
  char     buffer[P104_SETTINGS_BUFFER]; // Max. acceptable value would be ~1020
};

struct P104_data_struct : public PluginTaskData_base {
  P104_data_struct(MD_MAX72XX::moduleType_t _mod,
                   taskIndex_t              _taskIndex,
                   int8_t                   _cs_pin,
                   byte                     _modules);

  bool   begin();
  void   loadSettings();
  bool   saveSettings();
  bool   webform_load(struct EventStruct *event);
  bool   webform_save(struct EventStruct *event);
  String getError() {
    return error;
  }

  void configureZones();

  void setZones(uint16_t _zones) {
    expectedZones = _zones;
  }

  bool handlePluginWrite(taskIndex_t   taskIndex,
                         const String& string);
  bool handlePluginOncePerSecond(struct EventStruct *event);
  void checkRepeatTimer(uint8_t z);
  void updateZone(uint8_t                 zone,
                  const P104_zone_struct& zstruct);

  MD_Parola *P = nullptr;

  bool logAllText = false;

private:

  # ifdef P104_USE_BAR_GRAPH
  MD_MAX72XX *pM = nullptr;
  void displayBarGraph(uint8_t                 zone,
                       const P104_zone_struct& zstruct,
                       const String          & graph);
  void modulesOnOff(uint8_t                    start,
                    uint8_t                    end,
                    MD_MAX72XX::controlValue_t on_off);
  # endif // ifdef P104_USE_BAR_GRAPH

  void displayOneZoneText(uint8_t                 currentZone,
                          const P104_zone_struct& idx,
                          const String          & text);

  MD_MAX72XX::moduleType_t mod;
  taskIndex_t              taskIndex;
  int8_t                   cs_pin;
  uint8_t                  modules = 1u;

  bool    initialized   = false;
  int8_t  expectedZones = -1;
  int8_t  previousZones = -1;
  uint8_t numDevices    = 0;

  String error;

  std::vector<P104_zone_struct>zones;
  bool                         zonesInitialized = false;
  String                       sZoneBuffers[P104_MAX_ZONES];
  String                       sZoneInitial[P104_MAX_ZONES];

  // time/date stuff
  bool flasher = false;        // seconds passing flasher
  char szTimeL[P104_MAX_MESG]; // dd-mm-yy mm:ss\0
  char szTimeH[P104_MAX_MESG];

  // Stored settings
  tP104_StoredSettings StoredSettings;
};

#endif // ifdef USES_P104
#endif // ifndef PLUGINSTRUCTS_P104_DATA_STRUCT_H
