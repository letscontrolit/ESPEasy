#ifndef PLUGINSTRUCTS_P060_DATA_STRUCT_H
#define PLUGINSTRUCTS_P060_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P060

struct P060_data_struct : public PluginTaskData_base {
public:

  P060_data_struct(uint8_t i2c_addr);

  void  overSampleRead();

  float getValue();

private:

  uint16_t readMCP3221();

  uint32_t OversamplingValue = 0;
  uint16_t OversamplingCount = 0;

  uint8_t i2cAddress;
};

#endif // ifdef USES_P060
#endif // ifndef PLUGINSTRUCTS_P060_DATA_STRUCT_H
