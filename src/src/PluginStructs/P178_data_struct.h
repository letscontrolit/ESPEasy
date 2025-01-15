#ifndef PLUGINSTRUCTS_P178_DATA_STRUCT_H
#define PLUGINSTRUCTS_P178_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P178


# define LU9685_ADDRESS            0x00 // I2C address
# define LU9685_MAX_ADDRESS        0x1F
# define LU9685_MAX_PINS           20
# define LU9685_MAX_ANGLE          180
# define LU9685_DEFAULT_FREQUENCY  50
# define LU9685_MIN_FREQUENCY      20
# define LU9685_MAX_FREQUENCY      300

# define LU9685_OUTPUT_DISABLED    0xFF

# define LU9685_ALL_PINS           0xFF

# define LU9685_ALL_PIN_ENABLE_MASK    ((1 << LU9685_MAX_PINS) - 1)

struct P178_data_struct : public PluginTaskData_base {
  P178_data_struct(uint8_t  address,
                   uint16_t freq);
  virtual ~P178_data_struct() = default;


  // When enabled, the pulse corresponding with the set angle will be output on the pin.
  // When disabled, the pin will be 'silent'
  bool enablePin(uint8_t pin,
                 bool    enable);

  // Set the servo on given pin to specified angle.
  // If angle is out of range, the pin will be disabled.
  bool setAngle(uint8_t pin,
                int     angle);

  // Set enable mode explicitly
  // Angle will be constrained to the allowed range and stored internally
  bool setAngle(uint8_t pin,
                int     angle,
                bool    enable);

  // Update pins using value stored in system variables [int#N] ... [int#N+nrPins-1]
  // with nrPins <=0 update all pins from startPin ... lastPin
  bool setRange(uint8_t  startPin,
                uint32_t startVarIndex,
                int      nrPins);

  // Write all set angles in a single I2C call
  bool   update();

  void   setFrequency(uint16_t freq);

  String logPrefix();
  String logPrefix(const __FlashStringHelper *poststr);

private:

  void setAngle_internal(uint8_t pin,
                         int     angle);


  uint8_t getAngle(uint8_t pin) const;


  uint8_t  _address{};
  uint8_t  _data[LU9685_MAX_PINS]{};
  uint32_t _enabledPin{};
};

#endif // ifdef USES_P178
#endif // ifndef PLUGINSTRUCTS_P178_DATA_STRUCT_H
