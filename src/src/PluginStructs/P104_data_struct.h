#ifndef PLUGINSTRUCTS_P104_DATA_STRUCT_H
#define PLUGINSTRUCTS_P104_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P104

// # define P104_DEBUG // Log some extra (tech) data, also useful during development
// # define P104_DEBUG_DEV // Log some extra development info

# include "../CustomBuild/StorageLayout.h"
# include "../Globals/EventQueue.h"
# include "../Globals/MQTT.h"
# include "../Globals/CPlugins.h"
# include "../Globals/Plugins.h"
# include "../Helpers/ESPEasy_Storage.h"
# include "../Helpers/Hardware.h"
# include "../Helpers/Misc.h"
# include "../Helpers/StringParser.h"

# include <vector>

# if defined(PLUGIN_SET_MAX) || defined(PLUGIN_BUILD_CUSTOM)
#  define P104_USE_NUMERIC_DOUBLEHEIGHT_FONT // Enables double height numeric font for double-height time/date
# endif // if defined(PLUGIN_SET_MAX) || defined(PLUGIN_BUILD_CUSTOM)
# define P104_USE_FULL_DOUBLEHEIGHT_FONT     // Enables the use of a full (lower ascii only) set double height font
# define P104_USE_VERTICAL_FONT              // Enables the use of a vertical font
# define P104_USE_EXT_ASCII_FONT             // Enables the use of an extended ascii font
# define P104_USE_ARABIC_FONT                // Enables the use of a Arabic font (see usage in MD_Parola examples)
# define P104_USE_GREEK_FONT                 // Enables the use of a Greek font (see usage in MD_Parola examples)
# define P104_USE_KATAKANA_FONT              // Enables the use of a Katakana font (see usage in MD_Parola examples)
# define P104_USE_COMMANDS                   // Enables the use of all commands, not just clear, txt, settxt and update
# define P104_USE_DATETIME_OPTIONS           // Enables extra date/time options
# define P104_USE_BAR_GRAPH                  // Enables the use of Bar-graph feature
# define P104_USE_ZONE_ACTIONS               // Enables the use of Actions per zone (New above/New below/Delete)
# define P104_USE_ZONE_ORDERING              // Enables the use of Zone ordering (Numeric order (1..n)/Display order (n..1))

# define P104_ADD_SETTINGS_NOTES             // Adds some notes on the Settings page

// To make it fit in the ESP8266 display build
# if defined(PLUGIN_DISPLAY_COLLECTION) && defined(ESP8266) && !defined(LIMIT_BUILD_SIZE)
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
// #  ifdef P104_USE_COMMANDS
// #   undef P104_USE_COMMANDS
// #  endif  // ifdef P104_USE_COMMANDS
#  ifdef P104_USE_DATETIME_OPTIONS
#   undef P104_USE_DATETIME_OPTIONS
#  endif // ifdef P104_USE_DATETIME_OPTIONS
#  ifdef P104_ADD_SETTINGS_NOTES
#   undef P104_ADD_SETTINGS_NOTES
#  endif // ifdef P104_ADD_SETTINGS_NOTES
#  ifdef P104_DEBUG
#   undef P104_DEBUG
#  endif // ifdef P104_DEBUG
#  ifdef P104_DEBUG_DEV
#   undef P104_DEBUG_DEV
#  endif // ifdef P104_DEBUG_DEV
#  define P104_MEDIUM_ANIMATIONS
# endif   // if defined(PLUGIN_DISPLAY_COLLECTION) && defined(ESP8266)

// # define P104_MINIMAL_ANIMATIONS            // disable most animations
// # define P104_MEDIUM_ANIMATIONS             // disable some complex animations


# define P104_MAX_MESG             20        // Message size for time/date (dd-mm-yyyy hh:mm:ss\0)

# ifdef ESP32
#  define P104_MAX_ZONES           16u       // 1..P104_MAX_ZONES zones selectable
#  define P104_SETTINGS_BUFFER_V1  1020      // Bigger buffer possible on ESP32
# else // ifdef ESP32
#  define P104_MAX_ZONES           8u        // 1..P104_MAX_ZONES zones selectable
#  define P104_SETTINGS_BUFFER_V1  512
# endif // ifdef ESP32
# define P104_SETTINGS_BUFFER_V2   150       // Settings stored per zone only needs this kind of buffer size, max. length without text is 45
                                             // bytes, 100 chars + 2 quotes around it => 150 (rounded up to a nice number)

