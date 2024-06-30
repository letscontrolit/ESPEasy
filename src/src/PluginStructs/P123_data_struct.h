#ifndef PLUGINSTRUCTS_P123_DATA_STRUCT_H
#define PLUGINSTRUCTS_P123_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"

#ifdef USES_P123

# include "../Helpers/ESPEasy_TouchHandler.h"

# include <bb_captouch.h>

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

# define P123_I2C_ADDRESS           PCONFIG(6)
# define P123_CONFIG_DISPLAY_TASK   PCONFIG(0)

# define P123_CONFIG_FLAGS          PCONFIG_ULONG(0) // All flags
# define P123_CONFIG_FLAG_TOUCHTYPE 0                // Flag indexes

// We're storing an int8_t in range -128..127 in an uin8_t
# define P123_GET_TOUCH_TYPE        (get8BitFromUL(P123_CONFIG_FLAGS, P123_CONFIG_FLAG_TOUCHTYPE) - 128)
# define P123_SET_TOUCH_TYPE(T) (set8BitToUL(P123_CONFIG_FLAGS, P123_CONFIG_FLAG_TOUCHTYPE, T + 128))

# define P123_INTERRUPTPIN          (CONFIG_PIN1)
# define P123_RESETPIN              (CONFIG_PIN2)

# define P123_COLOR_DEPTH           PCONFIG_LONG(1)
# define P123_CONFIG_THRESHOLD      PCONFIG(1)
# define P123_CONFIG_ROTATION       PCONFIG(2)
# define P123_CONFIG_X_RES          PCONFIG(3)
# define P123_CONFIG_Y_RES          PCONFIG(4)
# define P123_CONFIG_VTYPE          PCONFIG(5)

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

enum class P123_TouchType_e : int8_t {
  Automatic = -1,
  FT62x6    = 0, // Also used as offset in I2C address array
  GT911_1   = 1,
  GT911_2   = 2,
  CST820    = 3,
  CST226    = 4,
  AXS15231  = 5,
  CHSC5816  = 6,
};

const __FlashStringHelper* toString(P123_TouchType_e tType);

// Data structure
struct P123_data_struct : public PluginTaskData_base
{
  P123_data_struct(P123_TouchType_e touchType);
  ~P123_data_struct();

  static bool      plugin_i2c_has_address(int Par1);
  static uint8_t   plugin_i2c_address(P123_TouchType_e touchType);

  P123_TouchType_e getTouchType();
  int              getBBCapTouchType(P123_TouchType_e touchType);

  void             reset();
  bool             init(struct EventStruct *event);
  bool             isInitialized() const;

  bool             plugin_webform_load(struct EventStruct *event);
  bool             plugin_webform_save(struct EventStruct *event);
  bool             plugin_write(struct EventStruct *event,
                                const String      & string);
  bool             plugin_fifty_per_second(struct EventStruct *event);
  bool             plugin_get_config_value(struct EventStruct *event,
                                           String            & string);

  void             loadTouchObjects(struct EventStruct *event);
  bool             touched();
  void             readData(int16_t& x,
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

  int16_t getButtonGroup() const;
  bool    validButtonGroup(int16_t buttonGroup,
                           bool    ignoreZero = false);
  bool    setButtonGroup(struct EventStruct *event,
                         int16_t             buttonGroup);
  bool    nextButtonGroup(struct EventStruct *event);
  bool    prevButtonGroup(struct EventStruct *event);
  bool    nextButtonPage(struct EventStruct *event);
  bool    prevButtonPage(struct EventStruct *event);
  void    displayButtonGroup(struct EventStruct *event,
                             int16_t             buttonGroup,
                             int8_t              mode = 0);
  bool    displayButton(struct EventStruct *event,
                        const int8_t      & buttonNr,
                        int16_t             buttonGroup = -1,
                        int8_t              mode        = 0);

private:

  // This is initialized by calling init()
  BBCapTouch *touchscreen = nullptr;
  uint8_t     _rotation   = 0u;
  uint16_t    _ts_x_res   = 0u;
  uint16_t    _ts_y_res   = 0u;

  int16_t          _i2caddr{};
  int16_t          _resetPin     = -1;
  int16_t          _interruptPin = -1;
  P123_TouchType_e _touchType;

  TOUCHINFO touchInfo;

  ESPEasy_TouchHandler *touchHandler = nullptr;
};

#endif // ifdef USES_P123
#endif // ifndef PLUGINSTRUCTS_P123_DATA_STRUCT_H
