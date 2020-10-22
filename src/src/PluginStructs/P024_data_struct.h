#ifndef PLUGINSTRUCTS_P024_DATA_STRUCT_H
#define PLUGINSTRUCTS_P024_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P024

#include "../../ESPEasy_common.h"


struct P024_data_struct : public PluginTaskData_base {
public:

  P024_data_struct(uint8_t i2c_addr);

  float readTemperature(uint8_t reg);

private:

  uint16_t readRegister024(uint8_t reg);

  uint8_t i2cAddress;
};
#endif // ifdef USES_P024
#endif // ifndef PLUGINSTRUCTS_P024_DATA_STRUCT_H
