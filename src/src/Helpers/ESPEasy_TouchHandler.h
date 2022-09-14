#ifndef HELPERS_ESPEASY_TOUCHHANDLER_H
#define HELPERS_ESPEASY_TOUCHHANDLER_H

#include "../../_Plugin_Helper.h"
#include "../../ESPEasy_common.h"
#include "../Helpers/AdafruitGFX_helper.h"

#ifdef PLUGIN_USES_TOUCHHANDLER
# include "../Commands/InternalCommands.h"
# include <set>

/*****
 * Changelog:
 * 2022-08-27 tonhuisman: Enable identifying an object using the <groupnr>.<objectnr> notation, where groupnr = 0..255, and objectnr
 *                        range starts at 1, in sequential order for the objects in that group
 *                        Add option to disable polling the touch-screen, to be able to only use the object/button draw features
 *                        f.e. when using a device like an M5Stack Core that has buttons below the screen
 * 2022-08-22 tonhuisman: Improve Slider range handling so a reverse range (40,-10) can also be used.
 * 2022-08-17 tonhuisman: Add support for range (x..y) for sliders
 * 2022-08-16 tonhuisman: Changed validButtonGroup() to ignore group 0 by default, and setButtonGroup(), and setgrp subcommand,
 *                        to allow group 0
 *                        Add setting for swapping (reversing) menu-swipe direction
 * 2022-08-15 tonhuisman: Add optional swipe/slider support, add swipe subcommand, add GetConfigValue options and docs
 *                        Replace ifdef *_USE_* defines by if *_FEATURE_*
 * 2022-08-13 tonhuisman: Replace _ in object name and on/off captions by space, to ease the use of object name as caption
 *                        On save, any spaces in captions are replaced by _ to avoid using 2 quotes around the value.
 *                        This implies that no underscores wil be shown in captions!
 * 2022-06-09 tonhuisman: Change method arguments to const-by-reference where possible for improved compile-time checks
 * 2022-06-06 tonhuisman: Move PLUGIN_WRITE handling from P123
 *                        Move PLUGIN_GET_CONFIG_VALUE handling from P123.
 *                        Add getters for on/off (state) and (enabled), and matching GET_CONFIG_VALUE commands
 *                        Add toggle subcommand for switching enabled on/off buttons to the other state
 *                        Extend on, off, toggle subcommands to support a list of objects
 * 2022-06-03 tonhuisman: Change default ON color to blue (from green, too bright/bad contrast with white captions)
 *                        Add options for auto Enable/Disable arrow buttons and invert pgup/pgdn
 *                        Bugfix: Also apply debouncing to non-button objects
 * 2022-06-02 tonhuisman: Reduce saved settings size by eliminating 0 and similar unneeded values
 *                        Move Touch minimal touch pressure back to P123
 * 2022-05-26 tonhuisman: Expand Captions to 30 characters
 * 2022-05 tonhuisman:    Testing, improving, bugfixing.
 * 2022-05-23 tonhuisman: Created from refactoring P123 Touch object handling into ESPEasy_TouchHandler
 *********************************************************************************************************************/

/**
 * Commands supported:
 * -------------------
 * touch,enable,<objectName|Nr>[,...]       : Enable disabled objectname(s)
 * touch,disable,<objectName|Nr>[,...]      : Disable enabled objectname(s)
 * touch,on,<buttonObjectName|Nr>[,...]     : Switch TouchButton(s) on (must be enabled)
 * touch,off,<buttonObjectName|Nr>[,...]    : Switch TouchButton(s) off (must be enabled)
 * touch,set,<buttonObjectName|Nr>,<value>  : Set TouchObject to value (slider) or 0=off >0=on (must be enabled)
 * touch,toggle,<buttonObjectName|Nr>[,...] : Switch TouchButton(s) to the other state (must be enabled)
 * touch,swipe,<swipeValue>                 : Switch button group according to swipe direction
 * touch,setgrp,<group>                     : Switch to button group
 * touch,incgrp                             : Switch to next button group
 * touch,decgrp                             : Switch to previous button group
 * touch,incpage                            : Switch to next button group page (+10)
 * touch,decpage                            : Switch to previous button group page (-10)
 * touch,updatebutton,<buttonName|Nr>[,<group>[,<mode>]] : Update a button by name or number
 */
