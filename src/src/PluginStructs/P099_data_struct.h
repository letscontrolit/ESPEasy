#ifndef PLUGINSTRUCTS_P099_DATA_STRUCT_H
#define PLUGINSTRUCTS_P099_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"

// #include "../../ESPEasy_common.h"

#ifdef USES_P099

# include "../Helpers/ESPEasy_TouchHandler.h"

# include <XPT2046_Touchscreen.h>

// #define PLUGIN_099_DEBUG    // Additional debugging information
# ifndef LIMIT_BUILD_SIZE
#  define PLUGIN_099_DEBUG      // Additional debugging information
# else // ifndef LIMIT_BUILD_SIZE
#  ifndef P099_LIMIT_BUILD_SIZE // Can be set from elsewhere
#   define P099_LIMIT_BUILD_SIZE
#  endif // ifndef P099_LIMIT_BUILD_SIZE
# endif // ifndef LIMIT_BUILD_SIZE

# if defined(BUILD_NO_DEBUG) && defined(PLUGIN_099_DEBUG)
#  undef PLUGIN_099_DEBUG
# endif // if defined(BUILD_NO_DEBUG) && defined(PLUGIN_099_DEBUG)

# ifndef P099_ENABLE_OLD_CONFIG
#  define P099_ENABLE_OLD_CONFIG   1 // When set to 0 will also inhibit the 1-time settings conversion in ESPEasy_TouchHandler
# endif // ifndef P099_ENABLE_OLD_CONFIG

// Define default values for both ESP32/lolin32 and D1 Mini
# ifdef ESP32
  #  define P099_TS_CS 12
# else // ESP8266/ESP8285
  #  define P099_TS_CS  0 // D3
# endif // ESP32

# if P099_ENABLE_OLD_CONFIG
#  define P099_MaxObjectNameLength 15 // 14 character objectnames + terminating 0
#  define P099_MaxObjectCount      40 // This count of touchobjects should be enough, because of limited settings storage, 960 bytes + 8
                                      // bytes calibration coordinates
# endif // if P099_ENABLE_OLD_CONFIG


# define P099_FLAGS_ON_OFF_BUTTON 0    // TouchObjects.flags On/Off Button function
# define P099_FLAGS_INVERT_BUTTON 1    // TouchObjects.flags Inverted On/Off Button function

# define P099_FLAGS_SEND_XY          0 // Set in P099_CONFIG_FLAGS
# define P099_FLAGS_SEND_Z           1 // Set in P099_CONFIG_FLAGS
# define P099_FLAGS_SEND_OBJECTNAME  2 // Set in P099_CONFIG_FLAGS
# define P099_FLAGS_USE_CALIBRATION  3 // Set in P099_CONFIG_FLAGS
# define P099_FLAGS_LOG_CALIBRATION  4 // Set in P099_CONFIG_FLAGS
# define P099_FLAGS_ROTATION_FLIPPED 5 // Set in P099_CONFIG_FLAGS
# define P099_FLAGS_CONFIG_VTYPE     6 // 4 bits to store the VType
# define P099_FLAGS_CONFIG_DISPLAY  10 // 8 bits to store the Display task

# define P099_CONFIG_VERSION      PCONFIG(0)
# define P099_CONFIG_CS_PIN       PIN(0)
# define P099_CONFIG_THRESHOLD    PCONFIG(1)
# define P099_CONFIG_ROTATION     PCONFIG(2)
# define P099_CONFIG_X_RES        PCONFIG(3)
# define P099_CONFIG_Y_RES        PCONFIG(4)
# if P099_ENABLE_OLD_CONFIG
#  define P099_CONFIG_OBJECTCOUNT  PCONFIG(5)
#  define P099_CONFIG_DEBOUNCE_MS  PCONFIG(6)
# endif // if P099_ENABLE_OLD_CONFIG
# define P099_CONFIG_DISPLAY_PREV PCONFIG(7)
# define P099_CONFIG_FLAGS        PCONFIG_LONG(0) // 0-31 flags
# define P099_COLOR_DEPTH         PCONFIG_LONG(1)

# define P099_GET_CONFIG_VTYPE  get4BitFromUL(P099_CONFIG_FLAGS, P099_FLAGS_CONFIG_VTYPE)
# define P099_SET_CONFIG_VTYPE(v) set4BitToUL(P099_CONFIG_FLAGS, P099_FLAGS_CONFIG_VTYPE, v)
# define P099_GET_CONFIG_DISPLAY  get8BitFromUL(P099_CONFIG_FLAGS, P099_FLAGS_CONFIG_DISPLAY)
# define P099_SET_CONFIG_DISPLAY(v) set8BitToUL(P099_CONFIG_FLAGS, P099_FLAGS_CONFIG_DISPLAY, v)

