#ifndef PLUGINSTRUCTS_P121_DATA_STRUCT_H
#define PLUGINSTRUCTS_P121_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P121

# include <Adafruit_Sensor.h>
# include <Adafruit_HMC5883_U.h>

struct P121_data_struct : public PluginTaskData_base
{
  P121_data_struct() = default;
  virtual ~P121_data_struct() = default;
  
  bool begin(int taskid);

  Adafruit_HMC5883_Unified mag;

  bool initialized = false;
};

#endif // ifdef USES_P121
#endif // ifndef PLUGINSTRUCTS_P121_DATA_STRUCT_H