/**
 * Get Config Variables supported: [<taskname>#<configvalue>{,arguments}]
 * <configvalue>{,arguments}    : Description
 * buttongroup                  : Get current buttongroup
 * hasgroup,groupNr             : Check if group exists, ignores group 0
 * enabled,objectName|objectNr  : Check if object is enabled
 * state,objectName|objectNr    : Get current object state (buttons: on = 1, off = 0, sliders: value 0..100 (=percentage))
 * pagemode                     : Get the PageUp/PageDown mode, 0 = up=pgup, 1 = up=pgdown
 * swipedir,directionId         : Get the name for the direction provided in numeric form
 */

# define TOUCH_DEBUG                    // Additional debugging information

# define TOUCH_FEATURE_TOOLTIPS       1 // Enable/disable tooltips in UI
# define TOUCH_FEATURE_EXTENDED_TOUCH 1 // Enable/disable extended touch settings
# define TOUCH_FEATURE_SWIPE          1 // Enable/disable Swipe support

# ifdef LIMIT_BUILD_SIZE
#  if TOUCH_FEATURE_TOOLTIPS
#   undef TOUCH_FEATURE_TOOLTIPS
#   define TOUCH_FEATURE_TOOLTIPS  0
#  endif // if TOUCH_FEATURE_TOOLTIPS
#  if TOUCH_FEATURE_EXTENDED_TOUCH
#   undef TOUCH_FEATURE_EXTENDED_TOUCH
#   define TOUCH_FEATURE_EXTENDED_TOUCH  0
#  endif // if TOUCH_FEATURE_EXTENDED_TOUCH
// #  if TOUCH_FEATURE_SWIPE
// #   undef TOUCH_FEATURE_SWIPE
// #   define TOUCH_FEATURE_SWIPE  0
// #  endif // if TOUCH_FEATURE_SWIPE
# endif  // ifdef LIMIT_BUILD_SIZE
# ifdef BUILD_NO_DEBUG
#  ifdef TOUCH_DEBUG
#   undef TOUCH_DEBUG
#  endif // ifdef TOUCH_DEBUG
# endif // ifdef BUILD_NO_DEBUG

# if TOUCH_FEATURE_TOOLTIPS && !FEATURE_TOOLTIPS
#  undef TOUCH_FEATURE_TOOLTIPS
#  define TOUCH_FEATURE_TOOLTIPS   0
# endif  // if TOUCH_FEATURE_TOOLTIPS && !FEATURE_TOOLTIPS

// Global Settings flags
# define TOUCH_FLAGS_SEND_XY            0  // Send X and Y coordinate events
# define TOUCH_FLAGS_SEND_Z             1  // Send Z coordinate (pressure) events
# define TOUCH_FLAGS_SEND_OBJECTNAME    2  // Send onjectname events
# define TOUCH_FLAGS_USE_CALIBRATION    3  // Enable calibration entry
# define TOUCH_FLAGS_LOG_CALIBRATION    4  // Enable logging for calibration
# define TOUCH_FLAGS_ROTATION_FLIPPED   5  // Rotation flipped 180 degrees
# define TOUCH_FLAGS_DEDUPLICATE        6  // Avoid duplicate events
# define TOUCH_FLAGS_INIT_OBJECTEVENT   7  // Draw button objects when started
# define TOUCH_FLAGS_INITIAL_GROUP      8  // Initial group to activate, 8 bits
# define TOUCH_FLAGS_DRAWBTN_VIA_RULES  16 // Draw buttons using rule
# define TOUCH_FLAGS_AUTO_PAGE_ARROWS   17 // Automatically enable/disable paging buttons
# define TOUCH_FLAGS_PGUP_BELOW_MENU    18 // Group-page below current menu (reverts Up/Down buttons)
# define TOUCH_FLAGS_SWAP_LEFT_RIGHT    19 // Swaps Left and Right, Up and Down swipe directions for menu actions
# define TOUCH_FLAGS_IGNORE_TOUCH       20 // Disable touch, use for object/button features only