# define P104_MAX_MODULES_PER_ZONE     255   // Maximum supported modules per zone
# define P104_MAX_TEXT_LENGTH_PER_ZONE 100   // Limit the Text content length
# define P104_MAX_SPEED_PAUSE_VALUE    65535 // Value is in milliseconds
# define P104_MAX_REPEATDELAY_VALUE    86400 // Value is in seconds

# define P104_USE_TOOLTIPS                   // Enable tooltips in UI

# ifdef LIMIT_BUILD_SIZE

// #  ifdef P104_DEBUG
// #   undef P104_DEBUG
// #  endif // ifdef P104_DEBUG
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
// #  ifdef P104_USE_VERTICAL_FONT
// #   undef P104_USE_VERTICAL_FONT
// #  endif // ifdef P104_USE_VERTICAL_FONT
// #  ifdef P104_USE_EXT_ASCII_FONT
// #   undef P104_USE_EXT_ASCII_FONT
// #  endif // ifdef P104_USE_EXT_ASCII_FONT
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

# if defined(P104_USE_TOOLTIPS) && !FEATURE_TOOLTIPS
#  undef P104_USE_TOOLTIPS
# endif // if defined(P104_USE_TOOLTIPS) && !FEATURE_TOOLTIPS


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
# define P104_OFFSET_INVERTED     14u
# define P104_OFFSET_ACTION       15u // Should be the last settings option, after all the settings that are stored

# define P104_OFFSET_COUNT        16u // Highest P104_OFFSET_* defined + 1

# define P104_CONFIG_ZONE_COUNT   PCONFIG(0)
# define P104_CONFIG_TOTAL_UNITS  PCONFIG(1)
# define P104_CONFIG_HARDWARETYPE PCONFIG(2)
# define P104_CONFIG_FLAGS        PCONFIG_ULONG(0)
# define P104_CONFIG_DATETIME     PCONFIG_ULONG(1)

# define P104_CONFIG_FLAG_CLEAR_DISABLE 0
# define P104_CONFIG_FLAG_LOG_ALL_TEXT  1
# define P104_CONFIG_FLAG_ZONE_ORDER    2

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

# define P104_ACTION_NONE         0
# define P104_ACTION_ADD_ABOVE    1
# define P104_ACTION_ADD_BELOW    2
# define P104_ACTION_DELETE       3

# define P104_CONTENT_TEXT        0
# define P104_CONTENT_TEXT_REV    1
# define P104_CONTENT_TIME        2
# define P104_CONTENT_TIME_SEC    3
# define P104_CONTENT_DATE4       4
# define P104_CONTENT_DATE6       5
# define P104_CONTENT_DATE_TIME   6
# define P104_CONTENT_BAR_GRAPH   7
# ifdef P104_USE_BAR_GRAPH
#  define P104_CONTENT_count       8 // The number of content type options
# else // ifdef P104_USE_BAR_GRAPH
#  define P104_CONTENT_count       7 // The number of content type options
# endif // ifdef P104_USE_BAR_GRAPH

# define P104_SPECIAL_EFFECT_NONE       0
# define P104_SPECIAL_EFFECT_UP_DOWN    1
# define P104_SPECIAL_EFFECT_LEFT_RIGHT 2
# define P104_SPECIAL_EFFECT_BOTH       (P104_SPECIAL_EFFECT_UP_DOWN + P104_SPECIAL_EFFECT_LEFT_RIGHT) // Used as a bitmap

# define P104_LAYOUT_STANDARD     0
# define P104_LAYOUT_DOUBLE_UPPER 1
# define P104_LAYOUT_DOUBLE_LOWER 2

# define P104_BRIGHTNESS_MAX      15 // Brightness levels range from 0 .. 15
# define P104_BRIGHTNESS_DEFAULT  3  // Default brightness level

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
//   the description should include the font ID for documentation purposes
//   don't forget to guard the extra code with #ifdef P104_USE_MY_FANCY_FONT
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
  P104_zone_struct() = delete; // Not used, so leave out explicitly
  P104_zone_struct(uint8_t _zone) :  text(F("\"\"")), zone(_zone) {}

  String   text;
  int32_t  repeatDelay  = -1;
  uint32_t _repeatTimer = 0u;
  uint16_t speed        = 0u;
  uint16_t pause        = 0u;
  uint8_t  zone;
  uint8_t  size          = 0u;
  uint8_t  alignment     = 0u;
  uint8_t  animationIn   = 1u; // Doesn't allow 'None'
  uint8_t  animationOut  = 0u;
  uint8_t  font          = 0u;
  uint8_t  content       = 0u;
  uint8_t  layout        = 0u;
  uint8_t  specialEffect = 0u;
  uint8_t  offset        = 0u;
  int8_t   brightness    = -1;
  int8_t   inverted      = 0;
  int8_t   _lastChecked  = -1;
  # ifdef P104_USE_BAR_GRAPH
  uint16_t _lower       = 0u;
  uint16_t _upper       = 0u; // lower and upper pixel numbers
  uint8_t  _startModule = 0u; // starting module, end module is _startModule + size - 1
  # endif // ifdef P104_USE_BAR_GRAPH
};

