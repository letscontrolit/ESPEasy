#ifndef PLUGINSTRUCTS_P143_DATA_STRUCT_H
#define PLUGINSTRUCTS_P143_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"

#ifdef USES_P143
# include <Adafruit_seesaw.h>                    // Adafruit I2C Rotary encoder (using a SeeSaw board/controller)
# include <seesaw_neopixel.h>                    // Adafruit NeoPixel on the Rotary encoder board
# include "../Helpers/I2C_access.h"              // Core I2C support functions (M5Stack, DFRobot)

# ifdef LIMIT_BUILD_SIZE
#  define PLUGIN_143_DEBUG                     0 // Set to 1 to enable, 0 to disable, extra log output (info)
# else // ifdef LIMIT_BUILD_SIZE
#  define PLUGIN_143_DEBUG                     1
# endif // ifdef LIMIT_BUILD_SIZE

// Adafruit encoder specific
# define P143_ADAFRUIT_ENCODER_PRODUCTID      4991 // ProductID is checked during initialization
# define P143_SEESAW_SWITCH                   24
# define P143_SEESAW_NEOPIX                   6

// M5Stack encoder specific
# define P143_M5STACK_REG_MODE                0x00 // Firmware version v1.1 supported only
# define P143_M5STACK_REG_ENCODER             0x10
# define P143_M5STACK_REG_BUTTON              0x20
# define P143_M5STACK_REG_LED                 0x30

// DFRobot encoder specific
# define P143_DFROBOT_ENCODER_PID             0x01F6 // ProductID is checked during initialization
# define P143_DFROBOT_ENCODER_PID_MSB_REG     0x00
# define P143_DFROBOT_ENCODER_PID_LSB_REG     0x01
# define P143_DFROBOT_ENCODER_COUNT_MSB_REG   0x08
# define P143_DFROBOT_ENCODER_COUNT_LSB_REG   0x09
# define P143_DFROBOT_ENCODER_KEY_STATUS_REG  0x0A
# define P143_DFROBOT_ENCODER_GAIN_REG        0x0B
# define P143_DFROBOT_MIN_GAIN                1
# define P143_DFROBOT_MAX_GAIN                51
# define P143_DFROBOT_MIN_OFFSET              0
# define P143_DFROBOT_MAX_OFFSET              1023

// Common
# define P143_STRINGS                         10 // # of values
# define P143_STRING_LEN                      50 // Line length limit
# define P143_OFFSET_MIN                      0
# define P143_OFFSET_MAX                      1023

# define P143_LONGPRESS_MIN_INTERVAL          500
# define P143_LONGPRESS_MAX_INTERVAL          5000
# define P143_DOUBLECLICK_MIN_INTERVAL        1000
# define P143_DOUBLECLICK_MAX_INTERVAL        3000


# define P143_I2C_ADDR                        PCONFIG(0)
# define P143_ENCODER_TYPE                    PCONFIG(1)
# define P143_PREVIOUS_TYPE                   PCONFIG(2)
# define P143_INITIAL_POSITION                PCONFIG(3)
# define P143_MINIMAL_POSITION                PCONFIG(4)
# define P143_MAXIMAL_POSITION                PCONFIG(5)
# define P143_OFFSET_POSITION                 PCONFIG(6) // Offset for encoder that has 0..1023 range
# define P143_DFROBOT_LED_GAIN                PCONFIG(7) // Range 1..51, 1 => 1 led/~2.5 turns, 51 => 1 led/detent

# define P143_SET_LONGPRESS_INTERVAL          PCONFIG_FLOAT(0)
# define P143_GET_LONGPRESS_INTERVAL          (static_cast<uint16_t>(P143_SET_LONGPRESS_INTERVAL))

# define P143_ADAFRUIT_COLOR_AND_BRIGHTNESS   PCONFIG_ULONG(0)
# define P143_ADAFRUIT_OFFSET_RED             24
# define P143_ADAFRUIT_OFFSET_GREEN           16
# define P143_ADAFRUIT_OFFSET_BLUE            8
# define P143_ADAFRUIT_OFFSET_BRIGHTNESS      0
# define P143_ADAFRUIT_COLOR_RED              (get8BitFromUL(P143_ADAFRUIT_COLOR_AND_BRIGHTNESS, P143_ADAFRUIT_OFFSET_RED))
# define P143_ADAFRUIT_COLOR_GREEN            (get8BitFromUL(P143_ADAFRUIT_COLOR_AND_BRIGHTNESS, P143_ADAFRUIT_OFFSET_GREEN))
# define P143_ADAFRUIT_COLOR_BLUE             (get8BitFromUL(P143_ADAFRUIT_COLOR_AND_BRIGHTNESS, P143_ADAFRUIT_OFFSET_BLUE))
# define P143_NEOPIXEL_BRIGHTNESS             (get8BitFromUL(P143_ADAFRUIT_COLOR_AND_BRIGHTNESS, P143_ADAFRUIT_OFFSET_BRIGHTNESS))

