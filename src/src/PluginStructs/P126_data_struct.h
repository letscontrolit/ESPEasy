#ifndef PLUGINSTRUCTS_P126_DATA_STRUCT_H
#define PLUGINSTRUCTS_P126_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P126

struct P126_data_struct : public PluginTaskData_base {
public:

  P126_data_struct(uint8_t i2c_addr,
                   uint8_t c1Cal,
                   uint8_t c2Cal);

  bool read();
  float_t getCurrent(uint8_t canal);

private:

  uint16_t readRegister126(uint8_t reg);

  uint8_t c1Calibre;
  uint8_t c2Calibre;
  uint8_t i2cAddress;
};

#endif // ifdef USES_P126
#endif // ifndef PLUGINSTRUCTS_P126_DATA_STRUCT_H