# define TOUCH_VALUE_X UserVar[event->BaseVarIndex + 0]
# define TOUCH_VALUE_Y UserVar[event->BaseVarIndex + 1]
# define TOUCH_VALUE_Z UserVar[event->BaseVarIndex + 2]

# define TOUCH_TS_ROTATION            0     // Rotation 0-3 = 0/90/180/270 degrees
# define TOUCH_TS_SEND_XY             true  // Enable/Disable X/Y events
# define TOUCH_TS_SEND_Z              false // Disable/Enable Z events
# define TOUCH_TS_SEND_OBJECTNAME     true  // Enable/Disable objectname events
# define TOUCH_TS_USE_CALIBRATION     false // Disable/Enable calibration
# define TOUCH_TS_LOG_CALIBRATION     true  // Enable/Disable calibration logging
# define TOUCH_TS_ROTATION_FLIPPED    false // Enable/Disable rotation flipped 180 deg.
# define TOUCH_TS_X_RES               320   // Pixels, should match with the screen it is mounted on
# define TOUCH_TS_Y_RES               480
# define TOUCH_DEBOUNCE_MILLIS        100   // Debounce delay for On/Off button function
# define TOUCH_DEF_SWIPE_MINIMAL        3   // Minimal swipe pixels
# define TOUCH_DEF_SWIPE_MARGIN        10   // Default swipe margin

# define TOUCH_MAX_COLOR_INPUTLENGTH  11    // 11 Characters is enough to type in all recognized color names and values
# define TOUCH_MaxObjectNameLength    15    // 15 character objectnames
# define TOUCH_MaxCaptionNameLength   30    // 30 character captions, to allow variable names
# define TOUCH_MAX_CALIBRATION_COUNT  1     //
# define TOUCH_MAX_OBJECT_COUNT       40    // This count of touchobjects should be enough, because of limited
                                            // settings storage, 1024 bytes
# define TOUCH_EXTRA_OBJECT_COUNT     5     // The number of empty objects to show if max not reached
# define TOUCH_ARRAY_SIZE (TOUCH_MAX_OBJECT_COUNT + TOUCH_MAX_CALIBRATION_COUNT)

# define TOUCH_MAX_BUTTON_GROUPS      255   // Max. allowed button groups

# define TOUCH_SETTINGS_SEPARATOR     '\x02'

// Settings array field offsets: Calibration
# define TOUCH_CALIBRATION_START            0  // Index into settings array
# define TOUCH_CALIBRATION_ENABLED          1  // Enabled 0/1 (parseString index starts at 1)
# define TOUCH_CALIBRATION_LOG_ENABLED      2  // Calibration Log Enabled 0/1
# define TOUCH_CALIBRATION_TOP_X            3  // Top X offset (uint16_t)
# define TOUCH_CALIBRATION_TOP_Y            4  // Top Y
# define TOUCH_CALIBRATION_BOTTOM_X         5  // Bottom X
# define TOUCH_CALIBRATION_BOTTOM_Y         6  // Bottom Y
# define TOUCH_COMMON_DEBOUNCE_MS           7  // Debounce milliseconds
# define TOUCH_COMMON_FLAGS                 8  // Common flags
# if TOUCH_FEATURE_EXTENDED_TOUCH
#  define TOUCH_COMMON_DEF_COLOR_ON         9  // Default Color ON (rgb565, uint16_t)
#  define TOUCH_COMMON_DEF_COLOR_OFF        10 // Default Color OFF
#  define TOUCH_COMMON_DEF_COLOR_BORDER     11 // Default Color Border
#  define TOUCH_COMMON_DEF_COLOR_CAPTION    12 // Default Color Caption
#  define TOUCH_COMMON_DEF_COLOR_DISABLED   13 // Default Disabled Color
#  define TOUCH_COMMON_DEF_COLOR_DISABCAPT  14 // Default Disabled Caption Color
#  define TOUCH_COMMON_SWIPE_MINIMAL        15 // Minimal swipe pixels
#  define TOUCH_COMMON_SWIPE_MARGIN         16 // Swipe margin
# else // if TOUCH_FEATURE_EXTENDED_TOUCH
#  define TOUCH_COMMON_SWIPE_MINIMAL         9 // Minimal swipe pixels
#  define TOUCH_COMMON_SWIPE_MARGIN         10 // Swipe margin
# endif // if TOUCH_FEATURE_EXTENDED_TOUCH