# define P143_M5STACK_COLOR_AND_SELECTION     PCONFIG_ULONG(1)
# define P143_M5STACK2_OFFSET_RED             24
# define P143_M5STACK2_OFFSET_GREEN           16
# define P143_M5STACK2_OFFSET_BLUE            8
# define P143_M5STACK_OFFSET_SELECTION        0
# define P143_M5STACK2_COLOR_RED              (get8BitFromUL(P143_M5STACK_COLOR_AND_SELECTION, P143_M5STACK2_OFFSET_RED))
# define P143_M5STACK2_COLOR_GREEN            (get8BitFromUL(P143_M5STACK_COLOR_AND_SELECTION, P143_M5STACK2_OFFSET_GREEN))
# define P143_M5STACK2_COLOR_BLUE             (get8BitFromUL(P143_M5STACK_COLOR_AND_SELECTION, P143_M5STACK2_OFFSET_BLUE))
# define P143_M5STACK_SELECTION               (get4BitFromUL(P143_M5STACK_COLOR_AND_SELECTION, P143_M5STACK_OFFSET_SELECTION))

# define P143_PLUGIN_FLAGS                    PCONFIG_ULONG(3)
# define P143_PLUGIN_OFFSET_COUNTER_MAPPING   0
# define P143_PLUGIN_OFFSET_BUTTON_ACTION     4
# define P143_PLUGIN_OFFSET_EXIT_LED_OFF      8
# define P143_PLUGIN_OFFSET_LONGPRESS         9
# define P143_PLUGIN_COUNTER_MAPPING          (get4BitFromUL(P143_PLUGIN_FLAGS, P143_PLUGIN_OFFSET_COUNTER_MAPPING))
# define P143_PLUGIN_BUTTON_ACTION            (get4BitFromUL(P143_PLUGIN_FLAGS, P143_PLUGIN_OFFSET_BUTTON_ACTION))
# define P143_PLUGIN_EXIT_LED_OFF             (!bitRead(P143_PLUGIN_FLAGS, P143_PLUGIN_OFFSET_EXIT_LED_OFF))
# define P143_PLUGIN_ENABLE_LONGPRESS         (bitRead(P143_PLUGIN_FLAGS, P143_PLUGIN_OFFSET_LONGPRESS))

/*******************************************
 * Feature toggles
 ******************************************/
# ifndef P143_FEATURE_INCLUDE_M5STACK
#  define P143_FEATURE_INCLUDE_M5STACK        1
# endif // ifndef P143_FEATURE_INCLUDE_M5STACK
# ifndef P143_FEATURE_M5STACK_V1_1
#  define P143_FEATURE_M5STACK_V1_1           0 // Untested so unsupported for now
# endif // ifndef P143_FEATURE_M5STACK_V1_1
# ifndef P143_FEATURE_INCLUDE_DFROBOT
#  define P143_FEATURE_INCLUDE_DFROBOT        1
# endif // ifndef P143_FEATURE_INCLUDE_DFROBOT
# ifndef P143_FEATURE_COUNTER_COLORMAPPING
#  define P143_FEATURE_COUNTER_COLORMAPPING   1
# endif // ifndef P143_FEATURE_COUNTER_COLORMAPPING

/*******************************************
 * Supported encoders
 ******************************************/
enum class P143_DeviceType_e : uint8_t {
  AdafruitEncoder = 0u, // Adafruit encoder is always included
  # if P143_FEATURE_INCLUDE_M5STACK
  M5StackEncoder = 1u,
  # endif // if P143_FEATURE_INCLUDE_M5STACK
  # if P143_FEATURE_INCLUDE_DFROBOT
  DFRobotEncoder = 2u,
  # endif // if P143_FEATURE_INCLUDE_DFROBOT
};

/*******************************************
 * Counter to LED color mapping
 ******************************************/
# if P143_FEATURE_COUNTER_COLORMAPPING
enum class P143_CounterMapping_e : uint8_t { // Max 16 values!
  None          = 0u,
  ColorMapping  = 1u,
  ColorGradient = 2u,
};
# endif // if P143_FEATURE_COUNTER_COLORMAPPING

