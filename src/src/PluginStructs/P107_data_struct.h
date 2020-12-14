#ifndef PLUGINSTRUCTS_P107_DATA_STRUCT_H
#define PLUGINSTRUCTS_P107_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P107

# include <Adafruit_SI1145.h>


struct P107_data_struct : public PluginTaskData_base {
  bool begin();

  Adafruit_SI1145 uv;
};

#endif // ifdef USES_P107
#endif // ifndef PLUGINSTRUCTS_P107_DATA_STRUCT_H
