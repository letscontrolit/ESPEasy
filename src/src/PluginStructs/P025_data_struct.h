#ifndef PLUGINSTRUCTS_P025_DATA_STRUCT_H
#define PLUGINSTRUCTS_P025_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P025


# define P025_I2C_ADDR        PCONFIG(0)
# define P025_GAIN            PCONFIG(1)
# define P025_MUX             PCONFIG(2)
# define P025_CAL_GET         bitRead(PCONFIG(3), 0)
# define P025_CAL_SET(x)      bitWrite(PCONFIG(3), 0, x)
# define P025_VOLT_OUT_GET    bitRead(PCONFIG(3), 1)
# define P025_VOLT_OUT_SET(x) bitWrite(PCONFIG(3), 1, x)

# define P025_CAL_ADC1    PCONFIG_LONG(0)
# define P025_CAL_OUT1    PCONFIG_FLOAT(0)
# define P025_CAL_ADC2    PCONFIG_LONG(1)
# define P025_CAL_OUT2    PCONFIG_FLOAT(1)


struct P025_data_struct : public PluginTaskData_base {
public:

  P025_data_struct(struct EventStruct *event);
  P025_data_struct()          = delete;
  virtual ~P025_data_struct() = default;

  bool        read(float& value) const;

  static bool webformLoad(struct EventStruct *event);

  static bool webformSave(struct EventStruct *event);

  static bool webform_showConfig(struct EventStruct *event);

private:

  bool readConversionRegister025(int16_t& value) const;
  bool waitReady025(unsigned long timeout_ms) const;

  float _fullScaleFactor{};
  uint16_t _configRegisterValue{};

  uint8_t _i2cAddress;
};

#endif // ifdef USES_P025
#endif // ifndef PLUGINSTRUCTS_P025_DATA_STRUCT_H