/*******************************************
 * Button behavior/action
 ******************************************/
enum class P143_ButtonAction_e : uint8_t { // Max 16 values!
  PushButton         = 0u,
  PushButtonInverted = 1u,
  ToggleSwitch       = 2u,
};

# if P143_FEATURE_INCLUDE_M5STACK

/*******************************************
 * M5Stack encoder Led options
 ******************************************/
enum class P143_M5StackLed_e : uint8_t { // Max 16 values
  BothLeds = 0u,
  Led1Only = 1u,
  Led2Only = 2u,
};
# endif // if P143_FEATURE_INCLUDE_M5STACK

/*******************************************
 * Global support functions
 ******************************************/
const __FlashStringHelper* toString(P143_DeviceType_e device);
# if P143_FEATURE_COUNTER_COLORMAPPING
const __FlashStringHelper* toString(P143_CounterMapping_e counter);
# endif // if P143_FEATURE_COUNTER_COLORMAPPING
const __FlashStringHelper* toString(P143_ButtonAction_e action);

void                       P143_CheckEncoderDefaultSettings(struct EventStruct *event); // Apply defaults if encoder selection changed

/*******************************************
 * P143 Plugin taskdata struct
 ******************************************/
struct P143_data_struct : public PluginTaskData_base {
public:

  P143_data_struct(struct EventStruct *event);

  P143_data_struct() = delete;
  ~P143_data_struct();

  bool plugin_init(struct EventStruct *event);
  bool plugin_exit(struct EventStruct *event);
  bool plugin_read(struct EventStruct *event);
  bool plugin_write(struct EventStruct *event,
                    String            & string);
  bool plugin_ten_per_second(struct EventStruct *event);
  bool plugin_fifty_per_second(struct EventStruct *event);

private:

  Adafruit_seesaw *Adafruit_Seesaw = nullptr;
  seesaw_NeoPixel *Adafruit_Spixel = nullptr;

  bool parseColorMapLine(const String& line,
                         int32_t     & count,
                         int16_t     & red,
                         int16_t     & green,
                         int16_t     & blue);
  bool    rangeCheck(int32_t count,
                     int32_t min,
                     int32_t max);
  void    counterToColorMapping(struct EventStruct *event);

  # if P143_FEATURE_INCLUDE_M5STACK
  uint8_t applyBrightness(uint8_t color);
  void    m5stack_setPixelColor(uint8_t pixel,
                                uint8_t red,
                                uint8_t green,
                                uint8_t blue);
  # endif // if P143_FEATURE_INCLUDE_M5STACK

  bool _initialized = false;

  P143_DeviceType_e _device;
  int8_t            _i2cAddress = -1;

  // Encoder
  int32_t _encoderPosition = 0;
  int32_t _encoderMin      = 0;
  int32_t _encoderMax      = 0;
  int32_t _previousEncoder = 0;
  # if P143_FEATURE_INCLUDE_DFROBOT
  int32_t _initialOffset = 0;
  # endif // if P143_FEATURE_INCLUDE_DFROBOT
  # if P143_FEATURE_INCLUDE_M5STACK
  int32_t _offsetEncoder = 0;
  bool    _useOffset     = false;
  # endif // if P143_FEATURE_INCLUDE_M5STACK

  uint8_t  _buttonState     = 0xFF;
  uint8_t  _buttonLast      = 0xFF;
  uint8_t  _buttonIgnore    = 0;
  uint16_t _buttonTime      = 0;
  bool     _buttonDown      = false;
  uint16_t _buttonLongPress = P143_LONGPRESS_MIN_INTERVAL;
  bool     _enableLongPress = false;

  # if PLUGIN_143_DEBUG
  int32_t _oldPosition = INT32_MIN;
  # endif // if PLUGIN_143_DEBUG
  # if P143_FEATURE_COUNTER_COLORMAPPING
  P143_CounterMapping_e _mapping = P143_CounterMapping_e::None;

  String _colorMapping[P143_STRINGS];
  int    _colorMaps = -1;
  # endif // if P143_FEATURE_COUNTER_COLORMAPPING
  uint8_t _red        = 0;
  uint8_t _green      = 0;
  uint8_t _blue       = 0;
  uint8_t _brightness = 0;
};

#endif // ifdef USES_P143
#endif // ifndef PLUGINSTRUCTS_P143_DATA_STRUCT_H