# define P099_VALUE_X UserVar.getFloat(event->TaskIndex, 0)
# define P099_VALUE_Y UserVar.getFloat(event->TaskIndex, 1)
# define P099_VALUE_Z UserVar.getFloat(event->TaskIndex, 2)

# define P099_SET_VALUE_X(v) UserVar.setFloat(event->TaskIndex, 0, v)
# define P099_SET_VALUE_Y(v) UserVar.setFloat(event->TaskIndex, 1, v)
# define P099_SET_VALUE_Z(v) UserVar.setFloat(event->TaskIndex, 2, v)

# define P099_TS_TRESHOLD         15    // Treshold before the value is registered as a proper touch
# define P099_TS_ROTATION         2     // Rotation 0-3 = 0/90/180/270 degrees, compatible with TFT ILI9341
# define P099_TS_SEND_XY          true  // Enable X/Y events
# define P099_TS_SEND_Z           false // Disable Z events
# define P099_TS_SEND_OBJECTNAME  true  // Enable objectname events
# define P099_TS_USE_CALIBRATION  false // Disable calibration
# define P099_TS_LOG_CALIBRATION  true  // Enable calibration logging
# define P099_TS_ROTATION_FLIPPED false // Enable rotation flipped 180 deg.
# define P099_TS_X_RES            240   // Pixels, should match with the screen it is mounted on
# define P099_TS_Y_RES            320
# if P099_ENABLE_OLD_CONFIG
#  define P099_INIT_OBJECTCOUNT    8    // Initial setting
#  define P099_DEBOUNCE_MILLIS     150  // Debounce delay for On/Off button function
# endif // if P099_ENABLE_OLD_CONFIG

# define P099_TOUCH_X_INVALID  4095     // When picking up spurious noise (or an open/not connected TS-CS pin), these are the values that
                                        // turn up
# define P099_TOUCH_Y_INVALID  4095
# define P099_TOUCH_Z_INVALID  255


// Data structure
struct P099_data_struct : public PluginTaskData_base
{
  P099_data_struct();
  virtual ~P099_data_struct();

  bool init(struct EventStruct *event,
            uint8_t             cs,
            uint8_t             rotation,
            bool                flipped,
            uint8_t             z_treshold,
            uint16_t            ts_x_res,
            uint16_t            ts_y_res);
  bool isInitialized() const;

  bool plugin_webform_load(struct EventStruct *event);
  bool plugin_webform_save(struct EventStruct *event);
  bool plugin_write(struct EventStruct *event,
                    const String      & string);
  bool plugin_fifty_per_second(struct EventStruct *event);
  bool plugin_get_config_value(struct EventStruct *event,
                               String            & string);

private:

  void reset();
  bool touched();
  void readData(int16_t& x,
                int16_t& y,
                uint8_t& z);
  void setRotation(uint8_t n);
  void setRotationFlipped(bool _flipped);
  void scaleRawToCalibrated(int16_t& x,
                            int16_t& y);

  // This is initialized by calling init()
  ESPEasy_TouchHandler *touchHandler   = nullptr;
  XPT2046_Touchscreen  *touchscreen    = nullptr;
  uint8_t               _address_ts_cs = 0;
  uint8_t               _rotation      = 0;
  bool                  _flipped       = false;
  uint8_t               _z_treshold    = 0;
  uint16_t              _ts_x_res      = 0;
  uint16_t              _ts_y_res      = 0;

  # if P099_ENABLE_OLD_CONFIG

  // The settings structures
  // Lets define our own coordinate point
  struct tP099_Point
  {
    uint16_t x = 0;
    uint16_t y = 0;
  };

  // For touch objects we store a name and 2 coordinates
  struct tP099_Touchobjects
  {
    char        objectname[P099_MaxObjectNameLength] = { 0 };
    uint8_t     flags                                = 0;
    tP099_Point top_left;
    tP099_Point bottom_right;
  };

  // Only 2 coordinates used for calibration (we must assume that the touch panel is mounted straight on to tft)
  struct tP099_Calibration
  {
    tP099_Point top_left;
    tP099_Point bottom_right;
  };

public:

  // The stuff we want to save between settings (Calibration coordinates and touchable objects)
  struct tP099_StoredSettings_struct
  {
    tP099_Calibration  Calibration;
    tP099_Touchobjects TouchObjects[P099_MaxObjectCount];
  };

  // Stored settings data:
  tP099_StoredSettings_struct StoredSettings;
  # endif // if P099_ENABLE_OLD_CONFIG
};

#endif  // ifdef USES_P099
#endif  // ifndef PLUGINSTRUCTS_P099_DATA_STRUCT_H