// Settings array field offsets: Touch objects
# define TOUCH_OBJECT_INDEX_START           (TOUCH_CALIBRATION_START + 1)
# define TOUCH_OBJECT_INDEX_END             (TOUCH_ARRAY_SIZE - (TOUCH_CALIBRATION_START + 1))
# define TOUCH_OBJECT_NAME                  1  // Name (String 14) (parseString index starts at 1)
# define TOUCH_OBJECT_FLAGS                 2  // Flags (uint32_t)
# define TOUCH_OBJECT_COORD_TOP_X           3  // Top X (uint16_t)
# define TOUCH_OBJECT_COORD_TOP_Y           4  // Top Y
# define TOUCH_OBJECT_COORD_WIDTH           5  // Width
# define TOUCH_OBJECT_COORD_HEIGHT          6  // Height
# if TOUCH_FEATURE_EXTENDED_TOUCH
#  define TOUCH_OBJECT_COLOR_ON             7  // Color ON (rgb565, uint16_t)
#  define TOUCH_OBJECT_COLOR_OFF            8  // Color OFF
#  define TOUCH_OBJECT_COLOR_CAPTION        9  // Color Caption
#  define TOUCH_OBJECT_CAPTION_ON           10 // Caption ON (String 12, quoted)
#  define TOUCH_OBJECT_CAPTION_OFF          11 // Caption OFF (String 12, quoted)
#  define TOUCH_OBJECT_COLOR_BORDER         12 // Color Border
#  define TOUCH_OBJECT_COLOR_DISABLED       13 // Disabled Color
#  define TOUCH_OBJECT_COLOR_DISABCAPT      14 // Disabled Caption Color
#  define TOUCH_OBJECT_GROUPFLAGS           15 // Group flags
# endif // if TOUCH_FEATURE_EXTENDED_TOUCH

# define TOUCH_OBJECT_FLAG_ENABLED          0  // Enabled
# define TOUCH_OBJECT_FLAG_BUTTON           1  // Button behavior
# define TOUCH_OBJECT_FLAG_INVERTED         2  // Inverted button
# define TOUCH_OBJECT_FLAG_FONTSCALE        3  // 4 bits used as button alignment
# define TOUCH_OBJECT_FLAG_BUTTONTYPE       7  // 4 bits used as button type (low 4 bits)
# define TOUCH_OBJECT_FLAG_BUTTONALIGN      11 // 4 bits used as button caption layout (high 4 bits)
# define TOUCH_OBJECT_FLAG_GROUP            16 // 8 bits used as button group
# define TOUCH_OBJECT_FLAG_SLIDER           24 // Slider object

# define TOUCH_OBJECT_GROUP_ACTIONGROUP     8  // 8 bits used as action group
# define TOUCH_OBJECT_GROUP_ACTION          16 // 4 bits used as action option

# define TOUCH_DEFAULT_COLOR_DISABLED         0x9410
# define TOUCH_DEFAULT_COLOR_DISABLED_CAPTION 0x5A69

