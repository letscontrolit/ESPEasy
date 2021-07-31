#ifndef PLUGINSTRUCTS_P057_DATA_STRUCT_H
#define PLUGINSTRUCTS_P057_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P057

# include <HT16K33.h>


struct P057_data_struct : public PluginTaskData_base {
public:

  P057_data_struct(uint8_t i2c_addr);

  CHT16K33 ledMatrix;
  uint8_t  i2cAddress;
};

#endif // ifdef USES_P057
#endif // ifndef PLUGINSTRUCTS_P057_DATA_STRUCT_H
