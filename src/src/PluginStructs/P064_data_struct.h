#ifndef PLUGINSTRUCTS_P064_DATA_STRUCT_H
#define PLUGINSTRUCTS_P064_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#include "../../ESPEasy_common.h"

#ifdef USES_P064

#include <SparkFun_APDS9960.h> // Lib is modified to work with ESP


struct P064_data_struct : public PluginTaskData_base {
public:

  P064_data_struct();

  SparkFun_APDS9960 sensor;
};

#endif // ifdef USES_P064
#endif // ifndef PLUGINSTRUCTS_P064_DATA_STRUCT_H