// Lets Touchne our own coordinate point
struct tTouch_Point
{
  uint16_t x = 0u;
  uint16_t y = 0u;
};

// For touch objects we store a name, 2 coordinates, flags and other options
struct tTouchObjects
{
  uint32_t     flags        = 0u;
  uint32_t     SurfaceAreas = 0u;
  uint32_t     TouchTimers  = 0u;
  tTouch_Point top_left;
  tTouch_Point width_height;
  int16_t      TouchStates = 0;
  # if TOUCH_FEATURE_EXTENDED_TOUCH
  uint32_t groupFlags           = 0u;
  uint16_t colorOn              = 0u;
  uint16_t colorOff             = 0u;
  uint16_t colorCaption         = 0u;
  uint16_t colorBorder          = 0u;
  uint16_t colorDisabled        = 0u;
  uint16_t colorDisabledCaption = 0u;
  # endif // if TOUCH_FEATURE_EXTENDED_TOUCH
  String objectName;
  String captionOn;
  String captionOff;
};

// Touch actions, max 16!
enum class Touch_action_e : uint8_t {
  Default         = 0u,
  ActivateGroup   = 1u,
  IncrementGroup  = 2u,
  DecrementGroup  = 3u,
  IncrementPage   = 4u,
  DecrementPage   = 5u,
};

# if TOUCH_FEATURE_SWIPE

// Swipe actions, start at 12 o'çlock, clock-wise
enum class Swipe_action_e : uint8_t {
  None            = 0u,
  Up              = 1u,
  UpRight         = 2u,
  Right           = 3u,
  RightDown       = 4u,
  Down            = 5u,
  DownLeft        = 6u,
  Left            = 7u,
  LeftUp          = 8u,
  SwipeAction_MAX = 9u // Last item is count
};
# endif // if TOUCH_FEATURE_SWIPE

const __FlashStringHelper* toString(Touch_action_e action);

# if TOUCH_FEATURE_SWIPE
const __FlashStringHelper* toString(Swipe_action_e action);
# endif // if TOUCH_FEATURE_SWIPE

class ESPEasy_TouchHandler {
public:

  ESPEasy_TouchHandler();
  ESPEasy_TouchHandler(const taskIndex_t     & displayTask,
                       const AdaGFXColorDepth& colorDepth);
  virtual ~ESPEasy_TouchHandler() {}

  void loadTouchObjects(struct EventStruct *event);
  void init(struct EventStruct *event);
  bool isCalibrationActive();
  bool isValidAndTouchedTouchObject(const int16_t& x,
                                    const int16_t& y,
                                    String       & selectedObjectName,
                                    int8_t       & selectedObjectIndex);
  int8_t  getTouchObjectIndex(struct EventStruct *event,
                              const String      & touchObject,
                              const bool        & isButton = false);
  bool    setTouchObjectState(struct EventStruct *event,
                              const String      & touchObject,
                              const bool        & state);
  int8_t  getTouchObjectState(struct EventStruct *event,
                              const String      & touchObject);
  bool    setTouchButtonOnOff(struct EventStruct *event,
                              const String      & touchObject,
                              const bool        & state);
  int16_t getTouchObjectValue(struct EventStruct *event,
                              const String      & touchObject);
  bool    setTouchObjectValue(struct EventStruct *event,
                              const String      & touchObject,
                              const int16_t     & value);
  bool    plugin_webform_load(struct EventStruct *event);
  bool    plugin_webform_save(struct EventStruct *event);
  bool    plugin_fifty_per_second(struct EventStruct *event,
                                  const int16_t     & x,
                                  const int16_t     & y,
                                  const int16_t     & ox,
                                  const int16_t     & oy,
                                  const int16_t     & rx,
                                  const int16_t     & ry,
                                  const int16_t     & z);
  bool    plugin_write(struct EventStruct *event,
                       const String      & string);
  bool    plugin_get_config_value(struct EventStruct *event,
                                  String            & string);
  void    releaseTouch(struct EventStruct *event);
  int16_t getButtonGroup() {
    return _buttonGroup;
  }

