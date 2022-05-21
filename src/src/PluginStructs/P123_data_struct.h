#ifndef PLUGINSTRUCTS_P123_DATA_STRUCT_H
#define PLUGINSTRUCTS_P123_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#include "../../ESPEasy_common.h"
#include "../Helpers/AdafruitGFX_helper.h"

#ifdef USES_P123

# include <Adafruit_FT6206.h>

# ifndef LIMIT_BUILD_SIZE
// #  define PLUGIN_123_DEBUG       // Additional debugging information
# endif // ifndef LIMIT_BUILD_SIZE

# define P123_USE_TOOLTIPS       // Enable tooltips in UI

# define P123_USE_EXTENDED_TOUCH // Enable extended touch settings

# ifdef LIMIT_BUILD_SIZE
#  ifdef P123_USE_TOOLTIPS
#   undef P123_USE_TOOLTIPS
#  endif // ifdef P123_USE_TOOLTIPS
#  ifdef P123_USE_EXTENDED_TOUCH
#   undef P123_USE_EXTENDED_TOUCH
#  endif // ifdef P123_USE_EXTENDED_TOUCH
# endif  // ifdef LIMIT_BUILD_SIZE
# if defined(P123_USE_TOOLTIPS) && !defined(ENABLE_TOOLTIPS)
#  undef P123_USE_TOOLTIPS
# endif  // if defined(P123_USE_TOOLTIPS) && !defined(ENABLE_TOOLTIPS)

# define P123_FLAGS_SEND_XY           0 // Set in Global Settings flags
# define P123_FLAGS_SEND_Z            1 // Set in Global Settings flags
# define P123_FLAGS_SEND_OBJECTNAME   2 // Set in Global Settings flags
# define P123_FLAGS_USE_CALIBRATION   3 // Set in Global Settings flags
# define P123_FLAGS_LOG_CALIBRATION   4 // Set in Global Settings flags
# define P123_FLAGS_ROTATION_FLIPPED  5 // Set in P123_CONFIG_FLAGS
# define P123_FLAGS_DEDUPLICATE       6 // Set in Global Settings flags
# define P123_FLAGS_INIT_OBJECTEVENT  7 // Set in Global Settings flags
# define P123_FLAGS_INITIAL_GROUP     8 // Initial group to activate, 8 bits uses only 6

# define P123_CONFIG_DISPLAY_TASK PCONFIG(0)

# define P123_COLOR_DEPTH         PCONFIG_LONG(1)
# define P123_CONFIG_ROTATION     PCONFIG(2)
# define P123_CONFIG_X_RES        PCONFIG(3)
# define P123_CONFIG_Y_RES        PCONFIG(4)
# define P123_CONFIG_OBJECTCOUNT  PCONFIG(5)
# define P123_CONFIG_DEBOUNCE_MS  PCONFIG(6)
# define P123_CONFIG_DISPLAY_PREV PCONFIG(7)

// # define P123_CONFIG_FLAGS        PCONFIG_LONG(0) // 0-31 flags

# define P123_VALUE_X UserVar[event->BaseVarIndex + 0]
# define P123_VALUE_Y UserVar[event->BaseVarIndex + 1]
# define P123_VALUE_Z UserVar[event->BaseVarIndex + 2]

// Default settings values
# define P123_TS_TRESHOLD           40    // Treshold before the value is registered as a proper touch
# define P123_TS_ROTATION           0     // Rotation 0-3 = 0/90/180/270 degrees
# define P123_TS_SEND_XY            true  // Enable/Disable X/Y events
# define P123_TS_SEND_Z             false // Disable/Enable Z events
# define P123_TS_SEND_OBJECTNAME    true  // Enable/Disable objectname events
# define P123_TS_USE_CALIBRATION    false // Disable/Enable calibration
# define P123_TS_LOG_CALIBRATION    true  // Enable/Disable calibration logging
# define P123_TS_ROTATION_FLIPPED   false // Enable/Disable rotation flipped 180 deg.
# define P123_TS_X_RES              320   // Pixels, should match with the screen it is mounted on
# define P123_TS_Y_RES              480
# define P123_DEBOUNCE_MILLIS       100   // Debounce delay for On/Off button function

# define P123_TOUCH_X_NATIVE        320   // Native touchscreen resolution
# define P123_TOUCH_Y_NATIVE        480

# define P123_MAX_COLOR_INPUTLENGTH 11    // 11 Characters is enough to type in all recognized color names and values
# define P123_MaxObjectNameLength   15    // 14 character objectnames + terminating 0
# define P123_MAX_CALIBRATION_COUNT 1     //
# define P123_MAX_OBJECT_COUNT      40    // This count of touchobjects should be enough, because of limited
                                          // settings storage, 960 bytes + 8 bytes calibration coordinates
# define P123_EXTRA_OBJECT_COUNT    5     // The number of empty objects to show if max not reached
# define P123_ARRAY_SIZE (P123_MAX_OBJECT_COUNT + P123_MAX_CALIBRATION_COUNT)

