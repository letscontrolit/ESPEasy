#ifndef PLUGINSTRUCTS_P132_DATA_STRUCT_H
#define PLUGINSTRUCTS_P132_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P132

// # define P132_DEBUG_LOG // Enable for some (extra) logging

# define P132_CONFIG_BASE       2 // Better not change this...
# define P132_I2C_ADDR          PCONFIG(1)
# define P132_VALUE_1           PCONFIG(P132_CONFIG_BASE)
# define P132_VALUE_2           PCONFIG(P132_CONFIG_BASE + 1)
# define P132_VALUE_3           PCONFIG(P132_CONFIG_BASE + 2)
# define P132_VALUE_4           PCONFIG(P132_CONFIG_BASE + 3)
# define P132_SHUNT             PCONFIG(6)

# define P132_CONFIG_FLAGS      PCONFIG_LONG(0)
# define P132_FLAG_AVERAGE      0
# define P132_FLAG_CONVERSION_B 3
# define P132_FLAG_CONVERSION_S 6

# define P132_GET_AVERAGE       get3BitFromUL(P132_CONFIG_FLAGS, P132_FLAG_AVERAGE)
# define P132_GET_CONVERSION_B  get3BitFromUL(P132_CONFIG_FLAGS, P132_FLAG_CONVERSION_B)
# define P132_GET_CONVERSION_S  get3BitFromUL(P132_CONFIG_FLAGS, P132_FLAG_CONVERSION_S)

# define INA3221_AVERAGE_BIT          9
# define INA3221_CONVERSION_BUS_BIT   6
# define INA3221_CONVERSION_SHUNT_BIT 3

struct P132_data_struct : public PluginTaskData_base {
public:

  P132_data_struct(struct EventStruct *event);

  P132_data_struct() = delete;
  virtual ~P132_data_struct() = default;

  float getShuntVoltage_mV(byte reg);
  float getBusVoltage_V(byte reg);

  void  setCalibration_INA3221(struct EventStruct *event);

private:
  int16_t getBusVoltage_raw(byte reg);
  int16_t getShuntVoltage_raw(byte reg);

  int8_t _i2c_address;
};
#endif // ifdef USES_P132
#endif // ifndef PLUGINSTRUCTS_P132_DATA_STRUCT_H