  # if TOUCH_FEATURE_EXTENDED_TOUCH
  bool validButtonGroup(const int16_t& group,
                        const bool   & ignoreZero = true);
  #  if TOUCH_FEATURE_SWIPE
  bool handleButtonSwipe(struct EventStruct *event,
                         const int16_t     & swipeValue);
  #  endif // if TOUCH_FEATURE_SWIPE
  bool setButtonGroup(struct EventStruct *event,
                      const int16_t     & buttonGroup);
  bool incrementButtonGroup(struct EventStruct *event);
  bool decrementButtonGroup(struct EventStruct *event);
  bool incrementButtonPage(struct EventStruct *event);
  bool decrementButtonPage(struct EventStruct *event);
  void displayButtonGroup(struct EventStruct *event,
                          const int16_t     & buttonGroup,
                          const int8_t      & mode = 0);
  bool displayButton(struct EventStruct *event,
                     const int8_t      & buttonNr,
                     const int16_t     & buttonGroup = -1,
                     int8_t              mode        = 0);
  # endif // if TOUCH_FEATURE_EXTENDED_TOUCH

  bool touchEnabled() {
    return _touchEnabled;
  }

private:

  int parseStringToInt(const String & string,
                       const uint8_t& indexFind,
                       const char   & separator    = ',',
                       const int    & defaultValue = 0);
  void generateObjectEvent(struct EventStruct *event,
                           const int8_t      & objectIndex,
                           const int16_t     & onOffState,
                           const int8_t      & mode        = 0,
                           const bool        & groupSwitch = false,
                           const int8_t      & factor      = 1);
  bool parseRangeToInt16(const String& range,
                         int16_t     & lowRange,
                         int16_t     & highRange);

  bool _deduplicate            = false;
  uint16_t _displayTask        = 0u;
  AdaGFXColorDepth _colorDepth = AdaGFXColorDepth::FullColor;
  int16_t _buttonGroup         = 0;

  std::set<int16_t>_buttonGroups;

  bool _settingsLoaded = false;
  bool _stillTouching  = false;
  bool _touchEnabled   = true;

  // Used to generate events on touch-release
  int8_t _lastObjectIndex = -1;
  String _lastObjectName;
  tTouch_Point _last_point;
  tTouch_Point _last_point_z; // Only used to store z in the x member
  # if TOUCH_FEATURE_SWIPE
  Swipe_action_e _lastSwipe = Swipe_action_e::None;
  int16_t _last_delta_x;
  int16_t _last_delta_y;
  # endif // if TOUCH_FEATURE_SWIPE

  struct tTouch_Globals
  {
    uint32_t     flags = 0u;
    tTouch_Point top_left;
    tTouch_Point bottom_right;
    # if TOUCH_FEATURE_EXTENDED_TOUCH
    uint16_t colorOn              = 0u;
    uint16_t colorOff             = 0u;
    uint16_t colorCaption         = 0u;
    uint16_t colorBorder          = 0u;
    uint16_t colorDisabled        = 0u;
    uint16_t colorDisabledCaption = 0u;
    # endif // if TOUCH_FEATURE_EXTENDED_TOUCH
    uint8_t debounceMs         = 0u;
    uint8_t swipeMargin        = 0u;
    uint8_t swipeMinimal       = 0u;
    bool    calibrationEnabled = false;
    bool    logEnabled         = false;
  };

  std::vector<tTouchObjects>TouchObjects;

public:

  bool _flipped        = false; // buffered settings
  bool _useCalibration = false;

  tTouch_Globals Touch_Settings;

  String settingsArray[TOUCH_ARRAY_SIZE];
  uint8_t lastObjectIndex = 0u;
  uint8_t objectCount     = 0u;
};
#endif // ifdef PLUGIN_USES_TOUCHHANDLER
#endif // ifndef HELPERS_ESPEASY_TOUCHHANDLER_H