# define P123_MAX_BUTTON_GROUPS     63    // Max. allowed button groups, technically limited to 6 bits = 0..63!

# define P123_FLAGS_ON_OFF_BUTTON 0       // TouchObjects.flags On/Off Button function
# define P123_FLAGS_INVERT_BUTTON 1       // TouchObjects.flags Inverted On/Off Button function

# define TOUCHOBJECTS_HELPER_ROTATION_0   0
# define TOUCHOBJECTS_HELPER_ROTATION_90  1
# define TOUCHOBJECTS_HELPER_ROTATION_180 2
# define TOUCHOBJECTS_HELPER_ROTATION_270 3

# define P123_SETTINGS_SEPARATOR      '\x02'

// Settings array field offsets: Calibration
# define P123_CALIBRATION_START             0  // Index into settings array
# define P123_CALIBRATION_ENABLED           1  // Enabled 0/1 (parseString index starts at 1)
# define P123_CALIBRATION_LOG_ENABLED       2  // Calibration Log Enabled 0/1
# define P123_CALIBRATION_TOP_X             3  // Top X offset (uint16_t)
# define P123_CALIBRATION_TOP_Y             4  // Top Y
# define P123_CALIBRATION_BOTTOM_X          5  // Bottom X
# define P123_CALIBRATION_BOTTOM_Y          6  // Bottom Y
# define P123_COMMON_DEBOUNCE_MS            7  // Debounce milliseconds
# define P123_COMMON_TOUCH_TRESHOLD         8  // Treshold setting
# define P123_COMMON_FLAGS                  9  // Common flags
# ifdef P123_USE_EXTENDED_TOUCH
#  define P123_COMMON_DEF_COLOR_ON          10 // Default Color ON (rgb565, uint16_t)
#  define P123_COMMON_DEF_COLOR_OFF         11 // Default Color OFF
#  define P123_COMMON_DEF_COLOR_BORDER      12 // Default Color Border
#  define P123_COMMON_DEF_COLOR_CAPTION     13 // Default Color Caption
#  define P123_COMMON_DEF_COLOR_DISABLED    14 // Default Disabled Color
#  define P123_COMMON_DEF_COLOR_DISABCAPT   15 // Default Disabled Caption Color
# endif // ifdef P123_USE_EXTENDED_TOUCH

// Settings array field offsets: Touch objects
# define P123_OBJECT_INDEX_START        (P123_CALIBRATION_START + 1)
# define P123_OBJECT_INDEX_END          (P123_ARRAY_SIZE - (P123_CALIBRATION_START + 1))
# define P123_OBJECT_NAME               1  // Name (String 14) (parseString index starts at 1)
# define P123_OBJECT_FLAGS              2  // Flags (uint32_t)
# define P123_OBJECT_COORD_TOP_X        3  // Top X (uint16_t)
# define P123_OBJECT_COORD_TOP_Y        4  // Top Y
# define P123_OBJECT_COORD_WIDTH        5  // Width
# define P123_OBJECT_COORD_HEIGHT       6  // Height
# ifdef P123_USE_EXTENDED_TOUCH
#  define P123_OBJECT_COLOR_ON          7  // Color ON (rgb565, uint16_t)
#  define P123_OBJECT_COLOR_OFF         8  // Color OFF
#  define P123_OBJECT_COLOR_CAPTION     9  // Color Caption
#  define P123_OBJECT_CAPTION_ON        10 // Caption ON (String 12, quoted)
#  define P123_OBJECT_CAPTION_OFF       11 // Caption OFF (String 12, quoted)
#  define P123_OBJECT_COLOR_BORDER      12 // Color Border
#  define P123_OBJECT_COLOR_DISABLED    13 // Disabled Color
#  define P123_OBJECT_COLOR_DISABCAPT   14 // Disabled Caption Color
# endif // ifdef P123_USE_EXTENDED_TOUCH

# define P123_OBJECT_FLAG_ENABLED       0  // Enabled
# define P123_OBJECT_FLAG_BUTTON        1  // Button behavior
# define P123_OBJECT_FLAG_INVERTED      2  // Inverted button
# define P123_OBJECT_FLAG_FONTSCALE     3  // 4 bits used as button alignment
# define P123_OBJECT_FLAG_BUTTONTYPE    7  // 8 bits used as button type
# define P123_OBJECT_FLAG_GROUP         15 // 8 bits used as button group
# define P123_OBJECT_FLAG_ACTIONGROUP   23 // 8 bits, 6 bits used as action group 0..63, 2 bits used as action option

# define P123_DEFAULT_COLOR_DISABLED          0x9410
# define P123_DEFAULT_COLOR_DISABLED_CAPTION  0x5A69

// Lets define our own coordinate point
struct tP123_Point
{
  uint16_t x = 0u;
  uint16_t y = 0u;
};

