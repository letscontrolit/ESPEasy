#ifndef PLUGINSTRUCTS_P069_DATA_STRUCT_H
#define PLUGINSTRUCTS_P069_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P069


struct P069_data_struct : public PluginTaskData_base {
public:

  P069_data_struct(bool A0_value = false,
                   bool A1_value = false,
                   bool A2_value = false);

  P069_data_struct(uint8_t addr);

  void  setAddress(uint8_t addr);

  float getTemperatureInDegrees() const;

private:

  uint8_t _i2c_device_address;
};

#endif // ifdef USES_P069
#endif // ifndef PLUGINSTRUCTS_P069_DATA_STRUCT_H
