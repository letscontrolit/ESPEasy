#ifndef PLUGINSTRUCTS_P003_DATA_STRUCT_H
#define PLUGINSTRUCTS_P003_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P003

# ifndef P003_USE_EXTRA_COUNTERTYPES
#  ifdef LIMIT_BUILD_SIZE
#   define P003_USE_EXTRA_COUNTERTYPES  0 // Don't include for limited builds
#  else // ifdef LIMIT_BUILD_SIZE
#   define P003_USE_EXTRA_COUNTERTYPES  1
#  endif // ifdef LIMIT_BUILD_SIZE
# endif // ifndef P003_USE_EXTRA_COUNTERTYPES

# include "../Helpers/_Internal_GPIO_pulseHelper.h"

struct P003_data_struct : public PluginTaskData_base {
  P003_data_struct(const Internal_GPIO_pulseHelper::pulseCounterConfig& config);
  P003_data_struct() = delete;

  virtual ~P003_data_struct() = default;

  Internal_GPIO_pulseHelper pulseHelper;
};


#endif // ifdef USES_P003
#endif // ifndef PLUGINSTRUCTS_P003_DATA_STRUCT_H