// For touch objects we store a name, 2 coordinates, flags and other options
struct tP123_TouchObjects
{
  String      objectName;
  String      captionOn;
  String      captionOff;
  uint32_t    flags        = 0u;
  uint32_t    SurfaceAreas = 0u;
  uint32_t    TouchTimers  = 0u;
  tP123_Point top_left;
  tP123_Point width_height;
  # ifdef P123_USE_EXTENDED_TOUCH
  uint16_t colorOn              = 0u;
  uint16_t colorOff             = 0u;
  uint16_t colorCaption         = 0u;
  uint16_t colorBorder          = 0u;
  uint16_t colorDisabled        = 0u;
  uint16_t colorDisabledCaption = 0u;
  # endif // ifdef P123_USE_EXTENDED_TOUCH
  bool TouchStates = false;
};

// Touch actions, use with mask 0xC0, other 6 bits are group/code to activate
enum class P123_touch_action_e : uint8_t {
  Default         = 0b00000000, // 0x00
  ActivateGroup   = 0b01000000, // 0x40
  IncrementGroup  = 0b10000000, // 0x80
  DecrementGroup  = 0b11000000, // 0xC0
  TouchAction_MAX = 4           // Last item is count, max 4!
};

const __FlashStringHelper* toString(P123_touch_action_e action);


// Data structure
struct P123_data_struct : public PluginTaskData_base
{
  P123_data_struct();
  ~P123_data_struct();

  void reset();
  bool init(const EventStruct *event,
            uint8_t            rotation,
            uint16_t           ts_x_res,
            uint16_t           ts_y_res,
            uint16_t           displayTask,
            AdaGFXColorDepth   colorDepth);
  bool isInitialized() const;
  void loadTouchObjects(const EventStruct *event);
  bool touched();
  void readData(int16_t& x,
                int16_t& y,
                int16_t& z,
                int16_t& ox,
                int16_t& oy);
  void setRotation(uint8_t n);
  void setRotationFlipped(bool _flipped);
  bool isCalibrationActive();
  bool isValidAndTouchedTouchObject(int16_t x,
                                    int16_t y,
                                    String& selectedObjectName,
                                    int8_t& selectedObjectIndex);
  bool setTouchObjectState(struct EventStruct *event,
                           const String      & touchObject,
                           bool                state);
  bool setTouchButtonOnOff(struct EventStruct *event,
                           const String      & touchObject,
                           bool                state);
  void scaleRawToCalibrated(int16_t& x,
                            int16_t& y);
  bool plugin_webform_load(struct EventStruct *event);
  bool plugin_webform_save(struct EventStruct *event);
  bool plugin_fifty_per_second(struct EventStruct *event);
  bool setButtonGroup(const EventStruct *event,
                      int8_t             buttonGroup);
  bool incrementButtonGroup(const EventStruct *event);
  bool decrementButtonGroup(const EventStruct *event);
  void displayButtonGroup(const EventStruct *event,
                          int8_t             buttonGroup,
                          int8_t             mode = 0);

private:

  int parseStringToInt(const String& string,
                       uint8_t       indexFind,
                       char          separator    = ',',
                       int           defaultValue = 0);
  void generateObjectEvent(const EventStruct *event,
                           const int8_t       objectIndex,
                           const int8_t       onOffState,
                           const bool         groupSwitch = false,
                           const int8_t       factor      = 1);

  // This is initialized by calling init()
  Adafruit_FT6206 *touchscreen     = nullptr;
  uint8_t          _rotation       = 0u;
  bool             _useCalibration = false;
  uint16_t         _ts_x_res       = 0u;
  uint16_t         _ts_y_res       = 0u;
  uint16_t         _displayTask    = 0u;
  AdaGFXColorDepth _colorDepth     = AdaGFXColorDepth::FullColor;

  bool _flipped     = false; // buffered settings
  bool _deduplicate = false;

  // Calibration and some other settings
  struct tP123_Globals
  {
    uint32_t    flags = 0u;
    tP123_Point top_left;
    tP123_Point bottom_right;
    uint16_t    treshold = 0u;
    # ifdef P123_USE_EXTENDED_TOUCH
    uint16_t colorOn              = 0u;
    uint16_t colorOff             = 0u;
    uint16_t colorCaption         = 0u;
    uint16_t colorBorder          = 0u;
    uint16_t colorDisabled        = 0u;
    uint16_t colorDisabledCaption = 0u;
    # endif // ifdef P123_USE_EXTENDED_TOUCH
    uint8_t debounceMs         = 0u;
    bool    calibrationEnabled = false;
    bool    logEnabled         = false;
  };

  tP123_Globals P123_Settings;

  std::vector<tP123_TouchObjects>TouchObjects;

  int8_t _buttonGroup    = 0;
  int8_t _minButtonGroup = 0;
  int8_t _maxButtonGroup = 0;

public:

  // This is filled during checking of a touchobject
  // std::vector < uint32_t; SurfaceAreas;

  // Counters for debouncing touch button
  // std::vector<uint32_t>TouchTimers;
  // std::vector<bool>    TouchStates;

  String  settingsArray[P123_ARRAY_SIZE];
  uint8_t lastObjectIndex = 0u;

  uint8_t objectCount = 0u;
};

#endif // ifdef USED_P123
#endif // ifndef PLUGINSTRUCTS_P123_DATA_STRUCT_H
