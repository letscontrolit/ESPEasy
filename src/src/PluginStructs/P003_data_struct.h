#ifndef PLUGINSTRUCTS_P003_DATA_STRUCT_H
#define PLUGINSTRUCTS_P003_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P003


# include "../Helpers/_Internal_GPIO_pulseHelper.h"

struct P003_data_struct : public PluginTaskData_base {
  P003_data_struct(const Internal_GPIO_pulseHelper::pulseCounterConfig& config);
  ~P003_data_struct();


  Internal_GPIO_pulseHelper pulseHelper;
};


#endif // ifdef USES_P003
#endif // ifndef PLUGINSTRUCTS_P003_DATA_STRUCT_H