# ifdef P104_USE_BAR_GRAPH
struct P104_bargraph_struct {
  P104_bargraph_struct() = delete; // Not used, so leave out explicitly
  P104_bargraph_struct(uint8_t _graph) : graph(_graph) {}

  double  value = 0.0;
  double  max   = 0.0;
  double  min   = 0.0;
  uint8_t graph;
  uint8_t barType   = 0u;
  uint8_t direction = 0u;
};
# endif // ifdef P104_USE_BAR_GRAPH

struct P104_data_struct : public PluginTaskData_base {
  P104_data_struct() = delete; // Not used, so leave out explicitly
  P104_data_struct(MD_MAX72XX::moduleType_t _mod,
                   taskIndex_t              _taskIndex,
                   int8_t                   _cs_pin,
                   uint8_t                  _modules,
                   uint8_t                  _zonesCount);
  virtual ~P104_data_struct();

  bool   begin();
  void   loadSettings();
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

  MD_Parola *P = nullptr;

  bool logAllText = false;

private:

  bool saveSettings();
  void updateZone(uint8_t                 zone,
                  const P104_zone_struct& zstruct);
  # ifdef P104_USE_BAR_GRAPH
  MD_MAX72XX *pM = nullptr;
  void displayBarGraph(uint8_t                 zone,
                       const P104_zone_struct& zstruct,
                       const String          & graph);
  void modulesOnOff(uint8_t                    start,
                    uint8_t                    end,
                    MD_MAX72XX::controlValue_t on_off);
  void drawOneBarGraph(uint16_t lower,
                       uint16_t upper,
                       int16_t  pixBottom,
                       int16_t  pixTop,
                       uint16_t zeroPoint,
                       uint8_t  barWidth,
                       uint8_t  barType,
                       uint8_t  row);
  # endif // ifdef P104_USE_BAR_GRAPH

  void displayOneZoneText(uint8_t                 currentZone,
                          const P104_zone_struct& idx,
                          const String          & text);

  String error;

  std::vector<P104_zone_struct>zones;
  String                       sZoneBuffers[P104_MAX_ZONES];
  String                       sZoneInitial[P104_MAX_ZONES];

  MD_MAX72XX::moduleType_t mod;
  taskIndex_t              taskIndex;
  int8_t                   cs_pin;
  uint8_t                  modules;

  uint16_t numDevices    = 0u;
  uint8_t  zoneOrder     = 0u;
  int8_t   expectedZones = -1;
  int8_t   previousZones = -1;
  bool     initialized   = false;
  bool     flasher       = false;      // seconds passing flasher

  // time/date stuff
  char szTimeL[P104_MAX_MESG] = { 0 }; // dd-mm-yyyy hh:mm:ss\0
  char szTimeH[P104_MAX_MESG] = { 0 };

  int8_t getTime(char *psz,
                 bool  seconds  = false,
                 bool  colon    = true,
                 bool  time12h  = false,
                 bool  timeAmpm = false);
  void getDate(char         *psz,
               bool          showYear = true,
               bool          fourDgt  = false
               # ifdef       P104_USE_DATETIME_OPTIONS
               ,
               const uint8_t dateFmt = 0
               ,
               const uint8_t dateSep = 0
               # endif // ifdef P104_USE_DATETIME_OPTIONS
               );
  uint8_t getDateTime(char         *psz,
                      bool          colon    = true,
                      bool          time12h  = false,
                      bool          timeAmpm = false,
                      bool          fourDgt  = false
                      # ifdef       P104_USE_DATETIME_OPTIONS
                      ,
                      const uint8_t dateFmt = 0
                      ,
                      const uint8_t dateSep = 0
                      # endif // ifdef P104_USE_DATETIME_OPTIONS
                      );
  # if defined(P104_USE_NUMERIC_DOUBLEHEIGHT_FONT) || defined(P104_USE_FULL_DOUBLEHEIGHT_FONT)
  void createHString(String& string);
  # endif // if defined(P104_USE_NUMERIC_DOUBLEHEIGHT_FONT) || defined(P104_USE_FULL_DOUBLEHEIGHT_FONT)
  void reverseStr(String& str);
};

#endif // ifdef USES_P104
#endif // ifndef PLUGINSTRUCTS_P104_DATA_STRUCT_H
