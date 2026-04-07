#ifndef PLUGINSTRUCTS_P099_DATA_STRUCT_H
#define PLUGINSTRUCTS_P099_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"

// #include "../../ESPEasy_common.h"

#ifdef USES_P099

# ifdef ESP32
#  include "../Helpers/ESPEasy_TouchHandler.h"
# endif // ifdef ESP32

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

# include "../PluginStructs/P099_data_struct_defs.h"

// Define default values for both ESP32/lolin32 and D1 Mini
# ifdef ESP32
  #  define P099_TS_CS 12
# else // ESP8266/ESP8285
  #  define P099_TS_CS  0               // D3
# endif // ESP32

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

# define P099_TOUCH_X_INVALID  4095     // When picking up spurious noise (or an open/not connected TS-CS pin), these are the values that
                                        // turn up
# define P099_TOUCH_Y_INVALID  4095
# define P099_TOUCH_Z_INVALID  255


// Data structure
struct P099_data_struct : public PluginTaskData_base
{
  P099_data_struct();
  virtual ~P099_data_struct();

  # ifdef ESP32
  bool init(struct EventStruct *event,
            uint8_t             cs,
            uint8_t             rotation,
            bool                flipped,
            uint8_t             z_treshold,
            uint16_t            ts_x_res,
            uint16_t            ts_y_res);
  # endif // ifdef ESP32
  # ifdef ESP8266
  bool init(taskIndex_t taskIndex,
            uint8_t     cs,
            uint8_t     rotation,
            bool        flipped,
            uint8_t     z_treshold,
            bool        send_xy,
            bool        send_z,
            bool        useCalibration,
            uint16_t    ts_x_res,
            uint16_t    ts_y_res);
  void loadTouchObjects(taskIndex_t taskIndex);
  # endif // ifdef ESP8266
  bool isInitialized() const;

  bool plugin_webform_load(struct EventStruct *event);
  bool plugin_webform_save(struct EventStruct *event);
  bool plugin_write(struct EventStruct *event,
                    const String      & string);
  # ifdef ESP32
  bool plugin_fifty_per_second(struct EventStruct *event);
  bool plugin_get_config_value(struct EventStruct *event,
                               String            & string);

private:

  # endif // ifdef ESP32

  void reset();
  bool touched();
  void setRotation(uint8_t n);
  void setRotationFlipped(bool _flipped);
  # ifdef ESP32
  void readData(int16_t& x,
                int16_t& y,
                uint8_t& z);
  void scaleRawToCalibrated(int16_t& x,
                            int16_t& y);
  # endif // ifdef ESP32
  # ifdef ESP8266
  void readData(uint16_t *x,
                uint16_t *y,
                uint8_t  *z);
  bool isCalibrationActive();
  bool isValidAndTouchedTouchObject(uint16_t x,
                                    uint16_t y,
                                    String & selectedObjectName,
                                    int    & selectedObjectIndex,
                                    uint8_t  checkObjectCount);
  bool setTouchObjectState(const String& touchObject,
                           bool          state,
                           uint8_t       checkObjectCount);
  void scaleRawToCalibrated(uint16_t& x,
                            uint16_t& y);
  # endif // ifdef ESP8266

  // This is initialized by calling init()
  # ifdef ESP32
  ESPEasy_TouchHandler *touchHandler = nullptr;
  # endif // ifdef ESP32
  XPT2046_Touchscreen *touchscreen    = nullptr;
  uint8_t              _address_ts_cs = 0;
  uint8_t              _rotation      = 0;
  bool                 _flipped       = false;
  uint8_t              _z_treshold    = 0;
  uint16_t             _ts_x_res      = 0;
  uint16_t             _ts_y_res      = 0;
  # ifdef ESP8266
  bool _send_xy        = 0;
  bool _send_z         = 0;
  bool _useCalibration = 0;

  // This is filled during checking of a touchobject
  uint32_t SurfaceAreas[P099_MaxObjectCount] = { 0 };

  // Counters for debouncing touch button
  uint32_t TouchTimers[P099_MaxObjectCount] = { 0 };
  bool     TouchStates[P099_MaxObjectCount] = { 0 };

  // Stored settings data:
  tP099_StoredSettings_struct StoredSettings;
  # endif // ifdef ESP8266
};

#endif  // ifdef USES_P099
#endif  // ifndef PLUGINSTRUCTS_P099_DATA_STRUCT_H
