#ifndef PLUGINSTRUCTS_P123_DATA_STRUCT_H
#define PLUGINSTRUCTS_P123_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"

#ifdef USES_P123

# include "../../ESPEasy_common.h"
# include "../Helpers/AdafruitGFX_helper.h"
# include "../Helpers/ESPEasy_TouchHandler.h"

# include <Adafruit_FT6206.h>

# ifndef LIMIT_BUILD_SIZE
#  define PLUGIN_123_DEBUG      // Additional debugging information
# else // ifndef LIMIT_BUILD_SIZE
#  ifndef P123_LIMIT_BUILD_SIZE // Can be set from elsewhere
#   define P123_LIMIT_BUILD_SIZE
#  endif // ifndef P123_LIMIT_BUILD_SIZE
# endif // ifndef LIMIT_BUILD_SIZE

# if defined(BUILD_NO_DEBUG) && defined(PLUGIN_123_DEBUG)
#  undef PLUGIN_123_DEBUG
# endif // if defined(BUILD_NO_DEBUG) && defined(PLUGIN_123_DEBUG)

# define P123_CONFIG_DISPLAY_TASK   PCONFIG(0)

# define P123_COLOR_DEPTH           PCONFIG_LONG(1)
# define P123_CONFIG_THRESHOLD      PCONFIG(1)
# define P123_CONFIG_ROTATION       PCONFIG(2)
# define P123_CONFIG_X_RES          PCONFIG(3)
# define P123_CONFIG_Y_RES          PCONFIG(4)

# define P123_CONFIG_DISPLAY_PREV   PCONFIG(7)

// Default settings values
# define P123_TS_THRESHOLD          40            // Threshold before the value is registered as a proper touch
# define P123_TS_ROTATION           0             // Rotation 0-3 = 0/90/180/270 degrees
# define P123_TS_X_RES              320           // Pixels, should match with the screen it is mounted on
# define P123_TS_Y_RES              480

# define P123_TOUCH_X_NATIVE        P123_TS_X_RES // Native touchscreen resolution, same as default display resolution
# define P123_TOUCH_Y_NATIVE        P123_TS_Y_RES

# define P123_ROTATION_0            0
# define P123_ROTATION_90           1
# define P123_ROTATION_180          2
# define P123_ROTATION_270          3

// Data structure
struct P123_data_struct : public PluginTaskData_base
{
  P123_data_struct();
  ~P123_data_struct();

  void reset();
  bool init(struct EventStruct *event);
  bool isInitialized() const;

  bool plugin_webform_load(struct EventStruct *event);
  bool plugin_webform_save(struct EventStruct *event);
  bool plugin_write(struct EventStruct *event,
                    const String      & string);
  bool plugin_fifty_per_second(struct EventStruct *event);
  bool plugin_get_config_value(struct EventStruct *event,
                               String            & string);

  void loadTouchObjects(struct EventStruct *event);
  bool touched();
  void readData(int16_t& x,
                int16_t& y,
                int16_t& z,
                int16_t& ox,
                int16_t& oy);

  void setRotation(uint8_t n);
  void setRotationFlipped(bool _flipped);
  bool isValidAndTouchedTouchObject(const int16_t& x,
                                    const int16_t& y,
                                    String       & selectedObjectName,
                                    int8_t       & selectedObjectIndex);
  int8_t  getTouchObjectIndex(struct EventStruct *event,
                              const String      & touchObject,
                              bool                isButton = false);
  bool    setTouchObjectState(struct EventStruct *event,
                              const String      & touchObject,
                              bool                state);
  bool    setTouchButtonOnOff(struct EventStruct *event,
                              const String      & touchObject,
                              bool                state);
  void    scaleRawToCalibrated(int16_t& x,
                               int16_t& y);

  int16_t getButtonGroup();
  bool    validButtonGroup(int16_t buttonGroup,
                           bool    ignoreZero = false);
  bool    setButtonGroup(struct EventStruct *event,
                         int16_t             buttonGroup);
  bool    incrementButtonGroup(struct EventStruct *event);
  bool    decrementButtonGroup(struct EventStruct *event);
  bool    incrementButtonPage(struct EventStruct *event);
  bool    decrementButtonPage(struct EventStruct *event);
  void    displayButtonGroup(struct EventStruct *event,
                             int16_t             buttonGroup,
                             int8_t              mode = 0);
  bool    displayButton(struct EventStruct *event,
                        const int8_t      & buttonNr,
                        int16_t             buttonGroup = -1,
                        int8_t              mode        = 0);

private:

  // This is initialized by calling init()
  Adafruit_FT6206 *touchscreen = nullptr;
  uint8_t          _rotation   = 0u;
  uint16_t         _ts_x_res   = 0u;
  uint16_t         _ts_y_res   = 0u;

  ESPEasy_TouchHandler *touchHandler = nullptr;
};

#endif // ifdef USED_P123
#endif // ifndef PLUGINSTRUCTS_P123_DATA_STRUCT_H
