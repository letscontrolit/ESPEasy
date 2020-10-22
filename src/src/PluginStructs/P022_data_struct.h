#ifndef PLUGINSTRUCTS_P022_DATA_STRUCT_H
#define PLUGINSTRUCTS_P022_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P022

#include "../../ESPEasy_common.h"


# define PLUGIN_022_PCA9685_MODE1   0x00 // location for Mode1 register address
# define PCA9685_MODE2              0x01 // location for Mode2 register address
# define PCA9685_MODE2_VALUES       0x20
# define PCA9685_LED0               0x06 // location for start of LED0 registers
# define PCA9685_ADDRESS            0x40 // I2C address
# define PCA9685_MAX_ADDRESS        0x7F
# define PCA9685_NUMS_ADDRESS       (PCA9685_MAX_ADDRESS - PCA9685_ADDRESS)
# define PCA9685_MAX_PINS           15
# define PCA9685_MAX_PWM            4095
# define PCA9685_MIN_FREQUENCY      23.0   // Min possible PWM cycle frequency
# define PCA9685_MAX_FREQUENCY      1500.0 // Max possible PWM cycle frequency
# define PCA9685_ALLLED_REG         (byte)0xFA

// FIXME TD-er: This still uses a bitmask to keep track of what address was initialized.
// That's no longer needed as it is now a data struct object per task instead of per address.
// Administration per address is no longer needed as it may be behind a multiplexer,
// so multiple instances using the same address is now allowed.
struct P022_data_struct : public PluginTaskData_base {
  bool p022_is_init(uint8_t address);

  bool p022_set_init(uint8_t address);

  bool p022_clear_init(uint8_t address);

  void Plugin_022_writeRegister(int  i2cAddress,
                                int  regAddress,
                                byte data);

  uint8_t Plugin_022_readRegister(int i2cAddress,
                                  int regAddress);

  void    Plugin_022_Off(int address,
                         int pin);

  void    Plugin_022_On(int address,
                        int pin);

  void    Plugin_022_Write(int address,
                           int Par1,
                           int Par2);

  void Plugin_022_Frequency(int      address,
                            uint16_t freq);

  void Plugin_022_initialize(int address);


  uint32_t initializeState_lo = 0;
  uint32_t initializeState_hi = 0;
};

#endif // ifdef USES_P022
#endif // ifndef PLUGINSTRUCTS_P022_DATA_STRUCT_H
